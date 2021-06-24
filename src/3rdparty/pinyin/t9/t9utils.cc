#include<t9utils.h>
#include<sstream>
char T9Utils::PINYIN_T9_MAP[]={
            '2', '2', '2',
            '3', '3', '3',
            '4', '4', '4',
            '5', '5', '5',
            '6', '6', '6',
            '7', '7', '7', '7',
            '8', '8', '8',
            '9', '9', '9', '9'
    };
char T9Utils::VALID_T9_KEYS[]= {
            '0', '1', '2', '3', '4', '5', '6',
            '7', '8', '9', '+', ',', '*', '#'
    };

bool T9Utils::isValidT9Key(char c) {
    return ((c >= '0') && (c <= '9')) || (c == ',') || (c == '+') || (c == '*') || (c == '#');
}

bool T9Utils::isValidT9Key(const std::string key) {
    const int LEN = key.length();
    for (int i = 0; i < LEN; i++) {
        if (!isValidT9Key(key[i])) {
            return false;
        }
    }
    return true;
}

int T9Utils::T9Char2Index(char c) {
    if ((c >= '0') && (c <= '9')) {
        return c - '0';
    }
    switch (c) {
    case '+':return 10;
    case ',':return 11;
    case '*':return 12;
    case '#':return 13;
    default :return -1;
    }
}

char T9Utils::CharToT9(char c) {
    if (c >= 'A' && c <= 'Z') {
        return PINYIN_T9_MAP[c - 'A'];
    } else if (c >= 'a' && c <= 'z') {
        return PINYIN_T9_MAP[c - 'a'];
    } else if (isValidT9Key(c)) {
        return c;
    }
    return '\0';
}

std::string T9Utils::stringToT9Key(const std::tring& input) {
    std::ostringstream sb;
    char cLast = ' ';
    const int LEN = input.length();
    for (int i = 0; i < LEN; i++) {
        char cSrc = input.charAt(i);
        char t9c = formatCharToT9(cSrc);

        if (t9c == 0) t9c = ' ';
        else if (Character.isUpperCase(cSrc) || i == 0
               || (Character.isLetter(cSrc) && !Character.isLetter(cLast)))
            t9c = convertDigitToInitial(t9c);
        else if (Character.isDigit(cSrc) && !Character.isDigit(cLast))
            t9c = convertDigitToInitial(t9c);
        else if (isValidT9Key(cSrc))
            t9c = convertDigitToInitial(t9c);
            sb<<t9c;
            cLast = cSrc;
    }
    return sb.str();
}
