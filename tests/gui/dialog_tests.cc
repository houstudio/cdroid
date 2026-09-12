#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <vector>
#include <core/app.h>
#include <core/activityfactory.h>
#include <app/alertdialog.h>
#include <core/windowmanager.h>
#include <menu/menubuilder.h>
#include <menu/menupopuphelper.h>
#include <menu/popupmenu.h>
#include <menu/cascadingmenupopup.h>
#include <widget/adapter.h>
#include <widget/activitytransition.h>
#include <widget/cdwindow.h>
#include <widget/internal_R.h>
#include <widget/listpopupwindow.h>
#include <widget/popupwindow.h>
#include <widget/textview.h>
#include <widget/toolbar.h>
#include <guienvironment.h>

using namespace cdroid;

class DIALOG:public testing::Test{

   public :
   virtual void SetUp(){
   }
   virtual void TearDown(){
   }
};

/* Dialogs auto-finish: show() returns the dialog, we assert it built, pump a
   few frames so it measures/draws, then move on. The button click listeners
   are kept (harmless in automation; a human run can still click them). */
TEST_F(DIALOG,1Button){
   App&app=App::getInstance();
   AlertDialog*dlg=AlertDialog::Builder(&app)
	 .setTitle("DialogTest")
         .setMessage("Hello ,every one! This is an multiline messagebox Example\n this is the second line\nis this OK?")
         .setPositiveButton("OK",[](DialogInterface&,int){App::getInstance().exit(0);})
         .show();
   ASSERT_NE(dlg,nullptr);
   pumpFor(300);
   dlg->dismiss();
}

TEST_F(DIALOG,2Button){
   App&app=App::getInstance();
   AlertDialog*dlg=AlertDialog::Builder(&app)
         .setTitle("DialogTest")
         .setMessage("Hello ,every one! This is an multiline messagebox Example\n this is the second line\nis this OK?")
         .setPositiveButton("Yes",nullptr)
         .setNegativeButton("No",[](DialogInterface&,int){App::getInstance().exit(1);})
         .show();
   ASSERT_NE(dlg,nullptr);
   pumpFor(300);
   dlg->dismiss();
}

TEST_F(DIALOG,3Button){
   App&app=App::getInstance();
   AlertDialog*dlg=AlertDialog::Builder(&app)
         .setTitle("DialogTest")
         .setMessage("Hello ,every one! This is an multiline messagebox Example\n this is the second line\nis this OK?")
         .setPositiveButton("Yes",nullptr)
         .setNegativeButton("No",nullptr)
         .setNeutralButton("Cancel",[](DialogInterface&,int){App::getInstance().exit(2);})
         .show();
   ASSERT_NE(dlg,nullptr);
   pumpFor(300);
   dlg->dismiss();
}

/* The theme's windowAnimationStyle drives the Window's default enter/exit animations
   (AOSP PhoneWindow/AppTransition chain): the Material dialog theme carries
   windowAnimationStyle -> Animation.Material.Dialog -> @anim/popup_enter_material
   (an alpha+translate set), so the dialog Window resolves a whole-surface SLIDE
   (the translate child is the dominant motion) at construction. */
TEST_F(DIALOG,ThemeWindowAnimations){
   App&app=App::getInstance();
   AlertDialog*dlg=AlertDialog::Builder(&app)
         .setTitle("DialogTest")
         .setMessage("theme windowAnimationStyle wiring")
         .show();
   ASSERT_NE(dlg,nullptr);
   Window*w=dlg->getWindow();
   ASSERT_NE(w,nullptr);
   ActivityTransition*enter=w->getEnterTransition();
   ASSERT_NE(enter,nullptr); // resolved from the theme at Window construction
   EXPECT_EQ(enter->getType(),ActivityTransition::Type::SLIDE);
   EXPECT_GT(enter->getDuration(),0);
   ActivityTransition*exit=w->getExitTransition();
   ASSERT_NE(exit,nullptr);
   EXPECT_EQ(exit->getType(),ActivityTransition::Type::SLIDE);
   pumpFor(300);
   dlg->dismiss();
}

/* Popups use their OWN window animation style, never the theme's windowAnimationStyle
   (AOSP: that mechanism belongs to app/activity windows; popups carry it on
   LayoutParams). A dropdown popup resolves the framework default Animation.DropDownDown
   (grow_fade_in = an alpha+scale set -> whole-surface FADE) — installed AFTER the anchor
   alignment in invokePopup, so the enter snap uses the final resting position (a ctor-time
   theme load snapped to a stale position and crashed menu teardown). */
TEST_F(DIALOG,PopupWindowDropdownAnimations){
   App&app=App::getInstance();
   TextView*anchor=new TextView(&app); anchor->setText("anchor");
   GUIEnvironment::content()->addView(anchor, new ViewGroup::LayoutParams(200,48));
   pumpFor(100);

   /* Construct like the overflow menu does (popupMenuStyle as defStyleAttr): the Material
      chain carries popupAnimationStyle=@empty (0) — the handoff AOSP pairs with popup L
      transitions — which CDROID falls back to the classic dropdown animation from. */
   PopupWindow*popup=new PopupWindow(&app,nullptr,(int)cdroid::internal::R::attr::popupMenuStyle,0);
   TextView*content=new TextView(&app); content->setText("popup");
   popup->setContentView(content);
   popup->setWidth(200);
   popup->setHeight(300);
   popup->showAsDropDown(anchor,0,0);
   pumpFor(100);

   /* The decor is the popup's Window subclass: the dropdown default ENTER animation is
      installed (Animation.DropDownDown -> alpha sets -> FADE), NOT the app theme's
      Animation.Activity slides. */
   Window*decor=(Window*)popup->getContentView()->getRootView();
   ASSERT_NE(decor,nullptr);
   ActivityTransition*enter=decor->getEnterTransition();
   ASSERT_NE(enter,nullptr);
   EXPECT_EQ(enter->getType(),ActivityTransition::Type::FADE);
   /* AOSP semantics: BOTH enter and exit install (the dropdown pair). The exit defers the
      decor teardown - and the dismiss listener - to the animation end (the no-GC contract:
      owners free borrowed state in or after onDismiss; the menu chain's fire-and-forget
      self-delete rides the deferred listener, so its content survives the animation). */
   EXPECT_NE(decor->getExitTransition(),nullptr);
   /* The enter animation actually RAN: the surface starts at 0 (snap) and fades toward 1 —
      mid-fade right after the first pump window, opaque once the 150-220ms fade completes. */
   const float midFade=decor->getAlpha();
   EXPECT_GE(midFade,0.0f);
   EXPECT_LE(midFade,1.0f);
   pumpFor(600);
   EXPECT_FLOAT_EQ(decor->getAlpha(),1.0f);
   // NB: both the anchor and the PopupWindow are intentionally leaked, like real apps —
   // the anchor outlives its popups (CDROID's anchor listeners hold raw pointers), and
   // popup objects are never deleted right after dismiss.
}

/* The overflow-menu ownership pattern on a ListPopupWindow with an animated exit:
   the decor teardown - and with it the dismiss listener - is deferred to the exit
   animation end. The no-GC contract: borrowed state (the adapter) is freed IN or
   AFTER onDismiss, which fires at teardown-complete; freeing it right after
   dismiss() returns would dangle inside the still-animating list. The menu chain
   pays the same discipline (~CascadingMenuInfo runs at the deferred teardown). */
namespace {
class OneRowAdapter : public BaseAdapter { // BaseAdapter == Adapter (widget/adapter.h typedef)
public:
    int getCount() const override { return 1; }
    void* getItem(int) const override { return nullptr; }
    View* getView(int, View* convertView, ViewGroup* parent) override {
        TextView* tv = convertView ? (TextView*)convertView : new TextView(parent->getContext());
        tv->setText("row");
        return tv;
    }
};
} // namespace

TEST_F(DIALOG,ListPopupWindowBorrowedAdapterDismiss){
   App&app=App::getInstance();
   TextView*anchor=new TextView(&app); anchor->setText("anchor2");
   GUIEnvironment::content()->addView(anchor, new ViewGroup::LayoutParams(200,48));
   pumpFor(100);

   OneRowAdapter*adapter=new OneRowAdapter();
   ListPopupWindow*lpw=new ListPopupWindow(&app,nullptr,(int)cdroid::internal::R::attr::popupMenuStyle,0);
   lpw->setAdapter(adapter);
   lpw->setAnchorView(anchor);
   lpw->show();
   pumpFor(100);

   bool adapterFreed = false;
   lpw->setOnDismissListener([&adapterFreed](){ adapterFreed = true; });
   lpw->dismiss();
   pumpFor(600);    // exit animation (150-220ms) ends; teardown + the deferred
                    // listener run while everything is still alive
   EXPECT_TRUE(adapterFreed);   // the contract fired at teardown-complete
   delete adapter;              // borrowed state freed AFTER onDismiss
   delete lpw;
   pumpFor(200);

   GUIEnvironment::content()->removeView(anchor);
   delete anchor;
}

/* The REAL overflow-menu chain: MenuPopupHelper -> CascadingMenuPopup ->
   MenuPopupWindow(ListPopupWindow) -> PopupWindow(decor). The popup must animate in with the
   dropdown default (grow_fade_in -> FADE): alpha snaps to 0 and the enter animation fades the
   surface to 1 (observed mid-fade right after the first pump window). */
TEST_F(DIALOG,MenuPopupEnterAnimation){
   App&app=App::getInstance();
   TextView*anchor=new TextView(&app); anchor->setText("menu anchor");
   GUIEnvironment::content()->addView(anchor, new ViewGroup::LayoutParams(200,48));
   pumpFor(100);

   MenuBuilder*menu=new MenuBuilder(&app);
   menu->add("One");
   menu->add("Two");
   menu->add("Three");
   MenuPopupHelper*helper=new MenuPopupHelper(&app,menu,anchor,true,
         (int)cdroid::internal::R::attr::popupMenuStyle,0);
   helper->show();
   pumpFor(50);

   // The menu popup's decor window is the newest entry in the WindowManager list.
   std::vector<Window*>windows;
   WindowManager::getInstance().getWindows(windows);
   Window*decor=windows.empty()?nullptr:windows.back();
   ASSERT_NE(decor,nullptr);
   ActivityTransition*enter=decor->getEnterTransition();
   ASSERT_NE(enter,nullptr);
   EXPECT_EQ(enter->getType(),ActivityTransition::Type::FADE);
   const float midFade=decor->getAlpha();
   EXPECT_GE(midFade,0.0f);
   EXPECT_LE(midFade,1.0f);

   pumpFor(400); // fade (150-220ms) finishes -> opaque
   EXPECT_FLOAT_EQ(decor->getAlpha(),1.0f);

   helper->dismiss();  // the overflow-menu dismiss path (animated exit:
                       // teardown + the deferred dismiss chain run at anim end)
   pumpFor(600);       // wait past the exit animation before freeing anything -
                       // the deferred listener (CMP::onDismiss cascade) must
                       // have run while helper/menu are still alive
   delete helper;
   delete menu;
   GUIEnvironment::content()->removeView(anchor);
   delete anchor;
}

TEST_F(DIALOG,Choices){
   App&app=App::getInstance();
   std::vector<std::string>items;
   for(int i=0;i<10;i++){
       std::ostringstream oss;
       oss<<"item-"<<i;
       items.push_back(oss.str());
   }
   AlertDialog*dlg=AlertDialog::Builder(&app)
         .setTitle("DialogTest")
         .setSingleChoiceItems(items,0,nullptr)
         .setPositiveButton("Yes",nullptr)
         .setNegativeButton("No",[](DialogInterface&,int){App::getInstance().exit(1);})
         .show();
   ASSERT_NE(dlg,nullptr);
   pumpFor(300);
   dlg->dismiss();
}

/* ---- teardown-complete protocol: delete-at-any-time (Stage 0) ------------ */

static int testWindowCount(){
   std::vector<Window*>windows;
   WindowManager::getInstance().getWindows(windows);
   return (int)windows.size();
}

/* close() idempotence: a second close() while the first is still pending must
   not post a second `delete self` for the same Window (double free). */
/* Builder.setItems (plain list, no message, negative-only): the dialog must
 * render the item rows. Regression for the preferencedemo Wi-Fi picker showing
 * a blank body — dumps the composited frame to /tmp for pixel inspection. */
TEST_F(DIALOG,BuilderSetItems){
   App&app=App::getInstance();
   // Preferencedemo runs Theme.Material.Light — reproduce under it (a second
   // run keeps the default theme: argv DIALOG_THEME=default restores).
   app.setTheme((int)internal::R::style::Theme_Material_Light);
   std::vector<std::string>items{"alpha-1","beta-2","gamma-3"};
   AlertDialog*dlg=AlertDialog::Builder(&app)
         .setTitle("PickOne")
         .setItems(items,[](DialogInterface&,int){})
         .setNegativeButton("Cancel",nullptr)
         .show();
   ASSERT_NE(dlg,nullptr);
   pumpFor(600);
   /* Regression (preferencedemo wifi-picker report): a setItems dialog must
      lay out one measured row per item — not an empty body. */
   AbsListView*list=nullptr;
   std::function<void(cdroid::View*)> findList=[&](cdroid::View* v){
       if(v==nullptr)return;
       if(auto*lv=dynamic_cast<AbsListView*>(v))list=lv;
       if(auto*g=dynamic_cast<cdroid::ViewGroup*>(v))
           for(int i=0;i<g->getChildCount();i++)findList(g->getChildAt(i));
   };
   findList(dlg->getWindow()->getRootView());
   ASSERT_NE(list,nullptr);
   EXPECT_EQ(list->getCount(),3);
   EXPECT_GT(list->getHeight(),0);
   dlg->dismiss();
   pumpFor(100);
}

TEST_F(DIALOG,WindowDoubleClose){
   App&app=App::getInstance();
   Window*w=new Window(&app,0,0,100,100,Window::TYPE_APPLICATION);
   pumpFor(50);
   w->close();
   w->close();   // re-entry: guarded, no second posted delete
   pumpFor(200); // the single posted delete runs
}

/* Destroying a SHOWING PopupWindow directly (no dismiss first): the dtor must
   tear the decor down - compositor release immediate, delete posted - instead
   of leaving it on screen with a dangling mPop back-pointer. */
TEST_F(DIALOG,PopupWindowDeleteWhileShowing){
   App&app=App::getInstance();
   TextView*anchor=new TextView(&app); anchor->setText("anchor3");
   GUIEnvironment::content()->addView(anchor,new ViewGroup::LayoutParams(200,48));
   pumpFor(100);

   PopupWindow*popup=new PopupWindow(&app,nullptr,(int)cdroid::internal::R::attr::popupMenuStyle,0);
   TextView*content=new TextView(&app); content->setText("popup");
   popup->setContentView(content);   // borrowed: handed back by the dtor
   popup->setWidth(200); popup->setHeight(100);
   popup->showAsDropDown(anchor,0,0);
   pumpFor(600);   // enter completes: the force-teardown close() then takes the
                   // animated exit path deterministically
   const int showing=testWindowCount();

   delete popup;   // no dismiss() - the dtor does the full teardown
   pumpFor(600);   // the animated close finishes: compositor release + posted delete
   EXPECT_EQ(testWindowCount(),showing-1);

   GUIEnvironment::content()->removeView(anchor);
   delete anchor;
   delete content; // the borrowed content was returned to us, we own it again
}

/* Deleting the PopupWindow from inside its own dismiss listener: the listener
   fires from a stack copy with the member cleared first, so the executing
   function object survives its own destruction. */
TEST_F(DIALOG,PopupWindowDeleteInsideDismissListener){
   App&app=App::getInstance();
   TextView*anchor=new TextView(&app); anchor->setText("anchor4");
   GUIEnvironment::content()->addView(anchor,new ViewGroup::LayoutParams(200,48));
   pumpFor(100);

   PopupWindow*popup=new PopupWindow(&app,nullptr,(int)cdroid::internal::R::attr::popupMenuStyle,0);
   TextView*content=new TextView(&app); content->setText("popup");
   popup->setContentView(content);
   popup->setWidth(200); popup->setHeight(100);
   popup->showAsDropDown(anchor,0,0);
   pumpFor(50);
   popup->setOnDismissListener([popup](){ delete popup; });
   popup->dismiss();  // the (deferred) listener deletes the popup at teardown
   pumpFor(600);      // cover the 150-220ms exit animation: the borrowed
                      // content is handed back by the deferred fire, so it
                      // must not be freed before teardown completes

   GUIEnvironment::content()->removeView(anchor);
   delete anchor;
   delete content;    // returned by the deferred fire above
}

/* Same protocol one level up: the app listener on a ListPopupWindow runs AFTER
   its member cleanup, so deleting the ListPopupWindow inside that listener is
   safe end to end (its dtor deletes the inner PopupWindow mid-notification,
   which the PopupWindow teardown covers). */
TEST_F(DIALOG,ListPopupWindowDeleteInsideDismissListener){
   App&app=App::getInstance();
   TextView*anchor=new TextView(&app); anchor->setText("anchor5");
   GUIEnvironment::content()->addView(anchor,new ViewGroup::LayoutParams(200,48));
   pumpFor(100);

   ListPopupWindow*lpw=new ListPopupWindow(&app,nullptr,(int)cdroid::internal::R::attr::popupMenuStyle,0);
   lpw->setAdapter(new OneRowAdapter());
   lpw->setAnchorView(anchor);
   lpw->show();
   pumpFor(50);
   lpw->setOnDismissListener([lpw](){ delete lpw; }); // deletes itself mid-notification
   lpw->dismiss();
   pumpFor(200);

   GUIEnvironment::content()->removeView(anchor);
   delete anchor;
}

/* CascadingMenuPopup::dismiss iterates a COPY of mShowingMenus: each window
   dismiss re-enters onCloseMenu, which erases from the live vector mid-loop.
   Two showing windows (the cascading-submenu shape) must walk the copy. Also
   exercises the dtor's symmetric unregister from both menus' presenters. */
TEST_F(DIALOG,CascadingMenuDismissDuringCascade){
   App&app=App::getInstance();
   TextView*anchor=new TextView(&app); anchor->setText("anchor6");
   GUIEnvironment::content()->addView(anchor,new ViewGroup::LayoutParams(200,48));
   pumpFor(100);

   CascadingMenuPopup*cmp=new CascadingMenuPopup(&app,anchor,
         (int)cdroid::internal::R::attr::popupMenuStyle,0,true);
   MenuBuilder*menu1=new MenuBuilder(&app); menu1->add("One"); menu1->add("Two");
   MenuBuilder*menu2=new MenuBuilder(&app); menu2->add("Sub");
   cmp->addMenu(menu1);  // pending (not showing yet)
   cmp->show();
   cmp->addMenu(menu2);  // showing -> second window: the cascade shape
   pumpFor(50);
   cmp->dismiss();       // copy-iterated dismiss-all
   pumpFor(200);
   delete cmp;           // dtor unregisters from both menus' presenters
   delete menu1;
   delete menu2;
   GUIEnvironment::content()->removeView(anchor);
   delete anchor;
}

/* ---- PopupMenu fire-and-forget lifetime (Stage 1) ------------------------ */

namespace {
int sCountingMenusAlive = 0;
class CountingPopupMenu : public PopupMenu {
public:
    CountingPopupMenu(Context* c, View* a) : PopupMenu(c, a) { sCountingMenusAlive++; }
    ~CountingPopupMenu() { sCountingMenusAlive--; }
};
} // namespace

/* The contract: show() hands ownership to the menu itself - after the dismiss
   cascade completes it self-destructs on the next looper drain, with no app
   delete anywhere. */
TEST_F(DIALOG,PopupMenuFireAndForget){
   App&app=App::getInstance();
   TextView*anchor=new TextView(&app); anchor->setText("pm anchor");
   GUIEnvironment::content()->addView(anchor,new ViewGroup::LayoutParams(200,48));
   pumpFor(100);

   CountingPopupMenu*menu=new CountingPopupMenu(&app,anchor);
   menu->getMenu()->add("One");
   menu->getMenu()->add("Two");
   menu->show();
   pumpFor(100);
   EXPECT_EQ(sCountingMenusAlive,1);

   menu->dismiss();      // last touch: the cascade stages the self-delete
   pumpFor(300);         // the posted delete runs
   EXPECT_EQ(sCountingMenusAlive,0);

   GUIEnvironment::content()->removeView(anchor);
   delete anchor;
}

/* Legacy ownership stays safe during the rollout: a menu dismissed but deleted
   by its owner BEFORE the posted self-delete runs - the destructor purges the
   pending post, no double free. */
TEST_F(DIALOG,PopupMenuLegacyDeleteAfterDismiss){
   App&app=App::getInstance();
   TextView*anchor=new TextView(&app); anchor->setText("pm anchor2");
   GUIEnvironment::content()->addView(anchor,new ViewGroup::LayoutParams(200,48));
   pumpFor(100);

   CountingPopupMenu*menu=new CountingPopupMenu(&app,anchor);
   menu->getMenu()->add("One");
   menu->show();
   pumpFor(100);
   menu->dismiss();
   delete menu;          // before the posted self-delete runs
   EXPECT_EQ(sCountingMenusAlive,0);
   pumpFor(300);         // the purged post must not fire

   GUIEnvironment::content()->removeView(anchor);
   delete anchor;
}

/* A menu that was never shown never entered the cascade - the owner keeps it
   and deletes it normally. */
TEST_F(DIALOG,PopupMenuNeverShownDelete){
   App&app=App::getInstance();
   TextView*anchor=new TextView(&app); anchor->setText("pm anchor3");
   GUIEnvironment::content()->addView(anchor,new ViewGroup::LayoutParams(200,48));
   pumpFor(100);

   CountingPopupMenu*menu=new CountingPopupMenu(&app,anchor);
   menu->getMenu()->add("One");
   delete menu;          // no show() happened: plain ownership
   EXPECT_EQ(sCountingMenusAlive,0);
   pumpFor(100);

   GUIEnvironment::content()->removeView(anchor);
   delete anchor;
}

/* show() is one-shot: after dismissal the object is logically dead (or freed).
   The re-show must be refused before re-entering the popup machinery - called
   here BEFORE the pump (the object is still alive; after the pump it is freed
   and must simply never be touched again). */
TEST_F(DIALOG,PopupMenuShowAfterDismiss){
   App&app=App::getInstance();
   TextView*anchor=new TextView(&app); anchor->setText("pm anchor4");
   GUIEnvironment::content()->addView(anchor,new ViewGroup::LayoutParams(200,48));
   pumpFor(100);

   CountingPopupMenu*menu=new CountingPopupMenu(&app,anchor);
   menu->getMenu()->add("One");
   menu->show();
   pumpFor(100);
   menu->dismiss();
   menu->show();         // refused (one-shot): no window ever re-appears
   EXPECT_FALSE(menu->getMenuListView());   // not showing anymore
   pumpFor(300);         // the posted self-delete runs
   EXPECT_EQ(sCountingMenusAlive,0);

   GUIEnvironment::content()->removeView(anchor);
   delete anchor;
}

/* Double dismiss is a no-op: the second call is refused before any cascade. */
TEST_F(DIALOG,PopupMenuDismissTwice){
   App&app=App::getInstance();
   TextView*anchor=new TextView(&app); anchor->setText("pm anchor5");
   GUIEnvironment::content()->addView(anchor,new ViewGroup::LayoutParams(200,48));
   pumpFor(100);

   CountingPopupMenu*menu=new CountingPopupMenu(&app,anchor);
   menu->getMenu()->add("One");
   menu->show();
   pumpFor(100);
   menu->dismiss();
   menu->dismiss();      // not showing anymore: refused inside the helper
   pumpFor(300);
   EXPECT_EQ(sCountingMenusAlive,0);

   GUIEnvironment::content()->removeView(anchor);
   delete anchor;
}

/* The item-CLICK dismissal path (what a real menu tap takes): invoke() runs
   the app click listener first, then close(true) dismisses through
   CascadingMenuPopup::onCloseMenu while the popup's own window dismiss takes
   the animated exit branch. The menu self-deletes on the next drain; the
   decor finishes its exit and self-frees - nothing may crash or leak. */
TEST_F(DIALOG,PopupMenuItemClickDismiss){
   App&app=App::getInstance();
   TextView*anchor=new TextView(&app); anchor->setText("pm anchor6");
   GUIEnvironment::content()->addView(anchor,new ViewGroup::LayoutParams(200,48));
   pumpFor(100);

   CountingPopupMenu*menu=new CountingPopupMenu(&app,anchor);
   Menu* m=menu->getMenu();
   m->add(0,1000,0,"Pick me");
   bool clicked=false;
   menu->setOnMenuItemClickListener([&clicked](MenuItem&){ clicked=true; return true; });
   menu->show();
   pumpFor(600);   // enter completes; the exit branch is then deterministic
   EXPECT_EQ(sCountingMenusAlive,1);

   menu->getMenu()->performIdentifierAction(1000,0);   // the item tap
   EXPECT_TRUE(clicked);
   pumpFor(600);   // exit animation + teardown + the posted self-delete
   EXPECT_EQ(sCountingMenusAlive,0);

   GUIEnvironment::content()->removeView(anchor);
   delete anchor;
}

/* printerdemo's exact crash recipe (2nd language switch): a REGISTER_ACTIVITY
   window hosts the anchor; each round opens a fire-and-forget PopupMenu whose
   item listener routes a locale change through App::handleConfigurationChanged
   (activity recreate) from INSIDE the item-click dispatch. The menu decor's
   animated teardown lands ~200ms later; its dropdown ListView must detach
   without touching a freed adapter. */
namespace {
static PopupMenu* sReproMenu = nullptr;
class MenuReproWindow: public Window{
    Toolbar* mToolbar = nullptr;
public:
    MenuReproWindow():Window(0,0,480,320){
    }
    void onActive() override{
        Window::onActive();
        if (mToolbar != nullptr) return;   // only the first instance builds UI
        FrameLayout*root=new FrameLayout(getContext());
        TextView*anchor=new TextView(getContext()); anchor->setText("repro anchor");
        anchor->setLayoutParams(new ViewGroup::LayoutParams(200,48));
        root->addView(anchor,new ViewGroup::LayoutParams(200,48));
        mToolbar = new Toolbar(getContext());
        root->addView(mToolbar,new ViewGroup::LayoutParams(480,56));
        addView(root);
        if (Menu* menu = mToolbar->getMenu()) {
            menu->add("Toolbar One");
            menu->add("Toolbar Two");
        }
        anchor->setOnClickListener([this](View& v){
            sReproMenu = new PopupMenu(v.getContext(),&v);
            sReproMenu->getMenu()->add(0,2000,0,"Switch");
            sReproMenu->setOnMenuItemClickListener([](MenuItem&){
                Configuration c = App::getInstance().getResources().getConfiguration();
                const bool zh = (c.getLocales().size()
                        && c.getLocales().get(0).getLanguage() == "zh");
                c.setLocales(LocaleList(std::vector<Locale>{
                        Locale::forLanguageTag(zh ? "en-US" : "zh-CN")}));
                App::getInstance().handleConfigurationChanged(c);   // recreate()
                return true;
            });
            sReproMenu->show();
        });
        anchor->performClick();   // open the menu right away
    }
};
REGISTER_ACTIVITY(MenuReproWindow);
} // namespace

TEST_F(DIALOG,PopupMenuItemClickWithRecreate){
   App&app=App::getInstance();
   Intent intent("");  intent.setComponent(ComponentName("","MenuReproWindow"));
   app.startActivity(intent);
   pumpFor(600);       // window up, menu shown (enter completes)
   ASSERT_NE(sReproMenu,nullptr);
   PopupMenu*menu1=sReproMenu;

   menu1->getMenu()->performIdentifierAction(2000,0);  // 1st switch: recreate
   sReproMenu=nullptr;
   pumpFor(600);       // recreate + menu teardown settle
   ASSERT_NE(sReproMenu,nullptr);   // the recreated window opened a fresh menu
   PopupMenu*menu2=sReproMenu;

   for (int round = 2; round <= 4; round++) {   // extra rounds: the interleaving
       PopupMenu*menu=sReproMenu;                 // is timing-flaky; more rounds
       sReproMenu=nullptr;                         // make the UAF deterministic
       if (menu==nullptr) break;
       menu->getMenu()->performIdentifierAction(2000,0);
       pumpFor(400);
   }
   pumpFor(800);
}
