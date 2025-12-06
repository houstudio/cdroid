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
#include <color.h>
#include <stdlib.h>
#include <map>
#include <mutex>
#include <iostream>
#include <exception>

namespace cdroid{
static const std::map<const std::string ,unsigned int>sColorNameMap={
    {"aquamarine",0xFF7FFFD4},    {"beige" , 0xFFF5F5DC},       {"black"    , 0xFF000000},     {"blue"      , 0xFF0000FF},
    {"blueviolet",0xFF8A2BE2},    {"brown" , 0xFFA52A2A},       {"cadetblue", 0xFF5F9EA0},     {"chartreuse", 0xFF7FFF00},
    {"chocolate" ,0xFFD2691E},    {"coral" , 0xFFFF7F50},       {"cornflowerblue",0xFF6495ED}, {"cyan"      , 0xFF00FFFF},
    {"firebrick" ,0xFFB22222},    {"forestgreen", 0xFF228B22},  {"gold"        , 0xFFFFD700},  {"goldenrod" , 0xFFDAA520},
    {"gray"      ,0xFFBEBEBE},    {"green" , 0xFF00FF00},       {"greenyellow" , 0xFFADFF2F},  {"hotpink"   , 0xFFFF69B4},
    {"indianred", 0xFFCD5C5C},    {"khaki" , 0xFFF0E68C},       {"lawngreen"   , 0xFF7CFC00},  {"limegreen" , 0xFF32CD32},
    {"magenta"  , 0xFFFF00FF},    {"maroon", 0xFFB03060},       {"navy"        , 0xFF000080},  {"orange"    , 0xFFFFA500},
    {"orangered", 0xFFFF4500},    {"orchid", 0xFFDA70D6},       {"palegoldenrod",0xFFEEE8AA},  {"palegreen" , 0xFF98FB98},
    {"palevioletred",0xFFDB7093}, {"papayawhip", 0xFFFFEFD5},   {"peachpuff", 0xFFFFDAB9},     {"peru"      , 0xFFCD853F},
    {"pink"  , 0xFFFFC0CB},       {"plum"  , 0xFFDDA0DD},       {"powderblue", 0xFFB0E0E6},    {"purple"    , 0xFFA020F0},
    {"red"   , 0xFFFF0000},       {"rosybrown" , 0xFFBC8F8F},   {"royalblue", 0xFF4169E1},     {"saddlebrown",0xFF8B4513},
    {"salmon", 0xFFFA8072},       {"sandybrown", 0xFFF4A460},   {"seagreen" , 0xFF2E8B57},     {"seashell"   , 0xFFFFF5EE},
    {"sienna", 0xFFA0522D},       {"skyblue"   , 0xFF87CEEB},   {"slateblue", 0xFF6A5ACD},     {"slategray"  , 0xFF708090},
    {"snow"  , 0xFFFFFAFA},       {"springgreen",0xFF00FF7F},   {"steelblue", 0xFF4682B4},     {"tan"        , 0xFFD2B48C},
    {"thistle",0xFFD8BFD8},       {"tomato"    , 0xFFFF6347},   {"turquoise", 0xFF40E0D0},     {"violet", 0xFFEE82EE},
    {"wheat" , 0xFFF5DEB3},       {"whitesmoke", 0xFFF5F5F5},   {"white"    , 0xFFFFFFFF},     {"yellow", 0xFFFFFF00},
    {"yellowgreen", 0x9ACD32},    {"transparent",0}
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

unsigned int Color::toArgb(uint8_t r,uint8_t g,uint8_t b,uint8_t a){
    return((unsigned int)a<<24)|(r<<16)|(g<<8)|b;
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
        unsigned int cc=strtoul(colorString.c_str()+1,nullptr,16);
        if(colorString.length()<=7)
            cc|=0xFF000000;
        return cc;
    }else{
        return getHtmlColor(colorString);
    }
}

unsigned int Color::getHtmlColor(const std::string&colorname){
     auto it = sColorNameMap.find(colorname);
     if(it== sColorNameMap.end())/*return -1;*/
         throw std::invalid_argument("invalid color");
     return it->second;
}

}

