#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <vector>
#include <core/app.h>
#include <app/alertdialog.h>
#include <widget/activitytransition.h>
#include <widget/cdwindow.h>
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

/* Popups opt out of theme window animations: AOSP windowAnimationStyle is the app/activity
   window mechanism (popups carry their own animation style), and CDROID popups align to
   their anchor AFTER creation, so a ctor-time enter snap would use a stale resting position
   and drag the popup to it. The popup decor IS a Window subclass, so this guards against
   the theme load creeping back in — it crashed menu teardown (the deferred exit-animation
   detach ran after ~CascadingMenuInfo had deleted the list adapter). */
TEST_F(DIALOG,PopupWindowSkipsThemeTransitions){
   App&app=App::getInstance();
   TextView*anchor=new TextView(&app); anchor->setText("anchor");
   GUIEnvironment::content()->addView(anchor, new ViewGroup::LayoutParams(200,48));
   pumpFor(100);

   PopupWindow*popup=new PopupWindow(&app);
   TextView*content=new TextView(&app); content->setText("popup");
   popup->setContentView(content);
   popup->setWidth(200);
   popup->setHeight(300);
   popup->showAsDropDown(anchor,0,0);
   pumpFor(100);

   /* The decor is the popup's Window subclass; with the theme load it would have both a
      SLIDE enter and exit transition (app theme -> Animation.Activity). */
   Window*decor=(Window*)popup->getContentView()->getRootView();
   ASSERT_NE(decor,nullptr);
   EXPECT_EQ(decor->getEnterTransition(),nullptr);
   EXPECT_EQ(decor->getExitTransition(),nullptr);

   popup->dismiss();  // exercises the dismiss path (sync, no animation)
   pumpFor(100);
   GUIEnvironment::content()->removeView(anchor);
   delete anchor;
   delete popup;
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
