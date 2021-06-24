#include <widget/toolbar.h>
namespace cdroid{

void ToolBar::setTitleMargin(int start, int top, int end, int bottom){
    mTitleMarginStart = start;
    mTitleMarginTop = top;
    mTitleMarginEnd = end;
    mTitleMarginBottom = bottom;

    requestLayout();
}

int ToolBar::getTitleMarginStart()const{
    return mTitleMarginStart;
}

void ToolBar::setTitleMarginStart(int margin) {
    mTitleMarginStart = margin;

    requestLayout();
}

int ToolBar::getTitleMarginTop()const {
    return mTitleMarginTop;
}

void ToolBar::setTitleMarginTop(int margin) {
    mTitleMarginTop = margin;

    requestLayout();
}


int ToolBar::getTitleMarginEnd()const{
    return mTitleMarginEnd;
}


void ToolBar::setTitleMarginEnd(int margin) {
    mTitleMarginEnd = margin;

    requestLayout();
}


int ToolBar::getTitleMarginBottom() const{
    return mTitleMarginBottom;
}

void ToolBar::setTitleMarginBottom(int margin) {
    mTitleMarginBottom = margin;
    requestLayout();
}

void ToolBar::setLogo(const std::string& resId){
}

void ToolBar::setLogo(Drawable* drawable){
}

Drawable* ToolBar::getLogo() {
    return mLogoView != nullptr ? mLogoView.getDrawable() : nullptr;
}

void ToolBar::setLogoDescription(const std::string& description){
    if (!description.empty()) {
        ensureLogoView();
    }
    if (mLogoView) {
        mLogoView->setContentDescription(description);
    }    
}

std::string ToolBar::getLogoDescription()const{
    return mLogoView ? mLogoView->getContentDescription() : "";
}

void ToolBar::ensureLogoView() {
    if (mLogoView == nullptr) {
        mLogoView = new ImageView(getContext());
    }
}

bool ToolBar::hasExpandedActionView()const{
}

void ToolBar::collapseActionView(){

}

std::string ToolBar::getTitle(){
    return mTitle;
}

void ToolBar::setTitle(const std::string&title){
    mTitle=title;
}

}
