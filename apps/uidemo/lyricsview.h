#ifndef __LYRICS_VIEW_H__
#define __LYRICS_VIEW_H__
#include <widget/textview.h>
#include <unordered_map>
#include <lyrics.h>

namespace cdroid{

class LyricsView:public TextView{
public:
protected:
    Lyrics id3;
    std::vector<Lyrics::LYRICSLINE>lyrics;
    
public:
    LyricsView(const std::string& text, int width, int height);
    void setText(const std::string&txt)override;
    virtual void onDraw(Canvas& canvas) override;
    virtual void setTime(UINT timems);
    const std::string getTitle()const;
    const std::string getArtist()const;
    const std::string getAlbum()const;
    const std::string getYear()const;
    const std::string getComment()const;
    const std::string getTrack()const;
    const std::string getGenre()const;
    RefPtr<ImageSurface>getImage(const std::string&picname="");
};

}
#endif
