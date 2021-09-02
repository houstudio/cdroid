#include <lyrics.h>
#include <core/assets.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef ENABLE_MP3ID3
#include <id3/getid3.h>
#include <id3/id3v2.h>
#include <id3/getid3v2.h>
#include <id3/getlyr3.h>
using namespace tag;
#endif

#include <cdtypes.h>
#include <cdlog.h>
#include <core/iostreams.h>
#include <core/textutils.h>


namespace priv{
#ifdef ENABLE_MP3ID3
const std::string W2S(const stredit::function::result&w){
    char u8bytes[256];
    return cdroid::TextUtils::unicode2utf8(w);
}
#endif

const char *membrk0(const char *buf, size_t size, int wide){
    const char* const end = buf + size - wide;
    const int step = 1+wide;
    for( ; buf < end; buf += step) {
        if(!buf[0] && !buf[wide])
            return buf;
    }
    return 0;
}
}

namespace cdroid{

Lyrics::Lyrics(){
   LOGD("wchar_t=%d",sizeof(wchar_t));
}
Lyrics::Lyrics(const std::string& text){
   url=text;
   hasid3=false;
   parseLyrics(text);
   parseID3(text); 
   getImage(text);
   LOGD("wchar_t=%d",sizeof(wchar_t));
}


static const char* picture_types[] = {
    "other",    "icon",    "other_icon",    "front_cover",    "back_cover",
    "leaflet",    "media",    "lead_artist",    "artist",    "conductor",
    "band",    "composer",    "lyricist",    "location",    "recording",
    "performance",    "screencap",    "red_herring",    "illustration",
    "logotype",    "studio_logotype" 
};

int Lyrics::loadImages(){
    int counter=0;
#ifdef ENABLE_MP3ID3
    void *tag = ID3_readf(url.c_str(), 0);
    ID3FRAME f;
    LOGV("%s",url.c_str());
    if(tag==nullptr)return 0;
    if(ID3_start(f,tag) >= 2) {
        while(ID3_frame(f)) {
            if(strcmp(f->ID, "APIC") == 0) {
                /* see ID3v2.3 4.15 -- 'Attached Picture' for reference */
                char wide = f->data[0] == 1 || f->data[0] == 2;
                const char *mime_type = f->data+1;
                const char *type = (const char*)memchr(mime_type, 0, f->size-(2+wide));
                const char *descr, *blob;
                if(!type || (type[1]&0xFFu) > sizeof picture_types/sizeof *picture_types) {
                     LOGD("%s has an incorrect ID3v2 tag!\n",url.c_str());
                     continue;
                } else {
                     ++type;          /* skip terminator */
                }
                descr = type+1;
                blob = priv::membrk0(descr, f->size-(descr-f->data), wide);
                if(!blob) {
                     LOGD("%s has an incorrect ID3v2 tag!\n",url.c_str());
                     continue;
                } else {
                     blob += 1+wide;  /* skip terminator */
                }
                MemoryInputStream stm(blob,f->size-(blob-f->data));
                counter++;
                RefPtr<ImageSurface>image=cdroid::Context::loadImage(stm);
                images[picture_types[type[1]&0xFF]]=image;
                LOGV("%s:imagesize=%dx%d",picture_types[type[1]&0xFF],image->get_width(),image->get_height());
           }
       }//while
       ID3_free(tag);
   }
#endif
   return counter;
}

RefPtr<ImageSurface>Lyrics::getImage(const std::string&name)const{
   if(name.empty()&&images.size())
      return images.begin()->second; 
   if(images.size())return images.at(name);
   return ImageSurface::create_from_png("");
}

Lyrics::operator bool()const{
    return hasid3;
}

int Lyrics::parseID3(const std::string&txt){
    struct stat stbuf;
    if(stat(txt.c_str(),&stbuf)!=0)return 0;
#ifdef ENABLE_MP3ID3
    tag::read::ID3v2 v2(txt.c_str());
    tag::read::ID3 v1(txt.c_str());

    tag::metadata*id3tag=nullptr;
    if(v2)
       id3tag=&v2;
    else if(v1)
       id3tag=&v1;

    if(id3tag){
       hasid3=true;
       id3title=priv::W2S((*id3tag)[tag::title]); 
       id3artist=priv::W2S((*id3tag)[tag::artist]); 
       id3album=priv::W2S((*id3tag)[tag::album]); 
       id3year=priv::W2S((*id3tag)[tag::year]); 
       id3comment=priv::W2S((*id3tag)[tag::cmnt]); 
       id3track=priv::W2S((*id3tag)[tag::track]); 
       id3genre=priv::W2S((*id3tag)[tag::genre]); 
    }
#endif
    LOGV("title:%s\r\nartist:%s\r\nalbum:%s\r\nyear:%s\r\ngenre:%s",id3title.c_str(),
        id3artist.c_str(),id3album.c_str(),id3year.c_str(),id3genre.c_str());
}

const std::string& Lyrics::getTitle()const{
    return id3title;
}

const std::string& Lyrics::getArtist()const{
    return id3artist;
}

const std::string& Lyrics::getAlbum()const{
    return id3album;
}

const std::string& Lyrics::getYear()const{
    return id3year;
}

const std::string& Lyrics::getComment()const{
    return id3comment;
}

const std::string& Lyrics::getTrack()const{
    return id3track;
}

const std::string& Lyrics::getGenre()const{
    return id3genre;
}

int Lyrics::parseLyrics(const std::string&txt){
    struct stat stbuf;
    std::string lyricstxt=txt;
#ifdef ENABLE_MP3ID3
    if(stat(txt.c_str(),&stbuf)==0){
        tag::read::Lyrics3 ly(txt.c_str());
        LOGV("getlyrics from %s %d",txt.c_str(),(bool)ly);
        if(ly){
           auto lst=ly.listing();//typedef std::vector< std::pair<std::string, value_string> > array
           for(auto l:lst)
               LOGD("%s::%s",l.first.c_str(),priv::W2S(l.second).c_str());
           return lst.size();
        }
    }

    void *tag = ID3_readf(txt.c_str(), 0);
    ID3FRAME f;
    if(tag==nullptr)return 0;
    if(ID3_start(f,tag) >= 2) {
        int counter=0;
        while(ID3_frame(f)) {
        }//while
    }
    ID3_free(tag);
#endif
    return 0;
}

void Lyrics::setUrl(const std::string&txt){
    url=txt;
    parseLyrics(txt);
    parseID3(txt);
    loadImages();
}

}//namespace
