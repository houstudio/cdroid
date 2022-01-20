#include <color.h>
#include <map>
namespace cdroid{
static std::map<const std::string ,unsigned int>sColorNameMap={
    {"aquamarine",0x7FFFD4},    {"beige", 0xF5F5DC},       {"black", 0x000000},         {"blue", 0x0000FF},
    {"blueviolet",0x8A2BE2},     {"brown", 0xA52A2A},       {"cadetblue", 0x5F9EA0},     {"chartreuse", 0x7FFF00},
    {"chocolate" ,0xD2691E},     {"coral", 0xFF7F50},       {"cornflowerblue", 0x6495ED},{"cyan", 0x00FFFF},
    {"firebrick" ,0xB22222},     {"forestgreen", 0x228B22}, {"gold", 0xFFD700},          {"goldenrod", 0xDAA520},
    {"gray", 0xBEBEBE},          {"green", 0x00FF00},       {"greenyellow", 0xADFF2F},   {"hotpink", 0xFF69B4},
    {"indianred", 0xCD5C5C},     {"khaki", 0xF0E68C},       {"lawngreen", 0x7CFC00},     {"limegreen", 0x32CD32},
    {"magenta", 0xFF00FF},       {"maroon", 0xB03060},      {"navy", 0x000080},          {"orange", 0xFFA500},
    {"orangered", 0xFF4500},     {"orchid", 0xDA70D6},      {"palegoldenrod", 0xEEE8AA}, {"palegreen", 0x98FB98},
    {"palevioletred", 0xDB7093}, {"papayawhip", 0xFFEFD5},  {"peachpuff", 0xFFDAB9},     {"peru", 0xCD853F},
    {"pink", 0xFFC0CB},          {"plum", 0xDDA0DD},        {"powderblue", 0xB0E0E6},    {"purple", 0xA020F0},
    {"red", 0xFF0000},           {"rosybrown", 0xBC8F8F},   {"royalblue", 0x4169E1},     {"saddlebrown", 0x8B4513},
    {"salmon", 0xFA8072},        {"sandybrown", 0xF4A460},  {"seagreen", 0x2E8B57},      {"seashell", 0xFFF5EE},
    {"sienna", 0xA0522D},        {"skyblue", 0x87CEEB},     {"slateblue", 0x6A5ACD},     {"slategray", 0x708090},
    {"snow", 0xFFFAFA},          {"springgreen", 0x00FF7F}, {"steelblue", 0x4682B4},     {"tan", 0xD2B48C},
    {"thistle", 0xD8BFD8},       {"tomato", 0xFF6347},      {"turquoise", 0x40E0D0},     {"violet", 0xEE82EE},
    {"wheat", 0xF5DEB3},         {"whitesmoke", 0xF5F5F5},  {"white", 0xFFFFFF},         {"yellow", 0xFFFF00},
    {"yellowgreen", 0x9ACD32}
};

Color::Color(unsigned int c){
    mComponents[0]=((c>>16)&0xFF)/255.f;
    mComponents[1]=((c>> 8)&0xFF)/255.f;
    mComponents[2]=(c&0xFF)/255.f;
    mComponents[3]=(c>> 24)/255.f ;
}

Color::Color(float r, float g, float b, float a){
    mComponents[0]=r;
    mComponents[1]=g;
    mComponents[2]=b;
    mComponents[3]=a;
}

unsigned int Color::toArgb(float r, float g, float b, float a){
    return ((int)(a * 255.0f + 0.5f) << 24) |
           ((int)(r * 255.0f + 0.5f) << 16) |
           ((int)(g * 255.0f + 0.5f) <<  8) |
           ((int)(b * 255.0f + 0.5f));
}

unsigned int Color::toArgb(int r,int g,int b,int a){
    return((unsigned int)r<<24)|(r<<16)|(g<<8)|b;
}

unsigned int Color::toArgb()const{
    return ((unsigned int) (mComponents[3] * 255.0f + 0.5f) << 24) |
           ((int) (mComponents[0] * 255.0f + 0.5f) << 16) |
           ((int) (mComponents[1] * 255.0f + 0.5f) <<  8) |
           ((int) (mComponents[2] * 255.0f + 0.5f)) ;
}

Color* Color::valueOf(float r,float g,float b,float a){
    return new Color(r,g,b,a);
}

unsigned int Color::parseColor(const std::string& colorString){
    if(colorString[0]=='#'){
        unsigned cc=std::strtol(colorString.c_str()+1,nullptr,16);
        if(colorString.length()<=7)
            cc|=0xFF000000;
        return cc;
    }else if(colorString.compare("transparent")==0){
        return 0;
    }else{
        return 0xFF000000|getHtmlColor(colorString);
    }
}

unsigned int Color::getHtmlColor(const std::string&colorname){
     auto it=sColorNameMap.find(colorname);
     if(it== sColorNameMap.end())return -1;
     return it->second;
}

}

