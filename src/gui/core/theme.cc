#include<theme.h>
#include<expat.h>
#include<cdtypes.h>
#include<cdlog.h>
#include<iostream>
#include <iomanip>
#include <sstream>
#include <string.h>


namespace cdroid{
    static std::map<const std::string ,unsigned int>name2colors={
		{"aquamarine", 0x7FFFD4},    {"beige", 0xF5F5DC},		{"black", 0x000000},         {"blue", 0x0000FF},
		{"blueviolet",0x8A2BE2},     {"brown", 0xA52A2A},		{"cadetblue", 0x5F9EA0},     {"chartreuse", 0x7FFF00},
		{"chocolate", 0xD2691E},     {"coral", 0xFF7F50},		{"cornflowerblue", 0x6495ED},{"cyan", 0x00FFFF},
		{"firebrick", 0xB22222},     {"forestgreen", 0x228B22},	{"gold", 0xFFD700},          {"goldenrod", 0xDAA520},
		{"gray", 0xBEBEBE},          {"green", 0x00FF00},		{"greenyellow", 0xADFF2F},   {"hotpink", 0xFF69B4},
		{"indianred", 0xCD5C5C},     {"khaki", 0xF0E68C},		{"lawngreen", 0x7CFC00},     {"limegreen", 0x32CD32},
		{"magenta", 0xFF00FF},       {"maroon", 0xB03060},		{"navy", 0x000080},          {"orange", 0xFFA500},
		{"orangered", 0xFF4500},     {"orchid", 0xDA70D6},		{"palegoldenrod", 0xEEE8AA}, {"palegreen", 0x98FB98},
		{"palevioletred", 0xDB7093}, {"papayawhip", 0xFFEFD5},	{"peachpuff", 0xFFDAB9},     {"peru", 0xCD853F},
		{"pink", 0xFFC0CB},          {"plum", 0xDDA0DD},		{"powderblue", 0xB0E0E6},    {"purple", 0xA020F0},
		{"red", 0xFF0000},           {"rosybrown", 0xBC8F8F},	{"royalblue", 0x4169E1},     {"saddlebrown", 0x8B4513},
		{"salmon", 0xFA8072},        {"sandybrown", 0xF4A460},	{"seagreen", 0x2E8B57},      {"seashell", 0xFFF5EE},
		{"sienna", 0xA0522D},        {"skyblue", 0x87CEEB},		{"slateblue", 0x6A5ACD},     {"slategray", 0x708090},
		{"snow", 0xFFFAFA},          {"springgreen", 0x00FF7F},	{"steelblue", 0x4682B4},     {"tan", 0xD2B48C},
		{"thistle", 0xD8BFD8},       {"tomato", 0xFF6347},		{"turquoise", 0x40E0D0},     {"violet", 0xEE82EE},
		{"wheat", 0xF5DEB3},         {"whitesmoke", 0xF5F5F5},	{"white", 0xFFFFFF},         {"yellow", 0xFFFF00},
		{"yellowgreen", 0x9ACD32}
};

StylePattern::StylePattern(){
    memset(states,0,sizeof(states));
}

static int parseProp(const std::map<const std::string,std::string>&atts,const std::string&prop,int v,int&add){
    auto it=atts.find(prop);
	if(it==atts.end())return 0;
	add++;
	return (it->second.compare("true")==0)?v:-v;
}

void StylePattern::parseStates(const char**atts){
	int i;
	std::map<const std::string,std::string>maps;
	for(i=0;atts[i];i+=2)
	   maps[atts[i]]=atts[i+1];
	i=0;
	states[i] = parseProp(maps,"enabled",Style::ENABLED,i);
	states[i] = parseProp(maps,"focused",Style::FOCUSED,i);
	states[i] = parseProp(maps,"selected",Style::SELECTED,i);
	states[i] = parseProp(maps,"pressed",Style::PRESSED,i);
	states[i] = parseProp(maps,"hovered",Style::HOVERED,i);
	LOGV("%s states=%x,%x,%x,%x",name.c_str(),states[0],states[1],states[2],states[3]);
}

bool StylePattern::match(int *st)const{
    for(int i=0;i<8;i++){
	    int stateSpecState = states[i];
		if(stateSpecState==0)/*end of states*/
			return true;
		const bool mustmatch=stateSpecState>0;
		if(stateSpecState<0){
			//we use negative value to indicate must-NOT-match states.
			stateSpecState*=-1;
		}
		bool found=false;
		for(int j=0;j<8;j++){
			if(st[j]==0){//we've reached the endof states to match
				if(mustmatch){
					//we didn't find this must-match state
					return false;
				}//else:continue checking other musr-not-match state
				break;
			}
			if(st[i]==stateSpecState){
				if(mustmatch){
					//Continue checking other other must-match states.
					found=true;
					break;
				}//else:Any match of a must-not-match state returns false
				return false;
			}
		}
		if(mustmatch&&!found){
			// We've reached the end of states to match and we didn't
            // find a must-match state.
			return false;
		}
	}
	return true;
}


Style::Style(const std::string&txt){
    mParent=nullptr;
    name=txt;
}

Style::~Style(){
    for_each(mPatterns.begin(),mPatterns.end(),[](const StylePattern*p){
		delete p;
	});
	mPatterns.clear();
}

RefPtr<const Pattern>Style::getPattern(const std::string&key,int*states)const{
    int idx=0;
	LOGD("Style %s has %d Patterns",name.c_str(),mPatterns.size());
    for(auto it:mPatterns){
		if((it->name==key) && it->match(states)){
			LOGD("%s:%d matched.pat=%p type=%d",it->name.c_str(),idx,it->pat.get(),it->pat->get_type());
			return it->pat;
		}idx++;
	}
    if(mParent)return mParent->getPattern(key,states);
    return nullptr; 
}

int Style::getProp(const std::string&name,int state)const{
    auto it=mProps.find(name);
    if(it!=mProps.end())return it->second;
    if(mParent)return mParent->getProp(name,state);
    return 0;
}


typedef struct ParserData{
 	Theme*theme;
    Style*style;
	StylePattern*spt;
	Pattern*pat;
	std::map<const std::string,RefPtr<Pattern>>pats;
    std::string characters;
    int ident;
    ParserData(Theme*v){
		theme=v;
        spt=nullptr;
        ident=0;
        characters=std::string();
    }
}PARSERDATA;

static unsigned int parseColor(const std::string&txt,bool*isnamecolor=nullptr){
    auto it=name2colors.find(txt);
	if(isnamecolor)*isnamecolor=(it!=name2colors.end());
	if(it!=name2colors.end())
		return it->second;
	return std::strtol(txt.c_str(),nullptr,16);
}

static double prop2Value(const std::map<const std::string,std::string>&attrs,const std::string&name,double defvalue=.0){
    auto it=attrs.find(name);
	if(it==attrs.end())return defvalue;
	return std::atof(it->second.c_str());
}

static RefPtr<Pattern>createPattern(const std::map<const std::string,std::string>&attrs){
	auto it=attrs.find("type");
	std::string type;
	if(it==attrs.end())return nullptr;
	type=it->second;
	if(type.compare("linear")==0){
		double x0=prop2Value(attrs,"x0");
		double y0=prop2Value(attrs,"y0");
		double x1=prop2Value(attrs,"x1");
		double y1=prop2Value(attrs,"y1");
		return LinearGradient::create(x0,y0,x1,y1);
	}else if(type.compare("radial")==0){
		double cx0=prop2Value(attrs,"cx0");
		double cy0=prop2Value(attrs,"cy0");
		double r0 =prop2Value(attrs,"radius0");
		double cx1=prop2Value(attrs,"cx1");
		double cy1=prop2Value(attrs,"cy1");
		double r1 =prop2Value(attrs,"radius1");
		return RadialGradient::create(cx0,cy0,r0,cx1,cy1,r1);
	}else if(type.compare("image")==0){
		RefPtr<Surface>img;
		return SurfacePattern::create(img);
	}
}
static Pattern::Extend parseExtend(std::map<const std::string,std::string>&maps){
    auto it=maps.find("extend");
	if(it==maps.end())return Pattern::Extend::NONE;
	const std::string ext=it->second;
	if(ext.compare("repeat")==0)return Pattern::Extend::REPEAT;
	else if(ext.compare("reflect")==0)return Pattern::Extend::REFLECT;
	else if(ext.compare("pad")==0)return Pattern::Extend::PAD;
	else return Pattern::Extend::NONE;
}

static void startElement(void *userData, const XML_Char *name, const XML_Char **atts){
    PARSERDATA*pd = (PARSERDATA*)userData;
	std::map<const std::string,std::string>maps;
    pd->ident++;
    pd->characters.erase();
    for(int i=0;atts[i];i+=2)maps[atts[i]]=atts[i+1];
	if(strcmp(name,"colorstop")==0 && pd->pat){
		double offset=prop2Value(maps,"offset");
		bool isnamecolor;
		auto ita=maps.find("alpha");
		unsigned int c=parseColor(maps["color"],&isnamecolor);
		unsigned int alpha=(unsigned int)prop2Value(maps,"alpha",0xFF);
		if(isnamecolor)c|=(alpha<<24);
		LOGV("colorstop %.2f  %08x",offset,c);
		((Gradient*)pd->pat)->add_color_stop_rgba(offset,((c>>16)&0xFF)/255.,
				((c>>8)&0xff)/255.,(c&0xff)/255.,(c>>24)/255.);
	}else if(strcmp(name,"pattern")==0){
	    RefPtr<Pattern>p=createPattern(maps);
		if(p){
			LOGV("pattern.name=%s type=%d",maps["name"].c_str(),p->get_type());
			pd->pats[maps["name"]]=p;
		    pd->pat=p.get();
		    pd->pat->set_extend(parseExtend(maps));
		}
	}else if(strcmp(name,"style")==0){
		pd->style=new Style(maps["name"]);
		pd->theme->addStyle(maps["name"],pd->style);
	}else if(strcmp(name,"item")==0){
		pd->spt=new StylePattern();
		pd->spt->name=maps["name"];
		pd->spt->parseStates(atts);
	}
}

static void endElement(void *userData, const XML_Char *name){
    PARSERDATA*pd = (PARSERDATA*)userData;
	if(strcmp(name,"item")==0){
		const std::string text=pd->characters;
		auto it=pd->pats.find(text);
		
		LOGV("pattern %s issolid:%d",text.c_str(),(it==pd->pats.end()));
		if(it!=pd->pats.end()){
			pd->spt->pat=it->second;
			pd->style->addPattern(pd->spt);
		}else{
			bool isnamecolor;
			unsigned int c=parseColor(text,&isnamecolor);
			LOGV("%s :%x",text.c_str(),c);
			if(isnamecolor)c|=0xFF000000;
			pd->spt->pat=SolidPattern::create_rgba(((c>>16)&0xFF)/255.,
					((c>>8)&0xff)/255.,(c&0xff)/255.,(c>>24)/255.);
			pd->style->addPattern(pd->spt);
		}
	}
    pd->characters=std::string();
    pd->ident--;
}

static void dataHandler(void *userData, const XML_Char *text,int len){
    PARSERDATA*pd = (PARSERDATA*)userData;
    if(len>1)pd->characters.append(text,len);
}

Theme*Theme::mInst=nullptr;
Theme&Theme::getInstance(){
    if(mInst==nullptr)
       mInst=new Theme();
    return *mInst;
}


void Theme::addStyle(const std::string&name,Style*style){
    if(mStyles.find(name)==mStyles.end()){
        mStyles[name].reset(style);
		return ;
    }
	LOGE("Style %s exists",name.c_str());
}

int Theme::parseStyles(std::istream&stream){
    int done=0;
    char buf[256];
    PARSERDATA data(this);

    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetUserData(parser,&data);
    XML_SetElementHandler(parser, startElement, endElement);
    XML_SetCharacterDataHandler(parser,dataHandler);

    do {
       int len=stream.readsome(buf,sizeof(buf));
       done=(len==0);
       if (XML_Parse(parser, buf,len,done) == XML_STATUS_ERROR) {
           const char*es=XML_ErrorString(XML_GetErrorCode(parser));
           LOGE("%s at line %ld",es, XML_GetCurrentLineNumber(parser));
           return 1;
       }
    } while(!done);
    XML_ParserFree(parser);
	LOGD("styles.count=%d",mStyles.size());
    for_each(mStyles.begin(),mStyles.end(),[](const std::map<const std::string,RefPtr<Style>>::reference&s){
          LOGV("style:%s has %d patterns",s.first.c_str(),s.second->mPatterns.size());
          for_each(s.second->mPatterns.begin(),s.second->mPatterns.end(),[](const StylePattern*p){
                  const int *ps=p->states;
                  LOGV("    pat.type=%d states:%3d,%3d,%3d,%3d name=%-16s pat=%p",p->pat->get_type(),
                          ps[0],ps[1],ps[2],ps[3],p->name.c_str(),p->pat.get());
          }); 
    });
    return 0;
}

Style*Theme::getStyle(const std::string&name)const{
    auto it=mStyles.find(name);
	if(it==mStyles.end())return nullptr;
	return it->second.get();
}

}//namespace
