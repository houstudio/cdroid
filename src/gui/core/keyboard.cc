#include <cdtypes.h>
#include <cdlog.h>
#include <textutils.h>
#include <keyboard.h>
#include <tokenizer.h>
#include <json/json.h>
#include <vector>
#include <fstream>
#include <expat.h>

namespace cdroid{

static std::map<const std::string,int>edgeFlagKVS={
   {"left"  ,(int)Keyboard::EDGE_LEFT},
   {"right" ,(int)Keyboard::EDGE_RIGHT},
   {"top"   ,(int)Keyboard::EDGE_TOP},
   {"bottom",(int)Keyboard::EDGE_BOTTOM}
};

int getDimensionOrFraction(const AttributeSet&attrs,const std::string&key,int base,int def){
    const std::string value=attrs.getAttributeValue(key);
    if(value.find("%p")!=std::string::npos){
        return base*std::stof(value)/100;
    }else if(value.find("px")!=std::string::npos){
        return std::stoi(value);
    }
    return def;
}

Keyboard::Key::Key(void*parent,int x,int y,Context*context,const AttributeSet&attrs)
  :Keyboard::Key(parent){
    this->x = x;
    this->y = y;
    Keyboard::Row*row=(Keyboard::Row*)parent;
    Keyboard*keyboard =row->parent;
    width = getDimensionOrFraction(attrs,"keyWidth" , keyboard->mDisplayWidth,  row->defaultWidth );
    height= getDimensionOrFraction(attrs,"keyHeight", keyboard->mDisplayHeight, row->defaultHeight);
    gap   = getDimensionOrFraction(attrs,"horizontalGap", keyboard->mDisplayWidth, row->defaultHorizontalGap);
    edgeFlags =row->rowEdgeFlags | attrs.getInt("keyEdgeFlags",edgeFlagKVS,0);
    this->x+=gap;
    const std::string resicon=attrs.getString("keyIcon");
    icon  = resicon.empty()?nullptr:context->getDrawable(resicon);
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
}

Keyboard::Key::Key(void*p){
    parent =p;
    Row*row=(Row*)parent;
    sticky =modifier=0;
    x=  y  = gap =0;
    width    = row->defaultWidth;
    height   = row->defaultHeight;
    edgeFlags= row->rowEdgeFlags;
    on = false;
    pressed=false;
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
    int ret=0;
    int idx=0;
    do{
        std::string s=token->nextToken(",");
        token->skipDelimiters(",");
        codes.push_back(strtoul(s.c_str(),nullptr,10));
    }while(!token->isEof());
    delete token;
    return codes.size();
}

bool Keyboard::Key::isInside(int x, int y) {
    bool leftEdge = (edgeFlags & EDGE_LEFT) > 0;
    bool rightEdge = (edgeFlags & EDGE_RIGHT) > 0;
    bool topEdge = (edgeFlags & EDGE_TOP) > 0;
    bool bottomEdge = (edgeFlags & EDGE_BOTTOM) > 0;
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
   int xDist = this->x + width / 2 - x;
   int yDist = this->y + height / 2 - y;
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

Keyboard::Row::Row(Keyboard*p,Context*ctx,const AttributeSet&attrs){
    parent =p;
    defaultWidth = getDimensionOrFraction(attrs,"keyWidth",   parent->mDisplayWidth, parent->mDefaultWidth);
    defaultHeight= getDimensionOrFraction(attrs,"keyHeight", parent->mDisplayHeight, parent->mDefaultHeight);
    defaultHorizontalGap = getDimensionOrFraction(attrs,"horizontalGap", parent->mDisplayWidth, parent->mDefaultHorizontalGap);
    verticalGap  = getDimensionOrFraction(attrs,"verticalGap", parent->mDisplayHeight, parent->mDefaultVerticalGap);
    rowEdgeFlags = attrs.getInt("rowEdgeFlags", EDGE_TOP|EDGE_BOTTOM);
    mode = attrs.getInt("keyboardMode",0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Keyboard::Keyboard(Context*context,const std::string& xmlLayoutResId,int width,int height,int modeId){
    mDisplayWidth = width;
    mDisplayHeight= height;
    mShifted =false;
    mDefaultHorizontalGap = 0;
    mDefaultWidth = mDisplayWidth / 10;
    mDefaultVerticalGap = 0;
    mDefaultHeight= mDefaultWidth;
    mKeyboardMode = modeId;
    loadKeyboard(context,xmlLayoutResId);
}

Keyboard::Keyboard(Context* context,const std::string& xmlLayoutResId, int modeId):Keyboard(context,xmlLayoutResId,0,0,modeId){
}

Keyboard::~Keyboard(){
    for(auto k:mKeys)
       delete k;
    mKeys.clear(); 
}

typedef struct{
    std::vector<Keyboard::Row*>*rows;
    std::vector<Keyboard::Key*>*keys;
    Context * context;
    Keyboard* keyboard;
    Keyboard::Row*row;
    Keyboard::Key*key;
    int x,y;
    int displayWidth,displayHeight;
    int keyboardMode;
    int minWidth;//min width for keyboard's key row
}KeyboardData;

static void startTag(void *userData, const XML_Char *name, const XML_Char **satts){
    KeyboardData*pd=(KeyboardData*)userData;
    AttributeSet atts(satts);
    Context*context=pd->context;
    Keyboard* keyboard=pd->keyboard;
    Keyboard::Row*row =pd->row;
    int sz;
    if(0==strcmp(name,"Keyboard")){
        sz= getDimensionOrFraction(atts,"horizontalGap",pd->displayWidth,keyboard->getHorizontalGap());
        keyboard->setHorizontalGap(sz);
        sz= getDimensionOrFraction(atts,"verticalGap",pd->displayHeight,keyboard->getVerticalGap());
        keyboard->setVerticalGap(sz);
        sz= getDimensionOrFraction(atts,"keyHeight",pd->displayHeight,0);
        keyboard->setKeyHeight(sz);
        pd->minWidth=pd->y=0;
    }else if(0==strcmp(name,"Row")){
        pd->row=row=new Keyboard::Row(keyboard,context,atts);
        row->rowEdgeFlags = atts.getInt("rowEdgeFlags",0);
        pd->x=0;
    }else if(0==strcmp(name,"Key")){
        if(row->mode==pd->keyboardMode){
            Keyboard::Key*key=new Keyboard::Key(row,pd->x,pd->y,context,atts);
            if(row->mode==pd->keyboardMode)
                row->mKeys.push_back(key);
            pd->key=key;
        }
    }
}

static void endTag(void *userData, const XML_Char *name){
    KeyboardData*pd=(KeyboardData*)userData;
    Context*context=pd->context;
    Keyboard* keyboard=pd->keyboard;
    Keyboard::Row*row =pd->row;
    if(0==strcmp(name,"Key")){
        if(row->mode==pd->keyboardMode){
            Keyboard::Key*key=pd->key;
            pd->keys->push_back(key);
            pd->x+=key->width+key->gap;
            if(key->codes[0]==Keyboard::KEYCODE_SHIFT||key->codes[0]==Keyboard::KEYCODE_ALT)
                keyboard->getModifierKeys().push_back(key);
        }
    }else if(0==strcmp(name,"Row")){
        pd->y+=row->defaultHeight+row->verticalGap;
        pd->minWidth=std::max(pd->x,pd->minWidth);
        pd->x=0;
        if(row->mode==pd->keyboardMode)
            pd->rows->push_back(pd->row);
        else delete pd->row;
    }
}

void Keyboard::loadKeyboard(Context*context,const std::string&resid){
    XML_Parser parser=XML_ParserCreateNS(NULL,':');
    KeyboardData pd={&rows,&mKeys,context,this,nullptr,0,0};
    ULONGLONG tstart=SystemClock::uptimeMillis();
    XML_SetUserData(parser,&pd);
    XML_SetElementHandler(parser, startTag, endTag);
    pd.displayWidth=mDisplayWidth;
    pd.displayHeight=mDisplayHeight;
    pd.keyboardMode=mKeyboardMode;
    int len = 0;
    std::unique_ptr<std::istream>stream=context->getInputStream(resid);
    if(stream){
        do{
            char buf[256];
            stream->read(buf,sizeof(buf));
            len=stream->gcount();
            if (XML_Parse(parser, buf,len,len==0) == XML_STATUS_ERROR) {
                const char*es=XML_ErrorString(XML_GetErrorCode(parser));
                LOGE("%s at line %ld",es, XML_GetCurrentLineNumber(parser));
                XML_ParserFree(parser);
                return ;
            }
        } while(len!=0);
    }
    XML_ParserFree(parser);
    mTotalHeight= pd.y-mDefaultVerticalGap;
    mTotalWidth = pd.minWidth;
    mProximityThreshold =mDefaultWidth*.6f;//SEARCH_DISTANCE;
    mProximityThreshold*=mProximityThreshold;
    LOGD("endof loadkeyboard %d rows %d keys parsed size=%dx%d",rows.size(),mKeys.size(),mTotalWidth,mTotalHeight);
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
    int indices[256] ;//= new int[mKeys.size()];
    const int gridWidth  = GRID_WIDTH * mCellWidth;
    const int gridHeight = GRID_HEIGHT * mCellHeight;
    for (int x = 0; x < gridWidth; x += mCellWidth) {
        for (int y = 0; y < gridHeight; y += mCellHeight) {
            int count = 0;
            for (int i = 0; i < mKeys.size(); i++) {
                Key* key = mKeys.at(i);
                if (key->squaredDistanceFrom(x, y) < mProximityThreshold ||
                    key->squaredDistanceFrom(x + mCellWidth - 1, y) < mProximityThreshold ||
                    key->squaredDistanceFrom(x + mCellWidth - 1, y + mCellHeight - 1) 
                        < mProximityThreshold ||
                        key->squaredDistanceFrom(x, y + mCellHeight - 1) < mProximityThreshold) {
                    indices[count++] = i;
                }
            }
            const int idx=(y / mCellHeight) * GRID_WIDTH + (x / mCellWidth);
            mGridNeighbors[idx] = std::vector<int>(indices,indices+count);
            //LOGV("Key[%d] has %d neighbors",idx,mGridNeighbors[idx].size());
        }
    }
}

std::vector<int> Keyboard::getNearestKeys(int x, int y){
    if (mGridNeighbors.size() ==0) computeNearestNeighbors();
    if (x >= 0 && x < getMinWidth() && y >= 0 && y < getHeight()) {
        int index = (y / mCellHeight) * GRID_WIDTH + (x / mCellWidth);
        if (index < GRID_SIZE) {
            return mGridNeighbors[index];
        }
    }
    return std::vector<int>();
}

}//end namespace cdroid
