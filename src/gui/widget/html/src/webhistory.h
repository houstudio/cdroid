#ifndef __LITEHTML_WEB_HISTORY_H__
#define __LITEHTML_WEB_HISTORY_H__
#include <vector>
#include <string>
namespace cdroid{
typedef std::vector<std::string> string_vector;

class WebHistory {
    string_vector m_items;
    string_vector::size_type m_current_item;
public:
    WebHistory();
    virtual ~WebHistory();

    void url_opened(const std::string& url);
    bool back(std::string& url);
    bool forward(std::string& url);
    std::string current() const;
};
}//endof namespace
#endif
