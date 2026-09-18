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
#include <widget/linearlayout.h>
#include <widget/framelayout.h>
#include <widget/scrollview.h>
#include <widget/listview.h>
#include <widget/adapterview.h>
#include <widget/adapter.h>
#include <drawable/colordrawable.h>
#include <text/spannablestringbuilder.h>
#include <text/spannablestring.h>       // Spanned flags
#include <text/style/characterstyles.h> // ForegroundColorSpan
#include <text/textutils.h>

using namespace cdroid;

namespace cdroid{ class Window; }

GUIEnvironment* GUIEnvironment::mInst=nullptr;
Window*         GUIEnvironment::mStage=nullptr;
LinearLayout*   GUIEnvironment::mPanel=nullptr;
ViewGroup*      GUIEnvironment::mContent=nullptr;

/* The results pane (left) + its data. The pane follows the currently running
   case: the upper suite list auto-selects (and scrolls to) the running case's
   suite; the lower detail shows only that suite's cases — colored per result
   (+ green / x red / ~ gray running). Global pass/fail totals are shown in
   the header. */
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
            tv=new TextView(&App::getInstance());
            tv->setLayoutParams(new AbsListView::LayoutParams(-1,52));
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
    LinearLayout*panel=GUIEnvironment::panel();
    panel->setBackgroundColor(0xEE0E1419);

    // header bar: title + global totals + close button
    LinearLayout*head=new LinearLayout(&App::getInstance());
    head->setOrientation(LinearLayout::HORIZONTAL);
    head->setBackgroundColor(0xFF1B262C);
    TextView*title=new TextView(&App::getInstance()); title->setText("Test Results");
    title->setTextSize(18);
    title->setTextColor(0xFFECEFF1);
    title->setPadding(24,0,0,0);
    title->setGravity(Gravity::START|Gravity::CENTER_VERTICAL);
    head->addView(title,new LinearLayout::LayoutParams(-2,56));
    gHeaderSum=new TextView(&App::getInstance());
    gHeaderSum->setTextSize(15);
    gHeaderSum->setTextColor(0xFFB0BEC5);
    gHeaderSum->setPadding(12,0,0,0);
    gHeaderSum->setGravity(Gravity::START|Gravity::CENTER_VERTICAL);
    head->addView(gHeaderSum,new LinearLayout::LayoutParams(-2,56));
    View*spacer=new View(&App::getInstance());
    head->addView(spacer,new LinearLayout::LayoutParams(0,0,1.0f));
    panel->addView(head,new LinearLayout::LayoutParams(-1,56));

    // upper: suite list (selectable) — auto-follows the running case
    gSuiteList=new ListView(&App::getInstance());
    gSuiteList->setBackgroundColor(0xFF141B22);
    gAdapter=new SuiteAdapter();
    gSuiteList->setAdapter(gAdapter);
    gSuiteList->setOnItemClickListener([](AdapterView&,View&,int pos,long){
        selectAt(pos);
    });
    gSuiteList->setSelector(new ColorDrawable(0x22FFFFFF));
    panel->addView(gSuiteList,new LinearLayout::LayoutParams(-1,0,1.0f));

    // lower: per-case detail for the current suite (scrollable, span-colored)
    gScroller=new ScrollView(&App::getInstance());
    gScroller->setBackgroundColor(0xFF0C1116);
    gDetail=new TextView(&App::getInstance());
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
    if(!GUIEnvironment::panel()) return;
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
        if(gSuiteList && idx>=0) gSuiteList->setSelection(idx);
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

        // reset the test screen for the next case (the results pane is a sibling → untouched)
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

/* Suites whose cases show UI on screen for human inspection (demo-style:
   build a screen, pumpFor, look at it). They are excluded from the default
   pure-logic regression run — run them alone with -visual, or mix everything
   back in with -all. When adding a new visual test, add its suite name here.
   Suite names must stay unique vs the pure suites (they are used verbatim in
   the gtest filter). */
const char*const VISUAL_SUITES[]={
    "LAYOUT","EDITTEXT","EDGEEFFECT","WIDGET","APP","DIALOG","CDCONTEXT",
    "FOCUS","ANIMATOR","ANIMATORINFLATOR","DRAWABLE_CDT","SCENE",
    "BaseKeyListenerTest","MultiTapKeyListenerTest",
};

std::string visualFilter(){
    std::string f;
    for(const char*s:VISUAL_SUITES){
        if(!f.empty()) f+=':';
        f+=std::string(s)+".*";
    }
    return f;
}

/* Run-mode selection, applied via the gtest filter:
     (default)  pure-logic regression — visual suites excluded
     -visual    only the on-screen suites
     -all       everything (pre-split behavior)
   An explicit --gtest_filter on the command line always wins. */
int main(int argc,char*argv[])
{
    bool visual=false, all=false;
    /* Strip our own mode flags first so neither LogParseModules, gtest, nor
       App ever sees them (unknown flags are not tolerated everywhere). */
    int kept=0;
    for(int i=0;i<argc;i++){
        if(!strcmp(argv[i],"-visual")){ visual=true; continue; }
        if(!strcmp(argv[i],"-all"))   { all=true;    continue; }
        argv[kept++]=argv[i];
    }
    argc=kept;
    /* Must scan before InitGoogleTest — gtest removes its own args from argv. */
    bool explicitFilter=false;
    for(int i=0;i<argc;i++)
        if(!strncmp(argv[i],"--gtest_filter",14)){ explicitFilter=true; break; }

    LogParseModules(argc,(const char**)argv);
    testing::InitGoogleTest(&argc,argv);
    if(!explicitFilter){
        if(visual)      testing::GTEST_FLAG(filter)=visualFilter();
        else if(!all)   testing::GTEST_FLAG(filter)="-"+visualFilter();
    }
    ::testing::AddGlobalTestEnvironment(new GUIEnvironment(argc,(const char**)argv));
    ::testing::UnitTest::GetInstance()->listeners().Append(new GuiTestListener);
    return RUN_ALL_TESTS();
}
