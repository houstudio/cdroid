#ifndef __CHARACTER_H__
#define __CHARACTER_H__
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <initializer_list>
namespace cdroid{
#define native 
class CharSequence;
using String=std::string;
class Character{
public:
    static constexpr int MIN_RADIX = 2;
    static constexpr int MAX_RADIX = 36;
    static constexpr char16_t MIN_VALUE = 0x0000;
    static constexpr char16_t MAX_VALUE = 0xFFFF;
    static constexpr uint8_t UNASSIGNED = 0;
    static constexpr uint8_t UPPERCASE_LETTER = 1;
    static constexpr uint8_t LOWERCASE_LETTER = 2;
    static constexpr uint8_t TITLECASE_LETTER = 3;
    static constexpr uint8_t MODIFIER_LETTER = 4;
    static constexpr uint8_t OTHER_LETTER = 5;
    static constexpr uint8_t NON_SPACING_MARK = 6;
    static constexpr uint8_t ENCLOSING_MARK = 7;
    static constexpr uint8_t COMBINING_SPACING_MARK = 8;
    static constexpr uint8_t DECIMAL_DIGIT_NUMBER = 9;
    static constexpr uint8_t LETTER_NUMBER = 10;

    static constexpr uint8_t OTHER_NUMBER = 11;
    static constexpr uint8_t SPACE_SEPARATOR = 12;
    static constexpr uint8_t LINE_SEPARATOR = 13;
    static constexpr uint8_t PARAGRAPH_SEPARATOR = 14;
    static constexpr uint8_t CONTROL = 15;
    static constexpr uint8_t FORMAT = 16;
    static constexpr uint8_t PRIVATE_USE = 18;
    static constexpr uint8_t SURROGATE = 19;
    static constexpr uint8_t DASH_PUNCTUATION = 20;
    static constexpr uint8_t START_PUNCTUATION = 21;
    static constexpr uint8_t END_PUNCTUATION = 22;
    static constexpr uint8_t CONNECTOR_PUNCTUATION = 23;
    static constexpr uint8_t OTHER_PUNCTUATION = 24;
    static constexpr uint8_t MATH_SYMBOL = 25;
    static constexpr uint8_t CURRENCY_SYMBOL = 26;
    static constexpr uint8_t MODIFIER_SYMBOL = 27;
    static constexpr uint8_t OTHER_SYMBOL = 28;
    static constexpr uint8_t INITIAL_QUOTE_PUNCTUATION = 29;
    static constexpr uint8_t FINAL_QUOTE_PUNCTUATION = 30;

    static constexpr int ERROR = 0xFFFFFFFF;

    static constexpr uint8_t DIRECTIONALITY_UNDEFINED = -1;
    static constexpr uint8_t DIRECTIONALITY_LEFT_TO_RIGHT = 0;
    static constexpr uint8_t DIRECTIONALITY_RIGHT_TO_LEFT = 1;
    static constexpr uint8_t DIRECTIONALITY_RIGHT_TO_LEFT_ARABIC = 2;
    static constexpr uint8_t DIRECTIONALITY_EUROPEAN_NUMBER = 3;
    static constexpr uint8_t DIRECTIONALITY_EUROPEAN_NUMBER_SEPARATOR = 4;
    static constexpr uint8_t DIRECTIONALITY_EUROPEAN_NUMBER_TERMINATOR = 5;
    static constexpr uint8_t DIRECTIONALITY_ARABIC_NUMBER = 6;
    static constexpr uint8_t DIRECTIONALITY_COMMON_NUMBER_SEPARATOR = 7;
    static constexpr uint8_t DIRECTIONALITY_NONSPACING_MARK = 8;
    static constexpr uint8_t DIRECTIONALITY_BOUNDARY_NEUTRAL = 9;
    static constexpr uint8_t DIRECTIONALITY_PARAGRAPH_SEPARATOR = 10;
    static constexpr uint8_t DIRECTIONALITY_SEGMENT_SEPARATOR = 11;
    static constexpr uint8_t DIRECTIONALITY_WHITESPACE = 12;
    static constexpr uint8_t DIRECTIONALITY_OTHER_NEUTRALS = 13;
    static constexpr uint8_t DIRECTIONALITY_LEFT_TO_RIGHT_EMBEDDING = 14;
    static constexpr uint8_t DIRECTIONALITY_LEFT_TO_RIGHT_OVERRIDE = 15;
    static constexpr uint8_t DIRECTIONALITY_RIGHT_TO_LEFT_EMBEDDING = 16;
    static constexpr uint8_t DIRECTIONALITY_RIGHT_TO_LEFT_OVERRIDE = 17;
    static constexpr uint8_t DIRECTIONALITY_POP_DIRECTIONAL_FORMAT = 18;
    static constexpr uint8_t DIRECTIONALITY_LEFT_TO_RIGHT_ISOLATE = 19;
    static constexpr uint8_t DIRECTIONALITY_RIGHT_TO_LEFT_ISOLATE = 20;
    static constexpr uint8_t DIRECTIONALITY_FIRST_STRONG_ISOLATE = 21;
    static constexpr uint8_t DIRECTIONALITY_POP_DIRECTIONAL_ISOLATE = 22;
    static constexpr char16_t MIN_HIGH_SURROGATE = 0xD800;
    static constexpr char16_t MAX_HIGH_SURROGATE = 0xDBFF;
    static constexpr char16_t MIN_LOW_SURROGATE  = 0xDC00;
    static constexpr char16_t MAX_LOW_SURROGATE  = 0xDFFF;
    static constexpr char16_t MIN_SURROGATE = MIN_HIGH_SURROGATE;
    static constexpr char16_t MAX_SURROGATE = MAX_LOW_SURROGATE;
    static constexpr int MIN_SUPPLEMENTARY_CODE_POINT = 0x010000;
    static constexpr int MIN_CODE_POINT = 0x000000;
    static constexpr int MAX_CODE_POINT = 0X10FFFF;

    // BEGIN Android-added: Use ICU.
    // The indices in int[] DIRECTIONALITY are based on icu4c's u_char16_tDirection(),
    // accessed via getDirectionalityImpl(), implemented in Character.cpp.
private: static const uint8_t DIRECTIONALITY[];
    // END Android-added: Use ICU.

public:
    class Subset  {
    private:
        std::string name;
    protected:
        Subset(const std::string& name) {
            if (name.empty()) {
                //throw new NullPointerException("name");
            }
            this->name = name;
        }
    public:
        int hashCode() const{
            return name.size();
        }
        std::string toString() const{
            return name;
        }
    };
    // See http://www.unicode.org/Public/UNIDATA/Blocks.txt
    // for the latest specification of Unicode Blocks.
public:
    class UnicodeBlock:public Subset {
    private:
        static constexpr int NUM_ENTITIES = 684;
        static std::unordered_map<std::string,const UnicodeBlock*> map;
        UnicodeBlock(const std::string& idName);
        // BEGIN Android-added: ICU consistency: Don't map deprecated SURROGATES_AREA. b/26140229
        // Add a (String, bool) constructor for use by SURROGATES_AREA.
        UnicodeBlock(const std::string& idName, bool isMap);
        // END Android-added: ICU consistency: Don't map deprecated SURROGATES_AREA. b/26140229
        UnicodeBlock(const std::string& idName, const std::string& alias);
        UnicodeBlock(const std::string& idName,const std::initializer_list<const std::string>& aliases);
    public:
        static const UnicodeBlock BASIC_LATIN;
        static const UnicodeBlock LATIN_1_SUPPLEMENT;
        static const UnicodeBlock LATIN_EXTENDED_A;
        static const UnicodeBlock LATIN_EXTENDED_B;
        static const UnicodeBlock IPA_EXTENSIONS;
        static const UnicodeBlock SPACING_MODIFIER_LETTERS;
        static const UnicodeBlock COMBINING_DIACRITICAL_MARKS;
        static const UnicodeBlock GREEK;
        static const UnicodeBlock CYRILLIC;
        static const UnicodeBlock ARMENIAN;
        static const UnicodeBlock HEBREW;
        static const UnicodeBlock ARABIC;
        static const UnicodeBlock DEVANAGARI;
        static const UnicodeBlock BENGALI;
        static const UnicodeBlock GURMUKHI;
        static const UnicodeBlock GUJARATI;
        static const UnicodeBlock ORIYA;
        static const UnicodeBlock TAMIL;
        static const UnicodeBlock TELUGU;
        static const UnicodeBlock KANNADA;
        static const UnicodeBlock MALAYALAM;
        static const UnicodeBlock THAI;
        static const UnicodeBlock LAO;
        static const UnicodeBlock TIBETAN;
        static const UnicodeBlock GEORGIAN;
        static const UnicodeBlock HANGUL_JAMO;
        static const UnicodeBlock LATIN_EXTENDED_ADDITIONAL;
        static const UnicodeBlock GREEK_EXTENDED;
        static const UnicodeBlock GENERAL_PUNCTUATION;
        static const UnicodeBlock SUPERSCRIPTS_AND_SUBSCRIPTS;
        static const UnicodeBlock CURRENCY_SYMBOLS;
        static const UnicodeBlock COMBINING_MARKS_FOR_SYMBOLS;
        static const UnicodeBlock LETTERLIKE_SYMBOLS;
        static const UnicodeBlock NUMBER_FORMS;
        static const UnicodeBlock ARROWS;
        static const UnicodeBlock MATHEMATICAL_OPERATORS;
        static const UnicodeBlock MISCELLANEOUS_TECHNICAL;
        static const UnicodeBlock CONTROL_PICTURES;
        static const UnicodeBlock OPTICAL_CHARACTER_RECOGNITION;
        static const UnicodeBlock ENCLOSED_ALPHANUMERICS;
        static const UnicodeBlock BOX_DRAWING;
        static const UnicodeBlock BLOCK_ELEMENTS;
        static const UnicodeBlock GEOMETRIC_SHAPES;
        static const UnicodeBlock MISCELLANEOUS_SYMBOLS;
        static const UnicodeBlock DINGBATS;
        static const UnicodeBlock CJK_SYMBOLS_AND_PUNCTUATION;
        static const UnicodeBlock HIRAGANA;
        static const UnicodeBlock KATAKANA;
        static const UnicodeBlock BOPOMOFO;
        static const UnicodeBlock HANGUL_COMPATIBILITY_JAMO;
        static const UnicodeBlock KANBUN;
        static const UnicodeBlock ENCLOSED_CJK_LETTERS_AND_MONTHS;
        static const UnicodeBlock CJK_COMPATIBILITY;
        static const UnicodeBlock CJK_UNIFIED_IDEOGRAPHS;
        static const UnicodeBlock HANGUL_SYLLABLES;
        static const UnicodeBlock PRIVATE_USE_AREA;
        static const UnicodeBlock CJK_COMPATIBILITY_IDEOGRAPHS;
        static const UnicodeBlock ALPHABETIC_PRESENTATION_FORMS;
        static const UnicodeBlock ARABIC_PRESENTATION_FORMS_A;
        static const UnicodeBlock COMBINING_HALF_MARKS;
        static const UnicodeBlock CJK_COMPATIBILITY_FORMS;
        static const UnicodeBlock SMALL_FORM_VARIANTS;
        static const UnicodeBlock ARABIC_PRESENTATION_FORMS_B;
        static const UnicodeBlock HALFWIDTH_AND_FULLWIDTH_FORMS;
        static const UnicodeBlock SPECIALS;
        static const UnicodeBlock SURROGATES_AREA;
        static const UnicodeBlock SYRIAC;
        static const UnicodeBlock THAANA;
        static const UnicodeBlock SINHALA;
        static const UnicodeBlock MYANMAR;
        static const UnicodeBlock ETHIOPIC;
        static const UnicodeBlock CHEROKEE;
        static const UnicodeBlock UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS;
        static const UnicodeBlock OGHAM;
        static const UnicodeBlock RUNIC;
        static const UnicodeBlock KHMER;
        static const UnicodeBlock MONGOLIAN;
        static const UnicodeBlock BRAILLE_PATTERNS;
        static const UnicodeBlock CJK_RADICALS_SUPPLEMENT;
        static const UnicodeBlock KANGXI_RADICALS;
        static const UnicodeBlock IDEOGRAPHIC_DESCRIPTION_CHARACTERS;
        static const UnicodeBlock BOPOMOFO_EXTENDED;
        static const UnicodeBlock CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A;
        static const UnicodeBlock YI_SYLLABLES;
        static const UnicodeBlock YI_RADICALS;
        static const UnicodeBlock CYRILLIC_SUPPLEMENTARY;
        static const UnicodeBlock TAGALOG;
        static const UnicodeBlock HANUNOO;
        static const UnicodeBlock BUHID;
        static const UnicodeBlock TAGBANWA;
        static const UnicodeBlock LIMBU;
        static const UnicodeBlock TAI_LE;
        static const UnicodeBlock KHMER_SYMBOLS;
        static const UnicodeBlock PHONETIC_EXTENSIONS;
        static const UnicodeBlock MISCELLANEOUS_MATHEMATICAL_SYMBOLS_A;
        static const UnicodeBlock SUPPLEMENTAL_ARROWS_A;
        static const UnicodeBlock SUPPLEMENTAL_ARROWS_B;
        static const UnicodeBlock MISCELLANEOUS_MATHEMATICAL_SYMBOLS_B;
        static const UnicodeBlock SUPPLEMENTAL_MATHEMATICAL_OPERATORS;
        static const UnicodeBlock MISCELLANEOUS_SYMBOLS_AND_ARROWS;
        static const UnicodeBlock KATAKANA_PHONETIC_EXTENSIONS;
        static const UnicodeBlock YIJING_HEXAGRAM_SYMBOLS;
        static const UnicodeBlock VARIATION_SELECTORS;
        static const UnicodeBlock LINEAR_B_SYLLABARY;
        static const UnicodeBlock LINEAR_B_IDEOGRAMS;
        static const UnicodeBlock AEGEAN_NUMBERS;
        static const UnicodeBlock OLD_ITALIC;
        static const UnicodeBlock GOTHIC;
        static const UnicodeBlock UGARITIC;
        static const UnicodeBlock DESERET;
        static const UnicodeBlock SHAVIAN;
        static const UnicodeBlock OSMANYA;
        static const UnicodeBlock CYPRIOT_SYLLABARY;
        static const UnicodeBlock BYZANTINE_MUSICAL_SYMBOLS;
        static const UnicodeBlock MUSICAL_SYMBOLS;
        static const UnicodeBlock TAI_XUAN_JING_SYMBOLS;
        static const UnicodeBlock MATHEMATICAL_ALPHANUMERIC_SYMBOLS;
        static const UnicodeBlock CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B;
        static const UnicodeBlock CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT;
        static const UnicodeBlock TAGS;
        static const UnicodeBlock VARIATION_SELECTORS_SUPPLEMENT;
        static const UnicodeBlock SUPPLEMENTARY_PRIVATE_USE_AREA_A;
        static const UnicodeBlock SUPPLEMENTARY_PRIVATE_USE_AREA_B;
        static const UnicodeBlock HIGH_SURROGATES;
        static const UnicodeBlock HIGH_PRIVATE_USE_SURROGATES;
        static const UnicodeBlock LOW_SURROGATES;
        static const UnicodeBlock ARABIC_SUPPLEMENT;
        static const UnicodeBlock NKO;
        static const UnicodeBlock SAMARITAN;
        static const UnicodeBlock MANDAIC;
        static const UnicodeBlock ETHIOPIC_SUPPLEMENT;
        static const UnicodeBlock UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_EXTENDED;
        static const UnicodeBlock NEW_TAI_LUE;
        static const UnicodeBlock BUGINESE;
        static const UnicodeBlock TAI_THAM;
        static const UnicodeBlock BALINESE;
        static const UnicodeBlock SUNDANESE;
        static const UnicodeBlock BATAK;
        static const UnicodeBlock LEPCHA;
        static const UnicodeBlock OL_CHIKI;
        static const UnicodeBlock VEDIC_EXTENSIONS;
        static const UnicodeBlock PHONETIC_EXTENSIONS_SUPPLEMENT;
        static const UnicodeBlock COMBINING_DIACRITICAL_MARKS_SUPPLEMENT;
        static const UnicodeBlock GLAGOLITIC;
        static const UnicodeBlock LATIN_EXTENDED_C;
        static const UnicodeBlock COPTIC;
        static const UnicodeBlock GEORGIAN_SUPPLEMENT;
        static const UnicodeBlock TIFINAGH;
        static const UnicodeBlock ETHIOPIC_EXTENDED;
        static const UnicodeBlock CYRILLIC_EXTENDED_A;
        static const UnicodeBlock SUPPLEMENTAL_PUNCTUATION;
        static const UnicodeBlock CJK_STROKES;
        static const UnicodeBlock LISU;
        static const UnicodeBlock VAI;
        static const UnicodeBlock CYRILLIC_EXTENDED_B;
        static const UnicodeBlock BAMUM;
        static const UnicodeBlock MODIFIER_TONE_LETTERS;
        static const UnicodeBlock LATIN_EXTENDED_D;
        static const UnicodeBlock SYLOTI_NAGRI;
        static const UnicodeBlock COMMON_INDIC_NUMBER_FORMS;
        static const UnicodeBlock PHAGS_PA;
        static const UnicodeBlock SAURASHTRA;
        static const UnicodeBlock DEVANAGARI_EXTENDED;
        static const UnicodeBlock KAYAH_LI;
        static const UnicodeBlock REJANG;
        static const UnicodeBlock HANGUL_JAMO_EXTENDED_A;
        static const UnicodeBlock JAVANESE;
        static const UnicodeBlock CHAM;
        static const UnicodeBlock MYANMAR_EXTENDED_A;
        static const UnicodeBlock TAI_VIET;
        static const UnicodeBlock ETHIOPIC_EXTENDED_A;
        static const UnicodeBlock MEETEI_MAYEK;
        static const UnicodeBlock HANGUL_JAMO_EXTENDED_B;
        static const UnicodeBlock VERTICAL_FORMS;
        static const UnicodeBlock ANCIENT_GREEK_NUMBERS;
        static const UnicodeBlock ANCIENT_SYMBOLS;
        static const UnicodeBlock PHAISTOS_DISC;
        static const UnicodeBlock LYCIAN;
        static const UnicodeBlock CARIAN;
        static const UnicodeBlock OLD_PERSIAN;
        static const UnicodeBlock IMPERIAL_ARAMAIC;
        static const UnicodeBlock PHOENICIAN;
        static const UnicodeBlock LYDIAN;
        static const UnicodeBlock KHAROSHTHI;
        static const UnicodeBlock OLD_SOUTH_ARABIAN;
        static const UnicodeBlock AVESTAN;
        static const UnicodeBlock INSCRIPTIONAL_PARTHIAN;
        static const UnicodeBlock INSCRIPTIONAL_PAHLAVI;
        static const UnicodeBlock OLD_TURKIC;
        static const UnicodeBlock RUMI_NUMERAL_SYMBOLS;
        static const UnicodeBlock BRAHMI;
        static const UnicodeBlock KAITHI;
        static const UnicodeBlock CUNEIFORM;
        static const UnicodeBlock CUNEIFORM_NUMBERS_AND_PUNCTUATION;
        static const UnicodeBlock EGYPTIAN_HIEROGLYPHS;
        static const UnicodeBlock BAMUM_SUPPLEMENT;
        static const UnicodeBlock KANA_SUPPLEMENT;
        static const UnicodeBlock ANCIENT_GREEK_MUSICAL_NOTATION;
        static const UnicodeBlock COUNTING_ROD_NUMERALS;
        static const UnicodeBlock MAHJONG_TILES;
        static const UnicodeBlock DOMINO_TILES;
        static const UnicodeBlock PLAYING_CARDS;
        static const UnicodeBlock ENCLOSED_ALPHANUMERIC_SUPPLEMENT;
        static const UnicodeBlock ENCLOSED_IDEOGRAPHIC_SUPPLEMENT;
        static const UnicodeBlock MISCELLANEOUS_SYMBOLS_AND_PICTOGRAPHS;
        static const UnicodeBlock EMOTICONS;
        static const UnicodeBlock TRANSPORT_AND_MAP_SYMBOLS;
        static const UnicodeBlock ALCHEMICAL_SYMBOLS;
        static const UnicodeBlock CJK_UNIFIED_IDEOGRAPHS_EXTENSION_C;
        static const UnicodeBlock CJK_UNIFIED_IDEOGRAPHS_EXTENSION_D;
        static const UnicodeBlock ARABIC_EXTENDED_A;
        static const UnicodeBlock SUNDANESE_SUPPLEMENT;
        static const UnicodeBlock MEETEI_MAYEK_EXTENSIONS;
        static const UnicodeBlock MEROITIC_HIEROGLYPHS;
        static const UnicodeBlock MEROITIC_CURSIVE;
        static const UnicodeBlock SORA_SOMPENG;
        static const UnicodeBlock CHAKMA;
        static const UnicodeBlock SHARADA;
        static const UnicodeBlock TAKRI;
        static const UnicodeBlock MIAO;
        static const UnicodeBlock ARABIC_MATHEMATICAL_ALPHABETIC_SYMBOLS;
        static const UnicodeBlock COMBINING_DIACRITICAL_MARKS_EXTENDED;
        static const UnicodeBlock MYANMAR_EXTENDED_B;
        static const UnicodeBlock LATIN_EXTENDED_E;
        static const UnicodeBlock COPTIC_EPACT_NUMBERS;
        static const UnicodeBlock OLD_PERMIC;
        static const UnicodeBlock ELBASAN;
        static const UnicodeBlock CAUCASIAN_ALBANIAN;
        static const UnicodeBlock LINEAR_A;
        static const UnicodeBlock PALMYRENE;
        static const UnicodeBlock NABATAEAN;
        static const UnicodeBlock OLD_NORTH_ARABIAN;
        static const UnicodeBlock MANICHAEAN;
        static const UnicodeBlock PSALTER_PAHLAVI;
        static const UnicodeBlock MAHAJANI;
        static const UnicodeBlock SINHALA_ARCHAIC_NUMBERS;
        static const UnicodeBlock KHOJKI;
        static const UnicodeBlock KHUDAWADI;
        static const UnicodeBlock GRANTHA;
        static const UnicodeBlock TIRHUTA;
        static const UnicodeBlock SIDDHAM;
        static const UnicodeBlock MODI;
        static const UnicodeBlock WARANG_CITI;
        static const UnicodeBlock PAU_CIN_HAU;
        static const UnicodeBlock MRO;
        static const UnicodeBlock BASSA_VAH;
        static const UnicodeBlock PAHAWH_HMONG;
        static const UnicodeBlock DUPLOYAN;
        static const UnicodeBlock SHORTHAND_FORMAT_CONTROLS;
        static const UnicodeBlock MENDE_KIKAKUI;
        static const UnicodeBlock ORNAMENTAL_DINGBATS;
        static const UnicodeBlock GEOMETRIC_SHAPES_EXTENDED;
        static const UnicodeBlock SUPPLEMENTAL_ARROWS_C;
        static const UnicodeBlock CHEROKEE_SUPPLEMENT;
        static const UnicodeBlock HATRAN;
        static const UnicodeBlock OLD_HUNGARIAN;
        static const UnicodeBlock MULTANI;
        static const UnicodeBlock AHOM;
        static const UnicodeBlock EARLY_DYNASTIC_CUNEIFORM;
        static const UnicodeBlock ANATOLIAN_HIEROGLYPHS;
        static const UnicodeBlock SUTTON_SIGNWRITING;
        static const UnicodeBlock SUPPLEMENTAL_SYMBOLS_AND_PICTOGRAPHS;
        static const UnicodeBlock CJK_UNIFIED_IDEOGRAPHS_EXTENSION_E;
        static const UnicodeBlock SYRIAC_SUPPLEMENT;
        static const UnicodeBlock CYRILLIC_EXTENDED_C;
        static const UnicodeBlock OSAGE;
        static const UnicodeBlock NEWA;
        static const UnicodeBlock MONGOLIAN_SUPPLEMENT;
        static const UnicodeBlock MARCHEN;
        static const UnicodeBlock IDEOGRAPHIC_SYMBOLS_AND_PUNCTUATION;
        static const UnicodeBlock TANGUT;
        static const UnicodeBlock TANGUT_COMPONENTS;
        static const UnicodeBlock KANA_EXTENDED_A;
        static const UnicodeBlock GLAGOLITIC_SUPPLEMENT;
        static const UnicodeBlock ADLAM;
        static const UnicodeBlock MASARAM_GONDI;
        static const UnicodeBlock ZANABAZAR_SQUARE;
        static const UnicodeBlock NUSHU;
        static const UnicodeBlock SOYOMBO;
        static const UnicodeBlock BHAIKSUKI;
        static const UnicodeBlock CJK_UNIFIED_IDEOGRAPHS_EXTENSION_F;
        static const UnicodeBlock GEORGIAN_EXTENDED;
        static const UnicodeBlock HANIFI_ROHINGYA;
        static const UnicodeBlock OLD_SOGDIAN;
        static const UnicodeBlock SOGDIAN;
        static const UnicodeBlock DOGRA;
        static const UnicodeBlock GUNJALA_GONDI;
        static const UnicodeBlock MAKASAR;
        static const UnicodeBlock MEDEFAIDRIN;
        static const UnicodeBlock MAYAN_NUMERALS;
        static const UnicodeBlock INDIC_SIYAQ_NUMBERS;
        static const UnicodeBlock CHESS_SYMBOLS;
        static const UnicodeBlock ELYMAIC;
        static const UnicodeBlock NANDINAGARI;
        static const UnicodeBlock TAMIL_SUPPLEMENT;
        static const UnicodeBlock EGYPTIAN_HIEROGLYPH_FORMAT_CONTROLS;
        static const UnicodeBlock SMALL_KANA_EXTENSION;
        static const UnicodeBlock NYIAKENG_PUACHUE_HMONG;
        static const UnicodeBlock WANCHO;
        static const UnicodeBlock OTTOMAN_SIYAQ_NUMBERS;
        static const UnicodeBlock SYMBOLS_AND_PICTOGRAPHS_EXTENDED_A;
        static const UnicodeBlock YEZIDI;
        static const UnicodeBlock CHORASMIAN;
        static const UnicodeBlock DIVES_AKURU;
        static const UnicodeBlock LISU_SUPPLEMENT;
        static const UnicodeBlock KHITAN_SMALL_SCRIPT;
        static const UnicodeBlock TANGUT_SUPPLEMENT;
        static const UnicodeBlock SYMBOLS_FOR_LEGACY_COMPUTING;
        static const UnicodeBlock CJK_UNIFIED_IDEOGRAPHS_EXTENSION_G;
    private:
        static const int blockStarts[];
        static const UnicodeBlock* blocks[];
    public:
        static const UnicodeBlock* of(char16_t c);
        static const UnicodeBlock* of(int codePoint);
        static const UnicodeBlock* forName(const std::string& blockName);
    };

public:
    class UnicodeScript {
    public:
    enum Script{
        COMMON,
        LATIN,
        GREEK,
        CYRILLIC,
        ARMENIAN,
        HEBREW,
        ARABIC,
        SYRIAC,
        THAANA,
        DEVANAGARI,
        BENGALI,
        GURMUKHI,
        GUJARATI,
        ORIYA,
        TAMIL,
        TELUGU,
        KANNADA,
        MALAYALAM,
        SINHALA,
        THAI,
        LAO,
        TIBETAN,
        MYANMAR,
        GEORGIAN,
        HANGUL,
        ETHIOPIC,
        CHEROKEE,
        CANADIAN_ABORIGINAL,
        OGHAM,
        RUNIC,
        KHMER,
        MONGOLIAN,
        HIRAGANA,
        KATAKANA,
        BOPOMOFO,
        HAN,
        YI,
        OLD_ITALIC,
        GOTHIC,
        DESERET,
        INHERITED,
        TAGALOG,
        HANUNOO,
        BUHID,
        TAGBANWA,
        LIMBU,
        TAI_LE,
        LINEAR_B,
        UGARITIC,
        SHAVIAN,
        OSMANYA,
        CYPRIOT,
        BRAILLE,
        BUGINESE,
        COPTIC,
        NEW_TAI_LUE,
        GLAGOLITIC,
        TIFINAGH,
        SYLOTI_NAGRI,
        OLD_PERSIAN,
        KHAROSHTHI,
        BALINESE,
        CUNEIFORM,
        PHOENICIAN,
        PHAGS_PA,
        NKO,
        SUNDANESE,
        BATAK,
        LEPCHA,
        OL_CHIKI,
        VAI,
        SAURASHTRA,
        KAYAH_LI,
        REJANG,
        LYCIAN,
        CARIAN,
        LYDIAN,
        CHAM,
        TAI_THAM,
        TAI_VIET,
        AVESTAN,
        EGYPTIAN_HIEROGLYPHS,
        SAMARITAN,
        MANDAIC,
        LISU,
        BAMUM,
        JAVANESE,
        MEETEI_MAYEK,
        IMPERIAL_ARAMAIC,
        OLD_SOUTH_ARABIAN,
        INSCRIPTIONAL_PARTHIAN,
        INSCRIPTIONAL_PAHLAVI,
        OLD_TURKIC,
        BRAHMI,
        KAITHI,
        MEROITIC_HIEROGLYPHS,
        MEROITIC_CURSIVE,
        SORA_SOMPENG,
        CHAKMA,
        SHARADA,
        TAKRI,
        MIAO,
        CAUCASIAN_ALBANIAN,
        BASSA_VAH,
        DUPLOYAN,
        ELBASAN,
        GRANTHA,
        PAHAWH_HMONG,
        KHOJKI,
        LINEAR_A,
        MAHAJANI,
        MANICHAEAN,
        MENDE_KIKAKUI,
        MODI,
        MRO,
        OLD_NORTH_ARABIAN,
        NABATAEAN,
        PALMYRENE,
        PAU_CIN_HAU,
        OLD_PERMIC,
        PSALTER_PAHLAVI,
        SIDDHAM,
        KHUDAWADI,
        TIRHUTA,
        WARANG_CITI,
        AHOM,
        ANATOLIAN_HIEROGLYPHS,
        HATRAN,
        MULTANI,
        OLD_HUNGARIAN,
        SIGNWRITING,
        ADLAM,
        BHAIKSUKI,
        MARCHEN,
        NEWA,
        OSAGE,
        TANGUT,
        MASARAM_GONDI,
        NUSHU,
        SOYOMBO,
        ZANABAZAR_SQUARE,
        HANIFI_ROHINGYA,
        OLD_SOGDIAN,
        SOGDIAN,
        DOGRA,
        GUNJALA_GONDI,
        MAKASAR,
        MEDEFAIDRIN,
        ELYMAIC,
        NANDINAGARI,
        NYIAKENG_PUACHUE_HMONG,
        WANCHO,
        YEZIDI,
        CHORASMIAN,
        DIVES_AKURU,
        KHITAN_SMALL_SCRIPT,
        UNKNOWN
    };
    private:
        static const int scriptStarts[];
        static const int scripts[];
        static const std::unordered_map<std::string, int> aliases;
    public:
        static int of(int codePoint);
        static int forName(const std::string& scriptName);
    };

private:
    char16_t value;
public:
    Character(char16_t value) {
        this->value = value;
    }
private:
    /*class CharacterCache {
    private:CharacterCache(){}
        static Character cache[];
        static Character archivedCache[];
    };*/
public:
    static Character* valueOf(char16_t c);
    char16_t charValue() const{
        return value;
    }

    static int hashCode(char16_t value) {
        return (int)value;
    }

    /*String toString() {
        return String.valueOf(value);
    }

    static String toString(char16_t c) {
        return String.valueOf(c);
    }

    static String toString(int codePoint) {
        return String.valueOfCodePoint(codePoint);
    }*/

    static bool isValidCodePoint(int codePoint);
    static bool isBmpCodePoint(int codePoint);
    static bool isSupplementaryCodePoint(int codePoint);
    static bool isHighSurrogate(char16_t ch);
    static bool isLowSurrogate(char16_t ch);
    static bool isSurrogate(char16_t ch);
    static bool isSurrogatePair(char16_t high, char16_t low);
    static int charCount(int codePoint);
    static int toCodePoint(char16_t high, char16_t low);

    static int codePointAt(const CharSequence* seq, int index);

    static int codePointAt(const char16_t* a, int index);
    // throws ArrayIndexOutOfBoundsException if index out of bounds
    static int codePointAtImpl(const char16_t* a, int index, int limit);
    static int codePointBefore(const CharSequence* seq, int index);

    static int codePointBefore(const char16_t* a, int index);

    static int codePointBefore(const char16_t* a, int index, int start);

    // throws ArrayIndexOutOfBoundsException if index-1 out of bounds
    static int codePointBeforeImpl(const char16_t* a, int index, int start);

    static char16_t highSurrogate(int codePoint);

    static char16_t lowSurrogate(int codePoint);
    static int toChars(int codePoint, char16_t* dst, int dstIndex);

    static void toSurrogates(int codePoint, char16_t* dst, int index);

    static int codePointCount(const CharSequence* seq, int beginIndex, int endIndex);
    static int codePointCount(const char16_t* a, int offset, int count);
    static int codePointCountImpl(const char16_t* a, int offset, int count);

    static int offsetByCodePoints(const CharSequence* seq, int index, int codePointOffset);
    static int offsetByCodePoints(const char16_t* a, int start, int count, int index, int codePointOffset);
    static int offsetByCodePointsImpl(const char16_t* a, int start, int count, int index, int codePointOffset);

    static bool isLowerCase(char16_t ch);
    static bool isLowerCase(int codePoint);
    static native bool isLowerCaseImpl(int codePoint);
    // END Android-changed: Reimplement methods natively on top of ICU4C.

    static bool isUpperCase(char16_t ch);
    static bool isUpperCase(int codePoint);
    static native bool isUpperCaseImpl(int codePoint);
    // END Android-changed: Reimplement methods natively on top of ICU4C.

    static bool isTitleCase(char16_t ch);
    static bool isTitleCase(int codePoint);
    static native bool isTitleCaseImpl(int codePoint);
    // END Android-changed: Reimplement methods natively on top of ICU4C.

    static bool isDigit(char16_t ch);
    static bool isDigit(int codePoint);
    static native bool isDigitImpl(int codePoint);
    // END Android-changed: Reimplement methods natively on top of ICU4C.

    static bool isDefined(char16_t ch);
    static bool isDefined(int codePoint);
    static native bool isDefinedImpl(int codePoint);
    // END Android-changed: Reimplement methods natively on top of ICU4C.

    static bool isLetter(char16_t ch);
    static bool isLetter(int codePoint);
    static native bool isLetterImpl(int codePoint);
    // END Android-changed: Reimplement methods natively on top of ICU4C.

    static bool isLetterOrDigit(char16_t ch);
    static bool isLetterOrDigit(int codePoint);
    static native bool isLetterOrDigitImpl(int codePoint);
    // END Android-changed: Reimplement methods natively on top of ICU4C.

    static bool isJavaLetter(char16_t ch);
    static bool isJavaLetterOrDigit(char16_t ch);
    static bool isAlphabetic(int codePoint);
    static native bool isAlphabeticImpl(int codePoint);
    // END Android-changed: Reimplement methods natively on top of ICU4C.

    static bool isIdeographic(int codePoint);
    static native bool isIdeographicImpl(int codePoint);
    // END Android-changed: Reimplement methods natively on top of ICU4C.

    // Android-changed: Removed @see tag (target does not exist on Android):
    // @see     javax.lang.model.SourceVersion#isIdentifier(CharSequence)
    static bool isJavaIdentifierStart(char16_t ch);
    // Android-changed: Removed @see tag (target does not exist on Android):
    // @see     javax.lang.model.SourceVersion#isIdentifier(CharSequence)
    static bool isJavaIdentifierStart(int codePoint);
    // END Android-changed: Use ICU.

    // Android-changed: Removed @see tag (target does not exist on Android):
    static bool isJavaIdentifierPart(char16_t ch);
    static bool isJavaIdentifierPart(int codePoint);
    static bool isUnicodeIdentifierStart(char16_t ch);
    static bool isUnicodeIdentifierStart(int codePoint);
    static native bool isUnicodeIdentifierStartImpl(int codePoint);
    // END Android-changed: Reimplement methods natively on top of ICU4C.

    static bool isUnicodeIdentifierPart(char16_t ch);
    static bool isUnicodeIdentifierPart(int codePoint);
    static native bool isUnicodeIdentifierPartImpl(int codePoint);
    // END Android-changed: Reimplement methods natively on top of ICU4C.

    static bool isIdentifierIgnorable(char16_t ch) ;
    static bool isIdentifierIgnorable(int codePoint);
    static native bool isIdentifierIgnorableImpl(int codePoint);
    // END Android-changed: Reimplement methods natively on top of ICU4C.

    static char16_t toLowerCase(char16_t ch);

    static int toLowerCase(int codePoint);

    static native int toLowerCaseImpl(int codePoint);
    // END Android-changed: Reimplement methods natively on top of ICU4C.

    static char16_t toUpperCase(char16_t ch);

    static int toUpperCase(int codePoint);

    static native int toUpperCaseImpl(int codePoint);
    // END Android-changed: Reimplement methods natively on top of ICU4C.

    static char16_t toTitleCase(char16_t ch);

    static int toTitleCase(int codePoint);

    static native int toTitleCaseImpl(int codePoint);
    // END Android-changed: Reimplement methods natively on top of ICU4C.

    static int digit(char16_t ch, int radix);

    static int digit(int codePoint, int radix);
    native static int digitImpl(int codePoint, int radix);
    // END Android-changed: Reimplement methods natively on top of ICU4C.

    static int getNumericValue(char16_t ch);
    static int getNumericValue(int codePoint);
    native static int getNumericValueImpl(int codePoint);
    // END Android-changed: Reimplement methods natively on top of ICU4C.

    static bool isSpace(char16_t ch);
    static bool isSpaceChar(char16_t ch);

    static bool isSpaceChar(int codePoint);

    static native bool isSpaceCharImpl(int codePoint);
    // END Android-changed: Reimplement methods natively on top of ICU4C.

    static bool isWhitespace(char16_t ch) ;
    static bool isWhitespace(int codePoint);
    native static bool isWhitespaceImpl(int codePoint);
    // END Android-changed: Reimplement methods natively on top of ICU4C.

    static bool isISOControl(char16_t ch);
    static bool isISOControl(int codePoint);

    static int getType(char16_t ch);
    static int getType(int codePoint);
    static native int getTypeImpl(int codePoint);
    // END Android-changed: Reimplement methods natively on top of ICU4C.

    static char16_t forDigit(int digit, int radix);

    static uint8_t getDirectionality(char16_t ch);
    static uint8_t getDirectionality(int codePoint);

    native static uint8_t getDirectionalityImpl(int codePoint);
    // END Android-changed: Reimplement methods natively on top of ICU4C.

    static bool isMirrored(char16_t ch);
    static bool isMirrored(int codePoint);
    // END Android-changed: Reimplement methods natively on top of ICU4C.

    int compareTo(const Character* anotherCharacter)const;
    static int compare(char16_t x, char16_t y) {
        return x - y;
    }

    // END Android-removed: Use ICU.

    static char16_t reverseBytes(char16_t ch) {
        return (char16_t) (((ch & 0xFF00) >> 8) | (ch << 8));
    }

    static std::string getName(int codePoint);
    static int codePointOf(const std::string& name);
    // END Android-removed: expose after CharacterName.getCodePoint() is imported.
private:
    // Android-added: Use ICU.
    // Implement getNameImpl() and codePointOfImpl() natively.
    static native String getNameImpl(int codePoint);
    static native int codePointOfImpl(String name);
};
}/*endof namespace*/
#endif/*__CHARACTER_H__*/
