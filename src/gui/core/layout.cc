#include <layout.h>
#include <view/gravity.h>
#include <wordbreak.h>
#include <linebreak.h>
#ifdef ENABLE_FRIBIDI
#include <fribidi.h>
#endif
#include <cdlog.h>
#include <textutils.h>

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

static unsigned char sData[4];
static RefPtr<ImageSurface>sImage = ImageSurface::create(sData,Surface::Format::ARGB32,1,1,4);

Layout::Layout(int fontSize,int width){
    mFontSize= fontSize;
    mWidth  = width;
    mLineCount = 0;
    mAlignment = ALIGN_LEFT;
    mColumns = COLUMNS_NORMAL;
    mEllipsis= ELLIPSIS_NONE;
    mEllipsizedWidth=0;
    mSpacingMult = 1.0;
    mSpacingAdd  = 0;
    mLayout  = 0;
    mCaretPos= 0;
    mMultiline=false;
    mBreakStrategy=BREAK_STRATEGY_SIMPLE ;
    mEditable = false;
    mContext=Cairo::Context::create(sImage);
    if(Typeface::DEFAULT==nullptr)
	Typeface::loadPreinstalledSystemFontMap();
    mTypeface = Typeface::DEFAULT;
}

Layout::Layout(const Layout&l){
    mFontSize = l.mFontSize;
    mWidth = l.mWidth;
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
    mContext=Cairo::Context::create(sImage);
    setFont(l.mTypeface);
}

void Layout::setWidth(int width){
    mWidth = width;
    mLayout++;
}

int  Layout::getWidth()const{
    return mWidth;
}

void Layout::setFont(Typeface*tf){
    mTypeface = tf ;
    mContext->set_font_face(mTypeface->getFontFace()->get_font_face());
}

void Layout::setFontSize(double size){
    if(mFontSize!=size){
        mFontSize=size;
        mLayout++;
        mContext->set_font_size(mFontSize);
    }
}

double Layout::getFontSize()const{
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

double Layout::measureSize(const std::wstring&text,TextExtents&te,FontExtents*fe)const{
    std::string utext = TextUtils::unicode2utf8(text);
    mContext->set_font_face(mTypeface->getFontFace()->get_font_face());
    mContext->set_font_size(mFontSize);
    mContext->get_text_extents(utext,te);
    auto face = mTypeface->getFontFace();
    const double scale= mTypeface->getScale();
    te.x_advance *= scale;
    te.y_advance *= scale;
    te.x_bearing *= scale;
    te.y_bearing *= scale;
    te.width *= scale;
    te.height*= scale;
    if(fe){
        mContext->get_font_extents(*fe);
	fe->max_x_advance *= scale;
	fe->max_y_advance *= scale;
	fe->height *= scale;
	fe->ascent *= scale;
	fe->descent*= scale;
    }
    return te.x_advance;
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
}

int Layout::getLineForOffset(int offset)const{
    int high = getLineCount(), low = -1, guess;
    while (high - low > 1) {
        guess = (high + low) / 2;
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

int Layout::getLineHeight(bool txtonly)const{
    if(txtonly==false)
        return mLineHeight;
    return getLineBottom(0)-getLineTop(0)+1;
}

int Layout::getLineBounds(int line, RECT& bounds)const{
    int top = getLineTop(line);
    int bottom =getLineBottom(line);
    bounds.set(0, top,mWidth,bottom-top+1);
    return getLineBaseline(line);
}

void Layout::getCaretRect(RECT&rect)const{
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
    mSpacingMult=spacingMult;
    mSpacingAdd =spacingAdd;
    mLayout++;
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
        start=bsearch(widths,nChars, ellipsisX)+1;
	count=nChars-start;
        LOGD("ELLIP_START ellipsisX=%d:%d startAt %d:%d els=%d",ellipsisX,nChars,start,count,els);
        setEllipse(line,start,nChars-start);
        break;
    case Layout::ELLIPSIS_MIDDLE:
        ellipsisX =(mWidth-ellipsisX)/2;
        start=bsearch(widths,nChars,ellipsisX);
        count=nChars-(start+start);
        setEllipse(line,start,count);
        LOGV("ELLIP_MIDDLE ellipsisX=%d:%d startAt %d:%d",ellipsisX,nChars,start,count);
        break;
    case Layout::ELLIPSIS_END:
	ellipsisX=mWidth-ellipsisX;
        start= bsearch(widths,nChars,ellipsisX)-1;
        count=nChars-start;
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
    int ellipsis=mEllipsis;
    std::wstring result;
    if(getEllipsisCount(line)==0)
        ellipsis=ELLIPSIS_NONE;
    switch(ellipsis){
    case ELLIPSIS_NONE:
        start=getLineStart(line);
        count=getLineEnd(line)-start;
        result=mText.substr(start,count);
        break;
    case ELLIPSIS_START:
        start=getEllipsisStart(line);
        count=getLineEnd(line)-start;
        result=L"..."+mText.substr(start,count);
        break;
    case ELLIPSIS_MIDDLE:
        start=getLineStart(line);
        result=mText.substr(start,getEllipsisStart(line)-start); //first string before ellipsis
        result.append(L"...");
        count=getLineEnd(line) - getEllipsisStart(line)+getEllipsisCount(line);//the second string.size after ellipsis
        result+=mText.substr(getEllipsisStart(line)+getEllipsisCount(line),count);
        break;
    case ELLIPSIS_END:
        start=getLineStart(line);
        count=getEllipsisStart(line)-start;
        result=mText.substr(start,count);
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
    mBreakStrategy = breakStrategy;
    mLayout++;
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
    TextExtents extents;
    FontExtents fontextents;
    double total_width = 0;
    int start=0,ytop=0;
    std::wstring word;
    if(!(force||mLayout)) return;
    mLineCount=0;
    mLines.clear();
    measureSize(L"",extents,&fontextents);
    mLineHeight = fontextents.height*mSpacingMult+mSpacingAdd;
    for(int i=0; mMultiline && ( i<mText.length() );i++){
        char breaks[2];
        wchar_t wch[2];
        wch[0]=mText[i];
        wch[1]=mText[i+1]; 
        set_wordbreaks_utf32((utf32_t*)wch,2,"",breaks);
        int linebreak=is_line_breakable(wch[0],wch[1],"");
        wch[1]=0;
        switch(breaks[0]){
        case WORDBREAK_NOBREAK:
            word.append(1,mText[i]);
            measureSize(word,extents);
            if(mBreakStrategy && (total_width + extents.x_advance > mWidth)){
                pushLineData(start,ytop,fontextents.descent,ceil(total_width));
                ytop += mLineHeight;
                mLineCount++;
                word.erase();
                total_width=0 ; start=i;
            }else if(mBreakStrategy){
                total_width += extents.x_advance;
            }
            if(mBreakStrategy)word.erase();
            break;
        case WORDBREAK_BREAK:{
            word.append(1,mText[i]);
            measureSize(word,extents);
            int outofwidth=(total_width + extents.x_advance >mWidth);
            if( (( (breaks[0]==WORDBREAK_BREAK) && ( outofwidth && (mBreakStrategy==0) ))||(linebreak==LINEBREAK_MUSTBREAK))){
                pushLineData(start,ytop,fontextents.descent,ceil(total_width));
                ytop += mLineHeight;
                mLineCount++;
                total_width=0;
                start=i+1;
                if(outofwidth)//char[i] is wordbreak char must be in old lines
                    start=i-(word.length()-1);
            }
            total_width += extents.x_advance;
            word.erase();
            }
            break;
        case WORDBREAK_INSIDEACHAR: break;
        default:break;
        }
    }

    if(start<=mText.length()){
        measureSize(mText.substr(start),extents);
        total_width = extents.x_advance;
        pushLineData(start,ytop,fontextents.descent,ceil(total_width));
        ytop += mLineHeight;
        if( (mColumns==COLUMNS_ELLIPSIZE) && (total_width>mWidth) ){
            calculateEllipsis(mLineCount,mText.length());
        }
        mLineCount++;
    }
    pushLineData(mText.length(),ytop,fontextents.descent,0);
    mLayout=0;
}

static const std::string processBidi(const std::wstring&logstr){
#ifdef ENABLE_FRIBIDI
    size_t wsize=logstr.length()+1;
    FriBidiCharType base_dir=FRIBIDI_TYPE_ON;
    FriBidiChar * visstr= new FriBidiChar[wsize] ;
    FriBidiLevel* level = new FriBidiLevel[wsize];
    FriBidiStrIndex *posLV= new FriBidiStrIndex[wsize];
    FriBidiStrIndex *posVL= new FriBidiStrIndex[wsize];

    fribidi_log2vis((const FriBidiChar*)logstr.c_str(),logstr.length(),&base_dir,visstr,posLV,posVL,level);
    std::wstring biditxt((const wchar_t*)visstr,wsize-1);
    delete [] visstr;
    delete [] posLV;
    delete [] posVL;
    delete [] level;
    return TextUtils::unicode2utf8(biditxt); 
#else
    return TextUtils::unicode2utf8(logstr);
#endif
}

void  Layout::drawText(Canvas&canvas,int firstLine,int lastLine){
    mCaretRect.setEmpty();
    for (int lineNum = firstLine; lineNum < lastLine; lineNum++) {
        int x = 0,lw = getLineWidth(lineNum,true);
        TextExtents te;
        int y = getLineBaseline(lineNum);
        int lineStart = getLineStart(lineNum);
        int lineEnd = getLineEnd(lineNum);
        std::wstring line = getLineText(lineNum);
        measureSize(line.substr(0,1),te,nullptr);
        switch(mAlignment){
        case ALIGN_NORMAL:
        case ALIGN_LEFT  : 
        default          : x = te.x_bearing  ; break;
        case ALIGN_CENTER: x = (mWidth-lw)/2 ; break;
        case ALIGN_OPPOSITE:
        case ALIGN_RIGHT : x = mWidth-lw ; break;
        }
        LOGV("line[%d] xy=%d,%d mWidth=%d [%s](%d)'s TextWidth=%d fontsize=%.f alignment=%x abearing=%f",
             lineNum,x,y,mWidth,TextUtils::unicode2utf8(line).c_str(),line.size(),lw,mFontSize,mAlignment,te.x_bearing);
        canvas.move_to(x - te.x_bearing,y);
        canvas.show_text(processBidi(line));
        if(mCaretPos>=lineStart&&mCaretPos<lineEnd){
            measureSize(line.substr(0,mCaretPos-lineStart),te,nullptr);
            mCaretRect.left= x + te.x_advance;
            mCaretRect.top = lineNum * mLineHeight;
            mCaretRect.height= mLineHeight;
            measureSize(line.substr(mCaretPos-lineStart,1),te,nullptr);
            mCaretRect.width = te.x_advance;
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

