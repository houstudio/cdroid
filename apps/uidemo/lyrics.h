#ifndef __ID3_LYRICS__H__
#define __ID3_LYRICS__H__
#include <core/canvas.h>
#include <string>
#include <vector>
#include <unordered_map>

namespace cdroid{

class Lyrics{
public:
    typedef struct LyricsLine{
        UINT time;
        std::string text;
    }LYRICSLINE;
protected:
    bool hasid3;
    std::string url;//music file name
    std::vector<struct LyricsLine>lyrics;
    std::string id3title;
    std::string id3artist;
    std::string id3album;
    std::string id3year;
    std::string id3comment;
    std::string id3track;
    std::string id3genre;
    std::unordered_map<std::string,RefPtr<ImageSurface>>images; 
    int parseLyrics(const std::string&);
    int parseID3(const std::string&);
    int loadImages();
public:
    Lyrics(const std::string& text);
    Lyrics();
    void setUrl(const std::string&txt);
    RefPtr<ImageSurface>getImage(const std::string&picname="")const;
    const std::string&getTitle()const;
    const std::string&getArtist()const;
    const std::string&getAlbum()const;
    const std::string&getYear()const;
    const std::string&getComment()const;
    const std::string&getTrack()const;
    const std::string&getGenre()const;
    operator bool()const;
};

}
#endif
