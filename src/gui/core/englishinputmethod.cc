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
#include <core/englishinputmethod.h>
#include <porting/cdlog.h>
#include <cctype>
#include <fstream>
#include <unordered_set>

namespace cdroid{

// A small baseline English word list, roughly frequency-ordered so that prefix
// matches come out most-common-first. It is intentionally modest ("a little
// spelling ability"); an external word-list file can override it via loadDicts.
// Duplicates are harmless -- the constructor stable-dedups.
static const char* const kBaselineEnglishWords[] = {
    "the","be","to","of","and","a","in","that","have","it",
    "for","not","on","with","as","you","do","at","this","but",
    "his","by","from","they","we","her","she","or","an","will",
    "my","one","all","would","there","their","what","up","out","if",
    "about","who","get","which","go","me","when","make","can","like",
    "time","no","just","him","know","take","people","into","year","your",
    "good","some","could","them","see","other","than","then","now","look",
    "only","come","over","think","also","back","after","use","two","how",
    "our","work","first","well","way","even","new","want","because","any",
    "these","give","day","most","are","was","were","been","has","had",
    "did","said","made","find","here","thing","those","water","long","world",
    "great","while","small","right","place","where","house","still","name","every",
    "between","head","point","ask","change","went","light","away","need","home",
    "hand","part","start","show","old","play","spell","air","animal","picture",
    "write","earth","mother","before","live","country","father","keep","eye","never",
    "last","door","far","story","draw","left","late","run","close","night",
    "real","stop","open","next","white","begin","walk","paper","often","always",
    "music","letter","until","river","feet","care","second","carry","rain","room",
    "friend","idea","fish","once","hear","horse","cut","watch","color","face",
    "wood","main","enough","girl","young","ready","above","list","feel","talk",
    "soon","body","family","leave","song","short","class","wind","happen","ship",
    "area","half","rock","order","fire","south","problem","piece","pass","farm",
    "whole","king","space","heard","best","hour","better","true","during","five",
    "remember","step","early","hold","west","ground","reach","fast","sing","listen",
    "table","travel","less","morning","ten","simple","several","toward","slow","center",
    "love","person","money","serve","appear","road","pull","cold","voice","town",
    "power","fine","fall","lead","mind","else","wheel","team","born","hello",
    "help","computer","please","thanks","sorry","welcome","message","phone","email","password",
    "user","date","search","settings","system","network","account","login","delete","edit",
    "save","cancel","confirm","today","tomorrow","yesterday","evening","week","month","number",
    "address","city","street","mobile","application","file","folder","image","video","audio",
    "photo","screen","button","click","volume","language","english","chinese","keyboard","input",
    "print","scan","copy","paste","select","default","custom","general","about","version",
    "update","install","download","upload","connect","enable","disable","function","variable","method",
    "public","private","static","virtual","interface","module","library","server","client","database",
    "table","value","string","integer","array","red","green","blue","black","yellow",
    "orange","purple","gray","brown","pink","large","tiny","huge","wide","narrow",
    "deep","high","low","quick","easy","hard","difficult","complex","clear","clean",
    "dirty","full","empty","happy","tired","hungry","warm","cool","sunny","cloudy",
    "monday","tuesday","wednesday","thursday","friday","saturday","sunday","january","february","march",
    "april","june","july","august","september","october","november","december","spring","summer",
    "autumn","winter","north","east","bottom","middle","inside","outside","below","near",
    "yes","ok","add","open","close","pause","play","next","previous","done"
};

EnglishInputMethod::EnglishInputMethod(){
    // Stable-dedup so the source list need not be perfect, preserving order.
    std::unordered_set<std::string> seen;
    mWords.reserve(sizeof(kBaselineEnglishWords)/sizeof(kBaselineEnglishWords[0]));
    for(const char* w : kBaselineEnglishWords){
        if(seen.insert(w).second) mWords.emplace_back(w);
    }
}

bool EnglishInputMethod::loadDicts(const std::string& file,const std::string&/*user*/){
    // One word per line; an empty/missing file keeps the built-in baseline.
    std::ifstream f(file);
    if(!f.is_open()) return false;
    std::vector<std::string> loaded;
    std::string line;
    while(std::getline(f,line)){
        const size_t a = line.find_first_not_of(" \t\r\n");
        if(a == std::string::npos) continue;
        const size_t b = line.find_last_not_of(" \t\r\n");
        std::string w = line.substr(a, b - a + 1);
        for(char& c : w) c = (char)tolower((unsigned char)c);
        if(!w.empty()) loaded.push_back(std::move(w));
    }
    if(loaded.empty()) return false;
    mWords = std::move(loaded);
    LOGD("EnglishInputMethod loaded %d words from %s",(int)mWords.size(),file.c_str());
    return true;
}

int EnglishInputMethod::search(const std::string& prefix,std::vector<std::string>&candidates){
    if(prefix.empty()) return -1;
    std::string pfx;
    pfx.reserve(prefix.size());
    for(char c : prefix) pfx.push_back((char)tolower((unsigned char)c));
    // candidates[0] is the typed text itself, kept visible and committable
    // as-is so the user is never forced to accept a completion.
    candidates.push_back(pfx);
    static constexpr int kMaxCandidates = 12;
    int n = 1;
    for(const std::string& w : mWords){
        if(w.size() > pfx.size() && w.compare(0,pfx.size(),pfx) == 0){
            candidates.push_back(w);
            if(++n >= kMaxCandidates) break;
        }
    }
    return n;
}

}/*endof namespace*/
