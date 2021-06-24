#ifndef __TOOLBAR_H__
#define __TOOLBAR_H__
#include <widget/viewgroup.h>

namespace cdroid{

class ToolBar:public ViewGroup{
private:
   ActionMenuView mMenuView;
   TextView* mTitleTextView;
   TextView* mSubtitleTextView;
   ImageButton* mNavButtonView;
   ImageView* mLogoView;

   Drawable* mCollapseIcon;
   std::string mCollapseDescription;
   ImageButton* mCollapseButtonView;
   View*mExpandedActionView;

   int mPopupTheme;

   int mTitleTextAppearance;
   int mSubtitleTextAppearance;
   int mNavButtonStyle;

   int mButtonGravity;

   int mMaxButtonHeight;

   int mTitleMarginStart;
   int mTitleMarginEnd;
   int mTitleMarginTop;
   int mTitleMarginBottom;

   //RtlSpacingHelper mContentInsets;
   int mContentInsetStartWithNavigation;
   int mContentInsetEndWithActions;

   int mGravity = Gravity::START | Gravity::CENTER_VERTICAL;

   std::string mTitleText;
   std::string mSubtitleText;

   int mTitleTextColor;
   int mSubtitleTextColor;

   bool mEatingTouch;
private:
   void ensureLogoView();
public:
   void setTitleMargin(int start, int top, int end, int bottom);
   int getTitleMarginStart()const;
   void setTitleMarginStart(int margin);
   int getTitleMarginTop()const;
   void setTitleMarginTop(int margin);
   int getTitleMarginEnd()const;
   void setTitleMarginEnd(int margin);
   int getTitleMarginBottom()const;
   void setTitleMarginBottom(int margin);
   void setLogo(const std::string& resId);
   void setLogo(Drawable* drawable);
   Drawable* getLogo()const;
   void setLogoDescription(const std::string& description);
   std::string getLogoDescription()const;
   bool hasExpandedActionView()const;
   void collapseActionView();
   std::string getTitle()const;
   void setTitle(const std::string&);
};

}//namespace
#endif
