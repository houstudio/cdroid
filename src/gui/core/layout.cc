/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <layout.h>
#include <view/gravity.h>
#include <wordbreak.h>
#include <linebreak.h>
#include <gui_features.h>
#if ENABLE(FRIBIDI)
#include <fribidi.h>
#endif
#include <cdlog.h>
#include <utils/textutils.h>
using namespace Cairo;

namespace cdroid{

#define START   0
#define TOP     1
#define DESCENT 2
#define LAYOUT_WIDTH   3
#define ELLIP_START 4
#define ELLIP_COUNT 5
#define COLUMNS_NORMAL  4
#define COLUMNS_ELLIPSIZE 6

Layout::Layout(int fontSize,int width)
       :mTypeface(nullptr){
    mFontSize= fontSize;
    mWidth  = width;
    mLineCount = 0;
    mAlignment = ALIGN_LEFT;
    mColumns = COLUMNS_NORMAL;
    mEllipsis= ELLIPSIS_NONE;
    mEllipsizedWidth = 0;
    mSpacingMult = 1.0;
    mSpacingAdd  = 0;
    mLineHeight  = 0;
    mLayout  = 0;
    mCaretPos= 0;
    mMultiline= false;
    mBreakStrategy = BREAK_STRATEGY_SIMPLE ;
    mEditable = false;
    mSelectionStart =-1;
    mSelectionEnd = -1;
    mTextDirection= 0;
    mText.clear();
    if(Typeface::DEFAULT == nullptr)
	    Typeface::loadPreinstalledSystemFontMap();
    setTypeface(Typeface::DEFAULT);
}

Layout::Layout(const Layout&l):Layout(l.mFontSize,l.mWidth){
    mLineCount = l.mLineCount;
    mAlignment = l.mAlignment;
    mColumns = l.mColumns;
    mEllipsis= l.mEllipsis;
    mEllipsizedWidth= l.mEllipsizedWidth;
    mSpacingMult= l.mSpacingMult;
    mSpacingAdd = l.mSpacingAdd;
    mMultiline  = l.mMultiline;
    mLineHeight = l.mLineHeight;
    mLayout  = l.mLayout;
    mCaretPos= l.mCaretPos;
    mBreakStrategy=l.mBreakStrategy;
    mEditable = l.mEditable;
    mText = l.mText;
    mLines= l.mLines;
    mTextDirection = l.mTextDirection;
    mSelectionStart = l.mSelectionStart;
    mSelectionEnd = l.mSelectionEnd;
    setTypeface(l.mTypeface);
}

void Layout::setWidth(int width){
    if(mWidth != width){
        mWidth = width;
        mLayout++;
    }
}

int  Layout::getWidth()const{
    return mWidth;
}

Typeface*Layout::getTypeface()const{
    return mTypeface;
}

void Layout::setTypeface(Typeface*tf){
    if(mTypeface!=tf){
        mTypeface = tf ;
        resetScaledFont();
    }
}

void Layout::resetScaledFont(){
    Cairo::Matrix font_mtx = Cairo::identity_matrix();
    font_mtx.scale(mFontSize, mFontSize);

    Cairo::FontOptions options;
    Cairo::Matrix ctm = Cairo::identity_matrix();
    auto face = mTypeface->getFontFace()->get_font_face();
    mTypeface->getFontFace()->get_font_options(options);
    options.set_hint_style(Cairo::FontOptions::HintStyle::MEDIUM);
    options.set_hint_metrics(Cairo::FontOptions::HintMetrics::OFF);
    mScaledFont = Cairo::ScaledFont::create(face, font_mtx, ctm, options);
}

void Layout::setFontSize(float size){
    if(mFontSize!=size){
        mFontSize=size;
        mLayout++;
        resetScaledFont();
    }
}

float Layout::getFontSize()const{
    return mFontSize;
}

void Layout::setEditable(bool b){
    mEditable=b;
}

bool Layout::isEditable()const{
    return mEditable;
}

int Layout::getEllipsis()const{
    return mEllipsis;
}

void Layout::setEllipsis(int ellipsis){
    if((mEllipsis!=ellipsis)&&(ellipsis>=ELLIPSIS_NONE)&&(ellipsis<=ELLIPSIS_MARQUEE)){
        mEllipsis= ellipsis;
        mColumns = (ellipsis==ELLIPSIS_NONE||ellipsis==ELLIPSIS_MARQUEE)?COLUMNS_NORMAL:COLUMNS_ELLIPSIZE;
        mLayout++;
    }
}

double Layout::measureSize(const std::string&text,TextExtents&te,FontExtents*fe)const{
    mScaledFont->get_text_extents(text,te);
    if(fe)mScaledFont->get_extents(*fe);
    return te.x_advance;
}

double Layout::measureSize(const std::wstring&text,TextExtents&te,FontExtents*fe)const{
    std::string utext = TextUtils::unicode2utf8(text);
    mScaledFont->get_text_extents(utext,te);
    if(fe) mScaledFont->get_extents(*fe);
    return te.x_advance;
}

const Cairo::FontExtents& Layout::getFontExtents()const{
    return mFontExtents;
}

int Layout::getEllipsizedWidth() const{
    return mEllipsizedWidth;
}

int  Layout::getHeight(bool cap)const{
    const int lc = getLineCount();
    return lc?getLineTop(lc):0;
}

int Layout::getLineCount()const{
    return mLineCount;
}

void Layout::setTextDirection(int dir){
    if(mTextDirection!= dir){
        mTextDirection = dir;
        mLayout++;
    }
}

int Layout::getTextDirection()const{
    return mTextDirection;
}

int Layout::getParagraphDirection(int line)const{
   return 1;
}

int Layout::getLineTop(int line)const{
    if(line<0)line = mLineCount-1;
    return mLines[line*mColumns+TOP];
}

int Layout::getLineDescent(int line)const{
    return mLines[line*mColumns+DESCENT];
}

int Layout::getLineStart(int line)const{
    return mLines[line*mColumns+START];
}

int Layout::getLineAscent(int line)const{
    return getLineTop(line) - (getLineTop(line+1) - getLineDescent(line));
}

int Layout::getLineBottom(int line)const{
    return getLineTop(line + 1);
}

int Layout::getLineEnd(int line)const{
    return getLineStart(line + 1);
}

int Layout::getLineWidth(int line,bool expandEllipsis)const{
    int width = mLines[line*mColumns+LAYOUT_WIDTH];
    if( expandEllipsis && (mColumns==COLUMNS_ELLIPSIZE) && (width>mWidth) )
        width = mWidth;
    return width;
}

int Layout::getMaxLineWidth()const{
    int maxW = 0;
    int cnt  = getLineCount();
    for(int i=0;i<cnt;i++)
        maxW = std::max(maxW,getLineWidth(i,true));
    return maxW;
}

int Layout::getLineLeft(int line)const{
    return 0;
}

/** Get the rightmost position that should be exposed for horizontal
  * scrolling on the specified line.*/
int Layout::getLineRight(int line)const{
    switch(mAlignment){
    case ALIGN_LEFT:
         return 0;//getLineMax(line); 
    case ALIGN_CENTER:
    case ALIGN_RIGHT: return mWidth;
    }
    return 0;
}

int Layout::getLineForOffset(int offset)const{
    int high = getLineCount(), low = -1;
    while (high - low > 1) {
        const int guess = (high + low) / 2;
        if (getLineStart(guess) > offset)
            high = guess;
        else
            low = guess;
    }
    return low<0?0:low;
}

int Layout::getOffsetToLeftRightOf(int caret, bool toLeft)const{
    int line = getLineForOffset(caret);
    int lineStart = getLineStart(line);
    int lineEnd = getLineEnd(line);
    //int lineDir = getParagraphDirection(line);

    bool lineChanged = false;
    bool advance = toLeft;// == (lineDir == DIR_RIGHT_TO_LEFT);
    // if walking off line, look at the line we're headed to
    if (advance) {
        if (caret == lineEnd) {
            if (line < getLineCount() - 1) {
                lineChanged = true;
                ++line;
            } else {
                //return caret; // at very end, don't move
            }
        }
    } else {
        if (caret == lineStart) {
            if (line > 0) {
                lineChanged = true;
                --line;
            } else {
                //return caret; // at very start, don't move
            }
        }
    }

    if (lineChanged) {
        lineStart = getLineStart(line);
        lineEnd = getLineEnd(line);
        /*int newDir = getParagraphDirection(line);
        if (newDir != lineDir) {
            // unusual case.  we want to walk onto the line, but it runs
            // in a different direction than this one, so we fake movement
            // in the opposite direction.
            toLeft = !toLeft;
            lineDir = newDir;
        }*/
    }

    //Directions directions = getLineDirections(line);

    //TextLine tl = TextLine.obtain();
    // XXX: we don't care about tabs
    //tl.set(mPaint, mText, lineStart, lineEnd, lineDir, directions, false, null);
    std::wstring subtxt = mText.substr(lineStart,caret-lineStart);
    TextExtents extents;
    measureSize(subtxt,extents) ;
    int x = (int)extents.x_advance;
    //caret=lineStart+ tl.getOffsetToLeftRightOf(caret - lineStart, toLeft);
    return x;
}

int Layout::getOffsetToLeftOf(int offset) const{
    return getOffsetToLeftRightOf(offset, true);
}

int Layout::getOffsetToRightOf(int offset) const{
    return getOffsetToLeftRightOf(offset, false);
}

int Layout::setSelection(int start,int stop){
    mSelectionStart = start;
    mSelectionEnd = stop;
    return 0;
}

int Layout::getSelectionStart()const{
    return mSelectionStart;
}

int Layout::getSelectionEnd()const{
    return mSelectionEnd;
}

int Layout::getLineHeight(bool txtonly)const{
    if(txtonly==false)
        return mLineHeight;
    return getLineBottom(0)-getLineTop(0)+1;
}

int Layout::getLineBounds(int line, Rect& bounds)const{
    int top = getLineTop(line);
    int bottom =getLineBottom(line);
    bounds.set(0, top,mWidth,bottom-top+1);
    return getLineBaseline(line);
}

void Layout::getCaretRect(Rect&rect)const{
    rect = mCaretRect;
}

int Layout::getLineBaseline(int line)const {
    TextExtents te;
    FontExtents fe;
    if(getLineCount())return getLineTop(line+1) - getLineDescent(line);
    measureSize(L" ",te,&fe);
    return fe.height*mSpacingMult+mSpacingAdd + fe.descent;
}

int Layout::getEllipsisStart(int line)const{
    if (mColumns < COLUMNS_ELLIPSIZE) {
        return 0;
    }
    return mLines[mColumns * line + ELLIP_START];
}

int Layout::getEllipsisCount(int line)const{
    if (mColumns < COLUMNS_ELLIPSIZE) {
        return 0;
    }
    return mLines[mColumns * line + ELLIP_COUNT];
}

void Layout::setLineSpacing(int spacingAdd, float spacingMult){
    if((mSpacingMult!=spacingMult)||(mSpacingAdd!=spacingAdd)){
        mSpacingMult = spacingMult;
        mSpacingAdd = spacingAdd;
        mLayout++;
    }
}

static int bsearch(int *widths,int size,int find){
    int lo=0;
    int hi=size-1;
    while((lo<hi)){
        int mid=(lo+hi)/2;
        if(widths[mid]>find)
            hi=mid-1;
        else if(widths[mid]<find)
            lo=mid+1;
	else break;
    }
    LOGV("char %d/%d atpos %d",lo,size,find);
    return lo;
}

void  Layout::setEllipse(int line,int start,int count){
    mLines[line*mColumns+ELLIP_START]=start;
    mLines[line*mColumns+ELLIP_COUNT]=count;
}

void Layout::calculateEllipsis(int line,int nChars){
    int start;
    int count;
    int ellipsisX;
    int wordstart=0;
    int *widths=new int[nChars];
    float total_width=.0;
    TextExtents extents;
    std::wstring sword;
    char breaks[2];
    utf32_t wch[2];
    for(int i=0,pos=0;i<nChars;i++){
        wch[0]=mText[i];
        wch[1]=mText[i+1];
        set_wordbreaks_utf32(wch,2,"",breaks);
        switch(breaks[0]){
        case WORDBREAK_BREAK:
        case WORDBREAK_NOBREAK:
            sword=mText.substr(wordstart,i-wordstart);
            measureSize(sword,extents);
            total_width+=extents.x_advance;
            widths[i]=total_width;
            wordstart=i;
            break;
        case WORDBREAK_INSIDEACHAR:
            break;
        }
    }
    ellipsisX=ceil(measureSize(L"...",extents));
    int els=ellipsisX;
    switch(mEllipsis){
    case ELLIPSIS_NONE :LOGD("calculateEllipsis:ELLIPSIS_NONE");
        delete []widths;return ;
    case Layout::ELLIPSIS_START:
        ellipsisX +=mLines[line*mColumns+LAYOUT_WIDTH]-mWidth;
        start = bsearch(widths,nChars, ellipsisX)+1;
        count = nChars-start;
        LOGD("ELLIP_START ellipsisX=%d:%d startAt %d:%d els=%d",ellipsisX,nChars,start,count,els);
        setEllipse(line,start,nChars-start);
        break;
    case Layout::ELLIPSIS_MIDDLE:
        ellipsisX = (mWidth-ellipsisX)/2;
        start = bsearch(widths,nChars,ellipsisX);
        count = nChars - (start+start);
        setEllipse(line,start,count);
        LOGV("ELLIP_MIDDLE ellipsisX=%d:%d startAt %d:%d",ellipsisX,nChars,start,count);
        break;
    case Layout::ELLIPSIS_END:
        ellipsisX = mWidth - ellipsisX;
        start = bsearch(widths,nChars,ellipsisX) - 1;
        count = nChars - start;
        setEllipse(line,start,count);
        LOGV("ELLIP_END ellipsisX=%d:%d startAt %d:%d",ellipsisX,nChars,start,count);
        break;
    default:break;
    }
    delete []widths;
    mLines[line*mColumns+ELLIP_START]+=getLineStart(line);
}

const std::wstring Layout::getLineText(int line,bool expandEllipsis)const{
    int start;
    int count;
    int ellipsis = mEllipsis;
    std::wstring result;
    if(getEllipsisCount(line)==0)
        ellipsis = ELLIPSIS_NONE;
    switch(ellipsis){
    case ELLIPSIS_NONE:
        start = getLineStart(line);
        count = getLineEnd(line) - start;
        result= mText.substr(start,count);
        break;
    case ELLIPSIS_START:
        start = getEllipsisStart(line);
        count = getLineEnd(line) - start;
        result= L"..."+mText.substr(start,count);
        break;
    case ELLIPSIS_MIDDLE:
        start = getLineStart(line);
        result= mText.substr(start,getEllipsisStart(line)-start); //first string before ellipsis
        result.append(L"...");
        count = getLineEnd(line) - getEllipsisStart(line) + getEllipsisCount(line);//the second string.size after ellipsis
        result += mText.substr(getEllipsisStart(line) + getEllipsisCount(line),count);
        break;
    case ELLIPSIS_END:
        start = getLineStart(line);
        count = getEllipsisStart(line)-start;
        result= mText.substr(start,count);
        result.append(L"...");
        break;
    }
    return result;//TextUtils::unicode2utf8(result);
}

bool Layout::setText(const std::wstring&txt){
    if(mText.compare(txt)){
        mText=txt;
        mLayout++;
        return true;
    }
    return false;
}

bool Layout::setText(const std::string&txt){
    std::wstring ws=TextUtils::utf8tounicode(txt);
    return setText(ws);
}

const std::string Layout::getString()const{
    return TextUtils::unicode2utf8(mText);
}

std::wstring&Layout::getText(){
    return mText;
}

void Layout::setAlignment(int align){
    mAlignment = align;
}
int Layout::getAlignment()const{
    return mAlignment;
}

void Layout::setMultiline(bool enable){
    mMultiline = enable;
}

void Layout::setCaretPos(int pos){
    mCaretPos = pos;
}

void Layout::setBreakStrategy(int breakStrategy){
    if(mBreakStrategy != breakStrategy){
        mBreakStrategy = breakStrategy;
        mLayout++;
    }
}

int Layout::getBreakStrategy()const{
    return mBreakStrategy;
}

void Layout::pushLineData(int start,int ytop,int descent,int width){
    mLines.push_back(start);
    mLines.push_back(ytop);
    mLines.push_back(descent);
    mLines.push_back(width);
    if(mColumns==COLUMNS_ELLIPSIZE){
        mLines.push_back(0);//4-- ELLIPSIS_START
        mLines.push_back(0);//5-- ELLIPSIS_COUNT
    }
}

void Layout::relayout(bool force){
    TextExtents extents,tee;
    FontExtents fontextents;
    double total_width = 0,word_width = 0;
    int start = 0,ytop = 0;

    std::wstring word;
    if(!(force||mLayout)) return;
    mLineCount = 0;
    mLines.clear();
    measureSize(L"",extents,&fontextents);
    mLineHeight = (fontextents.ascent + fontextents.descent);

    if(mLineHeight<mFontSize)
        mLineHeight = fontextents.height;
    mLineHeight = mLineHeight*mSpacingMult+mSpacingAdd;
    for(int i = 0; mMultiline && (i < mText.length());i++){
        char breaks[2];
        wchar_t wch[2];
        float line_width=0;
        wch[0] = mText[i];
        wch[1] = mText[i+1];
        set_wordbreaks_utf32((utf32_t*)wch,2,"",breaks);
        const int linebreak = is_line_breakable(wch[0],wch[1],"");
        wch[1] = 0;
        //measureSize(mText.substr(start,i-start+1),tee);
        switch(breaks[0]){
        case WORDBREAK_NOBREAK:
            word.append(1,mText[i]);
            measureSize(wch,extents);
            word_width += extents.x_advance;
            //line_width = total_width + word_width;
            if(std::ceil(line_width+word_width) > mWidth){
                pushLineData(start,ytop,fontextents.descent,std::ceil(line_width - extents.x_advance));
                ytop += mLineHeight;
                if(mBreakStrategy==BREAK_STRATEGY_SIMPLE){
                    start = std::max(mLineCount,int(i - 1));
                    total_width = extents.x_advance;
                    word_width = extents.x_advance;
                    word.clear();
                    word.append(1,mText[i]);
                }else{
                    start = std::max(mLineCount,int(i - word.length()));
                    start +=!!(mText[start]=='\n');
                    total_width = word_width - extents.x_advance;
                    word.erase();
                    word_width=0;
                }
                mLineCount++;
            }
            break;
        case WORDBREAK_BREAK:
            word.append(1,mText[i]);
            measureSize(wch,extents);
            if(mText[i]==10)extents.x_advance=0;
            word_width += extents.x_advance;
            line_width = total_width + word_width;
            if( (std::ceil(line_width)>mWidth) || (linebreak==LINEBREAK_MUSTBREAK) ){
                pushLineData(start,ytop,fontextents.descent,std::ceil(total_width));
                ytop += mLineHeight;
                mLineCount ++;
                //char[i] is wordbreak char must be in old lines
                start = i - word.length() + 1;//std::floor(line_width)>mWidth ? (i - word.length()): (i+1);
                start +=!!(mText[start]=='\n');
                total_width = 0;
            }
            total_width += word_width;
            word_width = 0;
            word.erase();
            break;
        case WORDBREAK_INSIDEACHAR: break;
        default:break;
        }
    }

    if(start <= mText.length()){
        measureSize(mText.substr(start),extents);
        total_width = extents.x_advance;
        pushLineData(start,ytop,fontextents.descent,ceil(total_width));
        ytop += mLineHeight;
        if( (mColumns == COLUMNS_ELLIPSIZE) && (total_width > mWidth) ){
            calculateEllipsis(mLineCount,mText.length());
        }
        mLineCount++;
    }
    pushLineData(mText.length(),ytop,fontextents.descent,0);
    mLayout = 0;
}

static const std::string processBidi(const std::wstring&logstr){
#if ENABLE(FRIBIDI)
    const size_t wsize = logstr.length();
    FriBidiCharType base_dir = FRIBIDI_TYPE_ON;
    auto visstr = std::make_unique<FriBidiChar[]>(wsize);
    auto bidi_types = std::make_unique<FriBidiCharType[]>(wsize);

    fribidi_get_bidi_types((const FriBidiChar*)logstr.c_str(), wsize, bidi_types.get());
    for (int i = 0; i < wsize; i++) {
        if (FRIBIDI_IS_STRONG(bidi_types[i])) {
            //LOGV("->Strong character[%d]: %s\n",i,FRIBIDI_IS_RTL(bidi_types[i]) ? "RTL" : "LTR");
        }
    }
    fribidi_log2vis((const FriBidiChar*)logstr.c_str(),logstr.length(),&base_dir,visstr.get(),nullptr,nullptr,nullptr);
    std::wstring biditxt((const wchar_t*)visstr.get(),wsize);
    return TextUtils::unicode2utf8(biditxt); 
#else
    return TextUtils::unicode2utf8(logstr);
#endif
}

void  Layout::drawText(Canvas&canvas,int firstLine,int lastLine){
    mCaretRect.setEmpty();
    LOGV("%p layoutWidth=%d fontSize=%.f alignment=%x breakStrategy=%d",this,mWidth,mFontSize,mAlignment,mBreakStrategy);
    for (int lineNum = firstLine; lineNum < lastLine; lineNum++) {
        TextExtents te = {0};
        int x , y = getLineBaseline(lineNum);
        int lineStart = getLineStart(lineNum);
        int lineEnd = getLineEnd(lineNum);
        std::string u8line;
        std::wstring line = getLineText(lineNum);
        const int last = line.back();
        if((last=='\n')||(last=='\r'))
           line.pop_back();
        u8line = processBidi(line);
        measureSize(u8line,te);
        switch(mAlignment){
        case ALIGN_NORMAL:
        case ALIGN_LEFT  : 
        default          : x = 0 ; break;
        case ALIGN_CENTER: x = (mWidth - te.x_advance)/2 ; break;
        case ALIGN_OPPOSITE:
        case ALIGN_RIGHT : x = mWidth - te.x_advance ; break;
        }

        LOGV("line[%d/%d](%d,%d) [%s](%d).width=%d",lineNum,mLineCount,x,y,TextUtils::unicode2utf8(line).c_str(),
            line.size(),int(te.x_advance));
        canvas.move_to(x,y);
        canvas.show_text(u8line);
        if( (mCaretPos>=lineStart) && (mCaretPos<lineEnd) &&(mCaretPos<lineStart+line.size()) ){
            measureSize(line.substr(0,mCaretPos-lineStart),te,nullptr);
            mCaretRect.left= int(x + te.x_advance);
            mCaretRect.top = int(lineNum * mLineHeight);
            mCaretRect.height= mLineHeight;
            measureSize(line.substr(mCaretPos-lineStart,1),te,nullptr);
            mCaretRect.width = int(te.x_advance);
        }
    }
}

void  Layout::draw(Canvas&canvas){
    relayout();
    canvas.set_font_size(mFontSize);
    canvas.set_font_face(mTypeface->getFontFace()->get_font_face());
    drawText(canvas,0,mLineCount);
}

}

