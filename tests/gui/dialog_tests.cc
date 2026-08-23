#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <vector>
#include <core/app.h>
#include <app/alertdialog.h>
#include <core/windowmanager.h>
#include <menu/menubuilder.h>
#include <menu/menupopuphelper.h>
#include <widget/adapter.h>
#include <widget/activitytransition.h>
#include <widget/cdwindow.h>
#include <widget/internal_R.h>
#include <widget/listpopupwindow.h>
#include <widget/popupwindow.h>
#include <widget/textview.h>
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
   /* ENTER ONLY: an animated exit defers the decor's detach past owners freeing borrowed
      content at dismiss (a ListView's adapter deleted right after PopupWindow::dismiss
      crashed exactly so); dismiss stays synchronous. */
   EXPECT_EQ(decor->getExitTransition(),nullptr);
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

/* The overflow-menu ownership pattern on a ListPopupWindow: dismiss() then immediately
   delete the (borrowed) adapter — exactly what ~CascadingMenuInfo does. With an animated
   popup exit this crashed: the deferred detach ran after the adapter free. Popups animate
   the ENTER only, so dismiss tears the decor down synchronously and the later view-tree
   destruction never touches the freed adapter. */
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

   lpw->dismiss();
   delete adapter;  // the menu's pattern: the borrowed adapter dies right after dismiss
   delete lpw;
   pumpFor(200);    // any deferred window teardown must not touch the freed adapter

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

   helper->dismiss();  // the overflow-menu dismiss path
   pumpFor(200);
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
