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
#include <porting/cdlog.h>
#include <utils/textutils.h>
#include <core/keyboard.h>
#include <core/tokenizer.h>
#include <core/xmlpullparser.h>
#include <core/typedarray.h>
#include <androidfw/typedvalue.h>
#include <widget/internal_R.h>
#include <widget/framework_styleable.h>
using namespace cdroid::internal;
#include <vector>
#include <fstream>

namespace cdroid{

// AOSP Keyboard.getDimensionOrFraction: dimension → pixels, fraction → ×base.
static int getDimensionOrFraction(const TypedArray& a,int idx,int base,int def){
    TypedValue value;
    if(!a.peekValue(idx,&value)) return def;
    if(value.type==TypedValue::TYPE_DIMENSION){
        return a.getDimensionPixelSize(idx,def);
    }else if(value.type==TypedValue::TYPE_FRACTION){
        // Round down to be close to the common behavior of layout dimensions
        return (int)(value.getFraction(base,base)*base);
    }
    return def;
}

Keyboard::Key::Key(Context*ctx,Row*parent,int x,int y,XmlPullParser&parser,const AttributeSet&attrs)
  :Keyboard::Key(parent){
    this->x = x;
    this->y = y;
    Keyboard::Row*row=(Keyboard::Row*)parent;
    Keyboard*keyboard = row->parent;
    auto a = ctx->obtainStyledAttributes(attrs, R::styleable::Keyboard);
    width = getDimensionOrFraction(*a, R::styleable::Keyboard_keyWidth,
            keyboard->mDisplayWidth, row->defaultWidth);
    height= getDimensionOrFraction(*a, R::styleable::Keyboard_keyHeight,
            keyboard->mDisplayHeight, row->defaultHeight);
    gap   = getDimensionOrFraction(*a, R::styleable::Keyboard_horizontalGap,
            keyboard->mDisplayWidth, row->defaultHorizontalGap);

    auto ka = ctx->obtainStyledAttributes(attrs, R::styleable::Keyboard_Key);
    this->x += gap;
    // AOSP: codes is a single int (TYPE_INT_DEC/HEX) or a CSV string.
    TypedValue codesValue;
    if (ka->peekValue(R::styleable::Keyboard_Key_codes, &codesValue)) {
        if (codesValue.type == TypedValue::TYPE_INT_DEC
                || codesValue.type == TypedValue::TYPE_INT_HEX) {
            codes.push_back(codesValue.data);
        } else if (codesValue.type == TypedValue::TYPE_STRING) {
            parseCSV(ka->getString(R::styleable::Keyboard_Key_codes), codes);
        }
    }

    iconPreview = ka->getDrawable(R::styleable::Keyboard_Key_iconPreview);
    if (iconPreview) {
        iconPreview->setBounds(0,0,iconPreview->getIntrinsicWidth(),iconPreview->getIntrinsicHeight());
    }
    popupCharacters = ka->getString(R::styleable::Keyboard_Key_popupCharacters);
    popupResId      = ka->getResourceId(R::styleable::Keyboard_Key_popupKeyboard, 0);
    repeatable= ka->getBoolean(R::styleable::Keyboard_Key_isRepeatable,false);
    sticky    = ka->getBoolean(R::styleable::Keyboard_Key_isSticky,false);
    modifier  = ka->getBoolean(R::styleable::Keyboard_Key_isModifier,false);
    // aapt2 compiles the keyEdgeFlags flags (left=1/right=2) to ints.
    edgeFlags = ka->getInt(R::styleable::Keyboard_Key_keyEdgeFlags, 0);
    edgeFlags |= row->rowEdgeFlags;

    icon = ka->getDrawable(R::styleable::Keyboard_Key_keyIcon);
    if (icon) {
        icon->setBounds(0,0,icon->getIntrinsicWidth(),icon->getIntrinsicHeight());
    }
    label = ka->getString(R::styleable::Keyboard_Key_keyLabel);
    text  = ka->getString(R::styleable::Keyboard_Key_keyOutputText);
    if(codes.size()==0&&label.empty()==false){
        std::wstring ws=TextUtils::utf8tounicode(label);
        codes.push_back(ws[0]);
    }
    LOGV("Key[%x]%s(%d,%d,%d,%d) gap=%d",codes[0],label.c_str(),x,y,width,height,gap);
}

Keyboard::Key::Key(Row*row){
    parent = row;
    sticky = modifier = 0;
    x=  y  = gap =0;
    width  = row->defaultWidth;
    height = row->defaultHeight;
    edgeFlags = row->rowEdgeFlags;
    on = false;
    pressed = false;
    repeatable = false;
    icon = nullptr;
    iconPreview = nullptr;
}

void Keyboard::Key::onPressed() {
    pressed = !pressed;
}

void Keyboard::Key::onReleased(bool inside) {
    pressed = !pressed;
    if (sticky && inside) {
       on = !on;
    }
}

int Keyboard::Key::parseCSV(const std::string& value,std::vector<int>& codes){
    Tokenizer*token;
    Tokenizer::fromContents("csv",value.c_str(),&token);
    do{
        std::string s=token->nextToken(",");
        token->skipDelimiters(",");
        codes.push_back(strtoul(s.c_str(),nullptr,10));
    }while(!token->isEof());
    delete token;
    return codes.size();
}

bool Keyboard::Key::isInside(int x, int y) {
    const bool leftEdge = (edgeFlags & EDGE_LEFT) > 0;
    const bool rightEdge = (edgeFlags & EDGE_RIGHT) > 0;
    const bool topEdge = (edgeFlags & EDGE_TOP) > 0;
    const bool bottomEdge = (edgeFlags & EDGE_BOTTOM) > 0;
    if ((x >= this->x || (leftEdge && x <= this->x + this->width))
            && (x < this->x + this->width || (rightEdge && x >= this->x))
            && (y >= this->y || (topEdge && y <= this->y + this->height))
            && (y < this->y + this->height || (bottomEdge && y >= this->y))) {
        return true;
    } else {
        return false;
    }
}

int Keyboard::Key::squaredDistanceFrom(int x, int y){
   const int xDist = this->x + width / 2 - x;
   const int yDist = this->y + height / 2 - y;
   return xDist * xDist + yDist * yDist;
}

static std::vector<int> KEY_STATE_NORMAL_ON = { 
     (int)cdroid::internal::R::attr::state_checkable,//StateSet::android.R.attr.state_checkable,
     (int)cdroid::internal::R::attr::state_checked   //android.R.attr.state_checked
};
        
static std::vector<int> KEY_STATE_PRESSED_ON = { 
     (int)cdroid::internal::R::attr::state_pressed  ,// android.R.attr.state_pressed,
     (int)cdroid::internal::R::attr::state_checkable,// android.R.attr.state_checkable,
     (int)cdroid::internal::R::attr::state_checked   // android.R.attr.state_checked
};
        
static std::vector<int> KEY_STATE_NORMAL_OFF = { 
     (int)cdroid::internal::R::attr::state_checkable//android.R.attr.state_checkable
};
        
static std::vector<int> KEY_STATE_PRESSED_OFF = { 
     (int)cdroid::internal::R::attr::state_pressed,//android.R.attr.state_pressed,
     (int)cdroid::internal::R::attr::state_checkable//android.R.attr.state_checkable
};
        
static std::vector<int> KEY_STATE_NORMAL = {
};
        
static std::vector<int> KEY_STATE_PRESSED = {
     (int)cdroid::internal::R::attr::state_pressed//android.R.attr.state_pressed
};

std::vector<int>Keyboard::Key::getCurrentDrawableState()const{
    if (on) {
        return pressed?KEY_STATE_PRESSED_ON:KEY_STATE_NORMAL_ON;
    } else {
        if (sticky) {
            return pressed?KEY_STATE_PRESSED_OFF:KEY_STATE_NORMAL_OFF;
        } else {
            if (pressed) {
                return KEY_STATE_PRESSED;
            }
        }
    }
    return KEY_STATE_NORMAL;
}

Keyboard::Row::Row(Context*ctx,Keyboard*p,XmlPullParser&parseer,const AttributeSet&attrs){
    parent =p;
    auto a = ctx->obtainStyledAttributes(attrs, R::styleable::Keyboard);
    defaultWidth = getDimensionOrFraction(*a, R::styleable::Keyboard_keyWidth,
            parent->mDisplayWidth, parent->mDefaultWidth);
    defaultHeight= getDimensionOrFraction(*a, R::styleable::Keyboard_keyHeight,
            parent->mDisplayHeight, parent->mDefaultHeight);
    defaultHorizontalGap = getDimensionOrFraction(*a, R::styleable::Keyboard_horizontalGap,
            parent->mDisplayWidth, parent->mDefaultHorizontalGap);
    verticalGap  = getDimensionOrFraction(*a, R::styleable::Keyboard_verticalGap,
            parent->mDisplayHeight, parent->mDefaultVerticalGap);

    auto ra = ctx->obtainStyledAttributes(attrs, R::styleable::Keyboard_Row);
    // aapt2 compiles the rowEdgeFlags flags (top=4/bottom=8) to ints.
    rowEdgeFlags = ra->getInt(R::styleable::Keyboard_Row_rowEdgeFlags, 0);
    mode = ra->getResourceId(R::styleable::Keyboard_Row_keyboardMode, 0);
}

/* AOSP-faithful Row(Keyboard) ctor: only records the parent. The mini-keyboard
 * constructor sets defaultWidth/Height/etc. explicitly afterwards. */
Keyboard::Row::Row(Keyboard*parent){
    this->parent = parent;
}

/* No-XML base ctor: zero everything; createMiniKeyboard populates the keys. */
Keyboard::Keyboard(){
    mDisplayWidth = mDisplayHeight = 0;
    mShifted = false;
    mDefaultHorizontalGap = 0;
    mDefaultVerticalGap = 0;
    mDefaultWidth = 0;
    mDefaultHeight = 0;
    mKeyboardMode = 0;
    mTotalWidth = mTotalHeight = 0;
    mCellWidth = mCellHeight = 0;
    mProximityThreshold = 0;
    keyGap = rowGap = 0;
}

/* Build a one-row mini-keyboard with explicit key sizing (each key
 * keyWidth x keyHeight), so the popup can match the main keyboard's actual
 * geometry (which CDROID resizes) rather than display-metric %p. */
Keyboard* Keyboard::createMiniKeyboard(Context* context,const std::string& characters,int keyWidth,int keyHeight,int columns){
    Keyboard* k = new Keyboard();
    const DisplayMetrics& dm = context->getDisplayMetrics();
    k->mDisplayWidth  = dm.widthPixels;
    k->mDisplayHeight = dm.heightPixels;
    k->mDefaultWidth  = keyWidth;
    k->mDefaultHeight = keyHeight;
    /* computeNearestNeighbors (a Keyboard method) uses Keyboard::mProximityThreshold,
     * which the no-XML ctor leaves 0 -- without this the grid ends up empty and no
     * mini key is ever hit. Match the XML-load formula at line 545. */
    k->mProximityThreshold = (int)(keyWidth * 0.6f);
    k->mProximityThreshold *= k->mProximityThreshold;
    /* Each visual row is its own Row object, so Keyboard::resize() lays each
     * row out independently (keys start at x=0) instead of treating all keys
     * as one long row and producing a staircase. */
    auto newRow = [&]() -> Row* {
        Row* r = new Row(k);
        r->defaultWidth  = keyWidth;
        r->defaultHeight = keyHeight;
        r->defaultHorizontalGap = 0;
        r->verticalGap  = 0;
        r->rowEdgeFlags = 0;
        r->mode = 0;
        k->rows.push_back(r);
        return r;
    };
    Row* row = newRow();
    const std::wstring chars = TextUtils::utf8tounicode(characters);
    /* Wrap into a compact multi-row grid (columns per row, like AOSP's mini
     * Keyboard ctor) so 5-7 accents form 2-3 rows instead of one wide strip. */
    const int maxColumns = (columns > 0) ? columns : INT_MAX;
    int x = 0, y = 0, column = 0;
    for(wchar_t ch : chars){
        if(column >= maxColumns){
            x = 0;
            y += keyHeight;
            column = 0;
            row = newRow(); // start a fresh Row for the next visual row
        }
        Key* key = new Key(row);
        key->x = x; key->y = y;
        key->width = keyWidth; key->height = keyHeight; key->gap = 0;
        key->codes.push_back((int)ch);
        key->label = TextUtils::unicode2utf8(std::wstring(1,ch));
        key->edgeFlags = 0;
        k->mKeys.push_back(key);
        row->mKeys.push_back(key);
        x += keyWidth;
        column++;
        if(x > k->mTotalWidth) k->mTotalWidth = x;
    }
    k->mTotalHeight = y + keyHeight;
    LOGV("mini keyboard %d keys (%dx%d)",(int)k->mKeys.size(),k->mTotalWidth,k->mTotalHeight);
    return k;
}

/* AOSP Keyboard(Context, int xmlLayoutResId, int modeId): load from XML using
 * the display dimensions. */
Keyboard::Keyboard(Context* context,const std::string& xmlLayoutResId,int modeId)
    : Keyboard(context, xmlLayoutResId,
               context->getDisplayMetrics().widthPixels,
               context->getDisplayMetrics().heightPixels, modeId){
}

/* AOSP Keyboard(Context, int layoutTemplateResId, CharSequence characters,
 * int columns, int horizontalPadding): build a mini-keyboard (one row of the
 * popup characters), sizing keys from the template's defaults. */
Keyboard::Keyboard(Context* context,const std::string& layoutTemplateResId,
                   const std::string& characters,int columns,int horizontalPadding)
    : Keyboard(context, layoutTemplateResId, 0){
    int x = 0, y = 0, column = 0;
    mTotalWidth = 0;
    Row* row = new Row(this);
    row->defaultHeight = mDefaultHeight;
    row->defaultWidth = mDefaultWidth;
    row->defaultHorizontalGap = mDefaultHorizontalGap;
    row->verticalGap = mDefaultVerticalGap;
    row->rowEdgeFlags = EDGE_TOP | EDGE_BOTTOM;
    const int maxColumns = columns == -1 ? INT_MAX : columns;
    const std::wstring chars = TextUtils::utf8tounicode(characters);
    for(wchar_t c : chars){
        if(column >= maxColumns || x + mDefaultWidth + horizontalPadding > mDisplayWidth){
            x = 0;
            y += mDefaultVerticalGap + mDefaultHeight;
            column = 0;
        }
        Key* key = new Key(row);
        key->x = x;
        key->y = y;
        key->label = TextUtils::unicode2utf8(std::wstring(1,c));
        key->codes.clear();
        key->codes.push_back((int)c);
        column++;
        x += key->width + key->gap;
        mKeys.push_back(key);
        row->mKeys.push_back(key);
        if(x > mTotalWidth) mTotalWidth = x;
    }
    mTotalHeight = y + mDefaultHeight;
    rows.push_back(row);
    LOGV("mini keyboard %d keys (%dx%d)",(int)mKeys.size(),mTotalWidth,mTotalHeight);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Keyboard::Keyboard(Context*context,const std::string& xmlLayoutResId,int width,int height,int modeId){
    const DisplayMetrics& dm = context->getDisplayMetrics();
    mDisplayWidth = width;//dm.widthPixels;
    mDisplayHeight= height;//dm.heightPixels;
    mTotalWidth = 0; // loadKeyboard only assigns when x > mTotalWidth
    mShifted =false;
    mDefaultHorizontalGap = 0;
    mDefaultWidth = mDisplayWidth / 10;
    mDefaultVerticalGap = 0;
    mDefaultHeight= mDefaultWidth;
    mKeyboardMode = modeId;
    XmlPullParser parser(context,xmlLayoutResId);
    loadKeyboard(context,parser);
}

Keyboard::~Keyboard(){
    for(auto k:mKeys)
       delete k;
    mKeys.clear(); 
}

void Keyboard::resize(int newWidth,int newHeight){
    int numRows = rows.size();
    for (int rowIndex = 0; rowIndex < numRows; ++rowIndex) {
        Row* row = rows.at(rowIndex);
        int numKeys = row->mKeys.size();
        int totalGap = 0;
        int totalWidth = 0;
        for (int keyIndex = 0; keyIndex < numKeys; ++keyIndex) {
            Key* key = row->mKeys.at(keyIndex);
            if (keyIndex > 0) {
                totalGap += key->gap;
            }
            totalWidth += key->width;
        }
        if (totalGap + totalWidth > newWidth) {
            int x = 0;
            float scaleFactor = (float)(newWidth - totalGap) / totalWidth;
            for (int keyIndex = 0; keyIndex < numKeys; ++keyIndex) {
                Key* key = row->mKeys.at(keyIndex);
                key->width *= scaleFactor;
                key->x = x+key->gap;
                x += key->width + key->gap;
            }
        }
    }
    mTotalWidth = newWidth;
}

std::vector<Keyboard::Key*>& Keyboard::getKeys() {
    return mKeys;
}

std::vector<Keyboard::Key*>& Keyboard::getModifierKeys() {
    return mModifierKeys;
}

int Keyboard::getHorizontalGap()const{
    return mDefaultHorizontalGap;
}

void Keyboard::setHorizontalGap(int gap) {
    mDefaultHorizontalGap = gap;
}

int Keyboard::getVerticalGap()const{
    return mDefaultVerticalGap;
}

void Keyboard::setVerticalGap(int gap) {
    mDefaultVerticalGap = gap;
}

int Keyboard::getKeyHeight()const{
    return mDefaultHeight;
}

void Keyboard::setKeyHeight(int height) {
    mDefaultHeight = height;
}

int Keyboard::getKeyWidth()const {
    return mDefaultWidth;
}

void Keyboard::setKeyWidth(int width) {
    mDefaultWidth = width;
}

int Keyboard::getHeight()const{
    return mTotalHeight;
}

int Keyboard::getMinWidth()const{
    return mTotalWidth;
}
int Keyboard::getRows()const{
    return rows.size();
}
bool Keyboard::setShifted(bool shiftState) {
    for (Key* shiftKey : mShiftKeys) {
        if (shiftKey != nullptr) {
            shiftKey->on = shiftState;
        }
    }
    if (mShifted != shiftState) {
        mShifted = shiftState;
        return true;
    }
    return false;
}

bool Keyboard::isShifted()const{
    return mShifted;
}

std::vector<int>& Keyboard::getShiftKeyIndices() {
    return mShiftKeyIndices;
}

int Keyboard::getShiftKeyIndex()const{
    return mShiftKeyIndices[0];
}

void Keyboard::computeNearestNeighbors() {
    // Round-up so we don't have any pixels outside the grid
    mCellWidth = (getMinWidth()+ GRID_WIDTH - 1) / GRID_WIDTH;
    mCellHeight= (getHeight()  + GRID_HEIGHT- 1) / GRID_HEIGHT;
    mGridNeighbors.resize(GRID_SIZE);
    std::vector<int> indices(mKeys.size());
    const int gridWidth  = GRID_WIDTH * mCellWidth;
    const int gridHeight = GRID_HEIGHT * mCellHeight;
    for (int x = 0; x < gridWidth; x += mCellWidth) {
        for (int y = 0; y < gridHeight; y += mCellHeight) {
            int count = 0;
            for (int i = 0; i < mKeys.size(); i++) {
                Key* key = mKeys.at(i);
                if ( (key->squaredDistanceFrom(x, y) < mProximityThreshold) ||
                     (key->squaredDistanceFrom(x + mCellWidth - 1, y) < mProximityThreshold) ||
                     (key->squaredDistanceFrom(x + mCellWidth - 1, y + mCellHeight - 1) < mProximityThreshold) ||
                     (key->squaredDistanceFrom(x, y + mCellHeight - 1) < mProximityThreshold) ) {
                    indices[count++] = i;
                }
            }
            const int idx=(y / mCellHeight) * GRID_WIDTH + (x / mCellWidth);
            mGridNeighbors[idx] = std::vector<int>(indices.begin(),indices.begin()+count);
            LOGV("Key[%d] has %d neighbors cellsize=%dx%d",idx,mGridNeighbors[idx].size(),mCellWidth,mCellHeight);
        }
    }
}

std::vector<int> Keyboard::getNearestKeys(int x, int y){
    if (mGridNeighbors.size() ==0) computeNearestNeighbors();
    if (x >= 0 && x < getMinWidth() && y >= 0 && y < getHeight()) {
        const int index = (y / mCellHeight) * GRID_WIDTH + (x / mCellWidth);
        if (index < GRID_SIZE) {
            return mGridNeighbors[index];
        }
    }
    return std::vector<int>();
}

Keyboard::Row* Keyboard::createRowFromXml(Context*context,XmlPullParser& parser,const AttributeSet&atts) {
    return new Row(context,this, parser,atts);
}

Keyboard::Key* Keyboard::createKeyFromXml(Context*context,Row* parent, int x, int y,XmlPullParser& parser,const AttributeSet&atts) {
    return new Key(context, parent, x, y, parser,atts);
}

void Keyboard::loadKeyboard(Context*context, XmlPullParser& parser){
    bool inKey = false;
    bool inRow = false;
    bool leftMostKey = false;
    int row = 0,eventType=0;
    int x=0 , y = 0;
    Key* key = nullptr;
    Row* currentRow = nullptr;
    bool skipRow = false;
    const AttributeSet& attrs = parser;
    while ((eventType = parser.next()) != XmlPullParser::END_DOCUMENT) {
        if (eventType == XmlPullParser::START_TAG) {
            std::string tag = parser.getName();
            if (tag.compare(TAG_ROW)==0) {
                inRow = true;
                x = 0;
                currentRow = createRowFromXml(context,parser,attrs);
                rows.push_back(currentRow);
                skipRow = currentRow->mode != 0 && currentRow->mode != mKeyboardMode;
                if (skipRow) {
                    skipToEndOfRow(parser);
                    inRow = false;
                }
           } else if (tag.compare(TAG_KEY)==0) {
                inKey = true;
                key = createKeyFromXml(context,currentRow, x, y, parser,attrs);
                mKeys.push_back(key);
                if (key->codes[0] == KEYCODE_SHIFT) {
                    // Find available shift key slot and put this shift key in it
                    for (int i = 0; i < mShiftKeys.size(); i++) {
                        if (mShiftKeys[i] == nullptr) {
                            mShiftKeys[i] = key;
                            mShiftKeyIndices[i] = mKeys.size()-1;
                            break;
                        }
                    }
                    mModifierKeys.push_back(key);
                } else if (key->codes[0] == KEYCODE_ALT) {
                    mModifierKeys.push_back(key);
                }
                currentRow->mKeys.push_back(key);
            } else if (tag.compare(TAG_KEYBOARD)==0) {
                parseKeyboardAttributes(context,parser,attrs);
            }
        } else if (eventType == XmlPullParser::END_TAG) {
            if (inKey) {
                inKey = false;
                x += key->gap + key->width;
                if (x > mTotalWidth) {
                    mTotalWidth = x;
                }
            } else if (inRow) {
                inRow = false;
                y += currentRow->verticalGap;
                y += currentRow->defaultHeight;
                row++;
            } else {
                // TODO: error or extend?
            }
        }
    }
    mTotalHeight = y - mDefaultVerticalGap;
}

void Keyboard::skipToEndOfRow(XmlPullParser&parser){
    int eventType;
    while ((eventType = parser.next()) != XmlPullParser::END_DOCUMENT) {
        if ((eventType == XmlPullParser::END_TAG) && (parser.getName().compare(TAG_ROW)==0)) {
            break;
        }
    }
}

void Keyboard::parseKeyboardAttributes(Context*context, XmlPullParser& parser,const AttributeSet&atts) {

    auto a = context->obtainStyledAttributes(atts, R::styleable::Keyboard);
    mDefaultWidth  = getDimensionOrFraction(*a, R::styleable::Keyboard_keyWidth,      mDisplayWidth,  mDisplayWidth / 10);
    mDefaultHeight = getDimensionOrFraction(*a, R::styleable::Keyboard_keyHeight,     mDisplayHeight, 50);
    mDefaultHorizontalGap = getDimensionOrFraction(*a, R::styleable::Keyboard_horizontalGap, mDisplayWidth, 0);
    mDefaultVerticalGap   = getDimensionOrFraction(*a, R::styleable::Keyboard_verticalGap,   mDisplayHeight, 0);
    mProximityThreshold = (int) (mDefaultWidth * 0.6f);//SEARCH_DISTANCE);
    mProximityThreshold*= mProximityThreshold; // Square it for comparison
}

}//end namespace cdroid
