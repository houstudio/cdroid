#include <drawables/colorstatelist.h>
#include <core/color.h>
#include <core/app.h>
#include <drawables/stateset.h>
#include <attributeset.h>
#include <expat.h>
#include <fstream>
#include <string.h>
#include <map>
#include <exception>
#include <cdtypes.h>
#include <cdlog.h>

namespace cdroid{

std::vector<std::vector<int>> ColorStateList::EMPTY={{}};

ColorStateList::ColorStateList(){
    mChangingConfigurations=0;
}

ColorStateList::ColorStateList(int color)
    :ColorStateList(EMPTY,{color}){
}

int ColorStateList::addStateColor(cdroid::Context*ctx,const AttributeSet&atts){
    std::vector<int>states;
    const int color = atts.getColor("color",0);
    StateSet::parseState(states,atts);
    return addStateColor(states,color);
}

ColorStateList::ColorStateList(const ColorStateList&other)
	:ColorStateList(other.mStateSpecs,other.mColors){
    mIsOpaque = other.mIsOpaque;
}

ColorStateList::ColorStateList(const std::vector<std::vector<int>>&states,const std::vector<int>&colors){
    mStateSpecs = states;
    mColors = colors;
    mChangingConfigurations = 0;
    onColorsChanged();
}

ColorStateList::~ColorStateList(){
    LOGV("%p",this);
}

void ColorStateList::dump()const{
    std::ostringstream oss;
    for(int i=0;i<mColors.size();i++){
	const std::vector<int>&state=mStateSpecs[i];
        oss<<"[";
        for(auto s=state.begin();s!=state.end();s++){
	    oss<<*s;
	    if(s<state.end()-1)oss<<",";
	}
        oss<<"]="<<std::hex<<(unsigned int)mColors.at(i)<<" ";
    }
    LOG(DEBUG)<<oss.str();
}

int ColorStateList::getChangingConfigurations()const{
    return mChangingConfigurations;
}

int ColorStateList::addStateColor(const std::vector<int>&stateSet,int color){
    mStateSpecs.push_back(stateSet);
    mColors.push_back(color);
    return mColors.size()-1;
}

int ColorStateList::modulateColorAlpha(int baseColor, float alphaMod)const{
    if (alphaMod == 1.0f)  return baseColor;

    const int baseAlpha = Color::alpha(baseColor);
    const int alpha = (int)(baseAlpha * alphaMod + 0.5f)&0xFF;
    //MathUtils.constrain((int) (baseAlpha * alphaMod + 0.5f), 0, 255);
    return (baseColor & 0xFFFFFF) | ((unsigned int)alpha << 24);    
}

ColorStateList*ColorStateList::withAlpha(int alpha)const{
    std::vector<int>colors = mColors;
    for(int i = 0 ; i < colors.size();i++)
        colors[i] = (colors[i] & 0x00FFFFFF) | ( alpha & 0xFF000000 );
    return new ColorStateList(mStateSpecs,colors);
}

ColorStateList&ColorStateList::operator=(const ColorStateList&other){
    mStateSpecs = other.mStateSpecs;
    mColors = other.mColors;
    mIsOpaque = other.mIsOpaque;
    onColorsChanged();
    return *this;
}

bool ColorStateList::operator!=(const ColorStateList&other)const{
    return (mColors != other.mColors) || (mStateSpecs != other.mStateSpecs);
}

bool ColorStateList::operator==(const ColorStateList&other)const{
    return (mColors == other.mColors) && (mStateSpecs != other.mStateSpecs);
}

bool ColorStateList::isOpaque()const{
    return mIsOpaque;
}

bool ColorStateList::isStateful()const{
    return  mStateSpecs.size()&&mStateSpecs[0].size();
}

bool ColorStateList::hasFocusStateSpecified()const{
    return StateSet::containsAttribute(mStateSpecs,StateSet::FOCUSED);
}

int ColorStateList::getDefaultColor()const{
    return mDefaultColor;
}

const std::vector<std::vector<int>>& ColorStateList::getStates()const{
    return mStateSpecs;
}

void ColorStateList::onColorsChanged(){
    int defaultColor = Color::RED;//DEFAULT_COLOR;
    bool isOpaque = true;
    const int N=(int)mStateSpecs.size();
    if ( N> 0) {
        defaultColor = mColors[0];

        for (int i = N - 1; i > 0; i--) {
            if (mStateSpecs[i].size() == 0) {
                defaultColor = mColors[i];
                break;
            }
        }

        for (int i = 0; i < N; i++) {
            if (Color::alpha(mColors[i]) != 0xFF) {
                isOpaque = false;
                break;
            }
        }
    }else if (mColors.size()){
        defaultColor = mColors[0];
    }
    mDefaultColor = defaultColor;
    mIsOpaque = isOpaque;	
}

int ColorStateList::getColorForState(const std::vector<int>&stateSet, int defaultColor)const{
    const int setLength = (int)mStateSpecs.size();
    for (int i = 0; i < setLength; i++) {
        if (StateSet::stateSetMatches(mStateSpecs[i], stateSet)) {
            return mColors[i];
        }
    }
    return defaultColor;
}

ColorStateList*ColorStateList::valueOf(int color){
    char buf[32];
    sprintf(buf,"#%06x",color);
    return App::getInstance().getColorStateList(buf); 
}

const std::vector<int>& ColorStateList::getColors()const{
    return mColors;
}

bool ColorStateList::hasState(int state)const{
    for(auto ss:mStateSpecs){
        for(auto s:ss){
            if(s==state||s==-state)return true;
        }
    }
    return false;
}
struct ColorsParserData{
    ColorStateList* colors;
    Context*ctx;
    std::string package;
};

static void startElement(void *userData, const XML_Char *name, const XML_Char **props){
    ColorsParserData*cd = static_cast<ColorsParserData*>(userData);
    AttributeSet atts(cd->ctx,cd->package);
    atts.set(props);
    if(strcmp(name,"item") == 0){
        std::vector<int>states;
        const int color = atts.getColor("color",0);
        StateSet::parseState(states,atts);
        if(cd->colors==nullptr)
           cd->colors = new ColorStateList();
        cd->colors->addStateColor(states,color);
    }
}

ColorStateList*ColorStateList::fromStream(Context*ctx,std::istream&stream,const std::string&resname,const std::string&package){
    int done = 0;
    char buf[256];
    XML_Parser parser = XML_ParserCreateNS(nullptr,' ');
    ColorsParserData cd;
    cd.ctx = ctx;
    cd.colors = nullptr;
    cd.package= package;
    XML_SetUserData(parser,&cd);
    XML_SetElementHandler(parser, startElement, nullptr/*endElement*/);
    do {
       stream.read(buf,sizeof(buf));
       int rdlen = (int)stream.gcount();
       done = (rdlen==0);
       if (XML_Parse(parser, buf,rdlen,done) == XML_STATUS_ERROR) {
           const char*es = XML_ErrorString(XML_GetErrorCode(parser));
           LOGE("%s at line %ld",es, XML_GetCurrentLineNumber(parser));
           XML_ParserFree(parser);
           return nullptr;
       }
    } while(!done);
    XML_ParserFree(parser);
    return cd.colors;
}

ColorStateList*ColorStateList::inflate(Context*ctx,const std::string&resname){
    ColorStateList*cs = nullptr;
    const std::string fullresid = resname;
    const size_t slashpos = fullresid.find("/");
    if( (fullresid[0]=='#') || (slashpos==std::string::npos) ) {/*digital colors and html colors*/
        const int color = Color::parseColor(fullresid);/*here maybe caused std::invalid_argument exception*/
        return new ColorStateList(color);
    }
    if(ctx == nullptr){
        std::ifstream fs(resname);
        cs = fromStream(ctx,fs,resname);
    }else{
        std::string package;
        std::unique_ptr<std::istream>is = ctx->getInputStream(resname,&package);
        cs = fromStream(ctx,*is,resname,package);
    }
    return cs;
}

}
