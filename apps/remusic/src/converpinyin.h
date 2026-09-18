// Port of com.wm.remusic.uitl.ConverPinYin (public surface).
#ifndef __REMUSIC_CONVERPINYIN_H__
#define __REMUSIC_CONVERPINYIN_H__

#include <string>

namespace remusic {

std::string getFullSpell(const std::string& utf8);
std::string getFirstSpell(const std::string& utf8);
char sectionLetter(const std::string& utf8);   // 'A'..'Z' or '#'

} // namespace remusic
#endif