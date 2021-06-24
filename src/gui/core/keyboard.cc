#include <cdtypes.h>
#include <cdlog.h>
#include <keyboard.h>
#include <json/json.h>
#include <vector>
#include <fstream>


namespace cdroid{

Keyboard::Keyboard(int w,int h){
    keyboardWidth=w;
    keyboardHeight=h;
    totalWidth=0;
    totalHeight=0;
    keyGap=0;
    rowGap=0;
    keyWidth=0;
    keyHeight=0;
    marginTB=.05f;
    marginLR=.02f;
}

Keyboard::Key::Key(){
    memset(codes,0,sizeof(codes));
    isSticky=isModifier=0;
    x=y=gap=0;
    width=height=0;
    on=false;
    pressed=false;
}

int Keyboard::getRows()const{
    return keyRows.size();
}

Keyboard::KeyRow& Keyboard::getKeyRow(int row){
    return keyRows[row];
}

void Keyboard::setShifted(int code,bool state){
    for(int i=0;i<keyRows.size();i++){
        KeyRow& r=keyRows[i];
        for(int j=0;j<r.size();j++){
            Key&k=r[j];
            if(k.isSticky&&k.codes[0]==code)
               k.on=state;
        }
    } 
}

bool  Keyboard::getShifted(int code)const{
    for(int i=0;i<keyRows.size();i++){
        const KeyRow& r=keyRows[i];
        for(int j=0;j<r.size();j++){
            const Key&k=r[j];
            if(k.isSticky&&k.codes[0]==code)
               return k.on;
        }
    } 
    return false;
}

static int splitString(const std::string& s,wchar_t*codes,int maxsize){
    size_t last = 0;
    size_t index=s.find_first_of(",",last);
    wchar_t*pc=codes;
    while (index!=std::string::npos && (pc-codes<maxsize)){
        std::string sub=s.substr(last,index-last);
        *pc++=std::stoi(sub);
        last=index+1;
        index=s.find_first_of(",",last);
    }
    if ( (index-last>0) && (pc-codes<maxsize)){
       std::string sub=s.substr(last,index-last);
       *pc++=std::stoi(sub);
    }
    return pc-codes;
}

static int getMaxKeys(const Json::Value&rows){
   int keys=0;
   for(int i=0;i<rows.size();i++)
      keys=std::max(keys,(int)rows[i].size());
   return keys;
}

static int getMaxKeys(std::vector<Keyboard::KeyRow>&rows){
   int keys=0;
   for(int i=0;i<rows.size();i++)
     keys=std::max(keys,(int)rows[i].size());
   return keys;
}

int getDimensionOrFraction(const std::string&value,int base,int def){
    if(value.find("%p")!=std::string::npos){
        return base*std::stof(value)/100;
    }else if(value.find("px")!=std::string::npos){
        return std::stoi(value);
    }
    return def;
}

std::shared_ptr<Keyboard> Keyboard::loadFrom(const std::string&resname,int w,int h){
    std::unique_ptr<std::istream> in;
    in.reset(new std::ifstream(resname));
    if(!in->good()){
       LOGD("file:%s notfound",resname.c_str());
       in=App::getInstance().getInputStream(resname);
       LOGD("resousce %s =%d",resname.c_str(),in.get()?in->good():-1);
    }
    if((in==nullptr)||!in->good())
        return nullptr;
    std::shared_ptr<Keyboard> kbd=std::make_shared<Keyboard>(w,h);
    kbd->loadLayout(*in);
    return kbd;
}

int Keyboard::loadLayout(std::istream&in){
    Json::Value d;
    Json::CharReaderBuilder builder;
    Json::String errs;

    bool rc=Json::parseFromStream(builder,in,&d,&errs);
    LOGE_IF(!rc,"Error %s",errs.c_str());

    if(d.isMember("rowgap"))   rowGap=getDimensionOrFraction(d["rowgap"].asString(),keyboardHeight,1);
    if(d.isMember("keygap"))   keyGap=getDimensionOrFraction(d["keygap"].asString(),keyboardWidth,1);
    if(d.isMember("keywidth")) keyWidth=getDimensionOrFraction(d["keywidth"].asString(),keyboardWidth,0);
    if(d.isMember("keyheight"))keyHeight=getDimensionOrFraction(d["keyheight"].asString(),keyboardHeight,0);
    if(!d.isMember("rows"))return 0;

    int y=rowGap;
    Json::Value &jrows=d["rows"];
    int maxKeys=getMaxKeys(jrows);
    if(keyWidth==0&&maxKeys)
       keyWidth=(keyboardWidth-keyGap*(maxKeys+1))/maxKeys;
    if(keyHeight==0&&jrows.size())
       keyHeight=(keyboardHeight-rowGap*(jrows.size()+1))/jrows.size();
    keyRows.clear();
    LOGD("keyboardsize=%dx%d rows:%d gaps=%d,%d keySize:%dx%d",keyboardWidth,keyboardHeight,jrows.size(),rowGap,keyGap,keyWidth,keyHeight);
    
    for(int i=0;i<jrows.size();i++){//loop for keyboard row
        int x=0;
        KeyRow row; 
        const Json::Value &jrow=jrows[i];
        if(!jrow.isArray())continue;

        for(int j=0;j<jrow.size();j++){
            const Json::Value &key=jrow[j];
            Keyboard::Key k;
            k.y=y;
            k.gap=keyGap;
            k.width=keyWidth;
            k.height=keyHeight;
            k.label=key["label"].asString();
            k.icon=std::string();
            splitString(key["codes"].asString(),k.codes,4);
            if(key.isMember("text"))   k.text=key["text"].asString();
            if(key.isMember("gap"))    k.gap=getDimensionOrFraction(key["gap"].asString(),keyboardWidth,keyGap);
            if(key.isMember("width"))  k.width=getDimensionOrFraction(key["width"].asString(),keyboardWidth,keyWidth);
            if(key.isMember("height")) k.height=getDimensionOrFraction(key["height"].asString(),keyboardHeight,keyHeight);
            if(key.isMember("icon"))   k.icon=key["icon"].asString();
            if(key.isMember("action")) k.action=key["action"].asString();
            k.isSticky  =key.isMember("sticky") && key["sticky"].asBool();
            k.isModifier=key.isMember("modifier")&&key["modifier"].asBool();
            k.x=x+k.gap;
            row.push_back(k);
            x+=k.width+k.gap;
            totalWidth=std::max(x,totalWidth);
            LOGV("row[%d].addKey(%s) modifier=%d sticky=%d code=%x,%x,%x,%x",i,k.label.c_str(),
                 k.isModifier,k.isSticky,k.codes[0],k.codes[1],k.codes[2],k.codes[3]);
        }
        y+=rowGap+keyHeight;
        totalHeight+=keyHeight;
        if(row.size()) keyRows.push_back(row);
   }
   return keyRows.size();
}

void Keyboard::setMargin(float tb,float lr){
    marginTB=tb;
    marginLR=lr;
}

void Keyboard::resize(int newWidth,int newHeight){
    int numRows =keyRows.size();
    const float scaleX=(float)newWidth/keyboardWidth;
    const float scaleY=(float)newHeight/keyboardHeight;
    int y=rowGap;
    LOGD("resize %d,%d-->%d,%d",keyboardWidth,keyboardHeight,newWidth,newHeight);
    const int maxKeys=getMaxKeys(keyRows);
    keyWidth=(keyboardWidth-keyGap*(maxKeys+1))/maxKeys;
    keyHeight=(keyboardHeight-rowGap*(keyRows.size()+1))/keyRows.size();
    keyHeight*=scaleY;
    for (int rowIndex = 0; rowIndex < numRows; ++rowIndex) {
        Keyboard::KeyRow&row = keyRows.at(rowIndex);
        int numKeys = row.size();
        int x=keyGap;
        for (int keyIndex = 0; keyIndex < numKeys; ++keyIndex) {
            Keyboard::Key& key = row.at(keyIndex);
            key.width *= scaleX;
            //if(keyIndex==0)
            key.x*= scaleX;
            key.y = y;
            key.height*=scaleY;
            x += (key.width + key.gap);
        }y+=keyHeight+rowGap;
    }
    totalWidth = newWidth;
}

}//end namespace cdroid
