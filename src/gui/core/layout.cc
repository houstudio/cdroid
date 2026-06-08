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
#include <text/spannablestring.h>
#if ENABLE(FRIBIDI)
#include <fribidi.h>
#endif
#include <cdlog.h>
#include <utils/textutils.h>
using namespace Cairo;

namespace cdroid{
static const std::string processBidi(const std::wstring& logstr);

#define START   0
#define TOP     1
#define DESCENT 2
#define ASCENT  3
#define LAYOUT_WIDTH   4
#define ELLIP_START 5
#define ELLIP_COUNT 6
#define COLUMNS_NORMAL  5
#define COLUMNS_ELLIPSIZE 7

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
    mFakeTextSkew = 0.f;
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
    mFakeTextSkew = l.mFakeTextSkew;
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

void Layout::setFakeTextSkew(float skew){
    if(mFakeTextSkew!=skew){
        mFakeTextSkew= skew;
        clearScaledFontCache();
        resetScaledFont();
    }
}

void Layout::resetScaledFont(){
    const double skewX= mFakeTextSkew;
    Cairo::Matrix font_mtx(mFontSize, 0.0, mFontSize * skewX, mFontSize, 0.0, 0.0);

    Cairo::FontOptions options;
    Cairo::Matrix ctm = Cairo::identity_matrix();
    auto face = mTypeface->getFontFace();//->get_font_face();
    //mTypeface->getFontFace()->get_font_options(options);
    options.set_hint_style(Cairo::FontOptions::HintStyle::MEDIUM);
    options.set_hint_metrics(Cairo::FontOptions::HintMetrics::OFF);

    mScaledFont = Cairo::ScaledFont::create(face, font_mtx, ctm, options);
}

void Layout::clearScaledFontCache(){
    mScaledFontCache.clear();
}

Cairo::RefPtr<Cairo::ScaledFont> Layout::getScaledFont(Typeface* tf, float size) const {
    if (!tf) tf = mTypeface;
    if (tf == mTypeface && size == mFontSize) {
        return mScaledFont;
    }
    for (const auto& item : mScaledFontCache) {
        if (item.typeface == tf && item.size == size) {
            return item.scaledFont;
        }
    }
    const double skewX = mFakeTextSkew;
    Cairo::Matrix font_mtx(size, 0.0, size * skewX, size, 0.0, 0.0);
    Cairo::FontOptions options;
    Cairo::Matrix ctm = Cairo::identity_matrix();
    auto face = tf->getFontFace();//->get_font_face();
    //tf->getFontFace()->get_font_options(options);
    options.set_hint_style(Cairo::FontOptions::HintStyle::MEDIUM);
    options.set_hint_metrics(Cairo::FontOptions::HintMetrics::OFF);
    auto sf = Cairo::ScaledFont::create(face, font_mtx, ctm, options);
    mScaledFontCache.push_back(ScaledFontCacheItem{tf, size, sf});
    return sf;
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
        LOGD("Layout::setEllipsis ellipsis=%d mColumns=%d", ellipsis, mColumns);
        mLayout++;
    }
}

double Layout::measureSize(const std::string&text,TextExtents&te,FontExtents*fe)const{
    mScaledFont->get_text_extents(text,te);
    if(fe)mScaledFont->get_extents(*fe);
    return te.x_advance;
}

double Layout::measureSize(const std::wstring&text,TextExtents&te,FontExtents*fe)const{
    std::string utext = processBidi(text);
    mScaledFont->get_text_extents(utext,te);
    if(fe) mScaledFont->get_extents(*fe);
    return te.x_advance;
}

double Layout::measureSizeWithFont(Typeface* tf, float size, const std::wstring& text, TextExtents& te, FontExtents* fe) const {
    if (!tf) tf = mTypeface;
    auto sf = getScaledFont(tf, size);
    std::string utext = processBidi(text);
    sf->get_text_extents(utext, te);
    if (fe) sf->get_extents(*fe);
    return te.x_advance;
}

void Layout::getEffectiveTypefaceAndSize(int pos, Typeface*& outTf, float& outSize) const {
    Typeface* effTf = mTypeface;
    float effSize = mFontSize;
    for (const SpanItem& span : mSpans) {
        if (span.start <= pos && pos < span.end) {
            if (auto ts = std::dynamic_pointer_cast<TypefaceSpan>(span.what)) {
                effTf = Typeface::create(ts->getFamily(), Typeface::NORMAL);
            }
            if (auto st = std::dynamic_pointer_cast<StyleSpan>(span.what)) {
                effTf = Typeface::create(effTf, st->getStyle());
            }
            if (auto abs = std::dynamic_pointer_cast<AbsoluteSizeSpan>(span.what)) {
                effSize = (float)abs->getSize();
            }
            if (auto rel = std::dynamic_pointer_cast<RelativeSizeSpan>(span.what)) {
                effSize = mFontSize * rel->getProportion();
            }
        }
    }
    outTf = effTf;
    outSize = effSize;
}

double Layout::measureSizeRange(int start, int end, TextExtents& te, FontExtents* fe) const {
    if (start >= end) { te.x_advance = 0; if (fe) { fe->ascent = 0; fe->descent = 0; fe->height = 0; } return 0; }
    double total = 0.0;
    TextExtents tmp;
    FontExtents maxFe;
    bool haveFe = false;
    int i = start;
    while (i < end) {
        Typeface* eftf = mTypeface;
        float efsize = mFontSize;
        getEffectiveTypefaceAndSize(i, eftf, efsize);
        // find j where typeface/size stays same
        int j = i + 1;
        for (; j < end; ++j) {
            Typeface* t2 = nullptr; float s2 = 0;
            getEffectiveTypefaceAndSize(j, t2, s2);
            if (t2 != eftf || s2 != efsize) break;
        }
        std::wstring segment = mText.substr(i, j - i);
        FontExtents segFe;
        measureSizeWithFont(eftf, efsize, segment, tmp, fe ? &segFe : nullptr);
        total += tmp.x_advance;
        if (fe) {
            if (!haveFe) {
                maxFe = segFe;
                haveFe = true;
            } else {
                maxFe.ascent = std::max(maxFe.ascent, segFe.ascent);
                maxFe.descent = std::max(maxFe.descent, segFe.descent);
                maxFe.height = std::max(maxFe.height, segFe.height);
            }
        }
        i = j;
    }
    te.x_advance = total;
    if (fe && haveFe) {
        *fe = maxFe;
    }
    return total;
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
    return mLines[line*mColumns+ASCENT];
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
    return getLineBottom(0)-getLineTop(0);
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
    if (getLineCount()) {
        return getLineTop(line) + getLineAscent(line);
    }
    TextExtents te;
    FontExtents fe;
    measureSize(L" ",te,&fe);
    return (int)std::ceil(fe.ascent * mSpacingMult + mSpacingAdd);
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
    int lo = 0;
    int hi = size;
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (widths[mid] <= find)
            lo = mid + 1;
        else
            hi = mid;
    }
    int result = lo - 1;
    if (result < 0)
        result = 0;
    LOGV("char %d/%d atpos %d",result,size,find);
    return result;
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
        start = bsearch(widths,nChars,ellipsisX);
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
        count = getLineEnd(line) - (getEllipsisStart(line) + getEllipsisCount(line)); // the remaining suffix length
        result += mText.substr(getEllipsisStart(line) + getEllipsisCount(line), count);
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
        mSpans.clear();
        mLayout++;
        return true;
    }
    return false;
}

bool Layout::setText(const Spanned& txt){
    /*const std::wstring ws = txt.toWString();
    const std::vector<SpanInfo> sourceSpans = txt.getSpans(0,0,SpanFilter([](const CharacterStyle*){return true;}));
    bool changed = (mText.compare(ws) != 0) || (mSpans.size() != sourceSpans.size());
    if (!changed) {
        for (size_t i = 0; i < sourceSpans.size(); ++i) {
            const SpanInfo& src = sourceSpans[i];
            const SpanItem& dst = mSpans[i];
            if (dst.start != src.start || dst.end != src.end || dst.flags != src.flags || dst.what != src.what) {
                changed = true;
                break;
            }
        }
    }
    if (changed) {
        mText = ws;
        mSpans.clear();
        mSpans.reserve(sourceSpans.size());
        for (const SpanInfo& span : sourceSpans) {
            mSpans.push_back(SpanItem{span.what, span.start, span.end, span.flags});
        }
        mLayout++;
        return true;
    }*/
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

void Layout::pushLineData(int start,int ytop,int descent,int ascent,int width){
    mLines.push_back(start);
    mLines.push_back(ytop);
    mLines.push_back(descent);
    mLines.push_back(ascent);
    mLines.push_back(width);
    if(mColumns==COLUMNS_ELLIPSIZE){
        mLines.push_back(0);//5-- ELLIPSIS_START
        mLines.push_back(0);//6-- ELLIPSIS_COUNT
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
    float defaultLineHeight = (fontextents.ascent + fontextents.descent);
    if(defaultLineHeight < mFontSize)
        defaultLineHeight = fontextents.height;
    defaultLineHeight = defaultLineHeight * mSpacingMult + mSpacingAdd;

    float lineAscent = fontextents.ascent;
    float lineDescent = fontextents.descent;
    float currentLineHeight = defaultLineHeight;
    const int spanCount= mSpans.size();
    for(int i = 0; mMultiline && (mText.length()>1) && (i < mText.length()-1);i++){
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
            {
                Typeface* eftf = mTypeface;
                float efsize = mFontSize;
                getEffectiveTypefaceAndSize(i, eftf, efsize);
                FontExtents fe;
                measureSizeWithFont(eftf, efsize, std::wstring(wch), extents, &fe);
                lineAscent = std::max(lineAscent, (float)fe.ascent);
                lineDescent = std::max(lineDescent, (float)fe.descent);
            }
            word_width += extents.x_advance;
            //line_width = total_width + word_width;
            if(std::ceil(line_width+word_width) > mWidth){
                currentLineHeight = std::max(defaultLineHeight, (lineAscent + lineDescent) * mSpacingMult + mSpacingAdd);
                pushLineData(start,ytop,(int)std::ceil(lineDescent),(int)std::ceil(lineAscent),std::ceil(line_width - extents.x_advance));
                ytop += (int)std::ceil(currentLineHeight);
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
                lineAscent = fontextents.ascent;
                lineDescent = fontextents.descent;
            }
            break;
        case WORDBREAK_BREAK:
            word.append(1,mText[i]);
            {
                Typeface* eftf = mTypeface;
                float efsize = mFontSize;
                getEffectiveTypefaceAndSize(i, eftf, efsize);
                FontExtents fe;
                measureSizeWithFont(eftf, efsize, std::wstring(wch), extents, &fe);
                lineAscent = std::max(lineAscent, (float)fe.ascent);
                lineDescent = std::max(lineDescent, (float)fe.descent);
            }
            if(mText[i]==10)extents.x_advance=0;
            word_width += extents.x_advance;
            line_width = total_width + word_width;
            if( (std::ceil(line_width)>mWidth) || (linebreak==LINEBREAK_MUSTBREAK) ){
                currentLineHeight = std::max(defaultLineHeight, (lineAscent + lineDescent) * mSpacingMult + mSpacingAdd);
                pushLineData(start,ytop,(int)std::ceil(lineDescent),(int)std::ceil(lineAscent),std::ceil(total_width));
                ytop += (int)std::ceil(currentLineHeight);
                mLineCount ++;
                //char[i] is wordbreak char must be in old lines
                start = i - word.length() + 1;//std::floor(line_width)>mWidth ? (i - word.length()): (i+1);
                start +=!!(mText[start]=='\n');
                total_width = 0;
                lineAscent = fontextents.ascent;
                lineDescent = fontextents.descent;
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
        FontExtents fe;
        total_width = measureSizeRange(start, (int)mText.length(), extents, &fe);
        lineAscent = std::max(lineAscent, (float)fe.ascent);
        lineDescent = std::max(lineDescent, (float)fe.descent);
        currentLineHeight = std::max(defaultLineHeight, (lineAscent + lineDescent) * mSpacingMult + mSpacingAdd);
        pushLineData(start,ytop,(int)std::ceil(lineDescent),(int)std::ceil(lineAscent),ceil(total_width));
        ytop += (int)std::ceil(currentLineHeight);
        if( (mColumns == COLUMNS_ELLIPSIZE) && (total_width > mWidth) ){
            calculateEllipsis(mLineCount,mText.length());
        }
        mLineCount++;
    }
    pushLineData(mText.length(),ytop,(int)std::ceil(lineDescent),(int)std::ceil(lineAscent),0);
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
        std::wstring line = getLineText(lineNum);
        const int last = line.empty() ? 0 : line.back();
        if((last=='\n')||(last=='\r'))
            line.pop_back();

        std::vector<int> boundaries;
        boundaries.reserve(4 + mSpans.size() * 2);
        boundaries.push_back(lineStart);
        boundaries.push_back(lineEnd);

        for (const SpanItem& span : mSpans) {
            if (span.start < lineEnd && span.end > lineStart) {
                boundaries.push_back(std::max(span.start, lineStart));
                boundaries.push_back(std::min(span.end, lineEnd));
            }
        }
        std::sort(boundaries.begin(), boundaries.end());
        boundaries.erase(std::unique(boundaries.begin(), boundaries.end()), boundaries.end());

        std::string u8line = processBidi(line);
        measureSize(u8line, te);
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

        if (mSpans.empty()) {
            canvas.move_to(x,y);
            canvas.show_text(u8line);
        } else {
            float xpos = (float)x;
            for (size_t i = 0; i + 1 < boundaries.size(); ++i) {
                int segStart = boundaries[i];
                int segEnd = boundaries[i + 1];
                if (segStart >= segEnd) continue;
                std::wstring segment = line.substr(segStart - lineStart, segEnd - segStart);
                std::string segUtf8 = processBidi(segment);

                int spanColor = 0;
                float verticalOffset = 0.0f;
                bool hasColor = false;
                bool underline = false;
                bool strikethrough = false;
                bool isSuperscript = false;
                bool isSubscript = false;
                // Effective font properties for this segment
                Typeface* effTypeface = mTypeface;
                float effFontSize = mFontSize;

                for (const SpanItem& span : mSpans) {
                    if (span.start < segEnd && span.end > segStart) {
                        if (auto fg = std::dynamic_pointer_cast<ForegroundColorSpan>(span.what)) {
                            hasColor = true;
                            spanColor = fg->getForegroundColor();
                        }
                        if (std::dynamic_pointer_cast<UnderlineSpan>(span.what)) {
                            underline = true;
                        }
                        if (std::dynamic_pointer_cast<StrikethroughSpan>(span.what)) {
                            strikethrough = true;
                        }
                        if (std::dynamic_pointer_cast<SuperscriptSpan>(span.what)) {
                            isSuperscript = true;
                        }
                        if (std::dynamic_pointer_cast<SubscriptSpan>(span.what)) {
                            isSubscript = true;
                        }
                        if (auto ts = std::dynamic_pointer_cast<TypefaceSpan>(span.what)) {
                            // create a typeface from family name
                            effTypeface = Typeface::create(ts->getFamily(), Typeface::NORMAL);
                        }
                        if (auto st = std::dynamic_pointer_cast<StyleSpan>(span.what)) {
                            // apply style on top of current effective typeface
                            effTypeface = Typeface::create(effTypeface, st->getStyle());
                        }
                        if (auto abs = std::dynamic_pointer_cast<AbsoluteSizeSpan>(span.what)) {
                            effFontSize = (float)abs->getSize();
                        }
                        if (auto rel = std::dynamic_pointer_cast<RelativeSizeSpan>(span.what)) {
                            effFontSize = mFontSize * rel->getProportion();
                        }
                    }
                }
                if (isSuperscript || isSubscript) {
                    FontExtents fontExtents;
                    TextExtents textExtents;
                    effFontSize *= 0.8f;
                    auto tempFont = getScaledFont(effTypeface ? effTypeface : mTypeface, effFontSize);
                    measureSizeWithFont(effTypeface,effFontSize,segment,textExtents, &fontExtents);
                    
                    if (isSuperscript) {
                        verticalOffset = -fontExtents.ascent * 0.5f;;
                    } else if (isSubscript) {
                        verticalOffset = fontExtents.descent * 0.5f;;
                    }
                }
                canvas.save();
                if (hasColor) {
                    canvas.set_color(spanColor);
                }
                // set scaled font for this segment
                auto segFont = getScaledFont(effTypeface ? effTypeface : mTypeface, effFontSize);
                canvas.set_scaled_font(segFont);
                canvas.move_to(xpos, y+verticalOffset);
                canvas.show_text(segUtf8);
                if (underline || strikethrough) {
                    TextExtents underlineExt;
                    measureSizeWithFont(effTypeface, effFontSize, segment, underlineExt);
                    const float lineY = underline ? (float)(y + 1) : (float)(y - (underlineExt.height / 3));
                    canvas.move_to(xpos, lineY);
                    canvas.line_to(xpos + float(underlineExt.x_advance), lineY);
                    canvas.stroke();
                }
                canvas.restore();

                TextExtents advance;
                // measure using effective font settings
                measureSizeWithFont(effTypeface, effFontSize, segment, advance);
                xpos += float(advance.x_advance);
            }
        }

        if ((mCaretPos >= lineStart) && (mCaretPos < lineEnd) && (mCaretPos < lineStart + (int)line.size())) {
            measureSize(line.substr(0, mCaretPos - lineStart), te, nullptr);
            mCaretRect.left = int(x + te.x_advance);
            mCaretRect.top = int(lineNum * mLineHeight);
            mCaretRect.height = mLineHeight;
            measureSize(line.substr(mCaretPos - lineStart, 1), te, nullptr);
            mCaretRect.width = int(te.x_advance);
        }
    }
}

void  Layout::draw(Canvas&canvas){
    relayout();
    canvas.set_scaled_font(mScaledFont);
    drawText(canvas,0,mLineCount);
}

}

