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
#include <vector>
#include <fstream>

namespace cdroid{

static std::unordered_map<std::string,int>edgeFlagKVS={
   {"left"  ,(int)Keyboard::EDGE_LEFT},
   {"right" ,(int)Keyboard::EDGE_RIGHT},
   {"top"   ,(int)Keyboard::EDGE_TOP},
   {"bottom",(int)Keyboard::EDGE_BOTTOM}
};

int getDimensionOrFraction(const AttributeSet&attrs,const std::string&key,int base,int def){
    const std::string value=attrs.getAttributeValue(key);
    if(value.find("%")!=std::string::npos){
	LOGD("%d %s[%.f]=%.2f",base,value.c_str(),std::stof(value),base*std::stof(value)/100);
        return base*std::stof(value)/100;
    }else if(value.find("px")!=std::string::npos){
        return std::stoi(value);
    }
    return def;
}

Keyboard::Key::Key(Row*parent,int x,int y,XmlPullParser&parser,const AttributeSet&attrs)
  :Keyboard::Key(parent){
    this->x = x;
    this->y = y;
    Keyboard::Row*row=(Keyboard::Row*)parent;
    Keyboard*keyboard = row->parent;
    width = getDimensionOrFraction(attrs,"keyWidth" , keyboard->mDisplayWidth,  row->defaultWidth );
    height= getDimensionOrFraction(attrs,"keyHeight", keyboard->mDisplayHeight, row->defaultHeight);
    gap   = getDimensionOrFraction(attrs,"horizontalGap", keyboard->mDisplayWidth, row->defaultHorizontalGap);
    edgeFlags =row->rowEdgeFlags | attrs.getInt("keyEdgeFlags",edgeFlagKVS,0);
    this->x += gap;
    const std::string resicon=attrs.getString("keyIcon");
    icon  = resicon.empty()?nullptr:attrs.getContext()->getDrawable(resicon);
    label = attrs.getString("keyLabel");
    text  = attrs.getString("keyOutputText");
    parseCSV(attrs.getString("codes"),codes);
    repeatable= attrs.getBoolean("isRepeatable",false);
    sticky    = attrs.getBoolean("isSticky",false);
    modifier  = attrs.getBoolean("isModifier",false);
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
     StateSet::CHECKABLE,//StateSet::android.R.attr.state_checkable, 
     StateSet::CHECKED   //android.R.attr.state_checked
};
        
static std::vector<int> KEY_STATE_PRESSED_ON = { 
     StateSet::PRESSED  ,// android.R.attr.state_pressed, 
     StateSet::CHECKABLE,// android.R.attr.state_checkable, 
     StateSet::CHECKED   // android.R.attr.state_checked 
};
        
static std::vector<int> KEY_STATE_NORMAL_OFF = { 
     StateSet::CHECKABLE//android.R.attr.state_checkable 
};
        
static std::vector<int> KEY_STATE_PRESSED_OFF = { 
     StateSet::PRESSED,//android.R.attr.state_pressed, 
     StateSet::CHECKABLE//android.R.attr.state_checkable 
};
        
static std::vector<int> KEY_STATE_NORMAL = {
};
        
static std::vector<int> KEY_STATE_PRESSED = {
     StateSet::PRESSED//android.R.attr.state_pressed
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
    defaultWidth = getDimensionOrFraction(attrs,"keyWidth",   parent->mDisplayWidth, parent->mDefaultWidth);
    defaultHeight= getDimensionOrFraction(attrs,"keyHeight", parent->mDisplayHeight, parent->mDefaultHeight);
    defaultHorizontalGap = getDimensionOrFraction(attrs,"horizontalGap", parent->mDisplayWidth, parent->mDefaultHorizontalGap);
    verticalGap  = getDimensionOrFraction(attrs,"verticalGap", parent->mDisplayHeight, parent->mDefaultVerticalGap);
    rowEdgeFlags = attrs.getInt("rowEdgeFlags",edgeFlagKVS,0);
    mode = attrs.getInt("keyboardMode",0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Keyboard::Keyboard(Context*context,const std::string& xmlLayoutResId,int width,int height,int modeId){
    const DisplayMetrics& dm = context->getDisplayMetrics();
    mDisplayWidth = width;//dm.widthPixels;
    mDisplayHeight= height;//dm.heightPixels;
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
    LOGD("%dx%d",newWidth,newHeight);
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
    LOGD("resizeTO(%dx%d)",newWidth,newHeight);
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

Keyboard::Row* Keyboard::createRowFromXml(XmlPullParser& parser,const AttributeSet&atts) {
    return new Row(atts.getContext(),this, parser,atts);
}

Keyboard::Key* Keyboard::createKeyFromXml(Row* parent, int x, int y,XmlPullParser& parser,const AttributeSet&atts) {
    return new Key(parent, x, y, parser,atts);
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
                currentRow = createRowFromXml(parser,attrs);
                rows.push_back(currentRow);
                skipRow = currentRow->mode != 0 && currentRow->mode != mKeyboardMode;
                if (skipRow) {
                    skipToEndOfRow(parser);
                    inRow = false;
                }
           } else if (tag.compare(TAG_KEY)==0) {
                inKey = true;
                key = createKeyFromXml(currentRow, x, y, parser,attrs);
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
                parseKeyboardAttributes(parser,attrs);
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

void Keyboard::parseKeyboardAttributes(XmlPullParser& parser,const AttributeSet&atts) {

    mDefaultWidth = getDimensionOrFraction(atts,"keyWidth", mDisplayWidth, mDisplayWidth / 10);
    mDefaultHeight = getDimensionOrFraction(atts,"keyHeight", mDisplayHeight, 50);
    mDefaultHorizontalGap = getDimensionOrFraction(atts,"horizontalGap", mDisplayWidth, 0);
    mDefaultVerticalGap = getDimensionOrFraction(atts,"verticalGap", mDisplayHeight, 0);
    mProximityThreshold = (int) (mDefaultWidth * 0.6f);//SEARCH_DISTANCE);
    mProximityThreshold*= mProximityThreshold; // Square it for comparison
}

}//end namespace cdroid
