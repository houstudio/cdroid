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
#ifndef __LAYOUT_H__
#define __LAYOUT_H__
#include <core/canvas.h>
#include <core/typeface.h>

namespace cdroid{
class Layout{
private:
    std::wstring mText;
    std::vector<int>mLines;
    Cairo::RefPtr<Cairo::ScaledFont>mScaledFont;
    Typeface *mTypeface;
    Cairo::FontExtents mFontExtents;
    int mWidth;
    float mLineHeight;
    int mLineCount;
    float mFontSize;
    int mEllipsizedWidth;
    int mColumns;
    int mEllipsis;
    int mAlignment;
    int mCaretPos;
    int mSelectionStart;
    int mSelectionEnd;
    int mTextDirection;
    bool mEditable;
    bool mMultiline;
    int mBreakStrategy;
    float mFakeTextSkew;
    float mSpacingMult; //spacingMult line spacing multiplier
    int mSpacingAdd;   //spacingAdd line spacing add
    int mLayout;        //mLayout>0 need relayout
    Rect mCaretRect;
    void pushLineData(int start,int ytop,int descent,int width);
    void resetScaledFont();
    double measureSize(const std::string&text,Cairo::TextExtents&te,Cairo::FontExtents*fe=nullptr)const;
    double measureSize(const std::wstring&text,Cairo::TextExtents&te,Cairo::FontExtents*fe=nullptr)const;
    void calculateEllipsis(int line,int linewidth);
    void setEllipse(int line,int start,int count);
    const std::wstring getLineText(int line,bool expandSllipsis=false)const;
    int getOffsetToLeftRightOf(int caret, bool toLeft)const;
public:
    enum Ellipsis{
        ELLIPSIS_NONE  =0,
        ELLIPSIS_START =1,
        ELLIPSIS_MIDDLE=2,
        ELLIPSIS_END   =3,
        ELLIPSIS_MARQUEE=4
    };
    enum Alignment {
        ALIGN_NORMAL,
        ALIGN_OPPOSITE,
        ALIGN_CENTER,
        ALIGN_LEFT,
        ALIGN_RIGHT
    };
    enum BreakStrategy{
        BREAK_STRATEGY_SIMPLE=0,
        BREAK_STRATEGY_HIGH_QUALITY=1,
        BREAK_STRATEGY_BALANCED=2,
    };
    Layout(int fontSize,int width);
    Layout(const Layout&l);
    void setAlignment(int alignment);
    int  getAlignment()const;
    void setWidth(int width);
    int  getWidth()const;
    Typeface*getTypeface()const;
    void setTypeface(Typeface*tf);
    void setFontSize(float size);
    float getFontSize()const;
    bool setText(const std::string&);
    bool setText(const std::wstring&);
    void setEditable(bool);
    bool isEditable()const;
    int getEllipsis()const;
    void setEllipsis(int);
    void setFakeTextSkew(float);
    const std::string getString()const;
    std::wstring & getText(); //for edit 
    void setMultiline(bool enabe);
    void setBreakStrategy(int breakStrategy);
    int getBreakStrategy()const;
    void setCaretPos(int caretpos);
    void relayout(bool force=false);
    //,bool multiline=false,bool wordbreak=false);
    void setLineSpacing(int spacingAdd, float spacingMult);
    virtual int getLineCount()const;
    virtual int getLineTop(int line)const;
    virtual int getLineLeft(int line)const;
    virtual int getLineRight(int line)const;
    virtual int getLineDescent(int line)const;
    virtual int getLineStart(int line)const;
    virtual int getLineWidth(int line,bool expandEllipsys=true)const;
    int getMaxLineWidth()const;
    int getLineForOffset(int offset)const;//get line by char offset
    int getOffsetToLeftOf(int offset)const;
    int getOffsetToRightOf(int offset)const;
    int setSelection(int start,int stop);
    int getSelectionStart()const;
    int getSelectionEnd()const;
    virtual int getEllipsisStart(int line)const;
    virtual int getEllipsisCount(int line)const;

    const Cairo::FontExtents& getFontExtents()const;
    int getLineBaseline(int line)const;
    int getLineAscent(int line)const;
    int getLineBottom(int line)const;
    int getLineEnd(int line)const;
    void setTextDirection(int);
    int getTextDirection()const;
    int getParagraphDirection(int line)const;
    int getLineHeight(bool txtonly=false)const;
    int getLineBounds(int line, Rect& bounds)const; 
    void getCaretRect(Rect&r)const;
    int getHeight(bool cap=false)const;
    int getEllipsizedWidth() const;
    void drawText(Canvas&canvas,int firstLine,int lastLine);
    void draw(Canvas&canvas);
};
};
#endif
