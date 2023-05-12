#include <htmlview.h>
#include <litehtml/url_path.h>
#include <litehtml/url.h>
#include <chrono>
#include <ostream>
#include <fstream>

namespace cdroid{

static const char* master_css =
#include "master.css.inc"
;

HtmlView::HtmlView(int w,int h)
  :cdroid::View(w,h) {
    m_hash_valid  = false;
    m_html = nullptr;
    mHtmlContext.load_master_stylesheet(master_css);
}

HtmlView::~HtmlView() {

}

void HtmlView::onDraw(cdroid::Canvas&canvas) {
    litehtml::position pos;
    double x1,y1,x2,y2;
    canvas.get_clip_extents(x1,y1,x2,y2);

    pos.width = x2-x1;
    pos.height= y2-y1;
    pos.x     = x1;
    pos.y     = y1;
    if((x2==x1)||(y2==y1)){
        pos.width = getWidth();
	pos.height= getHeight();
    }
    LOGD("rect=(%d,%d,%d,%d)",pos.x,pos.y,pos.width,pos.height); 
    canvas.rectangle(0, 0, getWidth(),getHeight());//get_allocated_width(), get_allocated_height());
    canvas.set_source_rgb(1,1,1);
    canvas.fill();

    if(m_html) {
        m_html->draw((litehtml::uint_ptr) canvas.cobj(), 0, 0, &pos);
    }
}

void HtmlView::get_client_rect(litehtml::position& client) const {
    client.width = getWidth(); //get_parent()->get_allocated_width();
    client.height= getHeight();//get_parent()->get_allocated_height();
    client.x = 0;
    client.y = 0;
}


void HtmlView::on_anchor_click(const char* url, const litehtml::element::ptr& el) {
    LOGD("%s",url);
    if(url) {
        make_url(url, m_base_url.c_str(), m_clicked_url);
    }
}


void HtmlView::import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl) {
    std::string css_url;
    make_url(url.c_str(), baseurl.c_str(), css_url);
    load_text_file(css_url, text);
    if(!text.empty()) {
        baseurl = css_url;
    }
}

void HtmlView::set_caption(const char* caption) {
    //called by litehtml::document parser
    /*if(get_parent_window()) {
        get_parent_window()->set_title(caption);
    }*/
    LOGI("[%s]",caption);
}

void HtmlView::set_base_url(const char* base_url) {
    if(base_url) {
        m_base_url = litehtml::resolve(litehtml::url(m_url), litehtml::url(base_url)).string();
    } else {
        m_base_url = m_url;
    }
}

std::string HtmlView::uri2cdroid(const std::string&str){
    std::string uri = str;
    size_t pos = uri.find('@');
    if(pos!=std::string::npos)uri[pos]=':';
    uri="@"+uri;
    return uri;
}

Cairo::RefPtr<Cairo::ImageSurface> HtmlView::get_image(const char* url, bool redraw_on_ready) {
    //Cairo::RefPtr< Gio::InputStream > stream = m_http.load_file(url);
    litehtml::url uri(url);
    Cairo::RefPtr<Cairo::ImageSurface> ptr;
    LOGD("[%s] scheme=%s path=%s",url,uri.scheme().c_str(),uri.path().c_str());
    if(uri.scheme().compare("file")==0){std::fstream fs(uri.path(),std::fstream::in); 
        ptr =Cairo::ImageSurface::create_from_stream(fs);
    }else if(uri.scheme().compare("assets")==0){
        const std::string cdroiduri = uri2cdroid(uri.authority()+uri.path());
        auto istrm = getContext()->getInputStream(cdroiduri);
        LOGE_IF(istrm==nullptr,"%s is not exist or it is an invalid uri",cdroiduri.c_str());
	if(istrm) ptr = Cairo::ImageSurface::create_from_stream(*istrm);
    }
    return ptr;
}

void HtmlView::open_page(const std::string& url, const std::string& hash) {
    m_url 	= url;
    m_base_url	= url;

    std::string html;
    load_text_file(url, html);
    //m_url 	= m_http.get_url();
    //m_base_url	= m_http.get_url();
    m_html = litehtml::document::createFromUTF8(html.c_str(), this,&mHtmlContext);//this);
    if(m_html) {
        m_html->render(getWidth());
        m_hash = hash;
        m_hash_valid = true;
        //set_size_request(m_html->width(), m_html->height());
    }
    invalidate();//queue_draw();
}

void HtmlView::scroll_to(int x, int y) {
#if 0
    auto vadj = m_browser->get_scrolled()->get_vadjustment();
    auto hadj = m_browser->get_scrolled()->get_hadjustment();
    vadj->set_value(vadj->get_lower() + y);
    hadj->set_value(hadj->get_lower() + x);
#else

#endif
}

void HtmlView::set_cursor(const litehtml::tchar_t* cursor){
}

void HtmlView::show_hash(const litehtml::tstring& hash) {
    if(hash.empty()) {
        scroll_to(0, 0);
    } else {
        std::string selector = "#" + hash;
        litehtml::element::ptr el = m_html->root()->select_one(selector);
        if (!el) {
            selector = "[name=" + hash + "]";
            el = m_html->root()->select_one(selector);
        }
        if (el) {
            litehtml::position pos = el->get_placement();
            scroll_to(0, pos.top());
        }
    }
}

void HtmlView::make_url(const char* url, const char* basepath, litehtml::tstring& out) {
    if(!basepath || !basepath[0]) {
        if(!m_base_url.empty()) {
            out = litehtml::resolve(litehtml::url(m_base_url), litehtml::url(url)).string();
        } else {
            out = url;
        }
    } else {
        out = litehtml::resolve(litehtml::url(basepath), litehtml::url(url)).string();
    }
}

bool HtmlView::onTouchEvent(MotionEvent&event){
    const int x = event.getX();
    const int y = event.getY();
    litehtml::position::vector redraw_boxes;

    if(m_html == nullptr)
	return View::onTouchEvent(event);

    switch(event.getActionMasked()){
    case MotionEvent::ACTION_DOWN:
	    m_html->on_lbutton_down(x,y,x,y,redraw_boxes);
	    break;
    case MotionEvent::ACTION_UP:
	    m_clicked_url.clear();
	    m_html->on_lbutton_up(x,y,x,y,redraw_boxes);
	    LOGI_IF(!m_clicked_url.empty(),"TODO open %s here",m_clicked_url.c_str());
	    break;
    case MotionEvent::ACTION_MOVE:
	    m_html->on_mouse_over(x,y,x,y,redraw_boxes);
	    break;
    }

    for(auto& pos : redraw_boxes){
        invalidate(pos.x, pos.y, pos.width, pos.height);
    }
    return true;
}

void HtmlView::load_text_file(const litehtml::tstring& url, litehtml::tstring& out) {
    litehtml::url uri(url);
    char buf[256];
    if(uri.scheme().empty())
	out = url;
    else if(uri.scheme().compare("file")==0){
        std::fstream fs(uri.path(),std::fstream::in);
        out.clear();
        if(!fs.good()){
	   out=url;
	   return;
        }
        while ( !fs.eof() ) {
	    fs.read(buf,sizeof(buf));
	    size_t len=fs.gcount();
	    buf[len]=0;
	    out.append(buf);
        }
    }else if(uri.scheme().compare("assets")==0){
        const std::string cdroiduri = uri2cdroid( uri.authority()+uri.path() );
	auto istrm = getContext()->getInputStream(cdroiduri);
	LOGE_IF(istrm==nullptr,"%s is not exist or it is an invalid uri",url);
	while(istrm && !istrm->eof()){
            istrm->read(buf,sizeof(buf));
            size_t len=istrm->gcount();
            buf[len]=0;
            out.append(buf);
	}
    }
}

long HtmlView::draw_measure(int number) {
    auto vadj = getScrollY();//m_browser->get_scrolled()->get_vadjustment();
    auto hadj = getScrollX();//m_browser->get_scrolled()->get_hadjustment();

    int width = 0;//(int) hadj->get_page_size();
    int height= 0;//(int) vadj->get_page_size();

    int stride = cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, width);
    auto image = (unsigned char*)malloc (stride * height);

    cairo_surface_t* surface = cairo_image_surface_create_for_data(image, CAIRO_FORMAT_ARGB32, width, height, stride);
    cairo_t* cr = cairo_create(surface);

    litehtml::position pos;
    pos.width 	= width;
    pos.height 	= height;
    pos.x 		= 0;
    pos.y 		= 0;

    int x = 0;//(int) (hadj->get_value() - hadj->get_lower());
    int y = 0;//(int) (vadj->get_value() - vadj->get_lower());

    cairo_rectangle(cr, 0, 0, width, height);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);
    m_html->draw((litehtml::uint_ptr) cr, -x, -y, &pos);
    cairo_surface_write_to_png(surface, "/tmp/litebrowser.png");

    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < number; i++) {
        m_html->draw((litehtml::uint_ptr) cr, -x, -y, &pos);
    }
    auto t2 = std::chrono::high_resolution_clock::now();

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    free(image);

    return (std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1)).count();
}

long HtmlView::render_measure(int number) {
    if(m_html) {
        auto t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < number; i++) {
            m_html->render(getWidth());//m_rendered_width);
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        return (std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1)).count();
    }
    return -1;
}

/*void HtmlView::on_size_allocate(Gtk::Allocation& allocation) {
    Gtk::DrawingArea::on_size_allocate(allocation);
    if(m_hash_valid) {
        show_hash(m_hash);
        m_hash_valid = false;
    }
}*/

void HtmlView::dump(const std::string& file_name) {
    if(m_html) {
        //html_dumper dumper(file_name);
        //m_html->dump(dumper);
        //dumper.print();
    }
}

}//endof namespace

