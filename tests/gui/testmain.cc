#include <string>
#include <vector>
#include <unordered_map>
#include <cstdio>
#include <gtest/gtest.h>
#include <guienvironment.h>
#include <porting/cdlog.h>
#include <core/windowmanager.h>
#include <core/callbackbase.h>     // Runnable
#include <widget/cdwindow.h>
#include <widget/textview.h>
#include <widget/button.h>
#include <widget/linearlayout.h>
#include <widget/framelayout.h>
#include <widget/scrollview.h>
#include <widget/listview.h>
#include <widget/adapterview.h>
#include <widget/adapter.h>
#include <widget/drawerlayout.h>
#include <drawable/colordrawable.h>
#include <text/spannablestringbuilder.h>
#include <text/spannablestring.h>       // Spanned flags
#include <text/style/characterstyles.h> // ForegroundColorSpan
#include <text/textutils.h>

using namespace cdroid;

namespace cdroid{ class Window; }

GUIEnvironment* GUIEnvironment::mInst=nullptr;
Window*         GUIEnvironment::mStage=nullptr;
DrawerLayout*   GUIEnvironment::mDrawerLayout=nullptr;
ViewGroup*      GUIEnvironment::mContent=nullptr;
LinearLayout*   GUIEnvironment::mDrawerPanel=nullptr;

/* The result drawer interior + its data. The drawer follows the currently
   running case: the upper suite list auto-selects (and scrolls to) the running
   case's suite; the lower detail shows only that suite's cases — colored per
   result (+ green / x red / ~ gray running). Global pass/fail totals are shown
   in the header. */
namespace{
constexpr int COL_PASS = 0xFF66BB6A; // green
constexpr int COL_FAIL = 0xFFEF5350; // red
constexpr int COL_RUN  = 0xFFB0BEC5; // gray (case still running)

enum Status{ ST_RUN=0, ST_PASS=1, ST_FAIL=2 };
struct CaseRec { std::string suite, name; int status; };
struct SuiteRec{ int total=0, passed=0, failed=0; std::vector<CaseRec> cases; };

struct State{
    std::unordered_map<std::string,SuiteRec> suites;
    std::vector<std::string> order;          // suite names, first-seen order
    std::string selected;                    // suite currently shown (auto-follows the running case)
    int gp=0, gf=0;
} g;

class SuiteAdapter; // forward (defined below; used by gAdapter)

ListView*    gSuiteList =nullptr;
ScrollView*  gScroller  =nullptr;
TextView*    gDetail    =nullptr;
TextView*    gHeaderSum =nullptr;
SuiteAdapter*gAdapter   =nullptr;
bool         gBuilt     =false;
bool         gReady     =false;

const std::vector<CaseRec>& selectedCases(){
    static const std::vector<CaseRec> empty;
    auto it=g.suites.find(g.selected);
    return it!=g.suites.end()? it->second.cases : empty;
}

/* Rebuild the lower detail from the currently-selected suite. Each line is a
   colored span: + green (pass), x red (fail), ~ gray (running). One new span per
   line — the SpannableStringBuilder owns each passed-in span (addSpan takes the
   raw pointer for non-NoCopySpan spans), so never reuse a span pointer. */
void rebuildDetail(){
    if(!gDetail) return;
    SpannableStringBuilder* b=new SpannableStringBuilder();
    const auto& src=selectedCases();
    const size_t cap=200;
    for(size_t i = src.size()>cap? src.size()-cap : 0; i<src.size(); i++){
        const CaseRec&c=src[i];
        int col = c.status==ST_PASS?COL_PASS : c.status==ST_FAIL?COL_FAIL : COL_RUN;
        std::string mark = c.status==ST_PASS?"+ " : c.status==ST_FAIL?"x ":"~ ";
        std::string line=mark+c.name+"\n";
        b->append(TextUtils::utf8_utf16(line), new ForegroundColorSpan(col),
                  Spanned::SPAN_EXCLUSIVE_EXCLUSIVE);
    }
    gDetail->setText(b); // TextView owns b; frees prior b + its owned spans
    if(gScroller){
        Runnable r; r=[](){ if(gScroller) gScroller->fullScroll(View::FOCUS_DOWN); };
        gScroller->post(r);
    }
}

/* Upper list: one row per TestSuite. The selected row (the running case's suite)
   is highlighted. */
class SuiteAdapter:public BaseAdapter{
public:
    int getCount()const override{ return (int)g.order.size(); }
    void*getItem(int position)const override{ return nullptr; }
    long getItemId(int position)const override{ return position; }
    View*getView(int position,View*convertView,ViewGroup*/*parent*/)override{
        TextView*tv=dynamic_cast<TextView*>(convertView);
        if(!tv){
            tv=new TextView("",-1,52);
            tv->setTextSize(15);
            tv->setPadding(28,10,16,10);
            tv->setFocusable(false);
            tv->setGravity(Gravity::START|Gravity::CENTER_VERTICAL);
        }
        std::string label; int done=0,total=0; bool sel=false;
        if((size_t)position<g.order.size()){
            const std::string&sn=g.order[position];
            const SuiteRec&sr=g.suites[sn];
            label=sn; done=sr.passed+sr.failed; total=sr.total;
            sel=(g.selected==sn);
        }
        char tail[32]; snprintf(tail,sizeof tail,"  %d/%d",done,total);
        tv->setText((sel?"> ":"  ")+label+tail);
        tv->setTextColor(sel?0xFFFFFFFF:0xFFB0BEC5);
        tv->setBackgroundColor(sel?0xFF2E3C44:0x00000000);
        return tv;
    }
};

void refreshSummary(){
    if(gAdapter) gAdapter->notifyDataSetChanged();
    if(gHeaderSum){
        char buf[96];
        snprintf(buf,sizeof buf,"P:%d  F:%d  T:%d",g.gp,g.gf,g.gp+g.gf);
        gHeaderSum->setText(buf);
    }
}

/* Manual click still works for inspection, but OnTestStart auto-follows the
   running case and overrides it. */
void selectAt(int position){
    if((size_t)position<g.order.size()){
        g.selected=g.order[position];
        if(gSuiteList) gSuiteList->setSelection(position);
        refreshSummary();
        rebuildDetail();
    }
}

int indexOfSuite(const std::string&sn){
    for(size_t i=0;i<g.order.size();i++) if(g.order[i]==sn) return (int)i;
    return -1;
}

void buildDrawer(){
    if(gBuilt) return; gBuilt=true;
    LinearLayout*panel=GUIEnvironment::drawerPanel();
    panel->setBackgroundColor(0xEE0E1419);

    // header bar: title + global totals + close button
    LinearLayout*head=new LinearLayout(-1,56);
    head->setOrientation(LinearLayout::HORIZONTAL);
    head->setBackgroundColor(0xFF1B262C);
    TextView*title=new TextView("Test Results",-2,56);
    title->setTextSize(18);
    title->setTextColor(0xFFECEFF1);
    title->setPadding(24,0,0,0);
    title->setGravity(Gravity::START|Gravity::CENTER_VERTICAL);
    head->addView(title);
    gHeaderSum=new TextView("",-2,56);
    gHeaderSum->setTextSize(15);
    gHeaderSum->setTextColor(0xFFB0BEC5);
    gHeaderSum->setPadding(12,0,0,0);
    gHeaderSum->setGravity(Gravity::START|Gravity::CENTER_VERTICAL);
    head->addView(gHeaderSum);
    View*spacer=new View(0,0);
    head->addView(spacer,new LinearLayout::LayoutParams(0,0,1.0f));
    Button*close=new Button("X",72,56);
    close->setTextSize(16);
    close->setOnClickListener([](View&){
        DrawerLayout*dl=GUIEnvironment::drawerLayout();
        if(dl) dl->closeDrawer(Gravity::START);
    });
    head->addView(close);
    panel->addView(head,new LinearLayout::LayoutParams(-1,56));

    // upper: suite list (selectable) — auto-follows the running case
    gSuiteList=new ListView(-1,-1);
    gSuiteList->setBackgroundColor(0xFF141B22);
    gAdapter=new SuiteAdapter();
    gSuiteList->setAdapter(gAdapter);
    gSuiteList->setOnItemClickListener([](AdapterView&,View&,int pos,long){
        selectAt(pos);
    });
    gSuiteList->setSelector(new ColorDrawable(0x22FFFFFF));
    panel->addView(gSuiteList,new LinearLayout::LayoutParams(-1,0,1.0f));

    // lower: per-case detail for the current suite (scrollable, span-colored)
    gScroller=new ScrollView(-1,-1);
    gScroller->setBackgroundColor(0xFF0C1116);
    gDetail=new TextView("",-1,-2);
    gDetail->setTextSize(13);
    gDetail->setTextColor(0xFFECEFF1);
    gDetail->setPadding(24,14,16,14);
    gScroller->addView(gDetail);
    panel->addView(gScroller,new LinearLayout::LayoutParams(-1,0,2.0f));
}

/* One-shot setup that must run after GUIEnvironment::SetUp built the tree. gtest
   fires OnTestProgramStart before Environment::SetUp, so guard on the panel being
   ready; the first OnTestStart (which always runs after SetUp) finishes the job. */
void ensureReady(const testing::UnitTest*unit){
    if(gReady) return;
    if(!GUIEnvironment::drawerPanel()) return;
    buildDrawer();
    /* CDROID's AttachInfo defaults mInTouchMode=true (Android defaults false).
       With no input in this harness the ListView stays in touch mode, which
       makes setSelectionFromTop() skip setting the selected position and so it
       never scrolls. Force the whole tree out of touch mode so the suite list
       auto-scrolls to the running case. */
    if(Window*s=GUIEnvironment::stage()) s->ensureTouchMode(false);
    if(unit){
        for(int i=0;i<unit->total_test_suite_count();i++){
            const testing::TestSuite*ts=unit->GetTestSuite(i);
            if(!ts) continue;
            const std::string name=ts->name();
            if(g.suites.find(name)==g.suites.end()) g.order.push_back(name);
            g.suites[name].total=ts->test_to_run_count();
        }
    }
    refreshSummary();
    rebuildDetail();
    DrawerLayout*dl=GUIEnvironment::drawerLayout();
    if(dl) dl->openDrawer(Gravity::START,false); // open, no slide-in animation
    pumpFor(40);
    gReady=true;
}
}//namespace

class GuiTestListener:public testing::EmptyTestEventListener{
public:
    void OnTestProgramStart(const testing::UnitTest&unit)override{
        ensureReady(&unit);
    }
    void OnTestStart(const testing::TestInfo&)override{
        ensureReady(testing::UnitTest::GetInstance());
        const testing::TestInfo&info = *testing::UnitTest::GetInstance()->current_test_info();
        const std::string suite=info.test_suite_name();
        const std::string name =info.name();
        if(g.suites.find(suite)==g.suites.end()) g.order.push_back(suite);
        CaseRec rec{suite,name,ST_RUN};
        g.suites[suite].cases.push_back(rec);
        // auto-follow: select the running case's suite and pin it to the top of
        // the list so the current item is always visible.
        g.selected=suite;
        int idx=indexOfSuite(suite);
        if(gSuiteList && idx>=0) gSuiteList->setSelectionFromTop(idx, 0);
        refreshSummary();
        rebuildDetail();
    }
    void OnTestEnd(const testing::TestInfo&info)override{
        const std::string suite=info.test_suite_name();
        const bool failed=info.result()->Failed();
        const int  st=failed?ST_FAIL:ST_PASS;
        auto sit=g.suites.find(suite);
        if(sit!=g.suites.end()){
            for(auto it=sit->second.cases.rbegin(); it!=sit->second.cases.rend(); ++it){
                if(it->name==info.name() && it->status==ST_RUN){ it->status=st; break; }
            }
            if(failed) sit->second.failed++; else sit->second.passed++;
        }
        if(failed) g.gf++; else g.gp++;

        refreshSummary();
        if(g.selected==suite) rebuildDetail();

        // reset the test screen for the next case (the drawer is a sibling → untouched)
        ViewGroup*content=GUIEnvironment::content();
        if(content) content->removeAllViews();
        // drop stray windows (dialogs / edge windows) the case may have created
        std::vector<Window*>wins;
        WindowManager&wm=WindowManager::getInstance();
        Window*stage=GUIEnvironment::stage();
        if(wm.getWindows(wins)>0){
            for(auto* w: wins) if(w!=stage) wm.removeWindow(w);
        }
        pumpFor(20);
    }
};

int main(int argc,char*argv[])
{
    LogParseModules(argc,(const char**)argv);
    testing::InitGoogleTest(&argc,argv);
    ::testing::AddGlobalTestEnvironment(new GUIEnvironment(argc,(const char**)argv));
    ::testing::UnitTest::GetInstance()->listeners().Append(new GuiTestListener);
    return RUN_ALL_TESTS();
}
