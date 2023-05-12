#ifndef __HTML_VIEW_H__
#define __HTML_VIEW_H__
#include <view/view.h>
#include "containers/linux/container_linux.h"
//#include "http_loader.h"
#include <litehtml/litehtml.h>

namespace cdroid{

class HtmlView :public View::View,public container_linux {
private:
    std::string	m_url;
    std::string	m_base_url;
    litehtml::context  mHtmlContext;
    litehtml::document::ptr m_html;
    std::string	m_cursor;
    std::string	m_clicked_url;
    //http_loader m_http;
    std::string m_hash;
    bool        m_hash_valid;
public:
    HtmlView(int,int);
    virtual ~HtmlView();

    void open_page(const std::string& url, const std::string& hash);
    void show_hash(const std::string& hash);
    void dump(const std::string& file_name);

    long render_measure(int number);
    long draw_measure(int number);
    static std::string uri2cdroid(const std::string&);
protected:
    void onDraw(cdroid::Canvas&)override;
    void scroll_to(int x, int y);
    void set_cursor(const litehtml::tchar_t* cursor) override;
    void get_client_rect(litehtml::position& client) const override;
    void on_anchor_click(const char* url, const litehtml::element::ptr& el) override;
    void import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl) override;
    void set_caption(const char* caption) override;
    void set_base_url(const char* base_url) override;
    Cairo::RefPtr<Cairo::ImageSurface>	get_image(const char* url, bool redraw_on_ready) override;
    void make_url( const char* url, const char* basepath, litehtml::tstring& out ) override;
    bool onTouchEvent(MotionEvent&)override;

    //void on_parent_changed(Gtk::Widget* previous_parent) override;
private:
    void load_text_file(const litehtml::tstring& url, litehtml::tstring& out);
};
}//endof namespace
#endif 
