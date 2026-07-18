#include "character.h"
#include <text/character.h>
#include <text/spannablestring.h>
#include <unicode/uchar.h>
namespace cdroid{

constexpr uint8_t Character::DIRECTIONALITY[]{
        DIRECTIONALITY_LEFT_TO_RIGHT, DIRECTIONALITY_RIGHT_TO_LEFT,
        DIRECTIONALITY_EUROPEAN_NUMBER,
        DIRECTIONALITY_EUROPEAN_NUMBER_SEPARATOR,
        DIRECTIONALITY_EUROPEAN_NUMBER_TERMINATOR,
        DIRECTIONALITY_ARABIC_NUMBER,
        DIRECTIONALITY_COMMON_NUMBER_SEPARATOR,
        DIRECTIONALITY_PARAGRAPH_SEPARATOR,
        DIRECTIONALITY_SEGMENT_SEPARATOR, DIRECTIONALITY_WHITESPACE,
        DIRECTIONALITY_OTHER_NEUTRALS,
        DIRECTIONALITY_LEFT_TO_RIGHT_EMBEDDING,
        DIRECTIONALITY_LEFT_TO_RIGHT_OVERRIDE,
        DIRECTIONALITY_RIGHT_TO_LEFT_ARABIC,
        DIRECTIONALITY_RIGHT_TO_LEFT_EMBEDDING,
        DIRECTIONALITY_RIGHT_TO_LEFT_OVERRIDE,
        DIRECTIONALITY_POP_DIRECTIONAL_FORMAT,
        DIRECTIONALITY_NONSPACING_MARK, DIRECTIONALITY_BOUNDARY_NEUTRAL };
// END Android-added: Use ICU.

// See http://www.unicode.org/Public/UNIDATA/Blocks.txt
// for the latest specification of Unicode Blocks.
std::unordered_map<std::string, const Character::UnicodeBlock*>& Character::UnicodeBlock::getMap() {
    static std::unordered_map<std::string, const UnicodeBlock*> map_instance;
    return map_instance;
}

Character::UnicodeBlock::UnicodeBlock(const std::string& idName)
    : Subset(idName) {
    getMap().insert({idName,this});
}

Character::UnicodeBlock::UnicodeBlock(const std::string& idName, bool isMap)
    : Subset(idName) {
    if(isMap)getMap().insert({idName,this});
}

Character::UnicodeBlock::UnicodeBlock(const std::string& idName, const std::string& alias)
    : Subset(idName) {
}

Character::UnicodeBlock::UnicodeBlock(const std::string& idName, const std::initializer_list<const std::string>& aliases)
    : UnicodeBlock(idName) {
    for(auto a:aliases){
        getMap().insert({a,this});
    }
}

const Character::UnicodeBlock  Character::UnicodeBlock::BASIC_LATIN("BASIC_LATIN",{"BASIC LATIN","BASICLATIN"});
const Character::UnicodeBlock Character::UnicodeBlock::LATIN_1_SUPPLEMENT("LATIN_1_SUPPLEMENT",{"LATIN-1 SUPPLEMENT","LATIN-1SUPPLEMENT"});
const Character::UnicodeBlock Character::UnicodeBlock::LATIN_EXTENDED_A("LATIN_EXTENDED_A",{"LATIN EXTENDED-A","LATINEXTENDED-A"});
const Character::UnicodeBlock Character::UnicodeBlock::LATIN_EXTENDED_B(
    "LATIN_EXTENDED_B",
    {"LATIN EXTENDED-B", "LATINEXTENDED-B"}
);

const Character::UnicodeBlock Character::UnicodeBlock::IPA_EXTENSIONS(
    "IPA_EXTENSIONS",
    {"IPA EXTENSIONS", "IPAEXTENSIONS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::SPACING_MODIFIER_LETTERS(
    "SPACING_MODIFIER_LETTERS",
    {"SPACING MODIFIER LETTERS", "SPACINGMODIFIERLETTERS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::COMBINING_DIACRITICAL_MARKS(
    "COMBINING_DIACRITICAL_MARKS",
    {"COMBINING DIACRITICAL MARKS", "COMBININGDIACRITICALMARKS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::GREEK(
    "GREEK",
    {"GREEK AND COPTIC", "GREEKANDCOPTIC"}
);

const Character::UnicodeBlock Character::UnicodeBlock::CYRILLIC("CYRILLIC");
const Character::UnicodeBlock Character::UnicodeBlock::ARMENIAN("ARMENIAN");
const Character::UnicodeBlock Character::UnicodeBlock::HEBREW("HEBREW");
const Character::UnicodeBlock Character::UnicodeBlock::ARABIC("ARABIC");
const Character::UnicodeBlock Character::UnicodeBlock::DEVANAGARI("DEVANAGARI");
const Character::UnicodeBlock Character::UnicodeBlock::BENGALI("BENGALI");
const Character::UnicodeBlock Character::UnicodeBlock::GURMUKHI("GURMUKHI");
const Character::UnicodeBlock Character::UnicodeBlock::GUJARATI("GUJARATI");
const Character::UnicodeBlock Character::UnicodeBlock::ORIYA("ORIYA");
const Character::UnicodeBlock Character::UnicodeBlock::TAMIL("TAMIL");
const Character::UnicodeBlock Character::UnicodeBlock::TELUGU("TELUGU");
const Character::UnicodeBlock Character::UnicodeBlock::KANNADA("KANNADA");
const Character::UnicodeBlock Character::UnicodeBlock::MALAYALAM("MALAYALAM");
const Character::UnicodeBlock Character::UnicodeBlock::THAI("THAI");
const Character::UnicodeBlock Character::UnicodeBlock::LAO("LAO");
const Character::UnicodeBlock Character::UnicodeBlock::TIBETAN("TIBETAN");
const Character::UnicodeBlock Character::UnicodeBlock::GEORGIAN("GEORGIAN");

const Character::UnicodeBlock Character::UnicodeBlock::HANGUL_JAMO(
    "HANGUL_JAMO",
    {"HANGUL JAMO", "HANGULJAMO"}
);

const Character::UnicodeBlock Character::UnicodeBlock::LATIN_EXTENDED_ADDITIONAL(
    "LATIN_EXTENDED_ADDITIONAL",
    {"LATIN EXTENDED ADDITIONAL", "LATINEXTENDEDADDITIONAL"}
);

const Character::UnicodeBlock Character::UnicodeBlock::GREEK_EXTENDED(
    "GREEK_EXTENDED",
    {"GREEK EXTENDED", "GREEKEXTENDED"}
);

const Character::UnicodeBlock Character::UnicodeBlock::GENERAL_PUNCTUATION(
    "GENERAL_PUNCTUATION",
    {"GENERAL PUNCTUATION", "GENERALPUNCTUATION"}
);

const Character::UnicodeBlock Character::UnicodeBlock::SUPERSCRIPTS_AND_SUBSCRIPTS(
    "SUPERSCRIPTS_AND_SUBSCRIPTS",
    {"SUPERSCRIPTS AND SUBSCRIPTS", "SUPERSCRIPTSANDSUBSCRIPTS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::CURRENCY_SYMBOLS(
    "CURRENCY_SYMBOLS",
    {"CURRENCY SYMBOLS", "CURRENCYSYMBOLS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::COMBINING_MARKS_FOR_SYMBOLS(
    "COMBINING_MARKS_FOR_SYMBOLS",
    {"COMBINING DIACRITICAL MARKS FOR SYMBOLS",
     "COMBININGDIACRITICALMARKSFORSYMBOLS",
     "COMBINING MARKS FOR SYMBOLS",
     "COMBININGMARKSFORSYMBOLS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::LETTERLIKE_SYMBOLS(
    "LETTERLIKE_SYMBOLS",
    {"LETTERLIKE SYMBOLS", "LETTERLIKESYMBOLS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::NUMBER_FORMS(
    "NUMBER_FORMS",
    {"NUMBER FORMS", "NUMBERFORMS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::ARROWS("ARROWS");

const Character::UnicodeBlock Character::UnicodeBlock::MATHEMATICAL_OPERATORS(
    "MATHEMATICAL_OPERATORS",
    {"MATHEMATICAL OPERATORS", "MATHEMATICALOPERATORS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::MISCELLANEOUS_TECHNICAL(
    "MISCELLANEOUS_TECHNICAL",
    {"MISCELLANEOUS TECHNICAL", "MISCELLANEOUSTECHNICAL"}
);

const Character::UnicodeBlock Character::UnicodeBlock::CONTROL_PICTURES(
    "CONTROL_PICTURES",
    {"CONTROL PICTURES", "CONTROLPICTURES"}
);

const Character::UnicodeBlock Character::UnicodeBlock::OPTICAL_CHARACTER_RECOGNITION(
    "OPTICAL_CHARACTER_RECOGNITION",
    {"OPTICAL CHARACTER RECOGNITION", "OPTICALCHARACTERRECOGNITION"}
);

const Character::UnicodeBlock Character::UnicodeBlock::ENCLOSED_ALPHANUMERICS(
    "ENCLOSED_ALPHANUMERICS",
    {"ENCLOSED ALPHANUMERICS", "ENCLOSEDALPHANUMERICS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::BOX_DRAWING(
    "BOX_DRAWING",
    {"BOX DRAWING", "BOXDRAWING"}
);

const Character::UnicodeBlock Character::UnicodeBlock::BLOCK_ELEMENTS(
    "BLOCK_ELEMENTS",
    {"BLOCK ELEMENTS", "BLOCKELEMENTS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::GEOMETRIC_SHAPES(
    "GEOMETRIC_SHAPES",
    {"GEOMETRIC SHAPES", "GEOMETRICSHAPES"}
);

const Character::UnicodeBlock Character::UnicodeBlock::MISCELLANEOUS_SYMBOLS(
    "MISCELLANEOUS_SYMBOLS",
    {"MISCELLANEOUS SYMBOLS", "MISCELLANEOUSSYMBOLS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::DINGBATS("DINGBATS");

const Character::UnicodeBlock Character::UnicodeBlock::CJK_SYMBOLS_AND_PUNCTUATION(
    "CJK_SYMBOLS_AND_PUNCTUATION",
    {"CJK SYMBOLS AND PUNCTUATION", "CJKSYMBOLSANDPUNCTUATION"}
);

const Character::UnicodeBlock Character::UnicodeBlock::HIRAGANA("HIRAGANA");
const Character::UnicodeBlock Character::UnicodeBlock::KATAKANA("KATAKANA");
const Character::UnicodeBlock Character::UnicodeBlock::BOPOMOFO("BOPOMOFO");

const Character::UnicodeBlock Character::UnicodeBlock::HANGUL_COMPATIBILITY_JAMO(
    "HANGUL_COMPATIBILITY_JAMO",
    {"HANGUL COMPATIBILITY JAMO", "HANGULCOMPATIBILITYJAMO"}
);

const Character::UnicodeBlock Character::UnicodeBlock::KANBUN("KANBUN");

const Character::UnicodeBlock Character::UnicodeBlock::ENCLOSED_CJK_LETTERS_AND_MONTHS(
    "ENCLOSED_CJK_LETTERS_AND_MONTHS",
    {"ENCLOSED CJK LETTERS AND MONTHS", "ENCLOSEDCJKLETTERSANDMONTHS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::CJK_COMPATIBILITY(
    "CJK_COMPATIBILITY",
    {"CJK COMPATIBILITY", "CJKCOMPATIBILITY"}
);

const Character::UnicodeBlock Character::UnicodeBlock::CJK_UNIFIED_IDEOGRAPHS(
    "CJK_UNIFIED_IDEOGRAPHS",
    {"CJK UNIFIED IDEOGRAPHS", "CJKUNIFIEDIDEOGRAPHS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::HANGUL_SYLLABLES(
    "HANGUL_SYLLABLES",
    {"HANGUL SYLLABLES", "HANGULSYLLABLES"}
);

const Character::UnicodeBlock Character::UnicodeBlock::PRIVATE_USE_AREA(
    "PRIVATE_USE_AREA",
    {"PRIVATE USE AREA", "PRIVATEUSEAREA"}
);

const Character::UnicodeBlock Character::UnicodeBlock::CJK_COMPATIBILITY_IDEOGRAPHS(
    "CJK_COMPATIBILITY_IDEOGRAPHS",
    {"CJK COMPATIBILITY IDEOGRAPHS", "CJKCOMPATIBILITYIDEOGRAPHS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::ALPHABETIC_PRESENTATION_FORMS(
    "ALPHABETIC_PRESENTATION_FORMS",
    {"ALPHABETIC PRESENTATION FORMS", "ALPHABETICPRESENTATIONFORMS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::ARABIC_PRESENTATION_FORMS_A(
    "ARABIC_PRESENTATION_FORMS_A",
    {"ARABIC PRESENTATION FORMS-A", "ARABICPRESENTATIONFORMS-A"}
);

const Character::UnicodeBlock Character::UnicodeBlock::COMBINING_HALF_MARKS(
    "COMBINING_HALF_MARKS",
    {"COMBINING HALF MARKS", "COMBININGHALFMARKS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::CJK_COMPATIBILITY_FORMS(
    "CJK_COMPATIBILITY_FORMS",
    {"CJK COMPATIBILITY FORMS", "CJKCOMPATIBILITYFORMS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::SMALL_FORM_VARIANTS(
    "SMALL_FORM_VARIANTS",
    {"SMALL FORM VARIANTS", "SMALLFORMVARIANTS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::ARABIC_PRESENTATION_FORMS_B(
    "ARABIC_PRESENTATION_FORMS_B",
    {"ARABIC PRESENTATION FORMS-B", "ARABICPRESENTATIONFORMS-B"}
);

const Character::UnicodeBlock Character::UnicodeBlock::HALFWIDTH_AND_FULLWIDTH_FORMS(
    "HALFWIDTH_AND_FULLWIDTH_FORMS",
    {"HALFWIDTH AND FULLWIDTH FORMS", "HALFWIDTHANDFULLWIDTHFORMS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::SPECIALS("SPECIALS");

const Character::UnicodeBlock Character::UnicodeBlock::SURROGATES_AREA(
    "SURROGATES_AREA"
);

const Character::UnicodeBlock Character::UnicodeBlock::SYRIAC("SYRIAC");
const Character::UnicodeBlock Character::UnicodeBlock::THAANA("THAANA");
const Character::UnicodeBlock Character::UnicodeBlock::SINHALA("SINHALA");
const Character::UnicodeBlock Character::UnicodeBlock::MYANMAR("MYANMAR");
const Character::UnicodeBlock Character::UnicodeBlock::ETHIOPIC("ETHIOPIC");
const Character::UnicodeBlock Character::UnicodeBlock::CHEROKEE("CHEROKEE");

const Character::UnicodeBlock Character::UnicodeBlock::UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS(
    "UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS",
    {"UNIFIED CANADIAN ABORIGINAL SYLLABICS",
     "UNIFIEDCANADIANABORIGINALSYLLABICS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::OGHAM("OGHAM");
const Character::UnicodeBlock Character::UnicodeBlock::RUNIC("RUNIC");
const Character::UnicodeBlock Character::UnicodeBlock::KHMER("KHMER");
const Character::UnicodeBlock Character::UnicodeBlock::MONGOLIAN("MONGOLIAN");

const Character::UnicodeBlock Character::UnicodeBlock::BRAILLE_PATTERNS(
    "BRAILLE_PATTERNS",
    {"BRAILLE PATTERNS", "BRAILLEPATTERNS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::CJK_RADICALS_SUPPLEMENT(
    "CJK_RADICALS_SUPPLEMENT",
    {"CJK RADICALS SUPPLEMENT", "CJKRADICALSSUPPLEMENT"}
);

const Character::UnicodeBlock Character::UnicodeBlock::KANGXI_RADICALS(
    "KANGXI_RADICALS",
    {"KANGXI RADICALS", "KANGXIRADICALS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::IDEOGRAPHIC_DESCRIPTION_CHARACTERS(
    "IDEOGRAPHIC_DESCRIPTION_CHARACTERS",
    {"IDEOGRAPHIC DESCRIPTION CHARACTERS",
     "IDEOGRAPHICDESCRIPTIONCHARACTERS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::BOPOMOFO_EXTENDED(
    "BOPOMOFO_EXTENDED",
    {"BOPOMOFO EXTENDED", "BOPOMOFOEXTENDED"}
);

const Character::UnicodeBlock Character::UnicodeBlock::CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A(
    "CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A",
    {"CJK UNIFIED IDEOGRAPHS EXTENSION A",
     "CJKUNIFIEDIDEOGRAPHSEXTENSIONA"}
);

const Character::UnicodeBlock Character::UnicodeBlock::YI_SYLLABLES(
    "YI_SYLLABLES",
    {"YI SYLLABLES", "YISYLLABLES"}
);

const Character::UnicodeBlock Character::UnicodeBlock::YI_RADICALS(
    "YI_RADICALS",
    {"YI RADICALS", "YIRADICALS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::CYRILLIC_SUPPLEMENTARY(
    "CYRILLIC_SUPPLEMENTARY",
    {"CYRILLIC SUPPLEMENTARY", "CYRILLICSUPPLEMENTARY",
     "CYRILLIC SUPPLEMENT", "CYRILLICSUPPLEMENT"}
);

const Character::UnicodeBlock Character::UnicodeBlock::TAGALOG("TAGALOG");
const Character::UnicodeBlock Character::UnicodeBlock::HANUNOO("HANUNOO");
const Character::UnicodeBlock Character::UnicodeBlock::BUHID("BUHID");
const Character::UnicodeBlock Character::UnicodeBlock::TAGBANWA("TAGBANWA");
const Character::UnicodeBlock Character::UnicodeBlock::LIMBU("LIMBU");

const Character::UnicodeBlock Character::UnicodeBlock::TAI_LE(
    "TAI_LE",
    {"TAI LE", "TAILE"}
);

const Character::UnicodeBlock Character::UnicodeBlock::KHMER_SYMBOLS(
    "KHMER_SYMBOLS",
    {"KHMER SYMBOLS", "KHMERSYMBOLS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::PHONETIC_EXTENSIONS(
    "PHONETIC_EXTENSIONS",
    {"PHONETIC EXTENSIONS", "PHONETICEXTENSIONS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::MISCELLANEOUS_MATHEMATICAL_SYMBOLS_A(
    "MISCELLANEOUS_MATHEMATICAL_SYMBOLS_A",
    {"MISCELLANEOUS MATHEMATICAL SYMBOLS-A",
     "MISCELLANEOUSMATHEMATICALSYMBOLS-A"}
);

const Character::UnicodeBlock Character::UnicodeBlock::SUPPLEMENTAL_ARROWS_A(
    "SUPPLEMENTAL_ARROWS_A",
    {"SUPPLEMENTAL ARROWS-A", "SUPPLEMENTALARROWS-A"}
);

const Character::UnicodeBlock Character::UnicodeBlock::SUPPLEMENTAL_ARROWS_B(
    "SUPPLEMENTAL_ARROWS_B",
    {"SUPPLEMENTAL ARROWS-B", "SUPPLEMENTALARROWS-B"}
);

const Character::UnicodeBlock Character::UnicodeBlock::MISCELLANEOUS_MATHEMATICAL_SYMBOLS_B(
    "MISCELLANEOUS_MATHEMATICAL_SYMBOLS_B",
    {"MISCELLANEOUS MATHEMATICAL SYMBOLS-B",
     "MISCELLANEOUSMATHEMATICALSYMBOLS-B"}
);

const Character::UnicodeBlock Character::UnicodeBlock::SUPPLEMENTAL_MATHEMATICAL_OPERATORS(
    "SUPPLEMENTAL_MATHEMATICAL_OPERATORS",
    {"SUPPLEMENTAL MATHEMATICAL OPERATORS",
     "SUPPLEMENTALMATHEMATICALOPERATORS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::MISCELLANEOUS_SYMBOLS_AND_ARROWS(
    "MISCELLANEOUS_SYMBOLS_AND_ARROWS",
    {"MISCELLANEOUS SYMBOLS AND ARROWS",
     "MISCELLANEOUSSYMBOLSANDARROWS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::KATAKANA_PHONETIC_EXTENSIONS(
    "KATAKANA_PHONETIC_EXTENSIONS",
    {"KATAKANA PHONETIC EXTENSIONS", "KATAKANAPHONETICEXTENSIONS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::YIJING_HEXAGRAM_SYMBOLS(
    "YIJING_HEXAGRAM_SYMBOLS",
    {"YIJING HEXAGRAM SYMBOLS", "YIJINGHEXAGRAMSYMBOLS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::VARIATION_SELECTORS(
    "VARIATION_SELECTORS",
    {"VARIATION SELECTORS", "VARIATIONSELECTORS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::LINEAR_B_SYLLABARY(
    "LINEAR_B_SYLLABARY",
    {"LINEAR B SYLLABARY", "LINEARBSYLLABARY"}
);

const Character::UnicodeBlock Character::UnicodeBlock::LINEAR_B_IDEOGRAMS(
    "LINEAR_B_IDEOGRAMS",
    {"LINEAR B IDEOGRAMS", "LINEARBIDEOGRAMS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::AEGEAN_NUMBERS(
    "AEGEAN_NUMBERS",
    {"AEGEAN NUMBERS", "AEGEANNUMBERS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::OLD_ITALIC(
    "OLD_ITALIC",
    {"OLD ITALIC", "OLDITALIC"}
);

const Character::UnicodeBlock Character::UnicodeBlock::GOTHIC("GOTHIC");
const Character::UnicodeBlock Character::UnicodeBlock::UGARITIC("UGARITIC");
const Character::UnicodeBlock Character::UnicodeBlock::DESERET("DESERET");
const Character::UnicodeBlock Character::UnicodeBlock::SHAVIAN("SHAVIAN");
const Character::UnicodeBlock Character::UnicodeBlock::OSMANYA("OSMANYA");

const Character::UnicodeBlock Character::UnicodeBlock::CYPRIOT_SYLLABARY(
    "CYPRIOT_SYLLABARY",
    {"CYPRIOT SYLLABARY", "CYPRIOTSYLLABARY"}
);

const Character::UnicodeBlock Character::UnicodeBlock::BYZANTINE_MUSICAL_SYMBOLS(
    "BYZANTINE_MUSICAL_SYMBOLS",
    {"BYZANTINE MUSICAL SYMBOLS", "BYZANTINEMUSICALSYMBOLS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::MUSICAL_SYMBOLS(
    "MUSICAL_SYMBOLS",
    {"MUSICAL SYMBOLS", "MUSICALSYMBOLS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::TAI_XUAN_JING_SYMBOLS(
    "TAI_XUAN_JING_SYMBOLS",
    {"TAI XUAN JING SYMBOLS", "TAIXUANJINGSYMBOLS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::MATHEMATICAL_ALPHANUMERIC_SYMBOLS(
    "MATHEMATICAL_ALPHANUMERIC_SYMBOLS",
    {"MATHEMATICAL ALPHANUMERIC SYMBOLS",
     "MATHEMATICALALPHANUMERICSYMBOLS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B(
    "CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B",
    {"CJK UNIFIED IDEOGRAPHS EXTENSION B",
     "CJKUNIFIEDIDEOGRAPHSEXTENSIONB"}
);

const Character::UnicodeBlock Character::UnicodeBlock::CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT(
    "CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT",
    {"CJK COMPATIBILITY IDEOGRAPHS SUPPLEMENT",
     "CJKCOMPATIBILITYIDEOGRAPHSSUPPLEMENT"}
);

const Character::UnicodeBlock Character::UnicodeBlock::TAGS("TAGS");

const Character::UnicodeBlock Character::UnicodeBlock::VARIATION_SELECTORS_SUPPLEMENT(
    "VARIATION_SELECTORS_SUPPLEMENT",
    {"VARIATION SELECTORS SUPPLEMENT", "VARIATIONSELECTORSSUPPLEMENT"}
);

const Character::UnicodeBlock Character::UnicodeBlock::SUPPLEMENTARY_PRIVATE_USE_AREA_A(
    "SUPPLEMENTARY_PRIVATE_USE_AREA_A",
    {"SUPPLEMENTARY PRIVATE USE AREA-A", "SUPPLEMENTARYPRIVATEUSEAREA-A"}
);

const Character::UnicodeBlock Character::UnicodeBlock::SUPPLEMENTARY_PRIVATE_USE_AREA_B(
    "SUPPLEMENTARY_PRIVATE_USE_AREA_B",
    {"SUPPLEMENTARY PRIVATE USE AREA-B", "SUPPLEMENTARYPRIVATEUSEAREA-B"}
);

const Character::UnicodeBlock Character::UnicodeBlock::HIGH_SURROGATES(
    "HIGH_SURROGATES",
    {"HIGH SURROGATES", "HIGHSURROGATES"}
);

const Character::UnicodeBlock Character::UnicodeBlock::HIGH_PRIVATE_USE_SURROGATES(
    "HIGH_PRIVATE_USE_SURROGATES",
    {"HIGH PRIVATE USE SURROGATES", "HIGHPRIVATEUSESURROGATES"}
);

const Character::UnicodeBlock Character::UnicodeBlock::LOW_SURROGATES(
    "LOW_SURROGATES",
    {"LOW SURROGATES", "LOWSURROGATES"}
);

const Character::UnicodeBlock Character::UnicodeBlock::ARABIC_SUPPLEMENT(
    "ARABIC_SUPPLEMENT",
    {"ARABIC SUPPLEMENT", "ARABICSUPPLEMENT"}
);

const Character::UnicodeBlock Character::UnicodeBlock::NKO("NKO");
const Character::UnicodeBlock Character::UnicodeBlock::SAMARITAN("SAMARITAN");
const Character::UnicodeBlock Character::UnicodeBlock::MANDAIC("MANDAIC");

const Character::UnicodeBlock Character::UnicodeBlock::ETHIOPIC_SUPPLEMENT(
    "ETHIOPIC_SUPPLEMENT",
    {"ETHIOPIC SUPPLEMENT", "ETHIOPICSUPPLEMENT"}
);

const Character::UnicodeBlock Character::UnicodeBlock::UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_EXTENDED(
    "UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_EXTENDED",
    {"UNIFIED CANADIAN ABORIGINAL SYLLABICS EXTENDED",
     "UNIFIEDCANADIANABORIGINALSYLLABICSEXTENDED"}
);

const Character::UnicodeBlock Character::UnicodeBlock::NEW_TAI_LUE(
    "NEW_TAI_LUE",
    {"NEW TAI LUE", "NEWTAILUE"}
);

const Character::UnicodeBlock Character::UnicodeBlock::BUGINESE("BUGINESE");

const Character::UnicodeBlock Character::UnicodeBlock::TAI_THAM(
    "TAI_THAM",
    {"TAI THAM", "TAITHAM"}
);

const Character::UnicodeBlock Character::UnicodeBlock::BALINESE("BALINESE");
const Character::UnicodeBlock Character::UnicodeBlock::SUNDANESE("SUNDANESE");
const Character::UnicodeBlock Character::UnicodeBlock::BATAK("BATAK");
const Character::UnicodeBlock Character::UnicodeBlock::LEPCHA("LEPCHA");

const Character::UnicodeBlock Character::UnicodeBlock::OL_CHIKI(
    "OL_CHIKI",
    {"OL CHIKI", "OLCHIKI"}
);

const Character::UnicodeBlock Character::UnicodeBlock::VEDIC_EXTENSIONS(
    "VEDIC_EXTENSIONS",
    {"VEDIC EXTENSIONS", "VEDICEXTENSIONS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::PHONETIC_EXTENSIONS_SUPPLEMENT(
    "PHONETIC_EXTENSIONS_SUPPLEMENT",
    {"PHONETIC EXTENSIONS SUPPLEMENT", "PHONETICEXTENSIONSSUPPLEMENT"}
);

const Character::UnicodeBlock Character::UnicodeBlock::COMBINING_DIACRITICAL_MARKS_SUPPLEMENT(
    "COMBINING_DIACRITICAL_MARKS_SUPPLEMENT",
    {"COMBINING DIACRITICAL MARKS SUPPLEMENT",
     "COMBININGDIACRITICALMARKSSUPPLEMENT"}
);

const Character::UnicodeBlock Character::UnicodeBlock::GLAGOLITIC("GLAGOLITIC");

const Character::UnicodeBlock Character::UnicodeBlock::LATIN_EXTENDED_C(
    "LATIN_EXTENDED_C",
    {"LATIN EXTENDED-C", "LATINEXTENDED-C"}
);

const Character::UnicodeBlock Character::UnicodeBlock::COPTIC("COPTIC");

const Character::UnicodeBlock Character::UnicodeBlock::GEORGIAN_SUPPLEMENT(
    "GEORGIAN_SUPPLEMENT",
    {"GEORGIAN SUPPLEMENT", "GEORGIANSUPPLEMENT"}
);

const Character::UnicodeBlock Character::UnicodeBlock::TIFINAGH("TIFINAGH");

const Character::UnicodeBlock Character::UnicodeBlock::ETHIOPIC_EXTENDED(
    "ETHIOPIC_EXTENDED",
    {"ETHIOPIC EXTENDED", "ETHIOPICEXTENDED"}
);

const Character::UnicodeBlock Character::UnicodeBlock::CYRILLIC_EXTENDED_A(
    "CYRILLIC_EXTENDED_A",
    {"CYRILLIC EXTENDED-A", "CYRILLICEXTENDED-A"}
);

const Character::UnicodeBlock Character::UnicodeBlock::SUPPLEMENTAL_PUNCTUATION(
    "SUPPLEMENTAL_PUNCTUATION",
    {"SUPPLEMENTAL PUNCTUATION", "SUPPLEMENTALPUNCTUATION"}
);

const Character::UnicodeBlock Character::UnicodeBlock::CJK_STROKES(
    "CJK_STROKES",
    {"CJK STROKES", "CJKSTROKES"}
);

const Character::UnicodeBlock Character::UnicodeBlock::LISU("LISU");
const Character::UnicodeBlock Character::UnicodeBlock::VAI("VAI");

const Character::UnicodeBlock Character::UnicodeBlock::CYRILLIC_EXTENDED_B(
    "CYRILLIC_EXTENDED_B",
    {"CYRILLIC EXTENDED-B", "CYRILLICEXTENDED-B"}
);

const Character::UnicodeBlock Character::UnicodeBlock::BAMUM("BAMUM");

const Character::UnicodeBlock Character::UnicodeBlock::MODIFIER_TONE_LETTERS(   
    "MODIFIER_TONE_LETTERS",
    {"MODIFIER TONE LETTERS", "MODIFIERTONELETTERS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::LATIN_EXTENDED_D(
    "LATIN_EXTENDED_D",
    {"LATIN EXTENDED-D", "LATINEXTENDED-D"}
);

const Character::UnicodeBlock Character::UnicodeBlock::SYLOTI_NAGRI(
    "SYLOTI_NAGRI",
    {"SYLOTI NAGRI", "SYLOTINAGRI"}
);

const Character::UnicodeBlock Character::UnicodeBlock::COMMON_INDIC_NUMBER_FORMS(
    "COMMON_INDIC_NUMBER_FORMS",
    {"COMMON INDIC NUMBER FORMS", "COMMONINDICNUMBERFORMS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::PHAGS_PA(
    "PHAGS_PA",
    {"PHAGS-PA"}
);

const Character::UnicodeBlock Character::UnicodeBlock::SAURASHTRA("SAURASHTRA");

const Character::UnicodeBlock Character::UnicodeBlock::DEVANAGARI_EXTENDED(
    "DEVANAGARI_EXTENDED",
    {"DEVANAGARI EXTENDED", "DEVANAGARIEXTENDED"}
);

const Character::UnicodeBlock Character::UnicodeBlock::KAYAH_LI(
    "KAYAH_LI",
    {"KAYAH LI", "KAYAHLI"}
);

const Character::UnicodeBlock Character::UnicodeBlock::REJANG("REJANG");

const Character::UnicodeBlock Character::UnicodeBlock::HANGUL_JAMO_EXTENDED_A(
    "HANGUL_JAMO_EXTENDED_A",
    {"HANGUL JAMO EXTENDED-A", "HANGULJAMOEXTENDED-A"}
);

const Character::UnicodeBlock Character::UnicodeBlock::JAVANESE("JAVANESE");
const Character::UnicodeBlock Character::UnicodeBlock::CHAM("CHAM");
const Character::UnicodeBlock Character::UnicodeBlock::MYANMAR_EXTENDED_A(
    "MYANMAR_EXTENDED_A",
    {"MYANMAR EXTENDED-A", "MYANMAREXTENDED-A"}
);

const Character::UnicodeBlock Character::UnicodeBlock::TAI_VIET(
    "TAI_VIET",
    {"TAI VIET", "TAIVIET"}
);

const Character::UnicodeBlock Character::UnicodeBlock::ETHIOPIC_EXTENDED_A(
    "ETHIOPIC_EXTENDED_A",
    {"ETHIOPIC EXTENDED-A", "ETHIOPICEXTENDED-A"}
);

const Character::UnicodeBlock Character::UnicodeBlock::MEETEI_MAYEK(
    "MEETEI_MAYEK",
    {"MEETEI MAYEK", "MEETEIMAYEK"}
);

const Character::UnicodeBlock Character::UnicodeBlock::HANGUL_JAMO_EXTENDED_B(
    "HANGUL_JAMO_EXTENDED_B",
    {"HANGUL JAMO EXTENDED-B", "HANGULJAMOEXTENDED-B"}
);

const Character::UnicodeBlock Character::UnicodeBlock::VERTICAL_FORMS(
    "VERTICAL_FORMS",
    {"VERTICAL FORMS", "VERTICALFORMS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::ANCIENT_GREEK_NUMBERS(
    "ANCIENT_GREEK_NUMBERS",
    {"ANCIENT GREEK NUMBERS", "ANCIENTGREEKNUMBERS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::ANCIENT_SYMBOLS(
    "ANCIENT_SYMBOLS",
    {"ANCIENT SYMBOLS", "ANCIENTSYMBOLS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::PHAISTOS_DISC(
    "PHAISTOS_DISC",
    {"PHAISTOS DISC", "PHAISTOSDISC"}
);

const Character::UnicodeBlock Character::UnicodeBlock::LYCIAN("LYCIAN");
const Character::UnicodeBlock Character::UnicodeBlock::CARIAN("CARIAN");

const Character::UnicodeBlock Character::UnicodeBlock::OLD_PERSIAN(
    "OLD_PERSIAN",
    {"OLD PERSIAN", "OLDPERSIAN"}
);

const Character::UnicodeBlock Character::UnicodeBlock::IMPERIAL_ARAMAIC(
    "IMPERIAL_ARAMAIC",
    {"IMPERIAL ARAMAIC", "IMPERIALARAMAIC"}
);

const Character::UnicodeBlock Character::UnicodeBlock::PHOENICIAN("PHOENICIAN");
const Character::UnicodeBlock Character::UnicodeBlock::LYDIAN("LYDIAN");
const Character::UnicodeBlock Character::UnicodeBlock::KHAROSHTHI("KHAROSHTHI");

const Character::UnicodeBlock Character::UnicodeBlock::OLD_SOUTH_ARABIAN(
    "OLD_SOUTH_ARABIAN",
    {"OLD SOUTH ARABIAN", "OLDSOUTHARABIAN"}
);

const Character::UnicodeBlock Character::UnicodeBlock::AVESTAN("AVESTAN");

const Character::UnicodeBlock Character::UnicodeBlock::INSCRIPTIONAL_PARTHIAN(
    "INSCRIPTIONAL_PARTHIAN",
    {"INSCRIPTIONAL PARTHIAN", "INSCRIPTIONALPARTHIAN"}
);

const Character::UnicodeBlock Character::UnicodeBlock::INSCRIPTIONAL_PAHLAVI(
    "INSCRIPTIONAL_PAHLAVI",
    {"INSCRIPTIONAL PAHLAVI", "INSCRIPTIONALPAHLAVI"}
);

const Character::UnicodeBlock Character::UnicodeBlock::OLD_TURKIC(
    "OLD_TURKIC",
    {"OLD TURKIC", "OLDTURKIC"}
);

const Character::UnicodeBlock Character::UnicodeBlock::RUMI_NUMERAL_SYMBOLS(
    "RUMI_NUMERAL_SYMBOLS",
    {"RUMI NUMERAL SYMBOLS", "RUMINUMERALSYMBOLS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::BRAHMI("BRAHMI");
const Character::UnicodeBlock Character::UnicodeBlock::KAITHI("KAITHI");
const Character::UnicodeBlock Character::UnicodeBlock::CUNEIFORM("CUNEIFORM");

const Character::UnicodeBlock Character::UnicodeBlock::CUNEIFORM_NUMBERS_AND_PUNCTUATION(
    "CUNEIFORM_NUMBERS_AND_PUNCTUATION",
    {"CUNEIFORM NUMBERS AND PUNCTUATION", "CUNEIFORMNUMBERSANDPUNCTUATION"}
);

const Character::UnicodeBlock Character::UnicodeBlock::EGYPTIAN_HIEROGLYPHS(
    "EGYPTIAN_HIEROGLYPHS",
    {"EGYPTIAN HIEROGLYPHS", "EGYPTIANHIEROGLYPHS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::BAMUM_SUPPLEMENT(
    "BAMUM_SUPPLEMENT",
    {"BAMUM SUPPLEMENT", "BAMUMSUPPLEMENT"}
);

const Character::UnicodeBlock Character::UnicodeBlock::KANA_SUPPLEMENT(
    "KANA_SUPPLEMENT",
    {"KANA SUPPLEMENT", "KANASUPPLEMENT"}
);

const Character::UnicodeBlock Character::UnicodeBlock::ANCIENT_GREEK_MUSICAL_NOTATION(
    "ANCIENT_GREEK_MUSICAL_NOTATION",
    {"ANCIENT GREEK MUSICAL NOTATION", "ANCIENTGREEKMUSICALNOTATION"}
);

const Character::UnicodeBlock Character::UnicodeBlock::COUNTING_ROD_NUMERALS(
    "COUNTING_ROD_NUMERALS",
    {"COUNTING ROD NUMERALS", "COUNTINGRODNUMERALS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::MAHJONG_TILES(
    "MAHJONG_TILES",
    {"MAHJONG TILES", "MAHJONGTILES"}
);

const Character::UnicodeBlock Character::UnicodeBlock::DOMINO_TILES(
    "DOMINO_TILES",
    {"DOMINO TILES", "DOMINOTILES"}
);

const Character::UnicodeBlock Character::UnicodeBlock::PLAYING_CARDS(
    "PLAYING_CARDS",
    {"PLAYING CARDS", "PLAYINGCARDS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::ENCLOSED_ALPHANUMERIC_SUPPLEMENT(
    "ENCLOSED_ALPHANUMERIC_SUPPLEMENT",
    {"ENCLOSED ALPHANUMERIC SUPPLEMENT", "ENCLOSEDALPHANUMERICSUPPLEMENT"}
);

const Character::UnicodeBlock Character::UnicodeBlock::ENCLOSED_IDEOGRAPHIC_SUPPLEMENT(
    "ENCLOSED_IDEOGRAPHIC_SUPPLEMENT",
    {"ENCLOSED IDEOGRAPHIC SUPPLEMENT", "ENCLOSEDIDEOGRAPHICSUPPLEMENT"}
);

const Character::UnicodeBlock Character::UnicodeBlock::MISCELLANEOUS_SYMBOLS_AND_PICTOGRAPHS(
    "MISCELLANEOUS_SYMBOLS_AND_PICTOGRAPHS",
    {"MISCELLANEOUS SYMBOLS AND PICTOGRAPHS",
     "MISCELLANEOUSSYMBOLSANDPICTOGRAPHS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::EMOTICONS("EMOTICONS");

const Character::UnicodeBlock Character::UnicodeBlock::TRANSPORT_AND_MAP_SYMBOLS(
    "TRANSPORT_AND_MAP_SYMBOLS",
    {"TRANSPORT AND MAP SYMBOLS", "TRANSPORTANDMAPSYMBOLS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::ALCHEMICAL_SYMBOLS(
    "ALCHEMICAL_SYMBOLS",
    {"ALCHEMICAL SYMBOLS", "ALCHEMICALSYMBOLS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::CJK_UNIFIED_IDEOGRAPHS_EXTENSION_C(
    "CJK_UNIFIED_IDEOGRAPHS_EXTENSION_C",
    {"CJK UNIFIED IDEOGRAPHS EXTENSION C",
     "CJKUNIFIEDIDEOGRAPHSEXTENSIONC"}
);

const Character::UnicodeBlock Character::UnicodeBlock::CJK_UNIFIED_IDEOGRAPHS_EXTENSION_D(
    "CJK_UNIFIED_IDEOGRAPHS_EXTENSION_D",
    {"CJK UNIFIED IDEOGRAPHS EXTENSION D",
     "CJKUNIFIEDIDEOGRAPHSEXTENSIOND"}
);

const Character::UnicodeBlock Character::UnicodeBlock::ARABIC_EXTENDED_A(
    "ARABIC_EXTENDED_A",
    {"ARABIC EXTENDED-A", "ARABICEXTENDED-A"}
);

const Character::UnicodeBlock Character::UnicodeBlock::SUNDANESE_SUPPLEMENT(
    "SUNDANESE_SUPPLEMENT",
    {"SUNDANESE SUPPLEMENT", "SUNDANESESUPPLEMENT"}
);

const Character::UnicodeBlock Character::UnicodeBlock::MEETEI_MAYEK_EXTENSIONS(
    "MEETEI_MAYEK_EXTENSIONS",
    {"MEETEI MAYEK EXTENSIONS", "MEETEIMAYEKEXTENSIONS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::MEROITIC_HIEROGLYPHS(
    "MEROITIC_HIEROGLYPHS",
    {"MEROITIC HIEROGLYPHS", "MEROITICHIEROGLYPHS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::MEROITIC_CURSIVE(
    "MEROITIC_CURSIVE",
    {"MEROITIC CURSIVE", "MEROITICCURSIVE"}
);

const Character::UnicodeBlock Character::UnicodeBlock::SORA_SOMPENG(
    "SORA_SOMPENG",
    {"SORA SOMPENG", "SORASOMPENG"}
);

const Character::UnicodeBlock Character::UnicodeBlock::CHAKMA("CHAKMA");
const Character::UnicodeBlock Character::UnicodeBlock::SHARADA("SHARADA");
const Character::UnicodeBlock Character::UnicodeBlock::TAKRI("TAKRI");
const Character::UnicodeBlock Character::UnicodeBlock::MIAO("MIAO");

const Character::UnicodeBlock Character::UnicodeBlock::ARABIC_MATHEMATICAL_ALPHABETIC_SYMBOLS(
    "ARABIC_MATHEMATICAL_ALPHABETIC_SYMBOLS",
    {"ARABIC MATHEMATICAL ALPHABETIC SYMBOLS",
     "ARABICMATHEMATICALALPHABETICSYMBOLS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::COMBINING_DIACRITICAL_MARKS_EXTENDED(
    "COMBINING_DIACRITICAL_MARKS_EXTENDED",
    {"COMBINING DIACRITICAL MARKS EXTENDED",
     "COMBININGDIACRITICALMARKSEXTENDED"}
);

const Character::UnicodeBlock Character::UnicodeBlock::MYANMAR_EXTENDED_B(
    "MYANMAR_EXTENDED_B",
    {"MYANMAR EXTENDED-B", "MYANMAREXTENDED-B"}
);

const Character::UnicodeBlock Character::UnicodeBlock::LATIN_EXTENDED_E(
    "LATIN_EXTENDED_E",
    {"LATIN EXTENDED-E", "LATINEXTENDED-E"}
);

const Character::UnicodeBlock Character::UnicodeBlock::COPTIC_EPACT_NUMBERS(
    "COPTIC_EPACT_NUMBERS",
    {"COPTIC EPACT NUMBERS", "COPTICEPACTNUMBERS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::OLD_PERMIC(
    "OLD_PERMIC",
    {"OLD PERMIC", "OLDPERMIC"}
);

const Character::UnicodeBlock Character::UnicodeBlock::ELBASAN("ELBASAN");

const Character::UnicodeBlock Character::UnicodeBlock::CAUCASIAN_ALBANIAN(
    "CAUCASIAN_ALBANIAN",
    {"CAUCASIAN ALBANIAN", "CAUCASIANALBANIAN"}
);

const Character::UnicodeBlock Character::UnicodeBlock::LINEAR_A(
    "LINEAR_A",
    {"LINEAR A", "LINEARA"}
);

const Character::UnicodeBlock Character::UnicodeBlock::PALMYRENE("PALMYRENE");
const Character::UnicodeBlock Character::UnicodeBlock::NABATAEAN("NABATAEAN");

const Character::UnicodeBlock Character::UnicodeBlock::OLD_NORTH_ARABIAN(
    "OLD_NORTH_ARABIAN",
    {"OLD NORTH ARABIAN", "OLDNORTHARABIAN"}
);

const Character::UnicodeBlock Character::UnicodeBlock::MANICHAEAN("MANICHAEAN");

const Character::UnicodeBlock Character::UnicodeBlock::PSALTER_PAHLAVI(
    "PSALTER_PAHLAVI",
    {"PSALTER PAHLAVI", "PSALTERPAHLAVI"}
);

const Character::UnicodeBlock Character::UnicodeBlock::MAHAJANI("MAHAJANI");

const Character::UnicodeBlock Character::UnicodeBlock::SINHALA_ARCHAIC_NUMBERS(
    "SINHALA_ARCHAIC_NUMBERS",
    {"SINHALA ARCHAIC NUMBERS", "SINHALAARCHAICNUMBERS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::KHOJKI("KHOJKI");
const Character::UnicodeBlock Character::UnicodeBlock::KHUDAWADI("KHUDAWADI");
const Character::UnicodeBlock Character::UnicodeBlock::GRANTHA("GRANTHA");
const Character::UnicodeBlock Character::UnicodeBlock::TIRHUTA("TIRHUTA");
const Character::UnicodeBlock Character::UnicodeBlock::SIDDHAM("SIDDHAM");
const Character::UnicodeBlock Character::UnicodeBlock::MODI("MODI");

const Character::UnicodeBlock Character::UnicodeBlock::WARANG_CITI(
    "WARANG_CITI",
    {"WARANG CITI", "WARANGCITI"}
);

const Character::UnicodeBlock Character::UnicodeBlock::PAU_CIN_HAU(
    "PAU_CIN_HAU",
    {"PAU CIN HAU", "PAUCINHAU"}
);

const Character::UnicodeBlock Character::UnicodeBlock::MRO("MRO");

const Character::UnicodeBlock Character::UnicodeBlock::BASSA_VAH(
    "BASSA_VAH",
    {"BASSA VAH", "BASSAVAH"}
);

const Character::UnicodeBlock Character::UnicodeBlock::PAHAWH_HMONG(
    "PAHAWH_HMONG",
    {"PAHAWH HMONG", "PAHAWHHMONG"}
);

const Character::UnicodeBlock Character::UnicodeBlock::DUPLOYAN("DUPLOYAN");

const Character::UnicodeBlock Character::UnicodeBlock::SHORTHAND_FORMAT_CONTROLS(
    "SHORTHAND_FORMAT_CONTROLS",
    {"SHORTHAND FORMAT CONTROLS", "SHORTHANDFORMATCONTROLS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::MENDE_KIKAKUI(
    "MENDE_KIKAKUI",
    {"MENDE KIKAKUI", "MENDEKIKAKUI"}
);

const Character::UnicodeBlock Character::UnicodeBlock::ORNAMENTAL_DINGBATS(
    "ORNAMENTAL_DINGBATS",
    {"ORNAMENTAL DINGBATS", "ORNAMENTALDINGBATS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::GEOMETRIC_SHAPES_EXTENDED(
    "GEOMETRIC_SHAPES_EXTENDED",
    {"GEOMETRIC SHAPES EXTENDED", "GEOMETRICSHAPESEXTENDED"}
);

const Character::UnicodeBlock Character::UnicodeBlock::SUPPLEMENTAL_ARROWS_C(
    "SUPPLEMENTAL_ARROWS_C",
    {"SUPPLEMENTAL ARROWS-C", "SUPPLEMENTALARROWS-C"}
);

const Character::UnicodeBlock Character::UnicodeBlock::CHEROKEE_SUPPLEMENT(
    "CHEROKEE_SUPPLEMENT",
    {"CHEROKEE SUPPLEMENT", "CHEROKEESUPPLEMENT"}
);

const Character::UnicodeBlock Character::UnicodeBlock::HATRAN("HATRAN");

const Character::UnicodeBlock Character::UnicodeBlock::OLD_HUNGARIAN(
    "OLD_HUNGARIAN",
    {"OLD HUNGARIAN", "OLDHUNGARIAN"}
);

const Character::UnicodeBlock Character::UnicodeBlock::MULTANI("MULTANI");
const Character::UnicodeBlock Character::UnicodeBlock::AHOM("AHOM");

const Character::UnicodeBlock Character::UnicodeBlock::EARLY_DYNASTIC_CUNEIFORM(
    "EARLY_DYNASTIC_CUNEIFORM",
    {"EARLY DYNASTIC CUNEIFORM", "EARLYDYNASTICCUNEIFORM"}
);

const Character::UnicodeBlock Character::UnicodeBlock::ANATOLIAN_HIEROGLYPHS(
    "ANATOLIAN_HIEROGLYPHS",
    {"ANATOLIAN HIEROGLYPHS", "ANATOLIANHIEROGLYPHS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::SUTTON_SIGNWRITING(
    "SUTTON_SIGNWRITING",
    {"SUTTON SIGNWRITING", "SUTTONSIGNWRITING"}
);

const Character::UnicodeBlock Character::UnicodeBlock::SUPPLEMENTAL_SYMBOLS_AND_PICTOGRAPHS(
    "SUPPLEMENTAL_SYMBOLS_AND_PICTOGRAPHS",
    {"SUPPLEMENTAL SYMBOLS AND PICTOGRAPHS",
     "SUPPLEMENTALSYMBOLSANDPICTOGRAPHS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::CJK_UNIFIED_IDEOGRAPHS_EXTENSION_E(
    "CJK_UNIFIED_IDEOGRAPHS_EXTENSION_E",
    {"CJK UNIFIED IDEOGRAPHS EXTENSION E",
     "CJKUNIFIEDIDEOGRAPHSEXTENSIONE"}
);

const Character::UnicodeBlock Character::UnicodeBlock::SYRIAC_SUPPLEMENT(
    "SYRIAC_SUPPLEMENT",
    {"SYRIAC SUPPLEMENT", "SYRIACSUPPLEMENT"}
);

const Character::UnicodeBlock Character::UnicodeBlock::CYRILLIC_EXTENDED_C(
    "CYRILLIC_EXTENDED_C",
    {"CYRILLIC EXTENDED-C", "CYRILLICEXTENDED-C"}
);

const Character::UnicodeBlock Character::UnicodeBlock::OSAGE("OSAGE");
const Character::UnicodeBlock Character::UnicodeBlock::NEWA("NEWA");

const Character::UnicodeBlock Character::UnicodeBlock::MONGOLIAN_SUPPLEMENT(
    "MONGOLIAN_SUPPLEMENT",
    {"MONGOLIAN SUPPLEMENT", "MONGOLIANSUPPLEMENT"}
);

const Character::UnicodeBlock Character::UnicodeBlock::MARCHEN("MARCHEN");

const Character::UnicodeBlock Character::UnicodeBlock::IDEOGRAPHIC_SYMBOLS_AND_PUNCTUATION(
    "IDEOGRAPHIC_SYMBOLS_AND_PUNCTUATION",
    {"IDEOGRAPHIC SYMBOLS AND PUNCTUATION",
     "IDEOGRAPHICSYMBOLSANDPUNCTUATION"}
);

const Character::UnicodeBlock Character::UnicodeBlock::TANGUT("TANGUT");

const Character::UnicodeBlock Character::UnicodeBlock::TANGUT_COMPONENTS(
    "TANGUT_COMPONENTS",
    {"TANGUT COMPONENTS", "TANGUTCOMPONENTS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::KANA_EXTENDED_A(
    "KANA_EXTENDED_A",
    {"KANA EXTENDED-A", "KANAEXTENDED-A"}
);

const Character::UnicodeBlock Character::UnicodeBlock::GLAGOLITIC_SUPPLEMENT(
    "GLAGOLITIC_SUPPLEMENT",
    {"GLAGOLITIC SUPPLEMENT", "GLAGOLITICSUPPLEMENT"}
);

const Character::UnicodeBlock Character::UnicodeBlock::ADLAM("ADLAM");

const Character::UnicodeBlock Character::UnicodeBlock::MASARAM_GONDI(
    "MASARAM_GONDI",
    {"MASARAM GONDI", "MASARAMGONDI"}
);

const Character::UnicodeBlock Character::UnicodeBlock::ZANABAZAR_SQUARE(
    "ZANABAZAR_SQUARE",
    {"ZANABAZAR SQUARE", "ZANABAZARSQUARE"}
);

const Character::UnicodeBlock Character::UnicodeBlock::NUSHU("NUSHU");
const Character::UnicodeBlock Character::UnicodeBlock::SOYOMBO("SOYOMBO");
const Character::UnicodeBlock Character::UnicodeBlock::BHAIKSUKI("BHAIKSUKI");

const Character::UnicodeBlock Character::UnicodeBlock::CJK_UNIFIED_IDEOGRAPHS_EXTENSION_F(
    "CJK_UNIFIED_IDEOGRAPHS_EXTENSION_F",
    {"CJK UNIFIED IDEOGRAPHS EXTENSION F",
     "CJKUNIFIEDIDEOGRAPHSEXTENSIONF"}
);

const Character::UnicodeBlock Character::UnicodeBlock::GEORGIAN_EXTENDED(
    "GEORGIAN_EXTENDED",
    {"GEORGIAN EXTENDED", "GEORGIANEXTENDED"}
);

const Character::UnicodeBlock Character::UnicodeBlock::HANIFI_ROHINGYA(
    "HANIFI_ROHINGYA",
    {"HANIFI ROHINGYA", "HANIFIROHINGYA"}
);

const Character::UnicodeBlock Character::UnicodeBlock::OLD_SOGDIAN(
    "OLD_SOGDIAN",
    {"OLD SOGDIAN", "OLDSOGDIAN"}
);

const Character::UnicodeBlock Character::UnicodeBlock::SOGDIAN("SOGDIAN");
const Character::UnicodeBlock Character::UnicodeBlock::DOGRA("DOGRA");

const Character::UnicodeBlock Character::UnicodeBlock::GUNJALA_GONDI(
    "GUNJALA_GONDI",
    {"GUNJALA GONDI", "GUNJALAGONDI"}
);

const Character::UnicodeBlock Character::UnicodeBlock::MAKASAR("MAKASAR");
const Character::UnicodeBlock Character::UnicodeBlock::MEDEFAIDRIN("MEDEFAIDRIN");

const Character::UnicodeBlock Character::UnicodeBlock::MAYAN_NUMERALS(
    "MAYAN_NUMERALS",
    {"MAYAN NUMERALS", "MAYANNUMERALS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::INDIC_SIYAQ_NUMBERS(
    "INDIC_SIYAQ_NUMBERS",
    {"INDIC SIYAQ NUMBERS", "INDICSIYAQNUMBERS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::CHESS_SYMBOLS(
    "CHESS_SYMBOLS",
    {"CHESS SYMBOLS", "CHESSSYMBOLS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::ELYMAIC("ELYMAIC");
const Character::UnicodeBlock Character::UnicodeBlock::NANDINAGARI("NANDINAGARI");

const Character::UnicodeBlock Character::UnicodeBlock::TAMIL_SUPPLEMENT(
    "TAMIL_SUPPLEMENT",
    {"TAMIL SUPPLEMENT", "TAMILSUPPLEMENT"}
);

const Character::UnicodeBlock Character::UnicodeBlock::EGYPTIAN_HIEROGLYPH_FORMAT_CONTROLS(
    "EGYPTIAN_HIEROGLYPH_FORMAT_CONTROLS",
    {"EGYPTIAN HIEROGLYPH FORMAT CONTROLS",
     "EGYPTIANHIEROGLYPHFORMATCONTROLS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::SMALL_KANA_EXTENSION(
    "SMALL_KANA_EXTENSION",
    {"SMALL KANA EXTENSION", "SMALLKANAEXTENSION"}
);

const Character::UnicodeBlock Character::UnicodeBlock::NYIAKENG_PUACHUE_HMONG(
    "NYIAKENG_PUACHUE_HMONG",
    {"NYIAKENG PUACHUE HMONG", "NYIAKENGPUACHUEHMONG"}
);

const Character::UnicodeBlock Character::UnicodeBlock::WANCHO("WANCHO");

const Character::UnicodeBlock Character::UnicodeBlock::OTTOMAN_SIYAQ_NUMBERS(
    "OTTOMAN_SIYAQ_NUMBERS",
    {"OTTOMAN SIYAQ NUMBERS", "OTTOMANSIYAQNUMBERS"}
);

const Character::UnicodeBlock Character::UnicodeBlock::SYMBOLS_AND_PICTOGRAPHS_EXTENDED_A(
    "SYMBOLS_AND_PICTOGRAPHS_EXTENDED_A",
    {"SYMBOLS AND PICTOGRAPHS EXTENDED-A",
     "SYMBOLSANDPICTOGRAPHSEXTENDED-A"}
);

const Character::UnicodeBlock Character::UnicodeBlock::YEZIDI("YEZIDI");
const Character::UnicodeBlock Character::UnicodeBlock::CHORASMIAN("CHORASMIAN");

const Character::UnicodeBlock Character::UnicodeBlock::DIVES_AKURU(
    "DIVES_AKURU",
    {"DIVES AKURU", "DIVESAKURU"}
);

const Character::UnicodeBlock Character::UnicodeBlock::LISU_SUPPLEMENT(
    "LISU_SUPPLEMENT",
    {"LISU SUPPLEMENT", "LISUSUPPLEMENT"}
);

const Character::UnicodeBlock Character::UnicodeBlock::KHITAN_SMALL_SCRIPT(
    "KHITAN_SMALL_SCRIPT",
    {"KHITAN SMALL SCRIPT", "KHITANSMALLSCRIPT"}
);

const Character::UnicodeBlock Character::UnicodeBlock::TANGUT_SUPPLEMENT(
    "TANGUT_SUPPLEMENT",
    {"TANGUT SUPPLEMENT", "TANGUTSUPPLEMENT"}
);

const Character::UnicodeBlock Character::UnicodeBlock::SYMBOLS_FOR_LEGACY_COMPUTING(
    "SYMBOLS_FOR_LEGACY_COMPUTING",
    {"SYMBOLS FOR LEGACY COMPUTING", "SYMBOLSFORLEGACYCOMPUTING"}
);

const Character::UnicodeBlock Character::UnicodeBlock::CJK_UNIFIED_IDEOGRAPHS_EXTENSION_G(
    "CJK_UNIFIED_IDEOGRAPHS_EXTENSION_G",
    {"CJK UNIFIED IDEOGRAPHS EXTENSION G",
     "CJKUNIFIEDIDEOGRAPHSEXTENSIONG"}
);

const int Character::UnicodeBlock::blockStarts[] = {
    0x0000,   // 0000..007F; Basic Latin
    0x0080,   // 0080..00FF; Latin-1 Supplement
    0x0100,   // 0100..017F; Latin Extended-A
    0x0180,   // 0180..024F; Latin Extended-B
    0x0250,   // 0250..02AF; IPA Extensions
    0x02B0,   // 02B0..02FF; Spacing Modifier Letters
    0x0300,   // 0300..036F; Combining Diacritical Marks
    0x0370,   // 0370..03FF; Greek and Coptic
    0x0400,   // 0400..04FF; Cyrillic
    0x0500,   // 0500..052F; Cyrillic Supplement
    0x0530,   // 0530..058F; Armenian
    0x0590,   // 0590..05FF; Hebrew
    0x0600,   // 0600..06FF; Arabic
    0x0700,   // 0700..074F; Syriac
    0x0750,   // 0750..077F; Arabic Supplement
    0x0780,   // 0780..07BF; Thaana
    0x07C0,   // 07C0..07FF; NKo
    0x0800,   // 0800..083F; Samaritan
    0x0840,   // 0840..085F; Mandaic
    0x0860,   // 0860..086F; Syriac Supplement
    0x0870,   //             unassigned
    0x08A0,   // 08A0..08FF; Arabic Extended-A
    0x0900,   // 0900..097F; Devanagari
    0x0980,   // 0980..09FF; Bengali
    0x0A00,   // 0A00..0A7F; Gurmukhi
    0x0A80,   // 0A80..0AFF; Gujarati
    0x0B00,   // 0B00..0B7F; Oriya
    0x0B80,   // 0B80..0BFF; Tamil
    0x0C00,   // 0C00..0C7F; Telugu
    0x0C80,   // 0C80..0CFF; Kannada
    0x0D00,   // 0D00..0D7F; Malayalam
    0x0D80,   // 0D80..0DFF; Sinhala
    0x0E00,   // 0E00..0E7F; Thai
    0x0E80,   // 0E80..0EFF; Lao
    0x0F00,   // 0F00..0FFF; Tibetan
    0x1000,   // 1000..109F; Myanmar
    0x10A0,   // 10A0..10FF; Georgian
    0x1100,   // 1100..11FF; Hangul Jamo
    0x1200,   // 1200..137F; Ethiopic
    0x1380,   // 1380..139F; Ethiopic Supplement
    0x13A0,   // 13A0..13FF; Cherokee
    0x1400,   // 1400..167F; Unified Canadian Aboriginal Syllabics
    0x1680,   // 1680..169F; Ogham
    0x16A0,   // 16A0..16FF; Runic
    0x1700,   // 1700..171F; Tagalog
    0x1720,   // 1720..173F; Hanunoo
    0x1740,   // 1740..175F; Buhid
    0x1760,   // 1760..177F; Tagbanwa
    0x1780,   // 1780..17FF; Khmer
    0x1800,   // 1800..18AF; Mongolian
    0x18B0,   // 18B0..18FF; Unified Canadian Aboriginal Syllabics Extended
    0x1900,   // 1900..194F; Limbu
    0x1950,   // 1950..197F; Tai Le
    0x1980,   // 1980..19DF; New Tai Lue
    0x19E0,   // 19E0..19FF; Khmer Symbols
    0x1A00,   // 1A00..1A1F; Buginese
    0x1A20,   // 1A20..1AAF; Tai Tham
    0x1AB0,   // 1AB0..1AFF; Combining Diacritical Marks Extended
    0x1B00,   // 1B00..1B7F; Balinese
    0x1B80,   // 1B80..1BBF; Sundanese
    0x1BC0,   // 1BC0..1BFF; Batak
    0x1C00,   // 1C00..1C4F; Lepcha
    0x1C50,   // 1C50..1C7F; Ol Chiki
    0x1C80,   // 1C80..1C8F; Cyrillic Extended-C
    0x1C90,   // 1C90..1CBF; Georgian Extended
    0x1CC0,   // 1CC0..1CCF; Sundanese Supplement
    0x1CD0,   // 1CD0..1CFF; Vedic Extensions
    0x1D00,   // 1D00..1D7F; Phonetic Extensions
    0x1D80,   // 1D80..1DBF; Phonetic Extensions Supplement
    0x1DC0,   // 1DC0..1DFF; Combining Diacritical Marks Supplement
    0x1E00,   // 1E00..1EFF; Latin Extended Additional
    0x1F00,   // 1F00..1FFF; Greek Extended
    0x2000,   // 2000..206F; General Punctuation
    0x2070,   // 2070..209F; Superscripts and Subscripts
    0x20A0,   // 20A0..20CF; Currency Symbols
    0x20D0,   // 20D0..20FF; Combining Diacritical Marks for Symbols
    0x2100,   // 2100..214F; Letterlike Symbols
    0x2150,   // 2150..218F; Number Forms
    0x2190,   // 2190..21FF; Arrows
    0x2200,   // 2200..22FF; Mathematical Operators
    0x2300,   // 2300..23FF; Miscellaneous Technical
    0x2400,   // 2400..243F; Control Pictures
    0x2440,   // 2440..245F; Optical Character Recognition
    0x2460,   // 2460..24FF; Enclosed Alphanumerics
    0x2500,   // 2500..257F; Box Drawing
    0x2580,   // 2580..259F; Block Elements
    0x25A0,   // 25A0..25FF; Geometric Shapes
    0x2600,   // 2600..26FF; Miscellaneous Symbols
    0x2700,   // 2700..27BF; Dingbats
    0x27C0,   // 27C0..27EF; Miscellaneous Mathematical Symbols-A
    0x27F0,   // 27F0..27FF; Supplemental Arrows-A
    0x2800,   // 2800..28FF; Braille Patterns
    0x2900,   // 2900..297F; Supplemental Arrows-B
    0x2980,   // 2980..29FF; Miscellaneous Mathematical Symbols-B
    0x2A00,   // 2A00..2AFF; Supplemental Mathematical Operators
    0x2B00,   // 2B00..2BFF; Miscellaneous Symbols and Arrows
    0x2C00,   // 2C00..2C5F; Glagolitic
    0x2C60,   // 2C60..2C7F; Latin Extended-C
    0x2C80,   // 2C80..2CFF; Coptic
    0x2D00,   // 2D00..2D2F; Georgian Supplement
    0x2D30,   // 2D30..2D7F; Tifinagh
    0x2D80,   // 2D80..2DDF; Ethiopic Extended
    0x2DE0,   // 2DE0..2DFF; Cyrillic Extended-A
    0x2E00,   // 2E00..2E7F; Supplemental Punctuation
    0x2E80,   // 2E80..2EFF; CJK Radicals Supplement
    0x2F00,   // 2F00..2FDF; Kangxi Radicals
    0x2FE0,   //             unassigned
    0x2FF0,   // 2FF0..2FFF; Ideographic Description Characters
    0x3000,   // 3000..303F; CJK Symbols and Punctuation
    0x3040,   // 3040..309F; Hiragana
    0x30A0,   // 30A0..30FF; Katakana
    0x3100,   // 3100..312F; Bopomofo
    0x3130,   // 3130..318F; Hangul Compatibility Jamo
    0x3190,   // 3190..319F; Kanbun
    0x31A0,   // 31A0..31BF; Bopomofo Extended
    0x31C0,   // 31C0..31EF; CJK Strokes
    0x31F0,   // 31F0..31FF; Katakana Phonetic Extensions
    0x3200,   // 3200..32FF; Enclosed CJK Letters and Months
    0x3300,   // 3300..33FF; CJK Compatibility
    0x3400,   // 3400..4DBF; CJK Unified Ideographs Extension A
    0x4DC0,   // 4DC0..4DFF; Yijing Hexagram Symbols
    0x4E00,   // 4E00..9FFF; CJK Unified Ideographs
    0xA000,   // A000..A48F; Yi Syllables
    0xA490,   // A490..A4CF; Yi Radicals
    0xA4D0,   // A4D0..A4FF; Lisu
    0xA500,   // A500..A63F; Vai
    0xA640,   // A640..A69F; Cyrillic Extended-B
    0xA6A0,   // A6A0..A6FF; Bamum
    0xA700,   // A700..A71F; Modifier Tone Letters
    0xA720,   // A720..A7FF; Latin Extended-D
    0xA800,   // A800..A82F; Syloti Nagri
    0xA830,   // A830..A83F; Common Indic Number Forms
    0xA840,   // A840..A87F; Phags-pa
    0xA880,   // A880..A8DF; Saurashtra
    0xA8E0,   // A8E0..A8FF; Devanagari Extended
    0xA900,   // A900..A92F; Kayah Li
    0xA930,   // A930..A95F; Rejang
    0xA960,   // A960..A97F; Hangul Jamo Extended-A
    0xA980,   // A980..A9DF; Javanese
    0xA9E0,   // A9E0..A9FF; Myanmar Extended-B
    0xAA00,   // AA00..AA5F; Cham
    0xAA60,   // AA60..AA7F; Myanmar Extended-A
    0xAA80,   // AA80..AADF; Tai Viet
    0xAAE0,   // AAE0..AAFF; Meetei Mayek Extensions
    0xAB00,   // AB00..AB2F; Ethiopic Extended-A
    0xAB30,   // AB30..AB6F; Latin Extended-E
    0xAB70,   // AB70..ABBF; Cherokee Supplement
    0xABC0,   // ABC0..ABFF; Meetei Mayek
    0xAC00,   // AC00..D7AF; Hangul Syllables
    0xD7B0,   // D7B0..D7FF; Hangul Jamo Extended-B
    0xD800,   // D800..DB7F; High Surrogates
    0xDB80,   // DB80..DBFF; High Private Use Surrogates
    0xDC00,   // DC00..DFFF; Low Surrogates
    0xE000,   // E000..F8FF; Private Use Area
    0xF900,   // F900..FAFF; CJK Compatibility Ideographs
    0xFB00,   // FB00..FB4F; Alphabetic Presentation Forms
    0xFB50,   // FB50..FDFF; Arabic Presentation Forms-A
    0xFE00,   // FE00..FE0F; Variation Selectors
    0xFE10,   // FE10..FE1F; Vertical Forms
    0xFE20,   // FE20..FE2F; Combining Half Marks
    0xFE30,   // FE30..FE4F; CJK Compatibility Forms
    0xFE50,   // FE50..FE6F; Small Form Variants
    0xFE70,   // FE70..FEFF; Arabic Presentation Forms-B
    0xFF00,   // FF00..FFEF; Halfwidth and Fullwidth Forms
    0xFFF0,   // FFF0..FFFF; Specials
    0x10000,  // 10000..1007F; Linear B Syllabary
    0x10080,  // 10080..100FF; Linear B Ideograms
    0x10100,  // 10100..1013F; Aegean Numbers
    0x10140,  // 10140..1018F; Ancient Greek Numbers
    0x10190,  // 10190..101CF; Ancient Symbols
    0x101D0,  // 101D0..101FF; Phaistos Disc
    0x10200,  //               unassigned
    0x10280,  // 10280..1029F; Lycian
    0x102A0,  // 102A0..102DF; Carian
    0x102E0,  // 102E0..102FF; Coptic Epact Numbers
    0x10300,  // 10300..1032F; Old Italic
    0x10330,  // 10330..1034F; Gothic
    0x10350,  // 10350..1037F; Old Permic
    0x10380,  // 10380..1039F; Ugaritic
    0x103A0,  // 103A0..103DF; Old Persian
    0x103E0,  //               unassigned
    0x10400,  // 10400..1044F; Deseret
    0x10450,  // 10450..1047F; Shavian
    0x10480,  // 10480..104AF; Osmanya
    0x104B0,  // 104B0..104FF; Osage
    0x10500,  // 10500..1052F; Elbasan
    0x10530,  // 10530..1056F; Caucasian Albanian
    0x10570,  //               unassigned
    0x10600,  // 10600..1077F; Linear A
    0x10780,  //               unassigned
    0x10800,  // 10800..1083F; Cypriot Syllabary
    0x10840,  // 10840..1085F; Imperial Aramaic
    0x10860,  // 10860..1087F; Palmyrene
    0x10880,  // 10880..108AF; Nabataean
    0x108B0,  //               unassigned
    0x108E0,  // 108E0..108FF; Hatran
    0x10900,  // 10900..1091F; Phoenician
    0x10920,  // 10920..1093F; Lydian
    0x10940,  //               unassigned
    0x10980,  // 10980..1099F; Meroitic Hieroglyphs
    0x109A0,  // 109A0..109FF; Meroitic Cursive
    0x10A00,  // 10A00..10A5F; Kharoshthi
    0x10A60,  // 10A60..10A7F; Old South Arabian
    0x10A80,  // 10A80..10A9F; Old North Arabian
    0x10AA0,  //               unassigned
    0x10AC0,  // 10AC0..10AFF; Manichaean
    0x10B00,  // 10B00..10B3F; Avestan
    0x10B40,  // 10B40..10B5F; Inscriptional Parthian
    0x10B60,  // 10B60..10B7F; Inscriptional Pahlavi
    0x10B80,  // 10B80..10BAF; Psalter Pahlavi
    0x10BB0,  //               unassigned
    0x10C00,  // 10C00..10C4F; Old Turkic
    0x10C50,  //               unassigned
    0x10C80,  // 10C80..10CFF; Old Hungarian
    0x10D00,  // 10D00..10D3F; Hanifi Rohingya
    0x10D40,  //               unassigned
    0x10E60,  // 10E60..10E7F; Rumi Numeral Symbols
    0x10E80,  // 10E80..10EBF; Yezidi
    0x10EC0,  //               unassigned
    0x10F00,  // 10F00..10F2F; Old Sogdian
    0x10F30,  // 10F30..10F6F; Sogdian
    0x10F70,  //               unassigned
    0x10FB0,  // 10FB0..10FDF; Chorasmian
    0x10FE0,  // 10FE0..10FFF; Elymaic
    0x11000,  // 11000..1107F; Brahmi
    0x11080,  // 11080..110CF; Kaithi
    0x110D0,  // 110D0..110FF; Sora Sompeng
    0x11100,  // 11100..1114F; Chakma
    0x11150,  // 11150..1117F; Mahajani
    0x11180,  // 11180..111DF; Sharada
    0x111E0,  // 111E0..111FF; Sinhala Archaic Numbers
    0x11200,  // 11200..1124F; Khojki
    0x11250,  //               unassigned
    0x11280,  // 11280..112AF; Multani
    0x112B0,  // 112B0..112FF; Khudawadi
    0x11300,  // 11300..1137F; Grantha
    0x11380,  //               unassigned
    0x11400,  // 11400..1147F; Newa
    0x11480,  // 11480..114DF; Tirhuta
    0x114E0,  //               unassigned
    0x11580,  // 11580..115FF; Siddham
    0x11600,  // 11600..1165F; Modi
    0x11660,  // 11660..1167F; Mongolian Supplement
    0x11680,  // 11680..116CF; Takri
    0x116D0,  //               unassigned
    0x11700,  // 11700..1173F; Ahom
    0x11740,  //               unassigned
    0x11800,  // 11800..1184F; Dogra
    0x11850,  //               unassigned
    0x118A0,  // 118A0..118FF; Warang Citi
    0x11900,  // 11900..1195F; Dives Akuru
    0x11960,  //               unassigned
    0x119A0,  // 119A0..119FF; Nandinagari
    0x11A00,  // 11A00..11A4F; Zanabazar Square
    0x11A50,  // 11A50..11AAF; Soyombo
    0x11AB0,  //               unassigned
    0x11AC0,  // 11AC0..11AFF; Pau Cin Hau
    0x11B00,  //               unassigned
    0x11C00,  // 11C00..11C6F; Bhaiksuki
    0x11C70,  // 11C70..11CBF; Marchen
    0x11CC0,  //               unassigned
    0x11D00,  // 11D00..11D5F; Masaram Gondi
    0x11D60,  // 11D60..11DAF; Gunjala Gondi
    0x11DB0,  //               unassigned
    0x11EE0,  // 11EE0..11EFF; Makasar
    0x11F00,  //               unassigned
    0x11FB0,  // 11FB0..11FBF; Lisu Supplement
    0x11FC0,  // 11FC0..11FFF; Tamil Supplement
    0x12000,  // 12000..123FF; Cuneiform
    0x12400,  // 12400..1247F; Cuneiform Numbers and Punctuation
    0x12480,  // 12480..1254F; Early Dynastic Cuneiform
    0x12550,  //               unassigned
    0x13000,  // 13000..1342F; Egyptian Hieroglyphs
    0x13430,  // 13430..1343F; Egyptian Hieroglyph Format Controls
    0x13440,  //               unassigned
    0x14400,  // 14400..1467F; Anatolian Hieroglyphs
    0x14680,  //               unassigned
    0x16800,  // 16800..16A3F; Bamum Supplement
    0x16A40,  // 16A40..16A6F; Mro
    0x16A70,  //               unassigned
    0x16AD0,  // 16AD0..16AFF; Bassa Vah
    0x16B00,  // 16B00..16B8F; Pahawh Hmong
    0x16B90,  //               unassigned
    0x16E40,  // 16E40..16E9F; Medefaidrin
    0x16EA0,  //               unassigned
    0x16F00,  // 16F00..16F9F; Miao
    0x16FA0,  //               unassigned
    0x16FE0,  // 16FE0..16FFF; Ideographic Symbols and Punctuation
    0x17000,  // 17000..187FF; Tangut
    0x18800,  // 18800..18AFF; Tangut Components
    0x18B00,  // 18B00..18CFF; Khitan Small Script
    0x18D00,  // 18D00..18D8F; Tangut Supplement
    0x18D90,  //               unassigned
    0x1B000,  // 1B000..1B0FF; Kana Supplement
    0x1B100,  // 1B100..1B12F; Kana Extended-A
    0x1B130,  // 1B130..1B16F; Small Kana Extension
    0x1B170,  // 1B170..1B2FF; Nushu
    0x1B300,  //               unassigned
    0x1BC00,  // 1BC00..1BC9F; Duployan
    0x1BCA0,  // 1BCA0..1BCAF; Shorthand Format Controls
    0x1BCB0,  //               unassigned
    0x1D000,  // 1D000..1D0FF; Byzantine Musical Symbols
    0x1D100,  // 1D100..1D1FF; Musical Symbols
    0x1D200,  // 1D200..1D24F; Ancient Greek Musical Notation
    0x1D250,  //               unassigned
    0x1D2E0,  // 1D2E0..1D2FF; Mayan Numerals
    0x1D300,  // 1D300..1D35F; Tai Xuan Jing Symbols
    0x1D360,  // 1D360..1D37F; Counting Rod Numerals
    0x1D380,  //               unassigned
    0x1D400,  // 1D400..1D7FF; Mathematical Alphanumeric Symbols
    0x1D800,  // 1D800..1DAAF; Sutton SignWriting
    0x1DAB0,  //               unassigned
    0x1E000,  // 1E000..1E02F; Glagolitic Supplement
    0x1E030,  //               unassigned
    0x1E100,  // 1E100..1E14F; Nyiakeng Puachue Hmong
    0x1E150,  //               unassigned
    0x1E2C0,  // 1E2C0..1E2FF; Wancho
    0x1E300,  //               unassigned
    0x1E800,  // 1E800..1E8DF; Mende Kikakui
    0x1E8E0,  //               unassigned
    0x1E900,  // 1E900..1E95F; Adlam
    0x1E960,  //               unassigned
    0x1EC70,  // 1EC70..1ECBF; Indic Siyaq Numbers
    0x1ECC0,  //               unassigned
    0x1ED00,  // 1ED00..1ED4F; Ottoman Siyaq Numbers
    0x1ED50,  //               unassigned
    0x1EE00,  // 1EE00..1EEFF; Arabic Mathematical Alphabetic Symbols
    0x1EF00,  //               unassigned
    0x1F000,  // 1F000..1F02F; Mahjong Tiles
    0x1F030,  // 1F030..1F09F; Domino Tiles
    0x1F0A0,  // 1F0A0..1F0FF; Playing Cards
    0x1F100,  // 1F100..1F1FF; Enclosed Alphanumeric Supplement
    0x1F200,  // 1F200..1F2FF; Enclosed Ideographic Supplement
    0x1F300,  // 1F300..1F5FF; Miscellaneous Symbols and Pictographs
    0x1F600,  // 1F600..1F64F; Emoticons
    0x1F650,  // 1F650..1F67F; Ornamental Dingbats
    0x1F680,  // 1F680..1F6FF; Transport and Map Symbols
    0x1F700,  // 1F700..1F77F; Alchemical Symbols
    0x1F780,  // 1F780..1F7FF; Geometric Shapes Extended
    0x1F800,  // 1F800..1F8FF; Supplemental Arrows-C
    0x1F900,  // 1F900..1F9FF; Supplemental Symbols and Pictographs
    0x1FA00,  // 1FA00..1FA6F; Chess Symbols
    0x1FA70,  // 1FA70..1FAFF; Symbols and Pictographs Extended-A
    0x1FB00,  // 1FB00..1FBFF; Symbols for Legacy Computing
    0x1FC00,  //               unassigned
    0x20000,  // 20000..2A6DF; CJK Unified Ideographs Extension B
    0x2A6E0,  //               unassigned
    0x2A700,  // 2A700..2B73F; CJK Unified Ideographs Extension C
    0x2B740,  // 2B740..2B81F; CJK Unified Ideographs Extension D
    0x2B820,  // 2B820..2CEAF; CJK Unified Ideographs Extension E
    0x2CEB0,  // 2CEB0..2EBEF; CJK Unified Ideographs Extension F
    0x2EBF0,  //               unassigned
    0x2F800,  // 2F800..2FA1F; CJK Compatibility Ideographs Supplement
    0x2FA20,  //               unassigned
    0x30000,  // 30000..3134F; CJK Unified Ideographs Extension G
    0x31350,  //               unassigned
    0xE0000,  // E0000..E007F; Tags
    0xE0080,  //               unassigned
    0xE0100,  // E0100..E01EF; Variation Selectors Supplement
    0xE01F0,  //               unassigned
    0xF0000,  // F0000..FFFFF; Supplementary Private Use Area-A
    0x100000, // 100000..10FFFF; Supplementary Private Use Area-B
};

const Character::UnicodeBlock* Character::UnicodeBlock::blocks[] = {
    &BASIC_LATIN,
    &LATIN_1_SUPPLEMENT,
    &LATIN_EXTENDED_A,
    &LATIN_EXTENDED_B,  
    &IPA_EXTENSIONS,
    &SPACING_MODIFIER_LETTERS,
    &COMBINING_DIACRITICAL_MARKS,
    &GREEK,
    &CYRILLIC,
    &CYRILLIC_SUPPLEMENTARY,
    &ARMENIAN,
    &HEBREW,
    &ARABIC,
    &SYRIAC,
    &ARABIC_SUPPLEMENT,
    &THAANA,
    &NKO,
    &SAMARITAN,
    &MANDAIC,
    &SYRIAC_SUPPLEMENT,
    nullptr,
    &ARABIC_EXTENDED_A,
    &DEVANAGARI,
    &BENGALI,
    &GURMUKHI,
    &GUJARATI,
    &ORIYA,
    &TAMIL,
    &TELUGU,
    &KANNADA,
    &MALAYALAM,
    &SINHALA,
    &THAI,
    &LAO,
    &TIBETAN,
    &MYANMAR,
    &GEORGIAN,
    &HANGUL_JAMO,
    &ETHIOPIC,
    &ETHIOPIC_SUPPLEMENT,
    &CHEROKEE,
    &UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS,
    &OGHAM,
    &RUNIC,
    &TAGALOG,
    &HANUNOO,
    &BUHID,
    &TAGBANWA,
    &KHMER,
    &MONGOLIAN,
    &UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS_EXTENDED,
    &LIMBU,
    &TAI_LE,
    &NEW_TAI_LUE,
    &KHMER_SYMBOLS,
    &BUGINESE,
    &TAI_THAM,
    &COMBINING_DIACRITICAL_MARKS_EXTENDED,
    &BALINESE,
    &SUNDANESE,
    &BATAK,
    &LEPCHA,
    &OL_CHIKI,
    &CYRILLIC_EXTENDED_C,
    &GEORGIAN_EXTENDED,
    &SUNDANESE_SUPPLEMENT,
    &VEDIC_EXTENSIONS,
    &PHONETIC_EXTENSIONS,
    &PHONETIC_EXTENSIONS_SUPPLEMENT,
    &COMBINING_DIACRITICAL_MARKS_SUPPLEMENT,
    &LATIN_EXTENDED_ADDITIONAL,
    &GREEK_EXTENDED,
    &GENERAL_PUNCTUATION,
    &SUPERSCRIPTS_AND_SUBSCRIPTS,
    &CURRENCY_SYMBOLS,
    &COMBINING_MARKS_FOR_SYMBOLS,
    &LETTERLIKE_SYMBOLS,
    &NUMBER_FORMS,
    &ARROWS,
    &MATHEMATICAL_OPERATORS,
    &MISCELLANEOUS_TECHNICAL,
    &CONTROL_PICTURES,
    &OPTICAL_CHARACTER_RECOGNITION,
    &ENCLOSED_ALPHANUMERICS,
    &BOX_DRAWING,
    &BLOCK_ELEMENTS,
    &GEOMETRIC_SHAPES,
    &MISCELLANEOUS_SYMBOLS,
    &DINGBATS,
    &MISCELLANEOUS_MATHEMATICAL_SYMBOLS_A,
    &SUPPLEMENTAL_ARROWS_A,
    &BRAILLE_PATTERNS,
    &SUPPLEMENTAL_ARROWS_B,
    &MISCELLANEOUS_MATHEMATICAL_SYMBOLS_B,
    &SUPPLEMENTAL_MATHEMATICAL_OPERATORS,
    &MISCELLANEOUS_SYMBOLS_AND_ARROWS,
    &GLAGOLITIC,
    &LATIN_EXTENDED_C,
    &COPTIC,
    &GEORGIAN_SUPPLEMENT,
    &TIFINAGH,
    &ETHIOPIC_EXTENDED,
    &CYRILLIC_EXTENDED_A,
    &SUPPLEMENTAL_PUNCTUATION,
    &CJK_RADICALS_SUPPLEMENT,
    &KANGXI_RADICALS,
    nullptr,
    &IDEOGRAPHIC_DESCRIPTION_CHARACTERS,
    &CJK_SYMBOLS_AND_PUNCTUATION,
    &HIRAGANA,
    &KATAKANA,
    &BOPOMOFO,
    &HANGUL_COMPATIBILITY_JAMO,
    &KANBUN,
    &BOPOMOFO_EXTENDED,
    &CJK_STROKES,
    &KATAKANA_PHONETIC_EXTENSIONS,
    &ENCLOSED_CJK_LETTERS_AND_MONTHS,
    &CJK_COMPATIBILITY,
    &CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A,
    &YIJING_HEXAGRAM_SYMBOLS,
    &CJK_UNIFIED_IDEOGRAPHS,
    &YI_SYLLABLES,
    &YI_RADICALS,
    &LISU,
    &VAI,
    &CYRILLIC_EXTENDED_B,
    &BAMUM,
    &MODIFIER_TONE_LETTERS,
    &LATIN_EXTENDED_D,
    &SYLOTI_NAGRI,
    &COMMON_INDIC_NUMBER_FORMS,
    &PHAGS_PA,
    &SAURASHTRA,
    &DEVANAGARI_EXTENDED,
    &KAYAH_LI,
    &REJANG,
    &HANGUL_JAMO_EXTENDED_A,
    &JAVANESE,
    &MYANMAR_EXTENDED_B,
    &CHAM,
    &MYANMAR_EXTENDED_A,
    &TAI_VIET,
    &MEETEI_MAYEK_EXTENSIONS,
    &ETHIOPIC_EXTENDED_A,
    &LATIN_EXTENDED_E,
    &CHEROKEE_SUPPLEMENT,
    &MEETEI_MAYEK,
    &HANGUL_SYLLABLES,
    &HANGUL_JAMO_EXTENDED_B,
    &HIGH_SURROGATES,
    &HIGH_PRIVATE_USE_SURROGATES,
    &LOW_SURROGATES,
    &PRIVATE_USE_AREA,
    &CJK_COMPATIBILITY_IDEOGRAPHS,
    &ALPHABETIC_PRESENTATION_FORMS,
    &ARABIC_PRESENTATION_FORMS_A,
    &VARIATION_SELECTORS,
    &VERTICAL_FORMS,
    &COMBINING_HALF_MARKS,
    &CJK_COMPATIBILITY_FORMS,
    &SMALL_FORM_VARIANTS,
    &ARABIC_PRESENTATION_FORMS_B,
    &HALFWIDTH_AND_FULLWIDTH_FORMS,
    &SPECIALS,
    &LINEAR_B_SYLLABARY,
    &LINEAR_B_IDEOGRAMS,
    &AEGEAN_NUMBERS,
    &ANCIENT_GREEK_NUMBERS,
    &ANCIENT_SYMBOLS,
    &PHAISTOS_DISC,
    nullptr,
    &LYCIAN,
    &CARIAN,
    &COPTIC_EPACT_NUMBERS,
    &OLD_ITALIC,
    &GOTHIC,
    &OLD_PERMIC,
    &UGARITIC,
    &OLD_PERSIAN,
    nullptr,
    &DESERET,
    &SHAVIAN,
    &OSMANYA,
    &OSAGE,
    &ELBASAN,
    &CAUCASIAN_ALBANIAN,
    nullptr,
    &LINEAR_A,
    nullptr,
    &CYPRIOT_SYLLABARY,
    &IMPERIAL_ARAMAIC,
    &PALMYRENE,
    &NABATAEAN,
    nullptr,
    &HATRAN,
    &PHOENICIAN,
    &LYDIAN,
    nullptr,
    &MEROITIC_HIEROGLYPHS,
    &MEROITIC_CURSIVE,
    &KHAROSHTHI,
    &OLD_SOUTH_ARABIAN,
    &OLD_NORTH_ARABIAN,
    nullptr,
    &MANICHAEAN,
    &AVESTAN,
    &INSCRIPTIONAL_PARTHIAN,
    &INSCRIPTIONAL_PAHLAVI,
    &PSALTER_PAHLAVI,
    nullptr,
    &OLD_TURKIC,
    nullptr,
    &OLD_HUNGARIAN,
    &HANIFI_ROHINGYA,
    nullptr,
    &RUMI_NUMERAL_SYMBOLS,
    &YEZIDI,
    nullptr,
    &OLD_SOGDIAN,
    &SOGDIAN,
    nullptr,
    &CHORASMIAN,
    &ELYMAIC,
    &BRAHMI,
    &KAITHI,
    &SORA_SOMPENG,
    &CHAKMA,
    &MAHAJANI,
    &SHARADA,
    &SINHALA_ARCHAIC_NUMBERS,
    &KHOJKI,
    nullptr,
    &MULTANI,
    &KHUDAWADI,
    &GRANTHA,
    nullptr,
    &NEWA,
    &TIRHUTA,
    nullptr,
    &SIDDHAM,
    &MODI,
    &MONGOLIAN_SUPPLEMENT,
    &TAKRI,
    nullptr,
    &AHOM,
    nullptr,
    &DOGRA,
    nullptr,
    &WARANG_CITI,
    &DIVES_AKURU,
    nullptr,
    &NANDINAGARI,
    &ZANABAZAR_SQUARE,
    &SOYOMBO,
    nullptr,
    &PAU_CIN_HAU,
    nullptr,
    &BHAIKSUKI,
    &MARCHEN,
    nullptr,
    &MASARAM_GONDI,
    &GUNJALA_GONDI,
    nullptr,
    &MAKASAR,
    nullptr,
    &LISU_SUPPLEMENT,
    &TAMIL_SUPPLEMENT,
    &CUNEIFORM,
    &CUNEIFORM_NUMBERS_AND_PUNCTUATION,
    &EARLY_DYNASTIC_CUNEIFORM,
    nullptr,
    &EGYPTIAN_HIEROGLYPHS,
    &EGYPTIAN_HIEROGLYPH_FORMAT_CONTROLS,
    nullptr,
    &ANATOLIAN_HIEROGLYPHS,
    nullptr,
    &BAMUM_SUPPLEMENT,
    &MRO,
    nullptr,
    &BASSA_VAH,
    &PAHAWH_HMONG,
    nullptr,
    &MEDEFAIDRIN,
    nullptr,
    &MIAO,
    nullptr,
    &IDEOGRAPHIC_SYMBOLS_AND_PUNCTUATION,
    &TANGUT,
    &TANGUT_COMPONENTS,
    &KHITAN_SMALL_SCRIPT,
    &TANGUT_SUPPLEMENT,
    nullptr,
    &KANA_SUPPLEMENT,
    &KANA_EXTENDED_A,
    &SMALL_KANA_EXTENSION,
    &NUSHU,
    nullptr,
    &DUPLOYAN,
    &SHORTHAND_FORMAT_CONTROLS,
    nullptr,
    &BYZANTINE_MUSICAL_SYMBOLS,
    &MUSICAL_SYMBOLS,
    &ANCIENT_GREEK_MUSICAL_NOTATION,
    nullptr,
    &MAYAN_NUMERALS,
    &TAI_XUAN_JING_SYMBOLS,
    &COUNTING_ROD_NUMERALS,
    nullptr,
    &MATHEMATICAL_ALPHANUMERIC_SYMBOLS,
    &SUTTON_SIGNWRITING,
    nullptr,
    &GLAGOLITIC_SUPPLEMENT,
    nullptr,
    &NYIAKENG_PUACHUE_HMONG,
    nullptr,
    &WANCHO,
    nullptr,
    &MENDE_KIKAKUI,
    nullptr,
    &ADLAM,
    nullptr,
    &INDIC_SIYAQ_NUMBERS,
    nullptr,
    &OTTOMAN_SIYAQ_NUMBERS,
    nullptr,
    &ARABIC_MATHEMATICAL_ALPHABETIC_SYMBOLS,
    nullptr,
    &MAHJONG_TILES,
    &DOMINO_TILES,
    &PLAYING_CARDS,
    &ENCLOSED_ALPHANUMERIC_SUPPLEMENT,
    &ENCLOSED_IDEOGRAPHIC_SUPPLEMENT,
    &MISCELLANEOUS_SYMBOLS_AND_PICTOGRAPHS,
    &EMOTICONS,
    &ORNAMENTAL_DINGBATS,
    &TRANSPORT_AND_MAP_SYMBOLS,
    &ALCHEMICAL_SYMBOLS,
    &GEOMETRIC_SHAPES_EXTENDED,
    &SUPPLEMENTAL_ARROWS_C,
    &SUPPLEMENTAL_SYMBOLS_AND_PICTOGRAPHS,
    &CHESS_SYMBOLS,
    &SYMBOLS_AND_PICTOGRAPHS_EXTENDED_A,
    &SYMBOLS_FOR_LEGACY_COMPUTING,
    nullptr,
    &CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B,
    nullptr,
    &CJK_UNIFIED_IDEOGRAPHS_EXTENSION_C,
    &CJK_UNIFIED_IDEOGRAPHS_EXTENSION_D,
    &CJK_UNIFIED_IDEOGRAPHS_EXTENSION_E,
    &CJK_UNIFIED_IDEOGRAPHS_EXTENSION_F,
    nullptr,
    &CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT,
    nullptr,
    &CJK_UNIFIED_IDEOGRAPHS_EXTENSION_G,
    nullptr,
    &TAGS,
    nullptr,
    &VARIATION_SELECTORS_SUPPLEMENT,
    nullptr,
    &SUPPLEMENTARY_PRIVATE_USE_AREA_A,
    &SUPPLEMENTARY_PRIVATE_USE_AREA_B,
};

const Character::UnicodeBlock* Character::UnicodeBlock::of(char16_t c) {
    return of((int)c);
}

const Character::UnicodeBlock* Character::UnicodeBlock::of(int codePoint) {
    if (!isValidCodePoint(codePoint)) {
        return nullptr;
    }

    int top, bottom, current;
    bottom = 0;
    top = sizeof(blockStarts)/sizeof(blockStarts[0]);
    current = top/2;

    // invariant: top > current >= bottom && codePoint >= unicodeBlockStarts[bottom]
    while (top - bottom > 1) {
        if (codePoint >= blockStarts[current]) {
            bottom = current;
        } else {
            top = current;
        }
        current = (top + bottom) / 2;
    }
    return blocks[current];
}

const Character::UnicodeBlock* Character::UnicodeBlock::forName(const std::string& blockName) {
    auto it = getMap().find(blockName);//.toUpperCase(Locale.US));
    if (it == getMap().end()) {
        //throw new IllegalArgumentException("Not a valid block name: %s",blockName);
    }
    return it->second;
}

////////////////////////////////////////////////////////////////////////////////////////////
//public static enum UnicodeScript {

const int Character::UnicodeScript::scriptStarts[] = {
    0x0000,   // 0000..0040; COMMON
    0x0041,   // 0041..005A; LATIN
    0x005B,   // 005B..0060; COMMON
    0x0061,   // 0061..007A; LATIN
    0x007B,   // 007B..00A9; COMMON
    0x00AA,   // 00AA      ; LATIN
    0x00AB,   // 00AB..00B9; COMMON
    0x00BA,   // 00BA      ; LATIN
    0x00BB,   // 00BB..00BF; COMMON
    0x00C0,   // 00C0..00D6; LATIN
    0x00D7,   // 00D7      ; COMMON
    0x00D8,   // 00D8..00F6; LATIN
    0x00F7,   // 00F7      ; COMMON
    0x00F8,   // 00F8..02B8; LATIN
    0x02B9,   // 02B9..02DF; COMMON
    0x02E0,   // 02E0..02E4; LATIN
    0x02E5,   // 02E5..02E9; COMMON
    0x02EA,   // 02EA..02EB; BOPOMOFO
    0x02EC,   // 02EC..02FF; COMMON
    0x0300,   // 0300..036F; INHERITED
    0x0370,   // 0370..0373; GREEK
    0x0374,   // 0374      ; COMMON
    0x0375,   // 0375..0377; GREEK
    0x0378,   // 0378..0379; UNKNOWN
    0x037A,   // 037A..037D; GREEK
    0x037E,   // 037E      ; COMMON
    0x037F,   // 037F      ; GREEK
    0x0380,   // 0380..0383; UNKNOWN
    0x0384,   // 0384      ; GREEK
    0x0385,   // 0385      ; COMMON
    0x0386,   // 0386      ; GREEK
    0x0387,   // 0387      ; COMMON
    0x0388,   // 0388..038A; GREEK
    0x038B,   // 038B      ; UNKNOWN
    0x038C,   // 038C      ; GREEK
    0x038D,   // 038D      ; UNKNOWN
    0x038E,   // 038E..03A1; GREEK
    0x03A2,   // 03A2      ; UNKNOWN
    0x03A3,   // 03A3..03E1; GREEK
    0x03E2,   // 03E2..03EF; COPTIC
    0x03F0,   // 03F0..03FF; GREEK
    0x0400,   // 0400..0484; CYRILLIC
    0x0485,   // 0485..0486; INHERITED
    0x0487,   // 0487..052F; CYRILLIC
    0x0530,   // 0530      ; UNKNOWN
    0x0531,   // 0531..0556; ARMENIAN
    0x0557,   // 0557..0558; UNKNOWN
    0x0559,   // 0559..058A; ARMENIAN
    0x058B,   // 058B..058C; UNKNOWN
    0x058D,   // 058D..058F; ARMENIAN
    0x0590,   // 0590      ; UNKNOWN
    0x0591,   // 0591..05C7; HEBREW
    0x05C8,   // 05C8..05CF; UNKNOWN
    0x05D0,   // 05D0..05EA; HEBREW
    0x05EB,   // 05EB..05EE; UNKNOWN
    0x05EF,   // 05EF..05F4; HEBREW
    0x05F5,   // 05F5..05FF; UNKNOWN
    0x0600,   // 0600..0604; ARABIC
    0x0605,   // 0605      ; COMMON
    0x0606,   // 0606..060B; ARABIC
    0x060C,   // 060C      ; COMMON
    0x060D,   // 060D..061A; ARABIC
    0x061B,   // 061B      ; COMMON
    0x061C,   // 061C      ; ARABIC
    0x061D,   // 061D      ; UNKNOWN
    0x061E,   // 061E      ; ARABIC
    0x061F,   // 061F      ; COMMON
    0x0620,   // 0620..063F; ARABIC
    0x0640,   // 0640      ; COMMON
    0x0641,   // 0641..064A; ARABIC
    0x064B,   // 064B..0655; INHERITED
    0x0656,   // 0656..066F; ARABIC
    0x0670,   // 0670      ; INHERITED
    0x0671,   // 0671..06DC; ARABIC
    0x06DD,   // 06DD      ; COMMON
    0x06DE,   // 06DE..06FF; ARABIC
    0x0700,   // 0700..070D; SYRIAC
    0x070E,   // 070E      ; UNKNOWN
    0x070F,   // 070F..074A; SYRIAC
    0x074B,   // 074B..074C; UNKNOWN
    0x074D,   // 074D..074F; SYRIAC
    0x0750,   // 0750..077F; ARABIC
    0x0780,   // 0780..07B1; THAANA
    0x07B2,   // 07B2..07BF; UNKNOWN
    0x07C0,   // 07C0..07FA; NKO
    0x07FB,   // 07FB..07FC; UNKNOWN
    0x07FD,   // 07FD..07FF; NKO
    0x0800,   // 0800..082D; SAMARITAN
    0x082E,   // 082E..082F; UNKNOWN
    0x0830,   // 0830..083E; SAMARITAN
    0x083F,   // 083F      ; UNKNOWN
    0x0840,   // 0840..085B; MANDAIC
    0x085C,   // 085C..085D; UNKNOWN
    0x085E,   // 085E      ; MANDAIC
    0x085F,   // 085F      ; UNKNOWN
    0x0860,   // 0860..086A; SYRIAC
    0x086B,   // 086B..089F; UNKNOWN
    0x08A0,   // 08A0..08B4; ARABIC
    0x08B5,   // 08B5      ; UNKNOWN
    0x08B6,   // 08B6..08C7; ARABIC
    0x08C8,   // 08C8..08D2; UNKNOWN
    0x08D3,   // 08D3..08E1; ARABIC
    0x08E2,   // 08E2      ; COMMON
    0x08E3,   // 08E3..08FF; ARABIC
    0x0900,   // 0900..0950; DEVANAGARI
    0x0951,   // 0951..0954; INHERITED
    0x0955,   // 0955..0963; DEVANAGARI
    0x0964,   // 0964..0965; COMMON
    0x0966,   // 0966..097F; DEVANAGARI
    0x0980,   // 0980..0983; BENGALI
    0x0984,   // 0984      ; UNKNOWN
    0x0985,   // 0985..098C; BENGALI
    0x098D,   // 098D..098E; UNKNOWN
    0x098F,   // 098F..0990; BENGALI
    0x0991,   // 0991..0992; UNKNOWN
    0x0993,   // 0993..09A8; BENGALI
    0x09A9,   // 09A9      ; UNKNOWN
    0x09AA,   // 09AA..09B0; BENGALI
    0x09B1,   // 09B1      ; UNKNOWN
    0x09B2,   // 09B2      ; BENGALI
    0x09B3,   // 09B3..09B5; UNKNOWN
    0x09B6,   // 09B6..09B9; BENGALI
    0x09BA,   // 09BA..09BB; UNKNOWN
    0x09BC,   // 09BC..09C4; BENGALI
    0x09C5,   // 09C5..09C6; UNKNOWN
    0x09C7,   // 09C7..09C8; BENGALI
    0x09C9,   // 09C9..09CA; UNKNOWN
    0x09CB,   // 09CB..09CE; BENGALI
    0x09CF,   // 09CF..09D6; UNKNOWN
    0x09D7,   // 09D7      ; BENGALI
    0x09D8,   // 09D8..09DB; UNKNOWN
    0x09DC,   // 09DC..09DD; BENGALI
    0x09DE,   // 09DE      ; UNKNOWN
    0x09DF,   // 09DF..09E3; BENGALI
    0x09E4,   // 09E4..09E5; UNKNOWN
    0x09E6,   // 09E6..09FE; BENGALI
    0x09FF,   // 09FF..0A00; UNKNOWN
    0x0A01,   // 0A01..0A03; GURMUKHI
    0x0A04,   // 0A04      ; UNKNOWN
    0x0A05,   // 0A05..0A0A; GURMUKHI
    0x0A0B,   // 0A0B..0A0E; UNKNOWN
    0x0A0F,   // 0A0F..0A10; GURMUKHI
    0x0A11,   // 0A11..0A12; UNKNOWN
    0x0A13,   // 0A13..0A28; GURMUKHI
    0x0A29,   // 0A29      ; UNKNOWN
    0x0A2A,   // 0A2A..0A30; GURMUKHI
    0x0A31,   // 0A31      ; UNKNOWN
    0x0A32,   // 0A32..0A33; GURMUKHI
    0x0A34,   // 0A34      ; UNKNOWN
    0x0A35,   // 0A35..0A36; GURMUKHI
    0x0A37,   // 0A37      ; UNKNOWN
    0x0A38,   // 0A38..0A39; GURMUKHI
    0x0A3A,   // 0A3A..0A3B; UNKNOWN
    0x0A3C,   // 0A3C      ; GURMUKHI
    0x0A3D,   // 0A3D      ; UNKNOWN
    0x0A3E,   // 0A3E..0A42; GURMUKHI
    0x0A43,   // 0A43..0A46; UNKNOWN
    0x0A47,   // 0A47..0A48; GURMUKHI
    0x0A49,   // 0A49..0A4A; UNKNOWN
    0x0A4B,   // 0A4B..0A4D; GURMUKHI
    0x0A4E,   // 0A4E..0A50; UNKNOWN
    0x0A51,   // 0A51      ; GURMUKHI
    0x0A52,   // 0A52..0A58; UNKNOWN
    0x0A59,   // 0A59..0A5C; GURMUKHI
    0x0A5D,   // 0A5D      ; UNKNOWN
    0x0A5E,   // 0A5E      ; GURMUKHI
    0x0A5F,   // 0A5F..0A65; UNKNOWN
    0x0A66,   // 0A66..0A76; GURMUKHI
    0x0A77,   // 0A77..0A80; UNKNOWN
    0x0A81,   // 0A81..0A83; GUJARATI
    0x0A84,   // 0A84      ; UNKNOWN
    0x0A85,   // 0A85..0A8D; GUJARATI
    0x0A8E,   // 0A8E      ; UNKNOWN
    0x0A8F,   // 0A8F..0A91; GUJARATI
    0x0A92,   // 0A92      ; UNKNOWN
    0x0A93,   // 0A93..0AA8; GUJARATI
    0x0AA9,   // 0AA9      ; UNKNOWN
    0x0AAA,   // 0AAA..0AB0; GUJARATI
    0x0AB1,   // 0AB1      ; UNKNOWN
    0x0AB2,   // 0AB2..0AB3; GUJARATI
    0x0AB4,   // 0AB4      ; UNKNOWN
    0x0AB5,   // 0AB5..0AB9; GUJARATI
    0x0ABA,   // 0ABA..0ABB; UNKNOWN
    0x0ABC,   // 0ABC..0AC5; GUJARATI
    0x0AC6,   // 0AC6      ; UNKNOWN
    0x0AC7,   // 0AC7..0AC9; GUJARATI
    0x0ACA,   // 0ACA      ; UNKNOWN
    0x0ACB,   // 0ACB..0ACD; GUJARATI
    0x0ACE,   // 0ACE..0ACF; UNKNOWN
    0x0AD0,   // 0AD0      ; GUJARATI
    0x0AD1,   // 0AD1..0ADF; UNKNOWN
    0x0AE0,   // 0AE0..0AE3; GUJARATI
    0x0AE4,   // 0AE4..0AE5; UNKNOWN
    0x0AE6,   // 0AE6..0AF1; GUJARATI
    0x0AF2,   // 0AF2..0AF8; UNKNOWN
    0x0AF9,   // 0AF9..0AFF; GUJARATI
    0x0B00,   // 0B00      ; UNKNOWN
    0x0B01,   // 0B01..0B03; ORIYA
    0x0B04,   // 0B04      ; UNKNOWN
    0x0B05,   // 0B05..0B0C; ORIYA
    0x0B0D,   // 0B0D..0B0E; UNKNOWN
    0x0B0F,   // 0B0F..0B10; ORIYA
    0x0B11,   // 0B11..0B12; UNKNOWN
    0x0B13,   // 0B13..0B28; ORIYA
    0x0B29,   // 0B29      ; UNKNOWN
    0x0B2A,   // 0B2A..0B30; ORIYA
    0x0B31,   // 0B31      ; UNKNOWN
    0x0B32,   // 0B32..0B33; ORIYA
    0x0B34,   // 0B34      ; UNKNOWN
    0x0B35,   // 0B35..0B39; ORIYA
    0x0B3A,   // 0B3A..0B3B; UNKNOWN
    0x0B3C,   // 0B3C..0B44; ORIYA
    0x0B45,   // 0B45..0B46; UNKNOWN
    0x0B47,   // 0B47..0B48; ORIYA
    0x0B49,   // 0B49..0B4A; UNKNOWN
    0x0B4B,   // 0B4B..0B4D; ORIYA
    0x0B4E,   // 0B4E..0B54; UNKNOWN
    0x0B55,   // 0B55..0B57; ORIYA
    0x0B58,   // 0B58..0B5B; UNKNOWN
    0x0B5C,   // 0B5C..0B5D; ORIYA
    0x0B5E,   // 0B5E      ; UNKNOWN
    0x0B5F,   // 0B5F..0B63; ORIYA
    0x0B64,   // 0B64..0B65; UNKNOWN
    0x0B66,   // 0B66..0B77; ORIYA
    0x0B78,   // 0B78..0B81; UNKNOWN
    0x0B82,   // 0B82..0B83; TAMIL
    0x0B84,   // 0B84      ; UNKNOWN
    0x0B85,   // 0B85..0B8A; TAMIL
    0x0B8B,   // 0B8B..0B8D; UNKNOWN
    0x0B8E,   // 0B8E..0B90; TAMIL
    0x0B91,   // 0B91      ; UNKNOWN
    0x0B92,   // 0B92..0B95; TAMIL
    0x0B96,   // 0B96..0B98; UNKNOWN
    0x0B99,   // 0B99..0B9A; TAMIL
    0x0B9B,   // 0B9B      ; UNKNOWN
    0x0B9C,   // 0B9C      ; TAMIL
    0x0B9D,   // 0B9D      ; UNKNOWN
    0x0B9E,   // 0B9E..0B9F; TAMIL
    0x0BA0,   // 0BA0..0BA2; UNKNOWN
    0x0BA3,   // 0BA3..0BA4; TAMIL
    0x0BA5,   // 0BA5..0BA7; UNKNOWN
    0x0BA8,   // 0BA8..0BAA; TAMIL
    0x0BAB,   // 0BAB..0BAD; UNKNOWN
    0x0BAE,   // 0BAE..0BB9; TAMIL
    0x0BBA,   // 0BBA..0BBD; UNKNOWN
    0x0BBE,   // 0BBE..0BC2; TAMIL
    0x0BC3,   // 0BC3..0BC5; UNKNOWN
    0x0BC6,   // 0BC6..0BC8; TAMIL
    0x0BC9,   // 0BC9      ; UNKNOWN
    0x0BCA,   // 0BCA..0BCD; TAMIL
    0x0BCE,   // 0BCE..0BCF; UNKNOWN
    0x0BD0,   // 0BD0      ; TAMIL
    0x0BD1,   // 0BD1..0BD6; UNKNOWN
    0x0BD7,   // 0BD7      ; TAMIL
    0x0BD8,   // 0BD8..0BE5; UNKNOWN
    0x0BE6,   // 0BE6..0BFA; TAMIL
    0x0BFB,   // 0BFB..0BFF; UNKNOWN
    0x0C00,   // 0C00..0C0C; TELUGU
    0x0C0D,   // 0C0D      ; UNKNOWN
    0x0C0E,   // 0C0E..0C10; TELUGU
    0x0C11,   // 0C11      ; UNKNOWN
    0x0C12,   // 0C12..0C28; TELUGU
    0x0C29,   // 0C29      ; UNKNOWN
    0x0C2A,   // 0C2A..0C39; TELUGU
    0x0C3A,   // 0C3A..0C3C; UNKNOWN
    0x0C3D,   // 0C3D..0C44; TELUGU
    0x0C45,   // 0C45      ; UNKNOWN
    0x0C46,   // 0C46..0C48; TELUGU
    0x0C49,   // 0C49      ; UNKNOWN
    0x0C4A,   // 0C4A..0C4D; TELUGU
    0x0C4E,   // 0C4E..0C54; UNKNOWN
    0x0C55,   // 0C55..0C56; TELUGU
    0x0C57,   // 0C57      ; UNKNOWN
    0x0C58,   // 0C58..0C5A; TELUGU
    0x0C5B,   // 0C5B..0C5F; UNKNOWN
    0x0C60,   // 0C60..0C63; TELUGU
    0x0C64,   // 0C64..0C65; UNKNOWN
    0x0C66,   // 0C66..0C6F; TELUGU
    0x0C70,   // 0C70..0C76; UNKNOWN
    0x0C77,   // 0C77..0C7F; TELUGU
    0x0C80,   // 0C80..0C8C; KANNADA
    0x0C8D,   // 0C8D      ; UNKNOWN
    0x0C8E,   // 0C8E..0C90; KANNADA
    0x0C91,   // 0C91      ; UNKNOWN
    0x0C92,   // 0C92..0CA8; KANNADA
    0x0CA9,   // 0CA9      ; UNKNOWN
    0x0CAA,   // 0CAA..0CB3; KANNADA
    0x0CB4,   // 0CB4      ; UNKNOWN
    0x0CB5,   // 0CB5..0CB9; KANNADA
    0x0CBA,   // 0CBA..0CBB; UNKNOWN
    0x0CBC,   // 0CBC..0CC4; KANNADA
    0x0CC5,   // 0CC5      ; UNKNOWN
    0x0CC6,   // 0CC6..0CC8; KANNADA
    0x0CC9,   // 0CC9      ; UNKNOWN
    0x0CCA,   // 0CCA..0CCD; KANNADA
    0x0CCE,   // 0CCE..0CD4; UNKNOWN
    0x0CD5,   // 0CD5..0CD6; KANNADA
    0x0CD7,   // 0CD7..0CDD; UNKNOWN
    0x0CDE,   // 0CDE      ; KANNADA
    0x0CDF,   // 0CDF      ; UNKNOWN
    0x0CE0,   // 0CE0..0CE3; KANNADA
    0x0CE4,   // 0CE4..0CE5; UNKNOWN
    0x0CE6,   // 0CE6..0CEF; KANNADA
    0x0CF0,   // 0CF0      ; UNKNOWN
    0x0CF1,   // 0CF1..0CF2; KANNADA
    0x0CF3,   // 0CF3..0CFF; UNKNOWN
    0x0D00,   // 0D00..0D0C; MALAYALAM
    0x0D0D,   // 0D0D      ; UNKNOWN
    0x0D0E,   // 0D0E..0D10; MALAYALAM
    0x0D11,   // 0D11      ; UNKNOWN
    0x0D12,   // 0D12..0D44; MALAYALAM
    0x0D45,   // 0D45      ; UNKNOWN
    0x0D46,   // 0D46..0D48; MALAYALAM
    0x0D49,   // 0D49      ; UNKNOWN
    0x0D4A,   // 0D4A..0D4F; MALAYALAM
    0x0D50,   // 0D50..0D53; UNKNOWN
    0x0D54,   // 0D54..0D63; MALAYALAM
    0x0D64,   // 0D64..0D65; UNKNOWN
    0x0D66,   // 0D66..0D7F; MALAYALAM
    0x0D80,   // 0D80      ; UNKNOWN
    0x0D81,   // 0D81..0D83; SINHALA
    0x0D84,   // 0D84      ; UNKNOWN
    0x0D85,   // 0D85..0D96; SINHALA
    0x0D97,   // 0D97..0D99; UNKNOWN
    0x0D9A,   // 0D9A..0DB1; SINHALA
    0x0DB2,   // 0DB2      ; UNKNOWN
    0x0DB3,   // 0DB3..0DBB; SINHALA
    0x0DBC,   // 0DBC      ; UNKNOWN
    0x0DBD,   // 0DBD      ; SINHALA
    0x0DBE,   // 0DBE..0DBF; UNKNOWN
    0x0DC0,   // 0DC0..0DC6; SINHALA
    0x0DC7,   // 0DC7..0DC9; UNKNOWN
    0x0DCA,   // 0DCA      ; SINHALA
    0x0DCB,   // 0DCB..0DCE; UNKNOWN
    0x0DCF,   // 0DCF..0DD4; SINHALA
    0x0DD5,   // 0DD5      ; UNKNOWN
    0x0DD6,   // 0DD6      ; SINHALA
    0x0DD7,   // 0DD7      ; UNKNOWN
    0x0DD8,   // 0DD8..0DDF; SINHALA
    0x0DE0,   // 0DE0..0DE5; UNKNOWN
    0x0DE6,   // 0DE6..0DEF; SINHALA
    0x0DF0,   // 0DF0..0DF1; UNKNOWN
    0x0DF2,   // 0DF2..0DF4; SINHALA
    0x0DF5,   // 0DF5..0E00; UNKNOWN
    0x0E01,   // 0E01..0E3A; THAI
    0x0E3B,   // 0E3B..0E3E; UNKNOWN
    0x0E3F,   // 0E3F      ; COMMON
    0x0E40,   // 0E40..0E5B; THAI
    0x0E5C,   // 0E5C..0E80; UNKNOWN
    0x0E81,   // 0E81..0E82; LAO
    0x0E83,   // 0E83      ; UNKNOWN
    0x0E84,   // 0E84      ; LAO
    0x0E85,   // 0E85      ; UNKNOWN
    0x0E86,   // 0E86..0E8A; LAO
    0x0E8B,   // 0E8B      ; UNKNOWN
    0x0E8C,   // 0E8C..0EA3; LAO
    0x0EA4,   // 0EA4      ; UNKNOWN
    0x0EA5,   // 0EA5      ; LAO
    0x0EA6,   // 0EA6      ; UNKNOWN
    0x0EA7,   // 0EA7..0EBD; LAO
    0x0EBE,   // 0EBE..0EBF; UNKNOWN
    0x0EC0,   // 0EC0..0EC4; LAO
    0x0EC5,   // 0EC5      ; UNKNOWN
    0x0EC6,   // 0EC6      ; LAO
    0x0EC7,   // 0EC7      ; UNKNOWN
    0x0EC8,   // 0EC8..0ECD; LAO
    0x0ECE,   // 0ECE..0ECF; UNKNOWN
    0x0ED0,   // 0ED0..0ED9; LAO
    0x0EDA,   // 0EDA..0EDB; UNKNOWN
    0x0EDC,   // 0EDC..0EDF; LAO
    0x0EE0,   // 0EE0..0EFF; UNKNOWN
    0x0F00,   // 0F00..0F47; TIBETAN
    0x0F48,   // 0F48      ; UNKNOWN
    0x0F49,   // 0F49..0F6C; TIBETAN
    0x0F6D,   // 0F6D..0F70; UNKNOWN
    0x0F71,   // 0F71..0F97; TIBETAN
    0x0F98,   // 0F98      ; UNKNOWN
    0x0F99,   // 0F99..0FBC; TIBETAN
    0x0FBD,   // 0FBD      ; UNKNOWN
    0x0FBE,   // 0FBE..0FCC; TIBETAN
    0x0FCD,   // 0FCD      ; UNKNOWN
    0x0FCE,   // 0FCE..0FD4; TIBETAN
    0x0FD5,   // 0FD5..0FD8; COMMON
    0x0FD9,   // 0FD9..0FDA; TIBETAN
    0x0FDB,   // 0FDB..0FFF; UNKNOWN
    0x1000,   // 1000..109F; MYANMAR
    0x10A0,   // 10A0..10C5; GEORGIAN
    0x10C6,   // 10C6      ; UNKNOWN
    0x10C7,   // 10C7      ; GEORGIAN
    0x10C8,   // 10C8..10CC; UNKNOWN
    0x10CD,   // 10CD      ; GEORGIAN
    0x10CE,   // 10CE..10CF; UNKNOWN
    0x10D0,   // 10D0..10FA; GEORGIAN
    0x10FB,   // 10FB      ; COMMON
    0x10FC,   // 10FC..10FF; GEORGIAN
    0x1100,   // 1100..11FF; HANGUL
    0x1200,   // 1200..1248; ETHIOPIC
    0x1249,   // 1249      ; UNKNOWN
    0x124A,   // 124A..124D; ETHIOPIC
    0x124E,   // 124E..124F; UNKNOWN
    0x1250,   // 1250..1256; ETHIOPIC
    0x1257,   // 1257      ; UNKNOWN
    0x1258,   // 1258      ; ETHIOPIC
    0x1259,   // 1259      ; UNKNOWN
    0x125A,   // 125A..125D; ETHIOPIC
    0x125E,   // 125E..125F; UNKNOWN
    0x1260,   // 1260..1288; ETHIOPIC
    0x1289,   // 1289      ; UNKNOWN
    0x128A,   // 128A..128D; ETHIOPIC
    0x128E,   // 128E..128F; UNKNOWN
    0x1290,   // 1290..12B0; ETHIOPIC
    0x12B1,   // 12B1      ; UNKNOWN
    0x12B2,   // 12B2..12B5; ETHIOPIC
    0x12B6,   // 12B6..12B7; UNKNOWN
    0x12B8,   // 12B8..12BE; ETHIOPIC
    0x12BF,   // 12BF      ; UNKNOWN
    0x12C0,   // 12C0      ; ETHIOPIC
    0x12C1,   // 12C1      ; UNKNOWN
    0x12C2,   // 12C2..12C5; ETHIOPIC
    0x12C6,   // 12C6..12C7; UNKNOWN
    0x12C8,   // 12C8..12D6; ETHIOPIC
    0x12D7,   // 12D7      ; UNKNOWN
    0x12D8,   // 12D8..1310; ETHIOPIC
    0x1311,   // 1311      ; UNKNOWN
    0x1312,   // 1312..1315; ETHIOPIC
    0x1316,   // 1316..1317; UNKNOWN
    0x1318,   // 1318..135A; ETHIOPIC
    0x135B,   // 135B..135C; UNKNOWN
    0x135D,   // 135D..137C; ETHIOPIC
    0x137D,   // 137D..137F; UNKNOWN
    0x1380,   // 1380..1399; ETHIOPIC
    0x139A,   // 139A..139F; UNKNOWN
    0x13A0,   // 13A0..13F5; CHEROKEE
    0x13F6,   // 13F6..13F7; UNKNOWN
    0x13F8,   // 13F8..13FD; CHEROKEE
    0x13FE,   // 13FE..13FF; UNKNOWN
    0x1400,   // 1400..167F; CANADIAN_ABORIGINAL
    0x1680,   // 1680..169C; OGHAM
    0x169D,   // 169D..169F; UNKNOWN
    0x16A0,   // 16A0..16EA; RUNIC
    0x16EB,   // 16EB..16ED; COMMON
    0x16EE,   // 16EE..16F8; RUNIC
    0x16F9,   // 16F9..16FF; UNKNOWN
    0x1700,   // 1700..170C; TAGALOG
    0x170D,   // 170D      ; UNKNOWN
    0x170E,   // 170E..1714; TAGALOG
    0x1715,   // 1715..171F; UNKNOWN
    0x1720,   // 1720..1734; HANUNOO
    0x1735,   // 1735..1736; COMMON
    0x1737,   // 1737..173F; UNKNOWN
    0x1740,   // 1740..1753; BUHID
    0x1754,   // 1754..175F; UNKNOWN
    0x1760,   // 1760..176C; TAGBANWA
    0x176D,   // 176D      ; UNKNOWN
    0x176E,   // 176E..1770; TAGBANWA
    0x1771,   // 1771      ; UNKNOWN
    0x1772,   // 1772..1773; TAGBANWA
    0x1774,   // 1774..177F; UNKNOWN
    0x1780,   // 1780..17DD; KHMER
    0x17DE,   // 17DE..17DF; UNKNOWN
    0x17E0,   // 17E0..17E9; KHMER
    0x17EA,   // 17EA..17EF; UNKNOWN
    0x17F0,   // 17F0..17F9; KHMER
    0x17FA,   // 17FA..17FF; UNKNOWN
    0x1800,   // 1800..1801; MONGOLIAN
    0x1802,   // 1802..1803; COMMON
    0x1804,   // 1804      ; MONGOLIAN
    0x1805,   // 1805      ; COMMON
    0x1806,   // 1806..180E; MONGOLIAN
    0x180F,   // 180F      ; UNKNOWN
    0x1810,   // 1810..1819; MONGOLIAN
    0x181A,   // 181A..181F; UNKNOWN
    0x1820,   // 1820..1878; MONGOLIAN
    0x1879,   // 1879..187F; UNKNOWN
    0x1880,   // 1880..18AA; MONGOLIAN
    0x18AB,   // 18AB..18AF; UNKNOWN
    0x18B0,   // 18B0..18F5; CANADIAN_ABORIGINAL
    0x18F6,   // 18F6..18FF; UNKNOWN
    0x1900,   // 1900..191E; LIMBU
    0x191F,   // 191F      ; UNKNOWN
    0x1920,   // 1920..192B; LIMBU
    0x192C,   // 192C..192F; UNKNOWN
    0x1930,   // 1930..193B; LIMBU
    0x193C,   // 193C..193F; UNKNOWN
    0x1940,   // 1940      ; LIMBU
    0x1941,   // 1941..1943; UNKNOWN
    0x1944,   // 1944..194F; LIMBU
    0x1950,   // 1950..196D; TAI_LE
    0x196E,   // 196E..196F; UNKNOWN
    0x1970,   // 1970..1974; TAI_LE
    0x1975,   // 1975..197F; UNKNOWN
    0x1980,   // 1980..19AB; NEW_TAI_LUE
    0x19AC,   // 19AC..19AF; UNKNOWN
    0x19B0,   // 19B0..19C9; NEW_TAI_LUE
    0x19CA,   // 19CA..19CF; UNKNOWN
    0x19D0,   // 19D0..19DA; NEW_TAI_LUE
    0x19DB,   // 19DB..19DD; UNKNOWN
    0x19DE,   // 19DE..19DF; NEW_TAI_LUE
    0x19E0,   // 19E0..19FF; KHMER
    0x1A00,   // 1A00..1A1B; BUGINESE
    0x1A1C,   // 1A1C..1A1D; UNKNOWN
    0x1A1E,   // 1A1E..1A1F; BUGINESE
    0x1A20,   // 1A20..1A5E; TAI_THAM
    0x1A5F,   // 1A5F      ; UNKNOWN
    0x1A60,   // 1A60..1A7C; TAI_THAM
    0x1A7D,   // 1A7D..1A7E; UNKNOWN
    0x1A7F,   // 1A7F..1A89; TAI_THAM
    0x1A8A,   // 1A8A..1A8F; UNKNOWN
    0x1A90,   // 1A90..1A99; TAI_THAM
    0x1A9A,   // 1A9A..1A9F; UNKNOWN
    0x1AA0,   // 1AA0..1AAD; TAI_THAM
    0x1AAE,   // 1AAE..1AAF; UNKNOWN
    0x1AB0,   // 1AB0..1AC0; INHERITED
    0x1AC1,   // 1AC1..1AFF; UNKNOWN
    0x1B00,   // 1B00..1B4B; BALINESE
    0x1B4C,   // 1B4C..1B4F; UNKNOWN
    0x1B50,   // 1B50..1B7C; BALINESE
    0x1B7D,   // 1B7D..1B7F; UNKNOWN
    0x1B80,   // 1B80..1BBF; SUNDANESE
    0x1BC0,   // 1BC0..1BF3; BATAK
    0x1BF4,   // 1BF4..1BFB; UNKNOWN
    0x1BFC,   // 1BFC..1BFF; BATAK
    0x1C00,   // 1C00..1C37; LEPCHA
    0x1C38,   // 1C38..1C3A; UNKNOWN
    0x1C3B,   // 1C3B..1C49; LEPCHA
    0x1C4A,   // 1C4A..1C4C; UNKNOWN
    0x1C4D,   // 1C4D..1C4F; LEPCHA
    0x1C50,   // 1C50..1C7F; OL_CHIKI
    0x1C80,   // 1C80..1C88; CYRILLIC
    0x1C89,   // 1C89..1C8F; UNKNOWN
    0x1C90,   // 1C90..1CBA; GEORGIAN
    0x1CBB,   // 1CBB..1CBC; UNKNOWN
    0x1CBD,   // 1CBD..1CBF; GEORGIAN
    0x1CC0,   // 1CC0..1CC7; SUNDANESE
    0x1CC8,   // 1CC8..1CCF; UNKNOWN
    0x1CD0,   // 1CD0..1CD2; INHERITED
    0x1CD3,   // 1CD3      ; COMMON
    0x1CD4,   // 1CD4..1CE0; INHERITED
    0x1CE1,   // 1CE1      ; COMMON
    0x1CE2,   // 1CE2..1CE8; INHERITED
    0x1CE9,   // 1CE9..1CEC; COMMON
    0x1CED,   // 1CED      ; INHERITED
    0x1CEE,   // 1CEE..1CF3; COMMON
    0x1CF4,   // 1CF4      ; INHERITED
    0x1CF5,   // 1CF5..1CF7; COMMON
    0x1CF8,   // 1CF8..1CF9; INHERITED
    0x1CFA,   // 1CFA      ; COMMON
    0x1CFB,   // 1CFB..1CFF; UNKNOWN
    0x1D00,   // 1D00..1D25; LATIN
    0x1D26,   // 1D26..1D2A; GREEK
    0x1D2B,   // 1D2B      ; CYRILLIC
    0x1D2C,   // 1D2C..1D5C; LATIN
    0x1D5D,   // 1D5D..1D61; GREEK
    0x1D62,   // 1D62..1D65; LATIN
    0x1D66,   // 1D66..1D6A; GREEK
    0x1D6B,   // 1D6B..1D77; LATIN
    0x1D78,   // 1D78      ; CYRILLIC
    0x1D79,   // 1D79..1DBE; LATIN
    0x1DBF,   // 1DBF      ; GREEK
    0x1DC0,   // 1DC0..1DF9; INHERITED
    0x1DFA,   // 1DFA      ; UNKNOWN
    0x1DFB,   // 1DFB..1DFF; INHERITED
    0x1E00,   // 1E00..1EFF; LATIN
    0x1F00,   // 1F00..1F15; GREEK
    0x1F16,   // 1F16..1F17; UNKNOWN
    0x1F18,   // 1F18..1F1D; GREEK
    0x1F1E,   // 1F1E..1F1F; UNKNOWN
    0x1F20,   // 1F20..1F45; GREEK
    0x1F46,   // 1F46..1F47; UNKNOWN
    0x1F48,   // 1F48..1F4D; GREEK
    0x1F4E,   // 1F4E..1F4F; UNKNOWN
    0x1F50,   // 1F50..1F57; GREEK
    0x1F58,   // 1F58      ; UNKNOWN
    0x1F59,   // 1F59      ; GREEK
    0x1F5A,   // 1F5A      ; UNKNOWN
    0x1F5B,   // 1F5B      ; GREEK
    0x1F5C,   // 1F5C      ; UNKNOWN
    0x1F5D,   // 1F5D      ; GREEK
    0x1F5E,   // 1F5E      ; UNKNOWN
    0x1F5F,   // 1F5F..1F7D; GREEK
    0x1F7E,   // 1F7E..1F7F; UNKNOWN
    0x1F80,   // 1F80..1FB4; GREEK
    0x1FB5,   // 1FB5      ; UNKNOWN
    0x1FB6,   // 1FB6..1FC4; GREEK
    0x1FC5,   // 1FC5      ; UNKNOWN
    0x1FC6,   // 1FC6..1FD3; GREEK
    0x1FD4,   // 1FD4..1FD5; UNKNOWN
    0x1FD6,   // 1FD6..1FDB; GREEK
    0x1FDC,   // 1FDC      ; UNKNOWN
    0x1FDD,   // 1FDD..1FEF; GREEK
    0x1FF0,   // 1FF0..1FF1; UNKNOWN
    0x1FF2,   // 1FF2..1FF4; GREEK
    0x1FF5,   // 1FF5      ; UNKNOWN
    0x1FF6,   // 1FF6..1FFE; GREEK
    0x1FFF,   // 1FFF      ; UNKNOWN
    0x2000,   // 2000..200B; COMMON
    0x200C,   // 200C..200D; INHERITED
    0x200E,   // 200E..2064; COMMON
    0x2065,   // 2065      ; UNKNOWN
    0x2066,   // 2066..2070; COMMON
    0x2071,   // 2071      ; LATIN
    0x2072,   // 2072..2073; UNKNOWN
    0x2074,   // 2074..207E; COMMON
    0x207F,   // 207F      ; LATIN
    0x2080,   // 2080..208E; COMMON
    0x208F,   // 208F      ; UNKNOWN
    0x2090,   // 2090..209C; LATIN
    0x209D,   // 209D..209F; UNKNOWN
    0x20A0,   // 20A0..20BF; COMMON
    0x20C0,   // 20C0..20CF; UNKNOWN
    0x20D0,   // 20D0..20F0; INHERITED
    0x20F1,   // 20F1..20FF; UNKNOWN
    0x2100,   // 2100..2125; COMMON
    0x2126,   // 2126      ; GREEK
    0x2127,   // 2127..2129; COMMON
    0x212A,   // 212A..212B; LATIN
    0x212C,   // 212C..2131; COMMON
    0x2132,   // 2132      ; LATIN
    0x2133,   // 2133..214D; COMMON
    0x214E,   // 214E      ; LATIN
    0x214F,   // 214F..215F; COMMON
    0x2160,   // 2160..2188; LATIN
    0x2189,   // 2189..218B; COMMON
    0x218C,   // 218C..218F; UNKNOWN
    0x2190,   // 2190..2426; COMMON
    0x2427,   // 2427..243F; UNKNOWN
    0x2440,   // 2440..244A; COMMON
    0x244B,   // 244B..245F; UNKNOWN
    0x2460,   // 2460..27FF; COMMON
    0x2800,   // 2800..28FF; BRAILLE
    0x2900,   // 2900..2B73; COMMON
    0x2B74,   // 2B74..2B75; UNKNOWN
    0x2B76,   // 2B76..2B95; COMMON
    0x2B96,   // 2B96      ; UNKNOWN
    0x2B97,   // 2B97..2BFF; COMMON
    0x2C00,   // 2C00..2C2E; GLAGOLITIC
    0x2C2F,   // 2C2F      ; UNKNOWN
    0x2C30,   // 2C30..2C5E; GLAGOLITIC
    0x2C5F,   // 2C5F      ; UNKNOWN
    0x2C60,   // 2C60..2C7F; LATIN
    0x2C80,   // 2C80..2CF3; COPTIC
    0x2CF4,   // 2CF4..2CF8; UNKNOWN
    0x2CF9,   // 2CF9..2CFF; COPTIC
    0x2D00,   // 2D00..2D25; GEORGIAN
    0x2D26,   // 2D26      ; UNKNOWN
    0x2D27,   // 2D27      ; GEORGIAN
    0x2D28,   // 2D28..2D2C; UNKNOWN
    0x2D2D,   // 2D2D      ; GEORGIAN
    0x2D2E,   // 2D2E..2D2F; UNKNOWN
    0x2D30,   // 2D30..2D67; TIFINAGH
    0x2D68,   // 2D68..2D6E; UNKNOWN
    0x2D6F,   // 2D6F..2D70; TIFINAGH
    0x2D71,   // 2D71..2D7E; UNKNOWN
    0x2D7F,   // 2D7F      ; TIFINAGH
    0x2D80,   // 2D80..2D96; ETHIOPIC
    0x2D97,   // 2D97..2D9F; UNKNOWN
    0x2DA0,   // 2DA0..2DA6; ETHIOPIC
    0x2DA7,   // 2DA7      ; UNKNOWN
    0x2DA8,   // 2DA8..2DAE; ETHIOPIC
    0x2DAF,   // 2DAF      ; UNKNOWN
    0x2DB0,   // 2DB0..2DB6; ETHIOPIC
    0x2DB7,   // 2DB7      ; UNKNOWN
    0x2DB8,   // 2DB8..2DBE; ETHIOPIC
    0x2DBF,   // 2DBF      ; UNKNOWN
    0x2DC0,   // 2DC0..2DC6; ETHIOPIC
    0x2DC7,   // 2DC7      ; UNKNOWN
    0x2DC8,   // 2DC8..2DCE; ETHIOPIC
    0x2DCF,   // 2DCF      ; UNKNOWN
    0x2DD0,   // 2DD0..2DD6; ETHIOPIC
    0x2DD7,   // 2DD7      ; UNKNOWN
    0x2DD8,   // 2DD8..2DDE; ETHIOPIC
    0x2DDF,   // 2DDF      ; UNKNOWN
    0x2DE0,   // 2DE0..2DFF; CYRILLIC
    0x2E00,   // 2E00..2E52; COMMON
    0x2E53,   // 2E53..2E7F; UNKNOWN
    0x2E80,   // 2E80..2E99; HAN
    0x2E9A,   // 2E9A      ; UNKNOWN
    0x2E9B,   // 2E9B..2EF3; HAN
    0x2EF4,   // 2EF4..2EFF; UNKNOWN
    0x2F00,   // 2F00..2FD5; HAN
    0x2FD6,   // 2FD6..2FEF; UNKNOWN
    0x2FF0,   // 2FF0..2FFB; COMMON
    0x2FFC,   // 2FFC..2FFF; UNKNOWN
    0x3000,   // 3000..3004; COMMON
    0x3005,   // 3005      ; HAN
    0x3006,   // 3006      ; COMMON
    0x3007,   // 3007      ; HAN
    0x3008,   // 3008..3020; COMMON
    0x3021,   // 3021..3029; HAN
    0x302A,   // 302A..302D; INHERITED
    0x302E,   // 302E..302F; HANGUL
    0x3030,   // 3030..3037; COMMON
    0x3038,   // 3038..303B; HAN
    0x303C,   // 303C..303F; COMMON
    0x3040,   // 3040      ; UNKNOWN
    0x3041,   // 3041..3096; HIRAGANA
    0x3097,   // 3097..3098; UNKNOWN
    0x3099,   // 3099..309A; INHERITED
    0x309B,   // 309B..309C; COMMON
    0x309D,   // 309D..309F; HIRAGANA
    0x30A0,   // 30A0      ; COMMON
    0x30A1,   // 30A1..30FA; KATAKANA
    0x30FB,   // 30FB..30FC; COMMON
    0x30FD,   // 30FD..30FF; KATAKANA
    0x3100,   // 3100..3104; UNKNOWN
    0x3105,   // 3105..312F; BOPOMOFO
    0x3130,   // 3130      ; UNKNOWN
    0x3131,   // 3131..318E; HANGUL
    0x318F,   // 318F      ; UNKNOWN
    0x3190,   // 3190..319F; COMMON
    0x31A0,   // 31A0..31BF; BOPOMOFO
    0x31C0,   // 31C0..31E3; COMMON
    0x31E4,   // 31E4..31EF; UNKNOWN
    0x31F0,   // 31F0..31FF; KATAKANA
    0x3200,   // 3200..321E; HANGUL
    0x321F,   // 321F      ; UNKNOWN
    0x3220,   // 3220..325F; COMMON
    0x3260,   // 3260..327E; HANGUL
    0x327F,   // 327F..32CF; COMMON
    0x32D0,   // 32D0..32FE; KATAKANA
    0x32FF,   // 32FF      ; COMMON
    0x3300,   // 3300..3357; KATAKANA
    0x3358,   // 3358..33FF; COMMON
    0x3400,   // 3400..4DBF; HAN
    0x4DC0,   // 4DC0..4DFF; COMMON
    0x4E00,   // 4E00..9FFC; HAN
    0x9FFD,   // 9FFD..9FFF; UNKNOWN
    0xA000,   // A000..A48C; YI
    0xA48D,   // A48D..A48F; UNKNOWN
    0xA490,   // A490..A4C6; YI
    0xA4C7,   // A4C7..A4CF; UNKNOWN
    0xA4D0,   // A4D0..A4FF; LISU
    0xA500,   // A500..A62B; VAI
    0xA62C,   // A62C..A63F; UNKNOWN
    0xA640,   // A640..A69F; CYRILLIC
    0xA6A0,   // A6A0..A6F7; BAMUM
    0xA6F8,   // A6F8..A6FF; UNKNOWN
    0xA700,   // A700..A721; COMMON
    0xA722,   // A722..A787; LATIN
    0xA788,   // A788..A78A; COMMON
    0xA78B,   // A78B..A7BF; LATIN
    0xA7C0,   // A7C0..A7C1; UNKNOWN
    0xA7C2,   // A7C2..A7CA; LATIN
    0xA7CB,   // A7CB..A7F4; UNKNOWN
    0xA7F5,   // A7F5..A7FF; LATIN
    0xA800,   // A800..A82C; SYLOTI_NAGRI
    0xA82D,   // A82D..A82F; UNKNOWN
    0xA830,   // A830..A839; COMMON
    0xA83A,   // A83A..A83F; UNKNOWN
    0xA840,   // A840..A877; PHAGS_PA
    0xA878,   // A878..A87F; UNKNOWN
    0xA880,   // A880..A8C5; SAURASHTRA
    0xA8C6,   // A8C6..A8CD; UNKNOWN
    0xA8CE,   // A8CE..A8D9; SAURASHTRA
    0xA8DA,   // A8DA..A8DF; UNKNOWN
    0xA8E0,   // A8E0..A8FF; DEVANAGARI
    0xA900,   // A900..A92D; KAYAH_LI
    0xA92E,   // A92E      ; COMMON
    0xA92F,   // A92F      ; KAYAH_LI
    0xA930,   // A930..A953; REJANG
    0xA954,   // A954..A95E; UNKNOWN
    0xA95F,   // A95F      ; REJANG
    0xA960,   // A960..A97C; HANGUL
    0xA97D,   // A97D..A97F; UNKNOWN
    0xA980,   // A980..A9CD; JAVANESE
    0xA9CE,   // A9CE      ; UNKNOWN
    0xA9CF,   // A9CF      ; COMMON
    0xA9D0,   // A9D0..A9D9; JAVANESE
    0xA9DA,   // A9DA..A9DD; UNKNOWN
    0xA9DE,   // A9DE..A9DF; JAVANESE
    0xA9E0,   // A9E0..A9FE; MYANMAR
    0xA9FF,   // A9FF      ; UNKNOWN
    0xAA00,   // AA00..AA36; CHAM
    0xAA37,   // AA37..AA3F; UNKNOWN
    0xAA40,   // AA40..AA4D; CHAM
    0xAA4E,   // AA4E..AA4F; UNKNOWN
    0xAA50,   // AA50..AA59; CHAM
    0xAA5A,   // AA5A..AA5B; UNKNOWN
    0xAA5C,   // AA5C..AA5F; CHAM
    0xAA60,   // AA60..AA7F; MYANMAR
    0xAA80,   // AA80..AAC2; TAI_VIET
    0xAAC3,   // AAC3..AADA; UNKNOWN
    0xAADB,   // AADB..AADF; TAI_VIET
    0xAAE0,   // AAE0..AAF6; MEETEI_MAYEK
    0xAAF7,   // AAF7..AB00; UNKNOWN
    0xAB01,   // AB01..AB06; ETHIOPIC
    0xAB07,   // AB07..AB08; UNKNOWN
    0xAB09,   // AB09..AB0E; ETHIOPIC
    0xAB0F,   // AB0F..AB10; UNKNOWN
    0xAB11,   // AB11..AB16; ETHIOPIC
    0xAB17,   // AB17..AB1F; UNKNOWN
    0xAB20,   // AB20..AB26; ETHIOPIC
    0xAB27,   // AB27      ; UNKNOWN
    0xAB28,   // AB28..AB2E; ETHIOPIC
    0xAB2F,   // AB2F      ; UNKNOWN
    0xAB30,   // AB30..AB5A; LATIN
    0xAB5B,   // AB5B      ; COMMON
    0xAB5C,   // AB5C..AB64; LATIN
    0xAB65,   // AB65      ; GREEK
    0xAB66,   // AB66..AB69; LATIN
    0xAB6A,   // AB6A..AB6B; COMMON
    0xAB6C,   // AB6C..AB6F; UNKNOWN
    0xAB70,   // AB70..ABBF; CHEROKEE
    0xABC0,   // ABC0..ABED; MEETEI_MAYEK
    0xABEE,   // ABEE..ABEF; UNKNOWN
    0xABF0,   // ABF0..ABF9; MEETEI_MAYEK
    0xABFA,   // ABFA..ABFF; UNKNOWN
    0xAC00,   // AC00..D7A3; HANGUL
    0xD7A4,   // D7A4..D7AF; UNKNOWN
    0xD7B0,   // D7B0..D7C6; HANGUL
    0xD7C7,   // D7C7..D7CA; UNKNOWN
    0xD7CB,   // D7CB..D7FB; HANGUL
    0xD7FC,   // D7FC..F8FF; UNKNOWN
    0xF900,   // F900..FA6D; HAN
    0xFA6E,   // FA6E..FA6F; UNKNOWN
    0xFA70,   // FA70..FAD9; HAN
    0xFADA,   // FADA..FAFF; UNKNOWN
    0xFB00,   // FB00..FB06; LATIN
    0xFB07,   // FB07..FB12; UNKNOWN
    0xFB13,   // FB13..FB17; ARMENIAN
    0xFB18,   // FB18..FB1C; UNKNOWN
    0xFB1D,   // FB1D..FB36; HEBREW
    0xFB37,   // FB37      ; UNKNOWN
    0xFB38,   // FB38..FB3C; HEBREW
    0xFB3D,   // FB3D      ; UNKNOWN
    0xFB3E,   // FB3E      ; HEBREW
    0xFB3F,   // FB3F      ; UNKNOWN
    0xFB40,   // FB40..FB41; HEBREW
    0xFB42,   // FB42      ; UNKNOWN
    0xFB43,   // FB43..FB44; HEBREW
    0xFB45,   // FB45      ; UNKNOWN
    0xFB46,   // FB46..FB4F; HEBREW
    0xFB50,   // FB50..FBC1; ARABIC
    0xFBC2,   // FBC2..FBD2; UNKNOWN
    0xFBD3,   // FBD3..FD3D; ARABIC
    0xFD3E,   // FD3E..FD3F; COMMON
    0xFD40,   // FD40..FD4F; UNKNOWN
    0xFD50,   // FD50..FD8F; ARABIC
    0xFD90,   // FD90..FD91; UNKNOWN
    0xFD92,   // FD92..FDC7; ARABIC
    0xFDC8,   // FDC8..FDEF; UNKNOWN
    0xFDF0,   // FDF0..FDFD; ARABIC
    0xFDFE,   // FDFE..FDFF; UNKNOWN
    0xFE00,   // FE00..FE0F; INHERITED
    0xFE10,   // FE10..FE19; COMMON
    0xFE1A,   // FE1A..FE1F; UNKNOWN
    0xFE20,   // FE20..FE2D; INHERITED
    0xFE2E,   // FE2E..FE2F; CYRILLIC
    0xFE30,   // FE30..FE52; COMMON
    0xFE53,   // FE53      ; UNKNOWN
    0xFE54,   // FE54..FE66; COMMON
    0xFE67,   // FE67      ; UNKNOWN
    0xFE68,   // FE68..FE6B; COMMON
    0xFE6C,   // FE6C..FE6F; UNKNOWN
    0xFE70,   // FE70..FE74; ARABIC
    0xFE75,   // FE75      ; UNKNOWN
    0xFE76,   // FE76..FEFC; ARABIC
    0xFEFD,   // FEFD..FEFE; UNKNOWN
    0xFEFF,   // FEFF      ; COMMON
    0xFF00,   // FF00      ; UNKNOWN
    0xFF01,   // FF01..FF20; COMMON
    0xFF21,   // FF21..FF3A; LATIN
    0xFF3B,   // FF3B..FF40; COMMON
    0xFF41,   // FF41..FF5A; LATIN
    0xFF5B,   // FF5B..FF65; COMMON
    0xFF66,   // FF66..FF6F; KATAKANA
    0xFF70,   // FF70      ; COMMON
    0xFF71,   // FF71..FF9D; KATAKANA
    0xFF9E,   // FF9E..FF9F; COMMON
    0xFFA0,   // FFA0..FFBE; HANGUL
    0xFFBF,   // FFBF..FFC1; UNKNOWN
    0xFFC2,   // FFC2..FFC7; HANGUL
    0xFFC8,   // FFC8..FFC9; UNKNOWN
    0xFFCA,   // FFCA..FFCF; HANGUL
    0xFFD0,   // FFD0..FFD1; UNKNOWN
    0xFFD2,   // FFD2..FFD7; HANGUL
    0xFFD8,   // FFD8..FFD9; UNKNOWN
    0xFFDA,   // FFDA..FFDC; HANGUL
    0xFFDD,   // FFDD..FFDF; UNKNOWN
    0xFFE0,   // FFE0..FFE6; COMMON
    0xFFE7,   // FFE7      ; UNKNOWN
    0xFFE8,   // FFE8..FFEE; COMMON
    0xFFEF,   // FFEF..FFF8; UNKNOWN
    0xFFF9,   // FFF9..FFFD; COMMON
    0xFFFE,   // FFFE..FFFF; UNKNOWN
    0x10000,  // 10000..1000B; LINEAR_B
    0x1000C,  // 1000C       ; UNKNOWN
    0x1000D,  // 1000D..10026; LINEAR_B
    0x10027,  // 10027       ; UNKNOWN
    0x10028,  // 10028..1003A; LINEAR_B
    0x1003B,  // 1003B       ; UNKNOWN
    0x1003C,  // 1003C..1003D; LINEAR_B
    0x1003E,  // 1003E       ; UNKNOWN
    0x1003F,  // 1003F..1004D; LINEAR_B
    0x1004E,  // 1004E..1004F; UNKNOWN
    0x10050,  // 10050..1005D; LINEAR_B
    0x1005E,  // 1005E..1007F; UNKNOWN
    0x10080,  // 10080..100FA; LINEAR_B
    0x100FB,  // 100FB..100FF; UNKNOWN
    0x10100,  // 10100..10102; COMMON
    0x10103,  // 10103..10106; UNKNOWN
    0x10107,  // 10107..10133; COMMON
    0x10134,  // 10134..10136; UNKNOWN
    0x10137,  // 10137..1013F; COMMON
    0x10140,  // 10140..1018E; GREEK
    0x1018F,  // 1018F       ; UNKNOWN
    0x10190,  // 10190..1019C; COMMON
    0x1019D,  // 1019D..1019F; UNKNOWN
    0x101A0,  // 101A0       ; GREEK
    0x101A1,  // 101A1..101CF; UNKNOWN
    0x101D0,  // 101D0..101FC; COMMON
    0x101FD,  // 101FD       ; INHERITED
    0x101FE,  // 101FE..1027F; UNKNOWN
    0x10280,  // 10280..1029C; LYCIAN
    0x1029D,  // 1029D..1029F; UNKNOWN
    0x102A0,  // 102A0..102D0; CARIAN
    0x102D1,  // 102D1..102DF; UNKNOWN
    0x102E0,  // 102E0       ; INHERITED
    0x102E1,  // 102E1..102FB; COMMON
    0x102FC,  // 102FC..102FF; UNKNOWN
    0x10300,  // 10300..10323; OLD_ITALIC
    0x10324,  // 10324..1032C; UNKNOWN
    0x1032D,  // 1032D..1032F; OLD_ITALIC
    0x10330,  // 10330..1034A; GOTHIC
    0x1034B,  // 1034B..1034F; UNKNOWN
    0x10350,  // 10350..1037A; OLD_PERMIC
    0x1037B,  // 1037B..1037F; UNKNOWN
    0x10380,  // 10380..1039D; UGARITIC
    0x1039E,  // 1039E       ; UNKNOWN
    0x1039F,  // 1039F       ; UGARITIC
    0x103A0,  // 103A0..103C3; OLD_PERSIAN
    0x103C4,  // 103C4..103C7; UNKNOWN
    0x103C8,  // 103C8..103D5; OLD_PERSIAN
    0x103D6,  // 103D6..103FF; UNKNOWN
    0x10400,  // 10400..1044F; DESERET
    0x10450,  // 10450..1047F; SHAVIAN
    0x10480,  // 10480..1049D; OSMANYA
    0x1049E,  // 1049E..1049F; UNKNOWN
    0x104A0,  // 104A0..104A9; OSMANYA
    0x104AA,  // 104AA..104AF; UNKNOWN
    0x104B0,  // 104B0..104D3; OSAGE
    0x104D4,  // 104D4..104D7; UNKNOWN
    0x104D8,  // 104D8..104FB; OSAGE
    0x104FC,  // 104FC..104FF; UNKNOWN
    0x10500,  // 10500..10527; ELBASAN
    0x10528,  // 10528..1052F; UNKNOWN
    0x10530,  // 10530..10563; CAUCASIAN_ALBANIAN
    0x10564,  // 10564..1056E; UNKNOWN
    0x1056F,  // 1056F       ; CAUCASIAN_ALBANIAN
    0x10570,  // 10570..105FF; UNKNOWN
    0x10600,  // 10600..10736; LINEAR_A
    0x10737,  // 10737..1073F; UNKNOWN
    0x10740,  // 10740..10755; LINEAR_A
    0x10756,  // 10756..1075F; UNKNOWN
    0x10760,  // 10760..10767; LINEAR_A
    0x10768,  // 10768..107FF; UNKNOWN
    0x10800,  // 10800..10805; CYPRIOT
    0x10806,  // 10806..10807; UNKNOWN
    0x10808,  // 10808       ; CYPRIOT
    0x10809,  // 10809       ; UNKNOWN
    0x1080A,  // 1080A..10835; CYPRIOT
    0x10836,  // 10836       ; UNKNOWN
    0x10837,  // 10837..10838; CYPRIOT
    0x10839,  // 10839..1083B; UNKNOWN
    0x1083C,  // 1083C       ; CYPRIOT
    0x1083D,  // 1083D..1083E; UNKNOWN
    0x1083F,  // 1083F       ; CYPRIOT
    0x10840,  // 10840..10855; IMPERIAL_ARAMAIC
    0x10856,  // 10856       ; UNKNOWN
    0x10857,  // 10857..1085F; IMPERIAL_ARAMAIC
    0x10860,  // 10860..1087F; PALMYRENE
    0x10880,  // 10880..1089E; NABATAEAN
    0x1089F,  // 1089F..108A6; UNKNOWN
    0x108A7,  // 108A7..108AF; NABATAEAN
    0x108B0,  // 108B0..108DF; UNKNOWN
    0x108E0,  // 108E0..108F2; HATRAN
    0x108F3,  // 108F3       ; UNKNOWN
    0x108F4,  // 108F4..108F5; HATRAN
    0x108F6,  // 108F6..108FA; UNKNOWN
    0x108FB,  // 108FB..108FF; HATRAN
    0x10900,  // 10900..1091B; PHOENICIAN
    0x1091C,  // 1091C..1091E; UNKNOWN
    0x1091F,  // 1091F       ; PHOENICIAN
    0x10920,  // 10920..10939; LYDIAN
    0x1093A,  // 1093A..1093E; UNKNOWN
    0x1093F,  // 1093F       ; LYDIAN
    0x10940,  // 10940..1097F; UNKNOWN
    0x10980,  // 10980..1099F; MEROITIC_HIEROGLYPHS
    0x109A0,  // 109A0..109B7; MEROITIC_CURSIVE
    0x109B8,  // 109B8..109BB; UNKNOWN
    0x109BC,  // 109BC..109CF; MEROITIC_CURSIVE
    0x109D0,  // 109D0..109D1; UNKNOWN
    0x109D2,  // 109D2..109FF; MEROITIC_CURSIVE
    0x10A00,  // 10A00..10A03; KHAROSHTHI
    0x10A04,  // 10A04       ; UNKNOWN
    0x10A05,  // 10A05..10A06; KHAROSHTHI
    0x10A07,  // 10A07..10A0B; UNKNOWN
    0x10A0C,  // 10A0C..10A13; KHAROSHTHI
    0x10A14,  // 10A14       ; UNKNOWN
    0x10A15,  // 10A15..10A17; KHAROSHTHI
    0x10A18,  // 10A18       ; UNKNOWN
    0x10A19,  // 10A19..10A35; KHAROSHTHI
    0x10A36,  // 10A36..10A37; UNKNOWN
    0x10A38,  // 10A38..10A3A; KHAROSHTHI
    0x10A3B,  // 10A3B..10A3E; UNKNOWN
    0x10A3F,  // 10A3F..10A48; KHAROSHTHI
    0x10A49,  // 10A49..10A4F; UNKNOWN
    0x10A50,  // 10A50..10A58; KHAROSHTHI
    0x10A59,  // 10A59..10A5F; UNKNOWN
    0x10A60,  // 10A60..10A7F; OLD_SOUTH_ARABIAN
    0x10A80,  // 10A80..10A9F; OLD_NORTH_ARABIAN
    0x10AA0,  // 10AA0..10ABF; UNKNOWN
    0x10AC0,  // 10AC0..10AE6; MANICHAEAN
    0x10AE7,  // 10AE7..10AEA; UNKNOWN
    0x10AEB,  // 10AEB..10AF6; MANICHAEAN
    0x10AF7,  // 10AF7..10AFF; UNKNOWN
    0x10B00,  // 10B00..10B35; AVESTAN
    0x10B36,  // 10B36..10B38; UNKNOWN
    0x10B39,  // 10B39..10B3F; AVESTAN
    0x10B40,  // 10B40..10B55; INSCRIPTIONAL_PARTHIAN
    0x10B56,  // 10B56..10B57; UNKNOWN
    0x10B58,  // 10B58..10B5F; INSCRIPTIONAL_PARTHIAN
    0x10B60,  // 10B60..10B72; INSCRIPTIONAL_PAHLAVI
    0x10B73,  // 10B73..10B77; UNKNOWN
    0x10B78,  // 10B78..10B7F; INSCRIPTIONAL_PAHLAVI
    0x10B80,  // 10B80..10B91; PSALTER_PAHLAVI
    0x10B92,  // 10B92..10B98; UNKNOWN
    0x10B99,  // 10B99..10B9C; PSALTER_PAHLAVI
    0x10B9D,  // 10B9D..10BA8; UNKNOWN
    0x10BA9,  // 10BA9..10BAF; PSALTER_PAHLAVI
    0x10BB0,  // 10BB0..10BFF; UNKNOWN
    0x10C00,  // 10C00..10C48; OLD_TURKIC
    0x10C49,  // 10C49..10C7F; UNKNOWN
    0x10C80,  // 10C80..10CB2; OLD_HUNGARIAN
    0x10CB3,  // 10CB3..10CBF; UNKNOWN
    0x10CC0,  // 10CC0..10CF2; OLD_HUNGARIAN
    0x10CF3,  // 10CF3..10CF9; UNKNOWN
    0x10CFA,  // 10CFA..10CFF; OLD_HUNGARIAN
    0x10D00,  // 10D00..10D27; HANIFI_ROHINGYA
    0x10D28,  // 10D28..10D2F; UNKNOWN
    0x10D30,  // 10D30..10D39; HANIFI_ROHINGYA
    0x10D3A,  // 10D3A..10E5F; UNKNOWN
    0x10E60,  // 10E60..10E7E; ARABIC
    0x10E7F,  // 10E7F       ; UNKNOWN
    0x10E80,  // 10E80..10EA9; YEZIDI
    0x10EAA,  // 10EAA       ; UNKNOWN
    0x10EAB,  // 10EAB..10EAD; YEZIDI
    0x10EAE,  // 10EAE..10EAF; UNKNOWN
    0x10EB0,  // 10EB0..10EB1; YEZIDI
    0x10EB2,  // 10EB2..10EFF; UNKNOWN
    0x10F00,  // 10F00..10F27; OLD_SOGDIAN
    0x10F28,  // 10F28..10F2F; UNKNOWN
    0x10F30,  // 10F30..10F59; SOGDIAN
    0x10F5A,  // 10F5A..10FAF; UNKNOWN
    0x10FB0,  // 10FB0..10FCB; CHORASMIAN
    0x10FCC,  // 10FCC..10FDF; UNKNOWN
    0x10FE0,  // 10FE0..10FF6; ELYMAIC
    0x10FF7,  // 10FF7..10FFF; UNKNOWN
    0x11000,  // 11000..1104D; BRAHMI
    0x1104E,  // 1104E..11051; UNKNOWN
    0x11052,  // 11052..1106F; BRAHMI
    0x11070,  // 11070..1107E; UNKNOWN
    0x1107F,  // 1107F       ; BRAHMI
    0x11080,  // 11080..110C1; KAITHI
    0x110C2,  // 110C2..110CC; UNKNOWN
    0x110CD,  // 110CD       ; KAITHI
    0x110CE,  // 110CE..110CF; UNKNOWN
    0x110D0,  // 110D0..110E8; SORA_SOMPENG
    0x110E9,  // 110E9..110EF; UNKNOWN
    0x110F0,  // 110F0..110F9; SORA_SOMPENG
    0x110FA,  // 110FA..110FF; UNKNOWN
    0x11100,  // 11100..11134; CHAKMA
    0x11135,  // 11135       ; UNKNOWN
    0x11136,  // 11136..11147; CHAKMA
    0x11148,  // 11148..1114F; UNKNOWN
    0x11150,  // 11150..11176; MAHAJANI
    0x11177,  // 11177..1117F; UNKNOWN
    0x11180,  // 11180..111DF; SHARADA
    0x111E0,  // 111E0       ; UNKNOWN
    0x111E1,  // 111E1..111F4; SINHALA
    0x111F5,  // 111F5..111FF; UNKNOWN
    0x11200,  // 11200..11211; KHOJKI
    0x11212,  // 11212       ; UNKNOWN
    0x11213,  // 11213..1123E; KHOJKI
    0x1123F,  // 1123F..1127F; UNKNOWN
    0x11280,  // 11280..11286; MULTANI
    0x11287,  // 11287       ; UNKNOWN
    0x11288,  // 11288       ; MULTANI
    0x11289,  // 11289       ; UNKNOWN
    0x1128A,  // 1128A..1128D; MULTANI
    0x1128E,  // 1128E       ; UNKNOWN
    0x1128F,  // 1128F..1129D; MULTANI
    0x1129E,  // 1129E       ; UNKNOWN
    0x1129F,  // 1129F..112A9; MULTANI
    0x112AA,  // 112AA..112AF; UNKNOWN
    0x112B0,  // 112B0..112EA; KHUDAWADI
    0x112EB,  // 112EB..112EF; UNKNOWN
    0x112F0,  // 112F0..112F9; KHUDAWADI
    0x112FA,  // 112FA..112FF; UNKNOWN
    0x11300,  // 11300..11303; GRANTHA
    0x11304,  // 11304       ; UNKNOWN
    0x11305,  // 11305..1130C; GRANTHA
    0x1130D,  // 1130D..1130E; UNKNOWN
    0x1130F,  // 1130F..11310; GRANTHA
    0x11311,  // 11311..11312; UNKNOWN
    0x11313,  // 11313..11328; GRANTHA
    0x11329,  // 11329       ; UNKNOWN
    0x1132A,  // 1132A..11330; GRANTHA
    0x11331,  // 11331       ; UNKNOWN
    0x11332,  // 11332..11333; GRANTHA
    0x11334,  // 11334       ; UNKNOWN
    0x11335,  // 11335..11339; GRANTHA
    0x1133A,  // 1133A       ; UNKNOWN
    0x1133B,  // 1133B       ; INHERITED
    0x1133C,  // 1133C..11344; GRANTHA
    0x11345,  // 11345..11346; UNKNOWN
    0x11347,  // 11347..11348; GRANTHA
    0x11349,  // 11349..1134A; UNKNOWN
    0x1134B,  // 1134B..1134D; GRANTHA
    0x1134E,  // 1134E..1134F; UNKNOWN
    0x11350,  // 11350       ; GRANTHA
    0x11351,  // 11351..11356; UNKNOWN
    0x11357,  // 11357       ; GRANTHA
    0x11358,  // 11358..1135C; UNKNOWN
    0x1135D,  // 1135D..11363; GRANTHA
    0x11364,  // 11364..11365; UNKNOWN
    0x11366,  // 11366..1136C; GRANTHA
    0x1136D,  // 1136D..1136F; UNKNOWN
    0x11370,  // 11370..11374; GRANTHA
    0x11375,  // 11375..113FF; UNKNOWN
    0x11400,  // 11400..1145B; NEWA
    0x1145C,  // 1145C       ; UNKNOWN
    0x1145D,  // 1145D..11461; NEWA
    0x11462,  // 11462..1147F; UNKNOWN
    0x11480,  // 11480..114C7; TIRHUTA
    0x114C8,  // 114C8..114CF; UNKNOWN
    0x114D0,  // 114D0..114D9; TIRHUTA
    0x114DA,  // 114DA..1157F; UNKNOWN
    0x11580,  // 11580..115B5; SIDDHAM
    0x115B6,  // 115B6..115B7; UNKNOWN
    0x115B8,  // 115B8..115DD; SIDDHAM
    0x115DE,  // 115DE..115FF; UNKNOWN
    0x11600,  // 11600..11644; MODI
    0x11645,  // 11645..1164F; UNKNOWN
    0x11650,  // 11650..11659; MODI
    0x1165A,  // 1165A..1165F; UNKNOWN
    0x11660,  // 11660..1166C; MONGOLIAN
    0x1166D,  // 1166D..1167F; UNKNOWN
    0x11680,  // 11680..116B8; TAKRI
    0x116B9,  // 116B9..116BF; UNKNOWN
    0x116C0,  // 116C0..116C9; TAKRI
    0x116CA,  // 116CA..116FF; UNKNOWN
    0x11700,  // 11700..1171A; AHOM
    0x1171B,  // 1171B..1171C; UNKNOWN
    0x1171D,  // 1171D..1172B; AHOM
    0x1172C,  // 1172C..1172F; UNKNOWN
    0x11730,  // 11730..1173F; AHOM
    0x11740,  // 11740..117FF; UNKNOWN
    0x11800,  // 11800..1183B; DOGRA
    0x1183C,  // 1183C..1189F; UNKNOWN
    0x118A0,  // 118A0..118F2; WARANG_CITI
    0x118F3,  // 118F3..118FE; UNKNOWN
    0x118FF,  // 118FF       ; WARANG_CITI
    0x11900,  // 11900..11906; DIVES_AKURU
    0x11907,  // 11907..11908; UNKNOWN
    0x11909,  // 11909       ; DIVES_AKURU
    0x1190A,  // 1190A..1190B; UNKNOWN
    0x1190C,  // 1190C..11913; DIVES_AKURU
    0x11914,  // 11914       ; UNKNOWN
    0x11915,  // 11915..11916; DIVES_AKURU
    0x11917,  // 11917       ; UNKNOWN
    0x11918,  // 11918..11935; DIVES_AKURU
    0x11936,  // 11936       ; UNKNOWN
    0x11937,  // 11937..11938; DIVES_AKURU
    0x11939,  // 11939..1193A; UNKNOWN
    0x1193B,  // 1193B..11946; DIVES_AKURU
    0x11947,  // 11947..1194F; UNKNOWN
    0x11950,  // 11950..11959; DIVES_AKURU
    0x1195A,  // 1195A..1199F; UNKNOWN
    0x119A0,  // 119A0..119A7; NANDINAGARI
    0x119A8,  // 119A8..119A9; UNKNOWN
    0x119AA,  // 119AA..119D7; NANDINAGARI
    0x119D8,  // 119D8..119D9; UNKNOWN
    0x119DA,  // 119DA..119E4; NANDINAGARI
    0x119E5,  // 119E5..119FF; UNKNOWN
    0x11A00,  // 11A00..11A47; ZANABAZAR_SQUARE
    0x11A48,  // 11A48..11A4F; UNKNOWN
    0x11A50,  // 11A50..11AA2; SOYOMBO
    0x11AA3,  // 11AA3..11ABF; UNKNOWN
    0x11AC0,  // 11AC0..11AF8; PAU_CIN_HAU
    0x11AF9,  // 11AF9..11BFF; UNKNOWN
    0x11C00,  // 11C00..11C08; BHAIKSUKI
    0x11C09,  // 11C09       ; UNKNOWN
    0x11C0A,  // 11C0A..11C36; BHAIKSUKI
    0x11C37,  // 11C37       ; UNKNOWN
    0x11C38,  // 11C38..11C45; BHAIKSUKI
    0x11C46,  // 11C46..11C4F; UNKNOWN
    0x11C50,  // 11C50..11C6C; BHAIKSUKI
    0x11C6D,  // 11C6D..11C6F; UNKNOWN
    0x11C70,  // 11C70..11C8F; MARCHEN
    0x11C90,  // 11C90..11C91; UNKNOWN
    0x11C92,  // 11C92..11CA7; MARCHEN
    0x11CA8,  // 11CA8       ; UNKNOWN
    0x11CA9,  // 11CA9..11CB6; MARCHEN
    0x11CB7,  // 11CB7..11CFF; UNKNOWN
    0x11D00,  // 11D00..11D06; MASARAM_GONDI
    0x11D07,  // 11D07       ; UNKNOWN
    0x11D08,  // 11D08..11D09; MASARAM_GONDI
    0x11D0A,  // 11D0A       ; UNKNOWN
    0x11D0B,  // 11D0B..11D36; MASARAM_GONDI
    0x11D37,  // 11D37..11D39; UNKNOWN
    0x11D3A,  // 11D3A       ; MASARAM_GONDI
    0x11D3B,  // 11D3B       ; UNKNOWN
    0x11D3C,  // 11D3C..11D3D; MASARAM_GONDI
    0x11D3E,  // 11D3E       ; UNKNOWN
    0x11D3F,  // 11D3F..11D47; MASARAM_GONDI
    0x11D48,  // 11D48..11D4F; UNKNOWN
    0x11D50,  // 11D50..11D59; MASARAM_GONDI
    0x11D5A,  // 11D5A..11D5F; UNKNOWN
    0x11D60,  // 11D60..11D65; GUNJALA_GONDI
    0x11D66,  // 11D66       ; UNKNOWN
    0x11D67,  // 11D67..11D68; GUNJALA_GONDI
    0x11D69,  // 11D69       ; UNKNOWN
    0x11D6A,  // 11D6A..11D8E; GUNJALA_GONDI
    0x11D8F,  // 11D8F       ; UNKNOWN
    0x11D90,  // 11D90..11D91; GUNJALA_GONDI
    0x11D92,  // 11D92       ; UNKNOWN
    0x11D93,  // 11D93..11D98; GUNJALA_GONDI
    0x11D99,  // 11D99..11D9F; UNKNOWN
    0x11DA0,  // 11DA0..11DA9; GUNJALA_GONDI
    0x11DAA,  // 11DAA..11EDF; UNKNOWN
    0x11EE0,  // 11EE0..11EF8; MAKASAR
    0x11EF9,  // 11EF9..11FAF; UNKNOWN
    0x11FB0,  // 11FB0       ; LISU
    0x11FB1,  // 11FB1..11FBF; UNKNOWN
    0x11FC0,  // 11FC0..11FF1; TAMIL
    0x11FF2,  // 11FF2..11FFE; UNKNOWN
    0x11FFF,  // 11FFF       ; TAMIL
    0x12000,  // 12000..12399; CUNEIFORM
    0x1239A,  // 1239A..123FF; UNKNOWN
    0x12400,  // 12400..1246E; CUNEIFORM
    0x1246F,  // 1246F       ; UNKNOWN
    0x12470,  // 12470..12474; CUNEIFORM
    0x12475,  // 12475..1247F; UNKNOWN
    0x12480,  // 12480..12543; CUNEIFORM
    0x12544,  // 12544..12FFF; UNKNOWN
    0x13000,  // 13000..1342E; EGYPTIAN_HIEROGLYPHS
    0x1342F,  // 1342F       ; UNKNOWN
    0x13430,  // 13430..13438; EGYPTIAN_HIEROGLYPHS
    0x13439,  // 13439..143FF; UNKNOWN
    0x14400,  // 14400..14646; ANATOLIAN_HIEROGLYPHS
    0x14647,  // 14647..167FF; UNKNOWN
    0x16800,  // 16800..16A38; BAMUM
    0x16A39,  // 16A39..16A3F; UNKNOWN
    0x16A40,  // 16A40..16A5E; MRO
    0x16A5F,  // 16A5F       ; UNKNOWN
    0x16A60,  // 16A60..16A69; MRO
    0x16A6A,  // 16A6A..16A6D; UNKNOWN
    0x16A6E,  // 16A6E..16A6F; MRO
    0x16A70,  // 16A70..16ACF; UNKNOWN
    0x16AD0,  // 16AD0..16AED; BASSA_VAH
    0x16AEE,  // 16AEE..16AEF; UNKNOWN
    0x16AF0,  // 16AF0..16AF5; BASSA_VAH
    0x16AF6,  // 16AF6..16AFF; UNKNOWN
    0x16B00,  // 16B00..16B45; PAHAWH_HMONG
    0x16B46,  // 16B46..16B4F; UNKNOWN
    0x16B50,  // 16B50..16B59; PAHAWH_HMONG
    0x16B5A,  // 16B5A       ; UNKNOWN
    0x16B5B,  // 16B5B..16B61; PAHAWH_HMONG
    0x16B62,  // 16B62       ; UNKNOWN
    0x16B63,  // 16B63..16B77; PAHAWH_HMONG
    0x16B78,  // 16B78..16B7C; UNKNOWN
    0x16B7D,  // 16B7D..16B8F; PAHAWH_HMONG
    0x16B90,  // 16B90..16E3F; UNKNOWN
    0x16E40,  // 16E40..16E9A; MEDEFAIDRIN
    0x16E9B,  // 16E9B..16EFF; UNKNOWN
    0x16F00,  // 16F00..16F4A; MIAO
    0x16F4B,  // 16F4B..16F4E; UNKNOWN
    0x16F4F,  // 16F4F..16F87; MIAO
    0x16F88,  // 16F88..16F8E; UNKNOWN
    0x16F8F,  // 16F8F..16F9F; MIAO
    0x16FA0,  // 16FA0..16FDF; UNKNOWN
    0x16FE0,  // 16FE0       ; TANGUT
    0x16FE1,  // 16FE1       ; NUSHU
    0x16FE2,  // 16FE2..16FE3; COMMON
    0x16FE4,  // 16FE4       ; KHITAN_SMALL_SCRIPT
    0x16FE5,  // 16FE5..16FEF; UNKNOWN
    0x16FF0,  // 16FF0..16FF1; HAN
    0x16FF2,  // 16FF2..16FFF; UNKNOWN
    0x17000,  // 17000..187F7; TANGUT
    0x187F8,  // 187F8..187FF; UNKNOWN
    0x18800,  // 18800..18AFF; TANGUT
    0x18B00,  // 18B00..18CD5; KHITAN_SMALL_SCRIPT
    0x18CD6,  // 18CD6..18CFF; UNKNOWN
    0x18D00,  // 18D00..18D08; TANGUT
    0x18D09,  // 18D09..1AFFF; UNKNOWN
    0x1B000,  // 1B000       ; KATAKANA
    0x1B001,  // 1B001..1B11E; HIRAGANA
    0x1B11F,  // 1B11F..1B14F; UNKNOWN
    0x1B150,  // 1B150..1B152; HIRAGANA
    0x1B153,  // 1B153..1B163; UNKNOWN
    0x1B164,  // 1B164..1B167; KATAKANA
    0x1B168,  // 1B168..1B16F; UNKNOWN
    0x1B170,  // 1B170..1B2FB; NUSHU
    0x1B2FC,  // 1B2FC..1BBFF; UNKNOWN
    0x1BC00,  // 1BC00..1BC6A; DUPLOYAN
    0x1BC6B,  // 1BC6B..1BC6F; UNKNOWN
    0x1BC70,  // 1BC70..1BC7C; DUPLOYAN
    0x1BC7D,  // 1BC7D..1BC7F; UNKNOWN
    0x1BC80,  // 1BC80..1BC88; DUPLOYAN
    0x1BC89,  // 1BC89..1BC8F; UNKNOWN
    0x1BC90,  // 1BC90..1BC99; DUPLOYAN
    0x1BC9A,  // 1BC9A..1BC9B; UNKNOWN
    0x1BC9C,  // 1BC9C..1BC9F; DUPLOYAN
    0x1BCA0,  // 1BCA0..1BCA3; COMMON
    0x1BCA4,  // 1BCA4..1CFFF; UNKNOWN
    0x1D000,  // 1D000..1D0F5; COMMON
    0x1D0F6,  // 1D0F6..1D0FF; UNKNOWN
    0x1D100,  // 1D100..1D126; COMMON
    0x1D127,  // 1D127..1D128; UNKNOWN
    0x1D129,  // 1D129..1D166; COMMON
    0x1D167,  // 1D167..1D169; INHERITED
    0x1D16A,  // 1D16A..1D17A; COMMON
    0x1D17B,  // 1D17B..1D182; INHERITED
    0x1D183,  // 1D183..1D184; COMMON
    0x1D185,  // 1D185..1D18B; INHERITED
    0x1D18C,  // 1D18C..1D1A9; COMMON
    0x1D1AA,  // 1D1AA..1D1AD; INHERITED
    0x1D1AE,  // 1D1AE..1D1E8; COMMON
    0x1D1E9,  // 1D1E9..1D1FF; UNKNOWN
    0x1D200,  // 1D200..1D245; GREEK
    0x1D246,  // 1D246..1D2DF; UNKNOWN
    0x1D2E0,  // 1D2E0..1D2F3; COMMON
    0x1D2F4,  // 1D2F4..1D2FF; UNKNOWN
    0x1D300,  // 1D300..1D356; COMMON
    0x1D357,  // 1D357..1D35F; UNKNOWN
    0x1D360,  // 1D360..1D378; COMMON
    0x1D379,  // 1D379..1D3FF; UNKNOWN
    0x1D400,  // 1D400..1D454; COMMON
    0x1D455,  // 1D455       ; UNKNOWN
    0x1D456,  // 1D456..1D49C; COMMON
    0x1D49D,  // 1D49D       ; UNKNOWN
    0x1D49E,  // 1D49E..1D49F; COMMON
    0x1D4A0,  // 1D4A0..1D4A1; UNKNOWN
    0x1D4A2,  // 1D4A2       ; COMMON
    0x1D4A3,  // 1D4A3..1D4A4; UNKNOWN
    0x1D4A5,  // 1D4A5..1D4A6; COMMON
    0x1D4A7,  // 1D4A7..1D4A8; UNKNOWN
    0x1D4A9,  // 1D4A9..1D4AC; COMMON
    0x1D4AD,  // 1D4AD       ; UNKNOWN
    0x1D4AE,  // 1D4AE..1D4B9; COMMON
    0x1D4BA,  // 1D4BA       ; UNKNOWN
    0x1D4BB,  // 1D4BB       ; COMMON
    0x1D4BC,  // 1D4BC       ; UNKNOWN
    0x1D4BD,  // 1D4BD..1D4C3; COMMON
    0x1D4C4,  // 1D4C4       ; UNKNOWN
    0x1D4C5,  // 1D4C5..1D505; COMMON
    0x1D506,  // 1D506       ; UNKNOWN
    0x1D507,  // 1D507..1D50A; COMMON
    0x1D50B,  // 1D50B..1D50C; UNKNOWN
    0x1D50D,  // 1D50D..1D514; COMMON
    0x1D515,  // 1D515       ; UNKNOWN
    0x1D516,  // 1D516..1D51C; COMMON
    0x1D51D,  // 1D51D       ; UNKNOWN
    0x1D51E,  // 1D51E..1D539; COMMON
    0x1D53A,  // 1D53A       ; UNKNOWN
    0x1D53B,  // 1D53B..1D53E; COMMON
    0x1D53F,  // 1D53F       ; UNKNOWN
    0x1D540,  // 1D540..1D544; COMMON
    0x1D545,  // 1D545       ; UNKNOWN
    0x1D546,  // 1D546       ; COMMON
    0x1D547,  // 1D547..1D549; UNKNOWN
    0x1D54A,  // 1D54A..1D550; COMMON
    0x1D551,  // 1D551       ; UNKNOWN
    0x1D552,  // 1D552..1D6A5; COMMON
    0x1D6A6,  // 1D6A6..1D6A7; UNKNOWN
    0x1D6A8,  // 1D6A8..1D7CB; COMMON
    0x1D7CC,  // 1D7CC..1D7CD; UNKNOWN
    0x1D7CE,  // 1D7CE..1D7FF; COMMON
    0x1D800,  // 1D800..1DA8B; SIGNWRITING
    0x1DA8C,  // 1DA8C..1DA9A; UNKNOWN
    0x1DA9B,  // 1DA9B..1DA9F; SIGNWRITING
    0x1DAA0,  // 1DAA0       ; UNKNOWN
    0x1DAA1,  // 1DAA1..1DAAF; SIGNWRITING
    0x1DAB0,  // 1DAB0..1DFFF; UNKNOWN
    0x1E000,  // 1E000..1E006; GLAGOLITIC
    0x1E007,  // 1E007       ; UNKNOWN
    0x1E008,  // 1E008..1E018; GLAGOLITIC
    0x1E019,  // 1E019..1E01A; UNKNOWN
    0x1E01B,  // 1E01B..1E021; GLAGOLITIC
    0x1E022,  // 1E022       ; UNKNOWN
    0x1E023,  // 1E023..1E024; GLAGOLITIC
    0x1E025,  // 1E025       ; UNKNOWN
    0x1E026,  // 1E026..1E02A; GLAGOLITIC
    0x1E02B,  // 1E02B..1E0FF; UNKNOWN
    0x1E100,  // 1E100..1E12C; NYIAKENG_PUACHUE_HMONG
    0x1E12D,  // 1E12D..1E12F; UNKNOWN
    0x1E130,  // 1E130..1E13D; NYIAKENG_PUACHUE_HMONG
    0x1E13E,  // 1E13E..1E13F; UNKNOWN
    0x1E140,  // 1E140..1E149; NYIAKENG_PUACHUE_HMONG
    0x1E14A,  // 1E14A..1E14D; UNKNOWN
    0x1E14E,  // 1E14E..1E14F; NYIAKENG_PUACHUE_HMONG
    0x1E150,  // 1E150..1E2BF; UNKNOWN
    0x1E2C0,  // 1E2C0..1E2F9; WANCHO
    0x1E2FA,  // 1E2FA..1E2FE; UNKNOWN
    0x1E2FF,  // 1E2FF       ; WANCHO
    0x1E300,  // 1E300..1E7FF; UNKNOWN
    0x1E800,  // 1E800..1E8C4; MENDE_KIKAKUI
    0x1E8C5,  // 1E8C5..1E8C6; UNKNOWN
    0x1E8C7,  // 1E8C7..1E8D6; MENDE_KIKAKUI
    0x1E8D7,  // 1E8D7..1E8FF; UNKNOWN
    0x1E900,  // 1E900..1E94B; ADLAM
    0x1E94C,  // 1E94C..1E94F; UNKNOWN
    0x1E950,  // 1E950..1E959; ADLAM
    0x1E95A,  // 1E95A..1E95D; UNKNOWN
    0x1E95E,  // 1E95E..1E95F; ADLAM
    0x1E960,  // 1E960..1EC70; UNKNOWN
    0x1EC71,  // 1EC71..1ECB4; COMMON
    0x1ECB5,  // 1ECB5..1ED00; UNKNOWN
    0x1ED01,  // 1ED01..1ED3D; COMMON
    0x1ED3E,  // 1ED3E..1EDFF; UNKNOWN
    0x1EE00,  // 1EE00..1EE03; ARABIC
    0x1EE04,  // 1EE04       ; UNKNOWN
    0x1EE05,  // 1EE05..1EE1F; ARABIC
    0x1EE20,  // 1EE20       ; UNKNOWN
    0x1EE21,  // 1EE21..1EE22; ARABIC
    0x1EE23,  // 1EE23       ; UNKNOWN
    0x1EE24,  // 1EE24       ; ARABIC
    0x1EE25,  // 1EE25..1EE26; UNKNOWN
    0x1EE27,  // 1EE27       ; ARABIC
    0x1EE28,  // 1EE28       ; UNKNOWN
    0x1EE29,  // 1EE29..1EE32; ARABIC
    0x1EE33,  // 1EE33       ; UNKNOWN
    0x1EE34,  // 1EE34..1EE37; ARABIC
    0x1EE38,  // 1EE38       ; UNKNOWN
    0x1EE39,  // 1EE39       ; ARABIC
    0x1EE3A,  // 1EE3A       ; UNKNOWN
    0x1EE3B,  // 1EE3B       ; ARABIC
    0x1EE3C,  // 1EE3C..1EE41; UNKNOWN
    0x1EE42,  // 1EE42       ; ARABIC
    0x1EE43,  // 1EE43..1EE46; UNKNOWN
    0x1EE47,  // 1EE47       ; ARABIC
    0x1EE48,  // 1EE48       ; UNKNOWN
    0x1EE49,  // 1EE49       ; ARABIC
    0x1EE4A,  // 1EE4A       ; UNKNOWN
    0x1EE4B,  // 1EE4B       ; ARABIC
    0x1EE4C,  // 1EE4C       ; UNKNOWN
    0x1EE4D,  // 1EE4D..1EE4F; ARABIC
    0x1EE50,  // 1EE50       ; UNKNOWN
    0x1EE51,  // 1EE51..1EE52; ARABIC
    0x1EE53,  // 1EE53       ; UNKNOWN
    0x1EE54,  // 1EE54       ; ARABIC
    0x1EE55,  // 1EE55..1EE56; UNKNOWN
    0x1EE57,  // 1EE57       ; ARABIC
    0x1EE58,  // 1EE58       ; UNKNOWN
    0x1EE59,  // 1EE59       ; ARABIC
    0x1EE5A,  // 1EE5A       ; UNKNOWN
    0x1EE5B,  // 1EE5B       ; ARABIC
    0x1EE5C,  // 1EE5C       ; UNKNOWN
    0x1EE5D,  // 1EE5D       ; ARABIC
    0x1EE5E,  // 1EE5E       ; UNKNOWN
    0x1EE5F,  // 1EE5F       ; ARABIC
    0x1EE60,  // 1EE60       ; UNKNOWN
    0x1EE61,  // 1EE61..1EE62; ARABIC
    0x1EE63,  // 1EE63       ; UNKNOWN
    0x1EE64,  // 1EE64       ; ARABIC
    0x1EE65,  // 1EE65..1EE66; UNKNOWN
    0x1EE67,  // 1EE67..1EE6A; ARABIC
    0x1EE6B,  // 1EE6B       ; UNKNOWN
    0x1EE6C,  // 1EE6C..1EE72; ARABIC
    0x1EE73,  // 1EE73       ; UNKNOWN
    0x1EE74,  // 1EE74..1EE77; ARABIC
    0x1EE78,  // 1EE78       ; UNKNOWN
    0x1EE79,  // 1EE79..1EE7C; ARABIC
    0x1EE7D,  // 1EE7D       ; UNKNOWN
    0x1EE7E,  // 1EE7E       ; ARABIC
    0x1EE7F,  // 1EE7F       ; UNKNOWN
    0x1EE80,  // 1EE80..1EE89; ARABIC
    0x1EE8A,  // 1EE8A       ; UNKNOWN
    0x1EE8B,  // 1EE8B..1EE9B; ARABIC
    0x1EE9C,  // 1EE9C..1EEA0; UNKNOWN
    0x1EEA1,  // 1EEA1..1EEA3; ARABIC
    0x1EEA4,  // 1EEA4       ; UNKNOWN
    0x1EEA5,  // 1EEA5..1EEA9; ARABIC
    0x1EEAA,  // 1EEAA       ; UNKNOWN
    0x1EEAB,  // 1EEAB..1EEBB; ARABIC
    0x1EEBC,  // 1EEBC..1EEEF; UNKNOWN
    0x1EEF0,  // 1EEF0..1EEF1; ARABIC
    0x1EEF2,  // 1EEF2..1EFFF; UNKNOWN
    0x1F000,  // 1F000..1F02B; COMMON
    0x1F02C,  // 1F02C..1F02F; UNKNOWN
    0x1F030,  // 1F030..1F093; COMMON
    0x1F094,  // 1F094..1F09F; UNKNOWN
    0x1F0A0,  // 1F0A0..1F0AE; COMMON
    0x1F0AF,  // 1F0AF..1F0B0; UNKNOWN
    0x1F0B1,  // 1F0B1..1F0BF; COMMON
    0x1F0C0,  // 1F0C0       ; UNKNOWN
    0x1F0C1,  // 1F0C1..1F0CF; COMMON
    0x1F0D0,  // 1F0D0       ; UNKNOWN
    0x1F0D1,  // 1F0D1..1F0F5; COMMON
    0x1F0F6,  // 1F0F6..1F0FF; UNKNOWN
    0x1F100,  // 1F100..1F1AD; COMMON
    0x1F1AE,  // 1F1AE..1F1E5; UNKNOWN
    0x1F1E6,  // 1F1E6..1F1FF; COMMON
    0x1F200,  // 1F200       ; HIRAGANA
    0x1F201,  // 1F201..1F202; COMMON
    0x1F203,  // 1F203..1F20F; UNKNOWN
    0x1F210,  // 1F210..1F23B; COMMON
    0x1F23C,  // 1F23C..1F23F; UNKNOWN
    0x1F240,  // 1F240..1F248; COMMON
    0x1F249,  // 1F249..1F24F; UNKNOWN
    0x1F250,  // 1F250..1F251; COMMON
    0x1F252,  // 1F252..1F25F; UNKNOWN
    0x1F260,  // 1F260..1F265; COMMON
    0x1F266,  // 1F266..1F2FF; UNKNOWN
    0x1F300,  // 1F300..1F6D7; COMMON
    0x1F6D8,  // 1F6D8..1F6DF; UNKNOWN
    0x1F6E0,  // 1F6E0..1F6EC; COMMON
    0x1F6ED,  // 1F6ED..1F6EF; UNKNOWN
    0x1F6F0,  // 1F6F0..1F6FC; COMMON
    0x1F6FD,  // 1F6FD..1F6FF; UNKNOWN
    0x1F700,  // 1F700..1F773; COMMON
    0x1F774,  // 1F774..1F77F; UNKNOWN
    0x1F780,  // 1F780..1F7D8; COMMON
    0x1F7D9,  // 1F7D9..1F7DF; UNKNOWN
    0x1F7E0,  // 1F7E0..1F7EB; COMMON
    0x1F7EC,  // 1F7EC..1F7FF; UNKNOWN
    0x1F800,  // 1F800..1F80B; COMMON
    0x1F80C,  // 1F80C..1F80F; UNKNOWN
    0x1F810,  // 1F810..1F847; COMMON
    0x1F848,  // 1F848..1F84F; UNKNOWN
    0x1F850,  // 1F850..1F859; COMMON
    0x1F85A,  // 1F85A..1F85F; UNKNOWN
    0x1F860,  // 1F860..1F887; COMMON
    0x1F888,  // 1F888..1F88F; UNKNOWN
    0x1F890,  // 1F890..1F8AD; COMMON
    0x1F8AE,  // 1F8AE..1F8AF; UNKNOWN
    0x1F8B0,  // 1F8B0..1F8B1; COMMON
    0x1F8B2,  // 1F8B2..1F8FF; UNKNOWN
    0x1F900,  // 1F900..1F978; COMMON
    0x1F979,  // 1F979       ; UNKNOWN
    0x1F97A,  // 1F97A..1F9CB; COMMON
    0x1F9CC,  // 1F9CC       ; UNKNOWN
    0x1F9CD,  // 1F9CD..1FA53; COMMON
    0x1FA54,  // 1FA54..1FA5F; UNKNOWN
    0x1FA60,  // 1FA60..1FA6D; COMMON
    0x1FA6E,  // 1FA6E..1FA6F; UNKNOWN
    0x1FA70,  // 1FA70..1FA74; COMMON
    0x1FA75,  // 1FA75..1FA77; UNKNOWN
    0x1FA78,  // 1FA78..1FA7A; COMMON
    0x1FA7B,  // 1FA7B..1FA7F; UNKNOWN
    0x1FA80,  // 1FA80..1FA86; COMMON
    0x1FA87,  // 1FA87..1FA8F; UNKNOWN
    0x1FA90,  // 1FA90..1FAA8; COMMON
    0x1FAA9,  // 1FAA9..1FAAF; UNKNOWN
    0x1FAB0,  // 1FAB0..1FAB6; COMMON
    0x1FAB7,  // 1FAB7..1FABF; UNKNOWN
    0x1FAC0,  // 1FAC0..1FAC2; COMMON
    0x1FAC3,  // 1FAC3..1FACF; UNKNOWN
    0x1FAD0,  // 1FAD0..1FAD6; COMMON
    0x1FAD7,  // 1FAD7..1FAFF; UNKNOWN
    0x1FB00,  // 1FB00..1FB92; COMMON
    0x1FB93,  // 1FB93       ; UNKNOWN
    0x1FB94,  // 1FB94..1FBCA; COMMON
    0x1FBCB,  // 1FBCB..1FBEF; UNKNOWN
    0x1FBF0,  // 1FBF0..1FBF9; COMMON
    0x1FBFA,  // 1FBFA..1FFFF; UNKNOWN
    0x20000,  // 20000..2A6DD; HAN
    0x2A6DE,  // 2A6DE..2A6FF; UNKNOWN
    0x2A700,  // 2A700..2B734; HAN
    0x2B735,  // 2B735..2B73F; UNKNOWN
    0x2B740,  // 2B740..2B81D; HAN
    0x2B81E,  // 2B81E..2B81F; UNKNOWN
    0x2B820,  // 2B820..2CEA1; HAN
    0x2CEA2,  // 2CEA2..2CEAF; UNKNOWN
    0x2CEB0,  // 2CEB0..2EBE0; HAN
    0x2EBE1,  // 2EBE1..2F7FF; UNKNOWN
    0x2F800,  // 2F800..2FA1D; HAN
    0x2FA1E,  // 2FA1E..2FFFF; UNKNOWN
    0x30000,  // 30000..3134A; HAN
    0x3134B,  // 3134B..E0000; UNKNOWN
    0xE0001,  // E0001       ; COMMON
    0xE0002,  // E0002..E001F; UNKNOWN
    0xE0020,  // E0020..E007F; COMMON
    0xE0080,  // E0080..E00FF; UNKNOWN
    0xE0100,  // E0100..E01EF; INHERITED
    0xE01F0,  // E01F0..10FFFF; UNKNOWN
};

const int Character::UnicodeScript::scripts[] = {
    Character::UnicodeScript::COMMON,                   // 0000..0040
    Character::UnicodeScript::LATIN,                    // 0041..005A
    Character::UnicodeScript::COMMON,                   // 005B..0060
    Character::UnicodeScript::LATIN,                    // 0061..007A
    Character::UnicodeScript::COMMON,                   // 007B..00A9
    Character::UnicodeScript::LATIN,                    // 00AA
    Character::UnicodeScript::COMMON,                   // 00AB..00B9
    Character::UnicodeScript::LATIN,                    // 00BA
    Character::UnicodeScript::COMMON,                   // 00BB..00BF
    Character::UnicodeScript::LATIN,                    // 00C0..00D6
    Character::UnicodeScript::COMMON,                   // 00D7
    Character::UnicodeScript::LATIN,                    // 00D8..00F6
    Character::UnicodeScript::COMMON,                   // 00F7
    Character::UnicodeScript::LATIN,                    // 00F8..02B8
    Character::UnicodeScript::COMMON,                   // 02B9..02DF
    Character::UnicodeScript::LATIN,                    // 02E0..02E4
    Character::UnicodeScript::COMMON,                   // 02E5..02E9
    Character::UnicodeScript::BOPOMOFO,                 // 02EA..02EB
    Character::UnicodeScript::COMMON,                   // 02EC..02FF
    Character::UnicodeScript::INHERITED,                // 0300..036F
    Character::UnicodeScript::GREEK,                    // 0370..0373
    Character::UnicodeScript::COMMON,                   // 0374
    Character::UnicodeScript::GREEK,                    // 0375..0377
    Character::UnicodeScript::UNKNOWN,                  // 0378..0379
    Character::UnicodeScript::GREEK,                    // 037A..037D
    Character::UnicodeScript::COMMON,                   // 037E
    Character::UnicodeScript::GREEK,                    // 037F
    Character::UnicodeScript::UNKNOWN,                  // 0380..0383
    Character::UnicodeScript::GREEK,                    // 0384
    Character::UnicodeScript::COMMON,                   // 0385
    Character::UnicodeScript::GREEK,                    // 0386
    Character::UnicodeScript::COMMON,                   // 0387
    Character::UnicodeScript::GREEK,                    // 0388..038A
    Character::UnicodeScript::UNKNOWN,                  // 038B
    Character::UnicodeScript::GREEK,                    // 038C
    Character::UnicodeScript::UNKNOWN,                  // 038D
    Character::UnicodeScript::GREEK,                    // 038E..03A1
    Character::UnicodeScript::UNKNOWN,                  // 03A2
    Character::UnicodeScript::GREEK,                    // 03A3..03E1
    Character::UnicodeScript::COPTIC,                   // 03E2..03EF
    Character::UnicodeScript::GREEK,                    // 03F0..03FF
    Character::UnicodeScript::CYRILLIC,                 // 0400..0484
    Character::UnicodeScript::INHERITED,                // 0485..0486
    Character::UnicodeScript::CYRILLIC,                 // 0487..052F
    Character::UnicodeScript::UNKNOWN,                  // 0530
    Character::UnicodeScript::ARMENIAN,                 // 0531..0556
    Character::UnicodeScript::UNKNOWN,                  // 0557..0558
    Character::UnicodeScript::ARMENIAN,                 // 0559..058A
    Character::UnicodeScript::UNKNOWN,                  // 058B..058C
    Character::UnicodeScript::ARMENIAN,                 // 058D..058F
    Character::UnicodeScript::UNKNOWN,                  // 0590
    Character::UnicodeScript::HEBREW,                   // 0591..05C7
    Character::UnicodeScript::UNKNOWN,                  // 05C8..05CF
    Character::UnicodeScript::HEBREW,                   // 05D0..05EA
    Character::UnicodeScript::UNKNOWN,                  // 05EB..05EE
    Character::UnicodeScript::HEBREW,                   // 05EF..05F4
    Character::UnicodeScript::UNKNOWN,                  // 05F5..05FF
    Character::UnicodeScript::ARABIC,                   // 0600..0604
    Character::UnicodeScript::COMMON,                   // 0605
    Character::UnicodeScript::ARABIC,                   // 0606..060B
    Character::UnicodeScript::COMMON,                   // 060C
    Character::UnicodeScript::ARABIC,                   // 060D..061A
    Character::UnicodeScript::COMMON,                   // 061B
    Character::UnicodeScript::ARABIC,                   // 061C
    Character::UnicodeScript::UNKNOWN,                  // 061D
    Character::UnicodeScript::ARABIC,                   // 061E
    Character::UnicodeScript::COMMON,                   // 061F
    Character::UnicodeScript::ARABIC,                   // 0620..063F
    Character::UnicodeScript::COMMON,                   // 0640
    Character::UnicodeScript::ARABIC,                   // 0641..064A
    Character::UnicodeScript::INHERITED,                // 064B..0655
    Character::UnicodeScript::ARABIC,                   // 0656..066F
    Character::UnicodeScript::INHERITED,                // 0670
    Character::UnicodeScript::ARABIC,                   // 0671..06DC
    Character::UnicodeScript::COMMON,                   // 06DD
    Character::UnicodeScript::ARABIC,                   // 06DE..06FF
    Character::UnicodeScript::SYRIAC,                   // 0700..070D
    Character::UnicodeScript::UNKNOWN,                  // 070E
    Character::UnicodeScript::SYRIAC,                   // 070F..074A
    Character::UnicodeScript::UNKNOWN,                  // 074B..074C
    Character::UnicodeScript::SYRIAC,                   // 074D..074F
    Character::UnicodeScript::ARABIC,                   // 0750..077F
    Character::UnicodeScript::THAANA,                   // 0780..07B1
    Character::UnicodeScript::UNKNOWN,                  // 07B2..07BF
    Character::UnicodeScript::NKO,                      // 07C0..07FA
    Character::UnicodeScript::UNKNOWN,                  // 07FB..07FC
    Character::UnicodeScript::NKO,                      // 07FD..07FF
    Character::UnicodeScript::SAMARITAN,                // 0800..082D
    Character::UnicodeScript::UNKNOWN,                  // 082E..082F
    Character::UnicodeScript::SAMARITAN,                // 0830..083E
    Character::UnicodeScript::UNKNOWN,                  // 083F
    Character::UnicodeScript::MANDAIC,                  // 0840..085B
    Character::UnicodeScript::UNKNOWN,                  // 085C..085D
    Character::UnicodeScript::MANDAIC,                  // 085E
    Character::UnicodeScript::UNKNOWN,                  // 085F
    Character::UnicodeScript::SYRIAC,                   // 0860..086A
    Character::UnicodeScript::UNKNOWN,                  // 086B..089F
    Character::UnicodeScript::ARABIC,                   // 08A0..08B4
    Character::UnicodeScript::UNKNOWN,                  // 08B5
    Character::UnicodeScript::ARABIC,                   // 08B6..08C7
    Character::UnicodeScript::UNKNOWN,                  // 08C8..08D2
    Character::UnicodeScript::ARABIC,                   // 08D3..08E1
    Character::UnicodeScript::COMMON,                   // 08E2
    Character::UnicodeScript::ARABIC,                   // 08E3..08FF
    Character::UnicodeScript::DEVANAGARI,               // 0900..0950
    Character::UnicodeScript::INHERITED,                // 0951..0954
    Character::UnicodeScript::DEVANAGARI,               // 0955..0963
    Character::UnicodeScript::COMMON,                   // 0964..0965
    Character::UnicodeScript::DEVANAGARI,               // 0966..097F
    Character::UnicodeScript::BENGALI,                  // 0980..0983
    Character::UnicodeScript::UNKNOWN,                  // 0984
    Character::UnicodeScript::BENGALI,                  // 0985..098C
    Character::UnicodeScript::UNKNOWN,                  // 098D..098E
    Character::UnicodeScript::BENGALI,                  // 098F..0990
    Character::UnicodeScript::UNKNOWN,                  // 0991..0992
    Character::UnicodeScript::BENGALI,                  // 0993..09A8
    Character::UnicodeScript::UNKNOWN,                  // 09A9
    Character::UnicodeScript::BENGALI,                  // 09AA..09B0
    Character::UnicodeScript::UNKNOWN,                  // 09B1
    Character::UnicodeScript::BENGALI,                  // 09B2
    Character::UnicodeScript::UNKNOWN,                  // 09B3..09B5
    Character::UnicodeScript::BENGALI,                  // 09B6..09B9
    Character::UnicodeScript::UNKNOWN,                  // 09BA..09BB
    Character::UnicodeScript::BENGALI,                  // 09BC..09C4
    Character::UnicodeScript::UNKNOWN,                  // 09C5..09C6
    Character::UnicodeScript::BENGALI,                  // 09C7..09C8
    Character::UnicodeScript::UNKNOWN,                  // 09C9..09CA
    Character::UnicodeScript::BENGALI,                  // 09CB..09CE
    Character::UnicodeScript::UNKNOWN,                  // 09CF..09D6
    Character::UnicodeScript::BENGALI,                  // 09D7
    Character::UnicodeScript::UNKNOWN,                  // 09D8..09DB
    Character::UnicodeScript::BENGALI,                  // 09DC..09DD
    Character::UnicodeScript::UNKNOWN,                  // 09DE
    Character::UnicodeScript::BENGALI,                  // 09DF..09E3
    Character::UnicodeScript::UNKNOWN,                  // 09E4..09E5
    Character::UnicodeScript::BENGALI,                  // 09E6..09FE
    Character::UnicodeScript::UNKNOWN,                  // 09FF..0A00
    Character::UnicodeScript::GURMUKHI,                 // 0A01..0A03
    Character::UnicodeScript::UNKNOWN,                  // 0A04
    Character::UnicodeScript::GURMUKHI,                 // 0A05..0A0A
    Character::UnicodeScript::UNKNOWN,                  // 0A0B..0A0E
    Character::UnicodeScript::GURMUKHI,                 // 0A0F..0A10
    Character::UnicodeScript::UNKNOWN,                  // 0A11..0A12
    Character::UnicodeScript::GURMUKHI,                 // 0A13..0A28
    Character::UnicodeScript::UNKNOWN,                  // 0A29
    Character::UnicodeScript::GURMUKHI,                 // 0A2A..0A30
    Character::UnicodeScript::UNKNOWN,                  // 0A31
    Character::UnicodeScript::GURMUKHI,                 // 0A32..0A33
    Character::UnicodeScript::UNKNOWN,                  // 0A34
    Character::UnicodeScript::GURMUKHI,                 // 0A35..0A36
    Character::UnicodeScript::UNKNOWN,                  // 0A37
    Character::UnicodeScript::GURMUKHI,                 // 0A38..0A39
    Character::UnicodeScript::UNKNOWN,                  // 0A3A..0A3B
    Character::UnicodeScript::GURMUKHI,                 // 0A3C
    Character::UnicodeScript::UNKNOWN,                  // 0A3D
    Character::UnicodeScript::GURMUKHI,                 // 0A3E..0A42
    Character::UnicodeScript::UNKNOWN,                  // 0A43..0A46
    Character::UnicodeScript::GURMUKHI,                 // 0A47..0A48
    Character::UnicodeScript::UNKNOWN,                  // 0A49..0A4A
    Character::UnicodeScript::GURMUKHI,                 // 0A4B..0A4D
    Character::UnicodeScript::UNKNOWN,                  // 0A4E..0A50
    Character::UnicodeScript::GURMUKHI,                 // 0A51
    Character::UnicodeScript::UNKNOWN,                  // 0A52..0A58
    Character::UnicodeScript::GURMUKHI,                 // 0A59..0A5C
    Character::UnicodeScript::UNKNOWN,                  // 0A5D
    Character::UnicodeScript::GURMUKHI,                 // 0A5E
    Character::UnicodeScript::UNKNOWN,                  // 0A5F..0A65
    Character::UnicodeScript::GURMUKHI,                 // 0A66..0A76
    Character::UnicodeScript::UNKNOWN,                  // 0A77..0A80
    Character::UnicodeScript::GUJARATI,                 // 0A81..0A83
    Character::UnicodeScript::UNKNOWN,                  // 0A84
    Character::UnicodeScript::GUJARATI,                 // 0A85..0A8D
    Character::UnicodeScript::UNKNOWN,                  // 0A8E
    Character::UnicodeScript::GUJARATI,                 // 0A8F..0A91
    Character::UnicodeScript::UNKNOWN,                  // 0A92
    Character::UnicodeScript::GUJARATI,                 // 0A93..0AA8
    Character::UnicodeScript::UNKNOWN,                  // 0AA9
    Character::UnicodeScript::GUJARATI,                 // 0AAA..0AB0
    Character::UnicodeScript::UNKNOWN,                  // 0AB1
    Character::UnicodeScript::GUJARATI,                 // 0AB2..0AB3
    Character::UnicodeScript::UNKNOWN,                  // 0AB4
    Character::UnicodeScript::GUJARATI,                 // 0AB5..0AB9
    Character::UnicodeScript::UNKNOWN,                  // 0ABA..0ABB
    Character::UnicodeScript::GUJARATI,                 // 0ABC..0AC5
    Character::UnicodeScript::UNKNOWN,                  // 0AC6
    Character::UnicodeScript::GUJARATI,                 // 0AC7..0AC9
    Character::UnicodeScript::UNKNOWN,                  // 0ACA
    Character::UnicodeScript::GUJARATI,                 // 0ACB..0ACD
    Character::UnicodeScript::UNKNOWN,                  // 0ACE..0ACF
    Character::UnicodeScript::GUJARATI,                 // 0AD0
    Character::UnicodeScript::UNKNOWN,                  // 0AD1..0ADF
    Character::UnicodeScript::GUJARATI,                 // 0AE0..0AE3
    Character::UnicodeScript::UNKNOWN,                  // 0AE4..0AE5
    Character::UnicodeScript::GUJARATI,                 // 0AE6..0AF1
    Character::UnicodeScript::UNKNOWN,                  // 0AF2..0AF8
    Character::UnicodeScript::GUJARATI,                 // 0AF9..0AFF
    Character::UnicodeScript::UNKNOWN,                  // 0B00
    Character::UnicodeScript::ORIYA,                    // 0B01..0B03
    Character::UnicodeScript::UNKNOWN,                  // 0B04
    Character::UnicodeScript::ORIYA,                    // 0B05..0B0C
    Character::UnicodeScript::UNKNOWN,                  // 0B0D..0B0E
    Character::UnicodeScript::ORIYA,                    // 0B0F..0B10
    Character::UnicodeScript::UNKNOWN,                  // 0B11..0B12
    Character::UnicodeScript::ORIYA,                    // 0B13..0B28
    Character::UnicodeScript::UNKNOWN,                  // 0B29
    Character::UnicodeScript::ORIYA,                    // 0B2A..0B30
    Character::UnicodeScript::UNKNOWN,                  // 0B31
    Character::UnicodeScript::ORIYA,                    // 0B32..0B33
    Character::UnicodeScript::UNKNOWN,                  // 0B34
    Character::UnicodeScript::ORIYA,                    // 0B35..0B39
    Character::UnicodeScript::UNKNOWN,                  // 0B3A..0B3B
    Character::UnicodeScript::ORIYA,                    // 0B3C..0B44
    Character::UnicodeScript::UNKNOWN,                  // 0B45..0B46
    Character::UnicodeScript::ORIYA,                    // 0B47..0B48
    Character::UnicodeScript::UNKNOWN,                  // 0B49..0B4A
    Character::UnicodeScript::ORIYA,                    // 0B4B..0B4D
    Character::UnicodeScript::UNKNOWN,                  // 0B4E..0B54
    Character::UnicodeScript::ORIYA,                    // 0B55..0B57
    Character::UnicodeScript::UNKNOWN,                  // 0B58..0B5B
    Character::UnicodeScript::ORIYA,                    // 0B5C..0B5D
    Character::UnicodeScript::UNKNOWN,                  // 0B5E
    Character::UnicodeScript::ORIYA,                    // 0B5F..0B63
    Character::UnicodeScript::UNKNOWN,                  // 0B64..0B65
    Character::UnicodeScript::ORIYA,                    // 0B66..0B77
    Character::UnicodeScript::UNKNOWN,                  // 0B78..0B81
    Character::UnicodeScript::TAMIL,                    // 0B82..0B83
    Character::UnicodeScript::UNKNOWN,                  // 0B84
    Character::UnicodeScript::TAMIL,                    // 0B85..0B8A
    Character::UnicodeScript::UNKNOWN,                  // 0B8B..0B8D
    Character::UnicodeScript::TAMIL,                    // 0B8E..0B90
    Character::UnicodeScript::UNKNOWN,                  // 0B91
    Character::UnicodeScript::TAMIL,                    // 0B92..0B95
    Character::UnicodeScript::UNKNOWN,                  // 0B96..0B98
    Character::UnicodeScript::TAMIL,                    // 0B99..0B9A
    Character::UnicodeScript::UNKNOWN,                  // 0B9B
    Character::UnicodeScript::TAMIL,                    // 0B9C
    Character::UnicodeScript::UNKNOWN,                  // 0B9D
    Character::UnicodeScript::TAMIL,                    // 0B9E..0B9F
    Character::UnicodeScript::UNKNOWN,                  // 0BA0..0BA2
    Character::UnicodeScript::TAMIL,                    // 0BA3..0BA4
    Character::UnicodeScript::UNKNOWN,                  // 0BA5..0BA7
    Character::UnicodeScript::TAMIL,                    // 0BA8..0BAA
    Character::UnicodeScript::UNKNOWN,                  // 0BAB..0BAD
    Character::UnicodeScript::TAMIL,                    // 0BAE..0BB9
    Character::UnicodeScript::UNKNOWN,                  // 0BBA..0BBD
    Character::UnicodeScript::TAMIL,                    // 0BBE..0BC2
    Character::UnicodeScript::UNKNOWN,                  // 0BC3..0BC5
    Character::UnicodeScript::TAMIL,                    // 0BC6..0BC8
    Character::UnicodeScript::UNKNOWN,                  // 0BC9
    Character::UnicodeScript::TAMIL,                    // 0BCA..0BCD
    Character::UnicodeScript::UNKNOWN,                  // 0BCE..0BCF
    Character::UnicodeScript::TAMIL,                    // 0BD0
    Character::UnicodeScript::UNKNOWN,                  // 0BD1..0BD6
    Character::UnicodeScript::TAMIL,                    // 0BD7
    Character::UnicodeScript::UNKNOWN,                  // 0BD8..0BE5
    Character::UnicodeScript::TAMIL,                    // 0BE6..0BFA
    Character::UnicodeScript::UNKNOWN,                  // 0BFB..0BFF
    Character::UnicodeScript::TELUGU,                   // 0C00..0C0C
    Character::UnicodeScript::UNKNOWN,                  // 0C0D
    Character::UnicodeScript::TELUGU,                   // 0C0E..0C10
    Character::UnicodeScript::UNKNOWN,                  // 0C11
    Character::UnicodeScript::TELUGU,                   // 0C12..0C28
    Character::UnicodeScript::UNKNOWN,                  // 0C29
    Character::UnicodeScript::TELUGU,                   // 0C2A..0C39
    Character::UnicodeScript::UNKNOWN,                  // 0C3A..0C3C
    Character::UnicodeScript::TELUGU,                   // 0C3D..0C44
    Character::UnicodeScript::UNKNOWN,                  // 0C45
    Character::UnicodeScript::TELUGU,                   // 0C46..0C48
    Character::UnicodeScript::UNKNOWN,                  // 0C49
    Character::UnicodeScript::TELUGU,                   // 0C4A..0C4D
    Character::UnicodeScript::UNKNOWN,                  // 0C4E..0C54
    Character::UnicodeScript::TELUGU,                   // 0C55..0C56
    Character::UnicodeScript::UNKNOWN,                  // 0C57
    Character::UnicodeScript::TELUGU,                   // 0C58..0C5A
    Character::UnicodeScript::UNKNOWN,                  // 0C5B..0C5F
    Character::UnicodeScript::TELUGU,                   // 0C60..0C63
    Character::UnicodeScript::UNKNOWN,                  // 0C64..0C65
    Character::UnicodeScript::TELUGU,                   // 0C66..0C6F
    Character::UnicodeScript::UNKNOWN,                  // 0C70..0C76
    Character::UnicodeScript::TELUGU,                   // 0C77..0C7F
    Character::UnicodeScript::KANNADA,                  // 0C80..0C8C
    Character::UnicodeScript::UNKNOWN,                  // 0C8D
    Character::UnicodeScript::KANNADA,                  // 0C8E..0C90
    Character::UnicodeScript::UNKNOWN,                  // 0C91
    Character::UnicodeScript::KANNADA,                  // 0C92..0CA8
    Character::UnicodeScript::UNKNOWN,                  // 0CA9
    Character::UnicodeScript::KANNADA,                  // 0CAA..0CB3
    Character::UnicodeScript::UNKNOWN,                  // 0CB4
    Character::UnicodeScript::KANNADA,                  // 0CB5..0CB9
    Character::UnicodeScript::UNKNOWN,                  // 0CBA..0CBB
    Character::UnicodeScript::KANNADA,                  // 0CBC..0CC4
    Character::UnicodeScript::UNKNOWN,                  // 0CC5
    Character::UnicodeScript::KANNADA,                  // 0CC6..0CC8
    Character::UnicodeScript::UNKNOWN,                  // 0CC9
    Character::UnicodeScript::KANNADA,                  // 0CCA..0CCD
    Character::UnicodeScript::UNKNOWN,                  // 0CCE..0CD4
    Character::UnicodeScript::KANNADA,                  // 0CD5..0CD6
    Character::UnicodeScript::UNKNOWN,                  // 0CD7..0CDD
    Character::UnicodeScript::KANNADA,                  // 0CDE
    Character::UnicodeScript::UNKNOWN,                  // 0CDF
    Character::UnicodeScript::KANNADA,                  // 0CE0..0CE3
    Character::UnicodeScript::UNKNOWN,                  // 0CE4..0CE5
    Character::UnicodeScript::KANNADA,                  // 0CE6..0CEF
    Character::UnicodeScript::UNKNOWN,                  // 0CF0
    Character::UnicodeScript::KANNADA,                  // 0CF1..0CF2
    Character::UnicodeScript::UNKNOWN,                  // 0CF3..0CFF
    Character::UnicodeScript::MALAYALAM,                // 0D00..0D0C
    Character::UnicodeScript::UNKNOWN,                  // 0D0D
    Character::UnicodeScript::MALAYALAM,                // 0D0E..0D10
    Character::UnicodeScript::UNKNOWN,                  // 0D11
    Character::UnicodeScript::MALAYALAM,                // 0D12..0D44
    Character::UnicodeScript::UNKNOWN,                  // 0D45
    Character::UnicodeScript::MALAYALAM,                // 0D46..0D48
    Character::UnicodeScript::UNKNOWN,                  // 0D49
    Character::UnicodeScript::MALAYALAM,                // 0D4A..0D4F
    Character::UnicodeScript::UNKNOWN,                  // 0D50..0D53
    Character::UnicodeScript::MALAYALAM,                // 0D54..0D63
    Character::UnicodeScript::UNKNOWN,                  // 0D64..0D65
    Character::UnicodeScript::MALAYALAM,                // 0D66..0D7F
    Character::UnicodeScript::UNKNOWN,                  // 0D80
    Character::UnicodeScript::SINHALA,                  // 0D81..0D83
    Character::UnicodeScript::UNKNOWN,                  // 0D84
    Character::UnicodeScript::SINHALA,                  // 0D85..0D96
    Character::UnicodeScript::UNKNOWN,                  // 0D97..0D99
    Character::UnicodeScript::SINHALA,                  // 0D9A..0DB1
    Character::UnicodeScript::UNKNOWN,                  // 0DB2
    Character::UnicodeScript::SINHALA,                  // 0DB3..0DBB
    Character::UnicodeScript::UNKNOWN,                  // 0DBC
    Character::UnicodeScript::SINHALA,                  // 0DBD
    Character::UnicodeScript::UNKNOWN,                  // 0DBE..0DBF
    Character::UnicodeScript::SINHALA,                  // 0DC0..0DC6
    Character::UnicodeScript::UNKNOWN,                  // 0DC7..0DC9
    Character::UnicodeScript::SINHALA,                  // 0DCA
    Character::UnicodeScript::UNKNOWN,                  // 0DCB..0DCE
    Character::UnicodeScript::SINHALA,                  // 0DCF..0DD4
    Character::UnicodeScript::UNKNOWN,                  // 0DD5
    Character::UnicodeScript::SINHALA,                  // 0DD6
    Character::UnicodeScript::UNKNOWN,                  // 0DD7
    Character::UnicodeScript::SINHALA,                  // 0DD8..0DDF
    Character::UnicodeScript::UNKNOWN,                  // 0DE0..0DE5
    Character::UnicodeScript::SINHALA,                  // 0DE6..0DEF
    Character::UnicodeScript::UNKNOWN,                  // 0DF0..0DF1
    Character::UnicodeScript::SINHALA,                  // 0DF2..0DF4
    Character::UnicodeScript::UNKNOWN,                  // 0DF5..0E00
    Character::UnicodeScript::THAI,                     // 0E01..0E3A
    Character::UnicodeScript::UNKNOWN,                  // 0E3B..0E3E
    Character::UnicodeScript::COMMON,                   // 0E3F
    Character::UnicodeScript::THAI,                     // 0E40..0E5B
    Character::UnicodeScript::UNKNOWN,                  // 0E5C..0E80
    Character::UnicodeScript::LAO,                      // 0E81..0E82
    Character::UnicodeScript::UNKNOWN,                  // 0E83
    Character::UnicodeScript::LAO,                      // 0E84
    Character::UnicodeScript::UNKNOWN,                  // 0E85
    Character::UnicodeScript::LAO,                      // 0E86..0E8A
    Character::UnicodeScript::UNKNOWN,                  // 0E8B
    Character::UnicodeScript::LAO,                      // 0E8C..0EA3
    Character::UnicodeScript::UNKNOWN,                  // 0EA4
    Character::UnicodeScript::LAO,                      // 0EA5
    Character::UnicodeScript::UNKNOWN,                  // 0EA6
    Character::UnicodeScript::LAO,                      // 0EA7..0EBD
    Character::UnicodeScript::UNKNOWN,                  // 0EBE..0EBF
    Character::UnicodeScript::LAO,                      // 0EC0..0EC4
    Character::UnicodeScript::UNKNOWN,                  // 0EC5
    Character::UnicodeScript::LAO,                      // 0EC6
    Character::UnicodeScript::UNKNOWN,                  // 0EC7
    Character::UnicodeScript::LAO,                      // 0EC8..0ECD
    Character::UnicodeScript::UNKNOWN,                  // 0ECE..0ECF
    Character::UnicodeScript::LAO,                      // 0ED0..0ED9
    Character::UnicodeScript::UNKNOWN,                  // 0EDA..0EDB
    Character::UnicodeScript::LAO,                      // 0EDC..0EDF
    Character::UnicodeScript::UNKNOWN,                  // 0EE0..0EFF
    Character::UnicodeScript::TIBETAN,                  // 0F00..0F47
    Character::UnicodeScript::UNKNOWN,                  // 0F48
    Character::UnicodeScript::TIBETAN,                  // 0F49..0F6C
    Character::UnicodeScript::UNKNOWN,                  // 0F6D..0F70
    Character::UnicodeScript::TIBETAN,                  // 0F71..0F97
    Character::UnicodeScript::UNKNOWN,                  // 0F98
    Character::UnicodeScript::TIBETAN,                  // 0F99..0FBC
    Character::UnicodeScript::UNKNOWN,                  // 0FBD
    Character::UnicodeScript::TIBETAN,                  // 0FBE..0FCC
    Character::UnicodeScript::UNKNOWN,                  // 0FCD
    Character::UnicodeScript::TIBETAN,                  // 0FCE..0FD4
    Character::UnicodeScript::COMMON,                   // 0FD5..0FD8
    Character::UnicodeScript::TIBETAN,                  // 0FD9..0FDA
    Character::UnicodeScript::UNKNOWN,                  // 0FDB..0FFF
    Character::UnicodeScript::MYANMAR,                  // 1000..109F
    Character::UnicodeScript::GEORGIAN,                 // 10A0..10C5
    Character::UnicodeScript::UNKNOWN,                  // 10C6
    Character::UnicodeScript::GEORGIAN,                 // 10C7
    Character::UnicodeScript::UNKNOWN,                  // 10C8..10CC
    Character::UnicodeScript::GEORGIAN,                 // 10CD
    Character::UnicodeScript::UNKNOWN,                  // 10CE..10CF
    Character::UnicodeScript::GEORGIAN,                 // 10D0..10FA
    Character::UnicodeScript::COMMON,                   // 10FB
    Character::UnicodeScript::GEORGIAN,                 // 10FC..10FF
    Character::UnicodeScript::HANGUL,                   // 1100..11FF
    Character::UnicodeScript::ETHIOPIC,                 // 1200..1248
    Character::UnicodeScript::UNKNOWN,                  // 1249
    Character::UnicodeScript::ETHIOPIC,                 // 124A..124D
    Character::UnicodeScript::UNKNOWN,                  // 124E..124F
    Character::UnicodeScript::ETHIOPIC,                 // 1250..1256
    Character::UnicodeScript::UNKNOWN,                  // 1257
    Character::UnicodeScript::ETHIOPIC,                 // 1258
    Character::UnicodeScript::UNKNOWN,                  // 1259
    Character::UnicodeScript::ETHIOPIC,                 // 125A..125D
    Character::UnicodeScript::UNKNOWN,                  // 125E..125F
    Character::UnicodeScript::ETHIOPIC,                 // 1260..1288
    Character::UnicodeScript::UNKNOWN,                  // 1289
    Character::UnicodeScript::ETHIOPIC,                 // 128A..128D
    Character::UnicodeScript::UNKNOWN,                  // 128E..128F
    Character::UnicodeScript::ETHIOPIC,                 // 1290..12B0
    Character::UnicodeScript::UNKNOWN,                  // 12B1
    Character::UnicodeScript::ETHIOPIC,                 // 12B2..12B5
    Character::UnicodeScript::UNKNOWN,                  // 12B6..12B7
    Character::UnicodeScript::ETHIOPIC,                 // 12B8..12BE
    Character::UnicodeScript::UNKNOWN,                  // 12BF
    Character::UnicodeScript::ETHIOPIC,                 // 12C0
    Character::UnicodeScript::UNKNOWN,                  // 12C1
    Character::UnicodeScript::ETHIOPIC,                 // 12C2..12C5
    Character::UnicodeScript::UNKNOWN,                  // 12C6..12C7
    Character::UnicodeScript::ETHIOPIC,                 // 12C8..12D6
    Character::UnicodeScript::UNKNOWN,                  // 12D7
    Character::UnicodeScript::ETHIOPIC,                 // 12D8..1310
    Character::UnicodeScript::UNKNOWN,                  // 1311
    Character::UnicodeScript::ETHIOPIC,                 // 1312..1315
    Character::UnicodeScript::UNKNOWN,                  // 1316..1317
    Character::UnicodeScript::ETHIOPIC,                 // 1318..135A
    Character::UnicodeScript::UNKNOWN,                  // 135B..135C
    Character::UnicodeScript::ETHIOPIC,                 // 135D..137C
    Character::UnicodeScript::UNKNOWN,                  // 137D..137F
    Character::UnicodeScript::ETHIOPIC,                 // 1380..1399
    Character::UnicodeScript::UNKNOWN,                  // 139A..139F
    Character::UnicodeScript::CHEROKEE,                 // 13A0..13F5
    Character::UnicodeScript::UNKNOWN,                  // 13F6..13F7
    Character::UnicodeScript::CHEROKEE,                 // 13F8..13FD
    Character::UnicodeScript::UNKNOWN,                  // 13FE..13FF
    Character::UnicodeScript::CANADIAN_ABORIGINAL,      // 1400..167F
    Character::UnicodeScript::OGHAM,                    // 1680..169C
    Character::UnicodeScript::UNKNOWN,                  // 169D..169F
    Character::UnicodeScript::RUNIC,                    // 16A0..16EA
    Character::UnicodeScript::COMMON,                   // 16EB..16ED
    Character::UnicodeScript::RUNIC,                    // 16EE..16F8
    Character::UnicodeScript::UNKNOWN,                  // 16F9..16FF
    Character::UnicodeScript::TAGALOG,                  // 1700..170C
    Character::UnicodeScript::UNKNOWN,                  // 170D
    Character::UnicodeScript::TAGALOG,                  // 170E..1714
    Character::UnicodeScript::UNKNOWN,                  // 1715..171F
    Character::UnicodeScript::HANUNOO,                  // 1720..1734
    Character::UnicodeScript::COMMON,                   // 1735..1736
    Character::UnicodeScript::UNKNOWN,                  // 1737..173F
    Character::UnicodeScript::BUHID,                    // 1740..1753
    Character::UnicodeScript::UNKNOWN,                  // 1754..175F
    Character::UnicodeScript::TAGBANWA,                 // 1760..176C
    Character::UnicodeScript::UNKNOWN,                  // 176D
    Character::UnicodeScript::TAGBANWA,                 // 176E..1770
    Character::UnicodeScript::UNKNOWN,                  // 1771
    Character::UnicodeScript::TAGBANWA,                 // 1772..1773
    Character::UnicodeScript::UNKNOWN,                  // 1774..177F
    Character::UnicodeScript::KHMER,                    // 1780..17DD
    Character::UnicodeScript::UNKNOWN,                  // 17DE..17DF
    Character::UnicodeScript::KHMER,                    // 17E0..17E9
    Character::UnicodeScript::UNKNOWN,                  // 17EA..17EF
    Character::UnicodeScript::KHMER,                    // 17F0..17F9
    Character::UnicodeScript::UNKNOWN,                  // 17FA..17FF
    Character::UnicodeScript::MONGOLIAN,                // 1800..1801
    Character::UnicodeScript::COMMON,                   // 1802..1803
    Character::UnicodeScript::MONGOLIAN,                // 1804
    Character::UnicodeScript::COMMON,                   // 1805
    Character::UnicodeScript::MONGOLIAN,                // 1806..180E
    Character::UnicodeScript::UNKNOWN,                  // 180F
    Character::UnicodeScript::MONGOLIAN,                // 1810..1819
    Character::UnicodeScript::UNKNOWN,                  // 181A..181F
    Character::UnicodeScript::MONGOLIAN,                // 1820..1878
    Character::UnicodeScript::UNKNOWN,                  // 1879..187F
    Character::UnicodeScript::MONGOLIAN,                // 1880..18AA
    Character::UnicodeScript::UNKNOWN,                  // 18AB..18AF
    Character::UnicodeScript::CANADIAN_ABORIGINAL,      // 18B0..18F5
    Character::UnicodeScript::UNKNOWN,                  // 18F6..18FF
    Character::UnicodeScript::LIMBU,                    // 1900..191E
    Character::UnicodeScript::UNKNOWN,                  // 191F
    Character::UnicodeScript::LIMBU,                    // 1920..192B
    Character::UnicodeScript::UNKNOWN,                  // 192C..192F
    Character::UnicodeScript::LIMBU,                    // 1930..193B
    Character::UnicodeScript::UNKNOWN,                  // 193C..193F
    Character::UnicodeScript::LIMBU,                    // 1940
    Character::UnicodeScript::UNKNOWN,                  // 1941..1943
    Character::UnicodeScript::LIMBU,                    // 1944..194F
    Character::UnicodeScript::TAI_LE,                   // 1950..196D
    Character::UnicodeScript::UNKNOWN,                  // 196E..196F
    Character::UnicodeScript::TAI_LE,                   // 1970..1974
    Character::UnicodeScript::UNKNOWN,                  // 1975..197F
    Character::UnicodeScript::NEW_TAI_LUE,              // 1980..19AB
    Character::UnicodeScript::UNKNOWN,                  // 19AC..19AF
    Character::UnicodeScript::NEW_TAI_LUE,              // 19B0..19C9
    Character::UnicodeScript::UNKNOWN,                  // 19CA..19CF
    Character::UnicodeScript::NEW_TAI_LUE,              // 19D0..19DA
    Character::UnicodeScript::UNKNOWN,                  // 19DB..19DD
    Character::UnicodeScript::NEW_TAI_LUE,              // 19DE..19DF
    Character::UnicodeScript::KHMER,                    // 19E0..19FF
    Character::UnicodeScript::BUGINESE,                 // 1A00..1A1B
    Character::UnicodeScript::UNKNOWN,                  // 1A1C..1A1D
    Character::UnicodeScript::BUGINESE,                 // 1A1E..1A1F
    Character::UnicodeScript::TAI_THAM,                 // 1A20..1A5E
    Character::UnicodeScript::UNKNOWN,                  // 1A5F
    Character::UnicodeScript::TAI_THAM,                 // 1A60..1A7C
    Character::UnicodeScript::UNKNOWN,                  // 1A7D..1A7E
    Character::UnicodeScript::TAI_THAM,                 // 1A7F..1A89
    Character::UnicodeScript::UNKNOWN,                  // 1A8A..1A8F
    Character::UnicodeScript::TAI_THAM,                 // 1A90..1A99
    Character::UnicodeScript::UNKNOWN,                  // 1A9A..1A9F
    Character::UnicodeScript::TAI_THAM,                 // 1AA0..1AAD
    Character::UnicodeScript::UNKNOWN,                  // 1AAE..1AAF
    Character::UnicodeScript::INHERITED,                // 1AB0..1AC0
    Character::UnicodeScript::UNKNOWN,                  // 1AC1..1AFF
    Character::UnicodeScript::BALINESE,                 // 1B00..1B4B
    Character::UnicodeScript::UNKNOWN,                  // 1B4C..1B4F
    Character::UnicodeScript::BALINESE,                 // 1B50..1B7C
    Character::UnicodeScript::UNKNOWN,                  // 1B7D..1B7F
    Character::UnicodeScript::SUNDANESE,                // 1B80..1BBF
    Character::UnicodeScript::BATAK,                    // 1BC0..1BF3
    Character::UnicodeScript::UNKNOWN,                  // 1BF4..1BFB
    Character::UnicodeScript::BATAK,                    // 1BFC..1BFF
    Character::UnicodeScript::LEPCHA,                   // 1C00..1C37
    Character::UnicodeScript::UNKNOWN,                  // 1C38..1C3A
    Character::UnicodeScript::LEPCHA,                   // 1C3B..1C49
    Character::UnicodeScript::UNKNOWN,                  // 1C4A..1C4C
    Character::UnicodeScript::LEPCHA,                   // 1C4D..1C4F
    Character::UnicodeScript::OL_CHIKI,                 // 1C50..1C7F
    Character::UnicodeScript::CYRILLIC,                 // 1C80..1C88
    Character::UnicodeScript::UNKNOWN,                  // 1C89..1C8F
    Character::UnicodeScript::GEORGIAN,                 // 1C90..1CBA
    Character::UnicodeScript::UNKNOWN,                  // 1CBB..1CBC
    Character::UnicodeScript::GEORGIAN,                 // 1CBD..1CBF
    Character::UnicodeScript::SUNDANESE,                // 1CC0..1CC7
    Character::UnicodeScript::UNKNOWN,                  // 1CC8..1CCF
    Character::UnicodeScript::INHERITED,                // 1CD0..1CD2
    Character::UnicodeScript::COMMON,                   // 1CD3
    Character::UnicodeScript::INHERITED,                // 1CD4..1CE0
    Character::UnicodeScript::COMMON,                   // 1CE1
    Character::UnicodeScript::INHERITED,                // 1CE2..1CE8
    Character::UnicodeScript::COMMON,                   // 1CE9..1CEC
    Character::UnicodeScript::INHERITED,                // 1CED
    Character::UnicodeScript::COMMON,                   // 1CEE..1CF3
    Character::UnicodeScript::INHERITED,                // 1CF4
    Character::UnicodeScript::COMMON,                   // 1CF5..1CF7
    Character::UnicodeScript::INHERITED,                // 1CF8..1CF9
    Character::UnicodeScript::COMMON,                   // 1CFA
    Character::UnicodeScript::UNKNOWN,                  // 1CFB..1CFF
    Character::UnicodeScript::LATIN,                    // 1D00..1D25
    Character::UnicodeScript::GREEK,                    // 1D26..1D2A
    Character::UnicodeScript::CYRILLIC,                 // 1D2B
    Character::UnicodeScript::LATIN,                    // 1D2C..1D5C
    Character::UnicodeScript::GREEK,                    // 1D5D..1D61
    Character::UnicodeScript::LATIN,                    // 1D62..1D65
    Character::UnicodeScript::GREEK,                    // 1D66..1D6A
    Character::UnicodeScript::LATIN,                    // 1D6B..1D77
    Character::UnicodeScript::CYRILLIC,                 // 1D78
    Character::UnicodeScript::LATIN,                    // 1D79..1DBE
    Character::UnicodeScript::GREEK,                    // 1DBF
    Character::UnicodeScript::INHERITED,                // 1DC0..1DF9
    Character::UnicodeScript::UNKNOWN,                  // 1DFA
    Character::UnicodeScript::INHERITED,                // 1DFB..1DFF
    Character::UnicodeScript::LATIN,                    // 1E00..1EFF
    Character::UnicodeScript::GREEK,                    // 1F00..1F15
    Character::UnicodeScript::UNKNOWN,                  // 1F16..1F17
    Character::UnicodeScript::GREEK,                    // 1F18..1F1D
    Character::UnicodeScript::UNKNOWN,                  // 1F1E..1F1F
    Character::UnicodeScript::GREEK,                    // 1F20..1F45
    Character::UnicodeScript::UNKNOWN,                  // 1F46..1F47
    Character::UnicodeScript::GREEK,                    // 1F48..1F4D
    Character::UnicodeScript::UNKNOWN,                  // 1F4E..1F4F
    Character::UnicodeScript::GREEK,                    // 1F50..1F57
    Character::UnicodeScript::UNKNOWN,                  // 1F58
    Character::UnicodeScript::GREEK,                    // 1F59
    Character::UnicodeScript::UNKNOWN,                  // 1F5A
    Character::UnicodeScript::GREEK,                    // 1F5B
    Character::UnicodeScript::UNKNOWN,                  // 1F5C
    Character::UnicodeScript::GREEK,                    // 1F5D
    Character::UnicodeScript::UNKNOWN,                  // 1F5E
    Character::UnicodeScript::GREEK,                    // 1F5F..1F7D
    Character::UnicodeScript::UNKNOWN,                  // 1F7E..1F7F
    Character::UnicodeScript::GREEK,                    // 1F80..1FB4
    Character::UnicodeScript::UNKNOWN,                  // 1FB5
    Character::UnicodeScript::GREEK,                    // 1FB6..1FC4
    Character::UnicodeScript::UNKNOWN,                  // 1FC5
    Character::UnicodeScript::GREEK,                    // 1FC6..1FD3
    Character::UnicodeScript::UNKNOWN,                  // 1FD4..1FD5
    Character::UnicodeScript::GREEK,                    // 1FD6..1FDB
    Character::UnicodeScript::UNKNOWN,                  // 1FDC
    Character::UnicodeScript::GREEK,                    // 1FDD..1FEF
    Character::UnicodeScript::UNKNOWN,                  // 1FF0..1FF1
    Character::UnicodeScript::GREEK,                    // 1FF2..1FF4
    Character::UnicodeScript::UNKNOWN,                  // 1FF5
    Character::UnicodeScript::GREEK,                    // 1FF6..1FFE
    Character::UnicodeScript::UNKNOWN,                  // 1FFF
    Character::UnicodeScript::COMMON,                   // 2000..200B
    Character::UnicodeScript::INHERITED,                // 200C..200D
    Character::UnicodeScript::COMMON,                   // 200E..2064
    Character::UnicodeScript::UNKNOWN,                  // 2065
    Character::UnicodeScript::COMMON,                   // 2066..2070
    Character::UnicodeScript::LATIN,                    // 2071
    Character::UnicodeScript::UNKNOWN,                  // 2072..2073
    Character::UnicodeScript::COMMON,                   // 2074..207E
    Character::UnicodeScript::LATIN,                    // 207F
    Character::UnicodeScript::COMMON,                   // 2080..208E
    Character::UnicodeScript::UNKNOWN,                  // 208F
    Character::UnicodeScript::LATIN,                    // 2090..209C
    Character::UnicodeScript::UNKNOWN,                  // 209D..209F
    Character::UnicodeScript::COMMON,                   // 20A0..20BF
    Character::UnicodeScript::UNKNOWN,                  // 20C0..20CF
    Character::UnicodeScript::INHERITED,                // 20D0..20F0
    Character::UnicodeScript::UNKNOWN,                  // 20F1..20FF
    Character::UnicodeScript::COMMON,                   // 2100..2125
    Character::UnicodeScript::GREEK,                    // 2126
    Character::UnicodeScript::COMMON,                   // 2127..2129
    Character::UnicodeScript::LATIN,                    // 212A..212B
    Character::UnicodeScript::COMMON,                   // 212C..2131
    Character::UnicodeScript::LATIN,                    // 2132
    Character::UnicodeScript::COMMON,                   // 2133..214D
    Character::UnicodeScript::LATIN,                    // 214E
    Character::UnicodeScript::COMMON,                   // 214F..215F
    Character::UnicodeScript::LATIN,                    // 2160..2188
    Character::UnicodeScript::COMMON,                   // 2189..218B
    Character::UnicodeScript::UNKNOWN,                  // 218C..218F
    Character::UnicodeScript::COMMON,                   // 2190..2426
    Character::UnicodeScript::UNKNOWN,                  // 2427..243F
    Character::UnicodeScript::COMMON,                   // 2440..244A
    Character::UnicodeScript::UNKNOWN,                  // 244B..245F
    Character::UnicodeScript::COMMON,                   // 2460..27FF
    Character::UnicodeScript::BRAILLE,                  // 2800..28FF
    Character::UnicodeScript::COMMON,                   // 2900..2B73
    Character::UnicodeScript::UNKNOWN,                  // 2B74..2B75
    Character::UnicodeScript::COMMON,                   // 2B76..2B95
    Character::UnicodeScript::UNKNOWN,                  // 2B96
    Character::UnicodeScript::COMMON,                   // 2B97..2BFF
    Character::UnicodeScript::GLAGOLITIC,               // 2C00..2C2E
    Character::UnicodeScript::UNKNOWN,                  // 2C2F
    Character::UnicodeScript::GLAGOLITIC,               // 2C30..2C5E
    Character::UnicodeScript::UNKNOWN,                  // 2C5F
    Character::UnicodeScript::LATIN,                    // 2C60..2C7F
    Character::UnicodeScript::COPTIC,                   // 2C80..2CF3
    Character::UnicodeScript::UNKNOWN,                  // 2CF4..2CF8
    Character::UnicodeScript::COPTIC,                   // 2CF9..2CFF
    Character::UnicodeScript::GEORGIAN,                 // 2D00..2D25
    Character::UnicodeScript::UNKNOWN,                  // 2D26
    Character::UnicodeScript::GEORGIAN,                 // 2D27
    Character::UnicodeScript::UNKNOWN,                  // 2D28..2D2C
    Character::UnicodeScript::GEORGIAN,                 // 2D2D
    Character::UnicodeScript::UNKNOWN,                  // 2D2E..2D2F
    Character::UnicodeScript::TIFINAGH,                 // 2D30..2D67
    Character::UnicodeScript::UNKNOWN,                  // 2D68..2D6E
    Character::UnicodeScript::TIFINAGH,                 // 2D6F..2D70
    Character::UnicodeScript::UNKNOWN,                  // 2D71..2D7E
    Character::UnicodeScript::TIFINAGH,                 // 2D7F
    Character::UnicodeScript::ETHIOPIC,                 // 2D80..2D96
    Character::UnicodeScript::UNKNOWN,                  // 2D97..2D9F
    Character::UnicodeScript::ETHIOPIC,                 // 2DA0..2DA6
    Character::UnicodeScript::UNKNOWN,                  // 2DA7
    Character::UnicodeScript::ETHIOPIC,                 // 2DA8..2DAE
    Character::UnicodeScript::UNKNOWN,                  // 2DAF
    Character::UnicodeScript::ETHIOPIC,                 // 2DB0..2DB6
    Character::UnicodeScript::UNKNOWN,                  // 2DB7
    Character::UnicodeScript::ETHIOPIC,                 // 2DB8..2DBE
    Character::UnicodeScript::UNKNOWN,                  // 2DBF
    Character::UnicodeScript::ETHIOPIC,                 // 2DC0..2DC6
    Character::UnicodeScript::UNKNOWN,                  // 2DC7
    Character::UnicodeScript::ETHIOPIC,                 // 2DC8..2DCE
    Character::UnicodeScript::UNKNOWN,                  // 2DCF
    Character::UnicodeScript::ETHIOPIC,                 // 2DD0..2DD6
    Character::UnicodeScript::UNKNOWN,                  // 2DD7
    Character::UnicodeScript::ETHIOPIC,                 // 2DD8..2DDE
    Character::UnicodeScript::UNKNOWN,                  // 2DDF
    Character::UnicodeScript::CYRILLIC,                 // 2DE0..2DFF
    Character::UnicodeScript::COMMON,                   // 2E00..2E52
    Character::UnicodeScript::UNKNOWN,                  // 2E53..2E7F
    Character::UnicodeScript::HAN,                      // 2E80..2E99
    Character::UnicodeScript::UNKNOWN,                  // 2E9A
    Character::UnicodeScript::HAN,                      // 2E9B..2EF3
    Character::UnicodeScript::UNKNOWN,                  // 2EF4..2EFF
    Character::UnicodeScript::HAN,                      // 2F00..2FD5
    Character::UnicodeScript::UNKNOWN,                  // 2FD6..2FEF
    Character::UnicodeScript::COMMON,                   // 2FF0..2FFB
    Character::UnicodeScript::UNKNOWN,                  // 2FFC..2FFF
    Character::UnicodeScript::COMMON,                   // 3000..3004
    Character::UnicodeScript::HAN,                      // 3005
    Character::UnicodeScript::COMMON,                   // 3006
    Character::UnicodeScript::HAN,                      // 3007
    Character::UnicodeScript::COMMON,                   // 3008..3020
    Character::UnicodeScript::HAN,                      // 3021..3029
    Character::UnicodeScript::INHERITED,                // 302A..302D
    Character::UnicodeScript::HANGUL,                   // 302E..302F
    Character::UnicodeScript::COMMON,                   // 3030..3037
    Character::UnicodeScript::HAN,                      // 3038..303B
    Character::UnicodeScript::COMMON,                   // 303C..303F
    Character::UnicodeScript::UNKNOWN,                  // 3040
    Character::UnicodeScript::HIRAGANA,                 // 3041..3096
    Character::UnicodeScript::UNKNOWN,                  // 3097..3098
    Character::UnicodeScript::INHERITED,                // 3099..309A
    Character::UnicodeScript::COMMON,                   // 309B..309C
    Character::UnicodeScript::HIRAGANA,                 // 309D..309F
    Character::UnicodeScript::COMMON,                   // 30A0
    Character::UnicodeScript::KATAKANA,                 // 30A1..30FA
    Character::UnicodeScript::COMMON,                   // 30FB..30FC
    Character::UnicodeScript::KATAKANA,                 // 30FD..30FF
    Character::UnicodeScript::UNKNOWN,                  // 3100..3104
    Character::UnicodeScript::BOPOMOFO,                 // 3105..312F
    Character::UnicodeScript::UNKNOWN,                  // 3130
    Character::UnicodeScript::HANGUL,                   // 3131..318E
    Character::UnicodeScript::UNKNOWN,                  // 318F
    Character::UnicodeScript::COMMON,                   // 3190..319F
    Character::UnicodeScript::BOPOMOFO,                 // 31A0..31BF
    Character::UnicodeScript::COMMON,                   // 31C0..31E3
    Character::UnicodeScript::UNKNOWN,                  // 31E4..31EF
    Character::UnicodeScript::KATAKANA,                 // 31F0..31FF
    Character::UnicodeScript::HANGUL,                   // 3200..321E
    Character::UnicodeScript::UNKNOWN,                  // 321F
    Character::UnicodeScript::COMMON,                   // 3220..325F
    Character::UnicodeScript::HANGUL,                   // 3260..327E
    Character::UnicodeScript::COMMON,                   // 327F..32CF
    Character::UnicodeScript::KATAKANA,                 // 32D0..32FE
    Character::UnicodeScript::COMMON,                   // 32FF
    Character::UnicodeScript::KATAKANA,                 // 3300..3357
    Character::UnicodeScript::COMMON,                   // 3358..33FF
    Character::UnicodeScript::HAN,                      // 3400..4DBF
    Character::UnicodeScript::COMMON,                   // 4DC0..4DFF
    Character::UnicodeScript::HAN,                      // 4E00..9FFC
    Character::UnicodeScript::UNKNOWN,                  // 9FFD..9FFF
    Character::UnicodeScript::YI,                       // A000..A48C
    Character::UnicodeScript::UNKNOWN,                  // A48D..A48F
    Character::UnicodeScript::YI,                       // A490..A4C6
    Character::UnicodeScript::UNKNOWN,                  // A4C7..A4CF
    Character::UnicodeScript::LISU,                     // A4D0..A4FF
    Character::UnicodeScript::VAI,                      // A500..A62B
    Character::UnicodeScript::UNKNOWN,                  // A62C..A63F
    Character::UnicodeScript::CYRILLIC,                 // A640..A69F
    Character::UnicodeScript::BAMUM,                    // A6A0..A6F7
    Character::UnicodeScript::UNKNOWN,                  // A6F8..A6FF
    Character::UnicodeScript::COMMON,                   // A700..A721
    Character::UnicodeScript::LATIN,                    // A722..A787
    Character::UnicodeScript::COMMON,                   // A788..A78A
    Character::UnicodeScript::LATIN,                    // A78B..A7BF
    Character::UnicodeScript::UNKNOWN,                  // A7C0..A7C1
    Character::UnicodeScript::LATIN,                    // A7C2..A7CA
    Character::UnicodeScript::UNKNOWN,                  // A7CB..A7F4
    Character::UnicodeScript::LATIN,                    // A7F5..A7FF
    Character::UnicodeScript::SYLOTI_NAGRI,             // A800..A82C
    Character::UnicodeScript::UNKNOWN,                  // A82D..A82F
    Character::UnicodeScript::COMMON,                   // A830..A839
    Character::UnicodeScript::UNKNOWN,                  // A83A..A83F
    Character::UnicodeScript::PHAGS_PA,                 // A840..A877
    Character::UnicodeScript::UNKNOWN,                  // A878..A87F
    Character::UnicodeScript::SAURASHTRA,               // A880..A8C5
    Character::UnicodeScript::UNKNOWN,                  // A8C6..A8CD
    Character::UnicodeScript::SAURASHTRA,               // A8CE..A8D9
    Character::UnicodeScript::UNKNOWN,                  // A8DA..A8DF
    Character::UnicodeScript::DEVANAGARI,               // A8E0..A8FF
    Character::UnicodeScript::KAYAH_LI,                 // A900..A92D
    Character::UnicodeScript::COMMON,                   // A92E
    Character::UnicodeScript::KAYAH_LI,                 // A92F
    Character::UnicodeScript::REJANG,                   // A930..A953
    Character::UnicodeScript::UNKNOWN,                  // A954..A95E
    Character::UnicodeScript::REJANG,                   // A95F
    Character::UnicodeScript::HANGUL,                   // A960..A97C
    Character::UnicodeScript::UNKNOWN,                  // A97D..A97F
    Character::UnicodeScript::JAVANESE,                 // A980..A9CD
    Character::UnicodeScript::UNKNOWN,                  // A9CE
    Character::UnicodeScript::COMMON,                   // A9CF
    Character::UnicodeScript::JAVANESE,                 // A9D0..A9D9
    Character::UnicodeScript::UNKNOWN,                  // A9DA..A9DD
    Character::UnicodeScript::JAVANESE,                 // A9DE..A9DF
    Character::UnicodeScript::MYANMAR,                  // A9E0..A9FE
    Character::UnicodeScript::UNKNOWN,                  // A9FF
    Character::UnicodeScript::CHAM,                     // AA00..AA36
    Character::UnicodeScript::UNKNOWN,                  // AA37..AA3F
    Character::UnicodeScript::CHAM,                     // AA40..AA4D
    Character::UnicodeScript::UNKNOWN,                  // AA4E..AA4F
    Character::UnicodeScript::CHAM,                     // AA50..AA59
    Character::UnicodeScript::UNKNOWN,                  // AA5A..AA5B
    Character::UnicodeScript::CHAM,                     // AA5C..AA5F
    Character::UnicodeScript::MYANMAR,                  // AA60..AA7F
    Character::UnicodeScript::TAI_VIET,                 // AA80..AAC2
    Character::UnicodeScript::UNKNOWN,                  // AAC3..AADA
    Character::UnicodeScript::TAI_VIET,                 // AADB..AADF
    Character::UnicodeScript::MEETEI_MAYEK,             // AAE0..AAF6
    Character::UnicodeScript::UNKNOWN,                  // AAF7..AB00
    Character::UnicodeScript::ETHIOPIC,                 // AB01..AB06
    Character::UnicodeScript::UNKNOWN,                  // AB07..AB08
    Character::UnicodeScript::ETHIOPIC,                 // AB09..AB0E
    Character::UnicodeScript::UNKNOWN,                  // AB0F..AB10
    Character::UnicodeScript::ETHIOPIC,                 // AB11..AB16
    Character::UnicodeScript::UNKNOWN,                  // AB17..AB1F
    Character::UnicodeScript::ETHIOPIC,                 // AB20..AB26
    Character::UnicodeScript::UNKNOWN,                  // AB27
    Character::UnicodeScript::ETHIOPIC,                 // AB28..AB2E
    Character::UnicodeScript::UNKNOWN,                  // AB2F
    Character::UnicodeScript::LATIN,                    // AB30..AB5A
    Character::UnicodeScript::COMMON,                   // AB5B
    Character::UnicodeScript::LATIN,                    // AB5C..AB64
    Character::UnicodeScript::GREEK,                    // AB65
    Character::UnicodeScript::LATIN,                    // AB66..AB69
    Character::UnicodeScript::COMMON,                   // AB6A..AB6B
    Character::UnicodeScript::UNKNOWN,                  // AB6C..AB6F
    Character::UnicodeScript::CHEROKEE,                 // AB70..ABBF
    Character::UnicodeScript::MEETEI_MAYEK,             // ABC0..ABED
    Character::UnicodeScript::UNKNOWN,                  // ABEE..ABEF
    Character::UnicodeScript::MEETEI_MAYEK,             // ABF0..ABF9
    Character::UnicodeScript::UNKNOWN,                  // ABFA..ABFF
    Character::UnicodeScript::HANGUL,                   // AC00..D7A3
    Character::UnicodeScript::UNKNOWN,                  // D7A4..D7AF
    Character::UnicodeScript::HANGUL,                   // D7B0..D7C6
    Character::UnicodeScript::UNKNOWN,                  // D7C7..D7CA
    Character::UnicodeScript::HANGUL,                   // D7CB..D7FB
    Character::UnicodeScript::UNKNOWN,                  // D7FC..F8FF
    Character::UnicodeScript::HAN,                      // F900..FA6D
    Character::UnicodeScript::UNKNOWN,                  // FA6E..FA6F
    Character::UnicodeScript::HAN,                      // FA70..FAD9
    Character::UnicodeScript::UNKNOWN,                  // FADA..FAFF
    Character::UnicodeScript::LATIN,                    // FB00..FB06
    Character::UnicodeScript::UNKNOWN,                  // FB07..FB12
    Character::UnicodeScript::ARMENIAN,                 // FB13..FB17
    Character::UnicodeScript::UNKNOWN,                  // FB18..FB1C
    Character::UnicodeScript::HEBREW,                   // FB1D..FB36
    Character::UnicodeScript::UNKNOWN,                  // FB37
    Character::UnicodeScript::HEBREW,                   // FB38..FB3C
    Character::UnicodeScript::UNKNOWN,                  // FB3D
    Character::UnicodeScript::HEBREW,                   // FB3E
    Character::UnicodeScript::UNKNOWN,                  // FB3F
    Character::UnicodeScript::HEBREW,                   // FB40..FB41
    Character::UnicodeScript::UNKNOWN,                  // FB42
    Character::UnicodeScript::HEBREW,                   // FB43..FB44
    Character::UnicodeScript::UNKNOWN,                  // FB45
    Character::UnicodeScript::HEBREW,                   // FB46..FB4F
    Character::UnicodeScript::ARABIC,                   // FB50..FBC1
    Character::UnicodeScript::UNKNOWN,                  // FBC2..FBD2
    Character::UnicodeScript::ARABIC,                   // FBD3..FD3D
    Character::UnicodeScript::COMMON,                   // FD3E..FD3F
    Character::UnicodeScript::UNKNOWN,                  // FD40..FD4F
    Character::UnicodeScript::ARABIC,                   // FD50..FD8F
    Character::UnicodeScript::UNKNOWN,                  // FD90..FD91
    Character::UnicodeScript::ARABIC,                   // FD92..FDC7
    Character::UnicodeScript::UNKNOWN,                  // FDC8..FDEF
    Character::UnicodeScript::ARABIC,                   // FDF0..FDFD
    Character::UnicodeScript::UNKNOWN,                  // FDFE..FDFF
    Character::UnicodeScript::INHERITED,                // FE00..FE0F
    Character::UnicodeScript::COMMON,                   // FE10..FE19
    Character::UnicodeScript::UNKNOWN,                  // FE1A..FE1F
    Character::UnicodeScript::INHERITED,                // FE20..FE2D
    Character::UnicodeScript::CYRILLIC,                 // FE2E..FE2F
    Character::UnicodeScript::COMMON,                   // FE30..FE52
    Character::UnicodeScript::UNKNOWN,                  // FE53
    Character::UnicodeScript::COMMON,                   // FE54..FE66
    Character::UnicodeScript::UNKNOWN,                  // FE67
    Character::UnicodeScript::COMMON,                   // FE68..FE6B
    Character::UnicodeScript::UNKNOWN,                  // FE6C..FE6F
    Character::UnicodeScript::ARABIC,                   // FE70..FE74
    Character::UnicodeScript::UNKNOWN,                  // FE75
    Character::UnicodeScript::ARABIC,                   // FE76..FEFC
    Character::UnicodeScript::UNKNOWN,                  // FEFD..FEFE
    Character::UnicodeScript::COMMON,                   // FEFF
    Character::UnicodeScript::UNKNOWN,                  // FF00
    Character::UnicodeScript::COMMON,                   // FF01..FF20
    Character::UnicodeScript::LATIN,                    // FF21..FF3A
    Character::UnicodeScript::COMMON,                   // FF3B..FF40
    Character::UnicodeScript::LATIN,                    // FF41..FF5A
    Character::UnicodeScript::COMMON,                   // FF5B..FF65
    Character::UnicodeScript::KATAKANA,                 // FF66..FF6F
    Character::UnicodeScript::COMMON,                   // FF70
    Character::UnicodeScript::KATAKANA,                 // FF71..FF9D
    Character::UnicodeScript::COMMON,                   // FF9E..FF9F
    Character::UnicodeScript::HANGUL,                   // FFA0..FFBE
    Character::UnicodeScript::UNKNOWN,                  // FFBF..FFC1
    Character::UnicodeScript::HANGUL,                   // FFC2..FFC7
    Character::UnicodeScript::UNKNOWN,                  // FFC8..FFC9
    Character::UnicodeScript::HANGUL,                   // FFCA..FFCF
    Character::UnicodeScript::UNKNOWN,                  // FFD0..FFD1
    Character::UnicodeScript::HANGUL,                   // FFD2..FFD7
    Character::UnicodeScript::UNKNOWN,                  // FFD8..FFD9
    Character::UnicodeScript::HANGUL,                   // FFDA..FFDC
    Character::UnicodeScript::UNKNOWN,                  // FFDD..FFDF
    Character::UnicodeScript::COMMON,                   // FFE0..FFE6
    Character::UnicodeScript::UNKNOWN,                  // FFE7
    Character::UnicodeScript::COMMON,                   // FFE8..FFEE
    Character::UnicodeScript::UNKNOWN,                  // FFEF..FFF8
    Character::UnicodeScript::COMMON,                   // FFF9..FFFD
    Character::UnicodeScript::UNKNOWN,                  // FFFE..FFFF
    Character::UnicodeScript::LINEAR_B,                 // 10000..1000B
    Character::UnicodeScript::UNKNOWN,                  // 1000C
    Character::UnicodeScript::LINEAR_B,                 // 1000D..10026
    Character::UnicodeScript::UNKNOWN,                  // 10027
    Character::UnicodeScript::LINEAR_B,                 // 10028..1003A
    Character::UnicodeScript::UNKNOWN,                  // 1003B
    Character::UnicodeScript::LINEAR_B,                 // 1003C..1003D
    Character::UnicodeScript::UNKNOWN,                  // 1003E
    Character::UnicodeScript::LINEAR_B,                 // 1003F..1004D
    Character::UnicodeScript::UNKNOWN,                  // 1004E..1004F
    Character::UnicodeScript::LINEAR_B,                 // 10050..1005D
    Character::UnicodeScript::UNKNOWN,                  // 1005E..1007F
    Character::UnicodeScript::LINEAR_B,                 // 10080..100FA
    Character::UnicodeScript::UNKNOWN,                  // 100FB..100FF
    Character::UnicodeScript::COMMON,                   // 10100..10102
    Character::UnicodeScript::UNKNOWN,                  // 10103..10106
    Character::UnicodeScript::COMMON,                   // 10107..10133
    Character::UnicodeScript::UNKNOWN,                  // 10134..10136
    Character::UnicodeScript::COMMON,                   // 10137..1013F
    Character::UnicodeScript::GREEK,                    // 10140..1018E
    Character::UnicodeScript::UNKNOWN,                  // 1018F
    Character::UnicodeScript::COMMON,                   // 10190..1019C
    Character::UnicodeScript::UNKNOWN,                  // 1019D..1019F
    Character::UnicodeScript::GREEK,                    // 101A0
    Character::UnicodeScript::UNKNOWN,                  // 101A1..101CF
    Character::UnicodeScript::COMMON,                   // 101D0..101FC
    Character::UnicodeScript::INHERITED,                // 101FD
    Character::UnicodeScript::UNKNOWN,                  // 101FE..1027F
    Character::UnicodeScript::LYCIAN,                   // 10280..1029C
    Character::UnicodeScript::UNKNOWN,                  // 1029D..1029F
    Character::UnicodeScript::CARIAN,                   // 102A0..102D0
    Character::UnicodeScript::UNKNOWN,                  // 102D1..102DF
    Character::UnicodeScript::INHERITED,                // 102E0
    Character::UnicodeScript::COMMON,                   // 102E1..102FB
    Character::UnicodeScript::UNKNOWN,                  // 102FC..102FF
    Character::UnicodeScript::OLD_ITALIC,               // 10300..10323
    Character::UnicodeScript::UNKNOWN,                  // 10324..1032C
    Character::UnicodeScript::OLD_ITALIC,               // 1032D..1032F
    Character::UnicodeScript::GOTHIC,                   // 10330..1034A
    Character::UnicodeScript::UNKNOWN,                  // 1034B..1034F
    Character::UnicodeScript::OLD_PERMIC,               // 10350..1037A
    Character::UnicodeScript::UNKNOWN,                  // 1037B..1037F
    Character::UnicodeScript::UGARITIC,                 // 10380..1039D
    Character::UnicodeScript::UNKNOWN,                  // 1039E
    Character::UnicodeScript::UGARITIC,                 // 1039F
    Character::UnicodeScript::OLD_PERSIAN,              // 103A0..103C3
    Character::UnicodeScript::UNKNOWN,                  // 103C4..103C7
    Character::UnicodeScript::OLD_PERSIAN,              // 103C8..103D5
    Character::UnicodeScript::UNKNOWN,                  // 103D6..103FF
    Character::UnicodeScript::DESERET,                  // 10400..1044F
    Character::UnicodeScript::SHAVIAN,                  // 10450..1047F
    Character::UnicodeScript::OSMANYA,                  // 10480..1049D
    Character::UnicodeScript::UNKNOWN,                  // 1049E..1049F
    Character::UnicodeScript::OSMANYA,                  // 104A0..104A9
    Character::UnicodeScript::UNKNOWN,                  // 104AA..104AF
    Character::UnicodeScript::OSAGE,                    // 104B0..104D3
    Character::UnicodeScript::UNKNOWN,                  // 104D4..104D7
    Character::UnicodeScript::OSAGE,                    // 104D8..104FB
    Character::UnicodeScript::UNKNOWN,                  // 104FC..104FF
    Character::UnicodeScript::ELBASAN,                  // 10500..10527
    Character::UnicodeScript::UNKNOWN,                  // 10528..1052F
    Character::UnicodeScript::CAUCASIAN_ALBANIAN,       // 10530..10563
    Character::UnicodeScript::UNKNOWN,                  // 10564..1056E
    Character::UnicodeScript::CAUCASIAN_ALBANIAN,       // 1056F
    Character::UnicodeScript::UNKNOWN,                  // 10570..105FF
    Character::UnicodeScript::LINEAR_A,                 // 10600..10736
    Character::UnicodeScript::UNKNOWN,                  // 10737..1073F
    Character::UnicodeScript::LINEAR_A,                 // 10740..10755
    Character::UnicodeScript::UNKNOWN,                  // 10756..1075F
    Character::UnicodeScript::LINEAR_A,                 // 10760..10767
    Character::UnicodeScript::UNKNOWN,                  // 10768..107FF
    Character::UnicodeScript::CYPRIOT,                  // 10800..10805
    Character::UnicodeScript::UNKNOWN,                  // 10806..10807
    Character::UnicodeScript::CYPRIOT,                  // 10808
    Character::UnicodeScript::UNKNOWN,                  // 10809
    Character::UnicodeScript::CYPRIOT,                  // 1080A..10835
    Character::UnicodeScript::UNKNOWN,                  // 10836
    Character::UnicodeScript::CYPRIOT,                  // 10837..10838
    Character::UnicodeScript::UNKNOWN,                  // 10839..1083B
    Character::UnicodeScript::CYPRIOT,                  // 1083C
    Character::UnicodeScript::UNKNOWN,                  // 1083D..1083E
    Character::UnicodeScript::CYPRIOT,                  // 1083F
    Character::UnicodeScript::IMPERIAL_ARAMAIC,         // 10840..10855
    Character::UnicodeScript::UNKNOWN,                  // 10856
    Character::UnicodeScript::IMPERIAL_ARAMAIC,         // 10857..1085F
    Character::UnicodeScript::PALMYRENE,                // 10860..1087F
    Character::UnicodeScript::NABATAEAN,                // 10880..1089E
    Character::UnicodeScript::UNKNOWN,                  // 1089F..108A6
    Character::UnicodeScript::NABATAEAN,                // 108A7..108AF
    Character::UnicodeScript::UNKNOWN,                  // 108B0..108DF
    Character::UnicodeScript::HATRAN,                   // 108E0..108F2
    Character::UnicodeScript::UNKNOWN,                  // 108F3
    Character::UnicodeScript::HATRAN,                   // 108F4..108F5
    Character::UnicodeScript::UNKNOWN,                  // 108F6..108FA
    Character::UnicodeScript::HATRAN,                   // 108FB..108FF
    Character::UnicodeScript::PHOENICIAN,               // 10900..1091B
    Character::UnicodeScript::UNKNOWN,                  // 1091C..1091E
    Character::UnicodeScript::PHOENICIAN,               // 1091F
    Character::UnicodeScript::LYDIAN,                   // 10920..10939
    Character::UnicodeScript::UNKNOWN,                  // 1093A..1093E
    Character::UnicodeScript::LYDIAN,                   // 1093F
    Character::UnicodeScript::UNKNOWN,                  // 10940..1097F
    Character::UnicodeScript::MEROITIC_HIEROGLYPHS,     // 10980..1099F
    Character::UnicodeScript::MEROITIC_CURSIVE,         // 109A0..109B7
    Character::UnicodeScript::UNKNOWN,                  // 109B8..109BB
    Character::UnicodeScript::MEROITIC_CURSIVE,         // 109BC..109CF
    Character::UnicodeScript::UNKNOWN,                  // 109D0..109D1
    Character::UnicodeScript::MEROITIC_CURSIVE,         // 109D2..109FF
    Character::UnicodeScript::KHAROSHTHI,               // 10A00..10A03
    Character::UnicodeScript::UNKNOWN,                  // 10A04
    Character::UnicodeScript::KHAROSHTHI,               // 10A05..10A06
    Character::UnicodeScript::UNKNOWN,                  // 10A07..10A0B
    Character::UnicodeScript::KHAROSHTHI,               // 10A0C..10A13
    Character::UnicodeScript::UNKNOWN,                  // 10A14
    Character::UnicodeScript::KHAROSHTHI,               // 10A15..10A17
    Character::UnicodeScript::UNKNOWN,                  // 10A18
    Character::UnicodeScript::KHAROSHTHI,               // 10A19..10A35
    Character::UnicodeScript::UNKNOWN,                  // 10A36..10A37
    Character::UnicodeScript::KHAROSHTHI,               // 10A38..10A3A
    Character::UnicodeScript::UNKNOWN,                  // 10A3B..10A3E
    Character::UnicodeScript::KHAROSHTHI,               // 10A3F..10A48
    Character::UnicodeScript::UNKNOWN,                  // 10A49..10A4F
    Character::UnicodeScript::KHAROSHTHI,               // 10A50..10A58
    Character::UnicodeScript::UNKNOWN,                  // 10A59..10A5F
    Character::UnicodeScript::OLD_SOUTH_ARABIAN,        // 10A60..10A7F
    Character::UnicodeScript::OLD_NORTH_ARABIAN,        // 10A80..10A9F
    Character::UnicodeScript::UNKNOWN,                  // 10AA0..10ABF
    Character::UnicodeScript::MANICHAEAN,               // 10AC0..10AE6
    Character::UnicodeScript::UNKNOWN,                  // 10AE7..10AEA
    Character::UnicodeScript::MANICHAEAN,               // 10AEB..10AF6
    Character::UnicodeScript::UNKNOWN,                  // 10AF7..10AFF
    Character::UnicodeScript::AVESTAN,                  // 10B00..10B35
    Character::UnicodeScript::UNKNOWN,                  // 10B36..10B38
    Character::UnicodeScript::AVESTAN,                  // 10B39..10B3F
    Character::UnicodeScript::INSCRIPTIONAL_PARTHIAN,   // 10B40..10B55
    Character::UnicodeScript::UNKNOWN,                  // 10B56..10B57
    Character::UnicodeScript::INSCRIPTIONAL_PARTHIAN,   // 10B58..10B5F
    Character::UnicodeScript::INSCRIPTIONAL_PAHLAVI,    // 10B60..10B72
    Character::UnicodeScript::UNKNOWN,                  // 10B73..10B77
    Character::UnicodeScript::INSCRIPTIONAL_PAHLAVI,    // 10B78..10B7F
    Character::UnicodeScript::PSALTER_PAHLAVI,          // 10B80..10B91
    Character::UnicodeScript::UNKNOWN,                  // 10B92..10B98
    Character::UnicodeScript::PSALTER_PAHLAVI,          // 10B99..10B9C
    Character::UnicodeScript::UNKNOWN,                  // 10B9D..10BA8
    Character::UnicodeScript::PSALTER_PAHLAVI,          // 10BA9..10BAF
    Character::UnicodeScript::UNKNOWN,                  // 10BB0..10BFF
    Character::UnicodeScript::OLD_TURKIC,               // 10C00..10C48
    Character::UnicodeScript::UNKNOWN,                  // 10C49..10C7F
    Character::UnicodeScript::OLD_HUNGARIAN,            // 10C80..10CB2
    Character::UnicodeScript::UNKNOWN,                  // 10CB3..10CBF
    Character::UnicodeScript::OLD_HUNGARIAN,            // 10CC0..10CF2
    Character::UnicodeScript::UNKNOWN,                  // 10CF3..10CF9
    Character::UnicodeScript::OLD_HUNGARIAN,            // 10CFA..10CFF
    Character::UnicodeScript::HANIFI_ROHINGYA,          // 10D00..10D27
    Character::UnicodeScript::UNKNOWN,                  // 10D28..10D2F
    Character::UnicodeScript::HANIFI_ROHINGYA,          // 10D30..10D39
    Character::UnicodeScript::UNKNOWN,                  // 10D3A..10E5F
    Character::UnicodeScript::ARABIC,                   // 10E60..10E7E
    Character::UnicodeScript::UNKNOWN,                  // 10E7F
    Character::UnicodeScript::YEZIDI,                   // 10E80..10EA9
    Character::UnicodeScript::UNKNOWN,                  // 10EAA
    Character::UnicodeScript::YEZIDI,                   // 10EAB..10EAD
    Character::UnicodeScript::UNKNOWN,                  // 10EAE..10EAF
    Character::UnicodeScript::YEZIDI,                   // 10EB0..10EB1
    Character::UnicodeScript::UNKNOWN,                  // 10EB2..10EFF
    Character::UnicodeScript::OLD_SOGDIAN,              // 10F00..10F27
    Character::UnicodeScript::UNKNOWN,                  // 10F28..10F2F
    Character::UnicodeScript::SOGDIAN,                  // 10F30..10F59
    Character::UnicodeScript::UNKNOWN,                  // 10F5A..10FAF
    Character::UnicodeScript::CHORASMIAN,               // 10FB0..10FCB
    Character::UnicodeScript::UNKNOWN,                  // 10FCC..10FDF
    Character::UnicodeScript::ELYMAIC,                  // 10FE0..10FF6
    Character::UnicodeScript::UNKNOWN,                  // 10FF7..10FFF
    Character::UnicodeScript::BRAHMI,                   // 11000..1104D
    Character::UnicodeScript::UNKNOWN,                  // 1104E..11051
    Character::UnicodeScript::BRAHMI,                   // 11052..1106F
    Character::UnicodeScript::UNKNOWN,                  // 11070..1107E
    Character::UnicodeScript::BRAHMI,                   // 1107F
    Character::UnicodeScript::KAITHI,                   // 11080..110C1
    Character::UnicodeScript::UNKNOWN,                  // 110C2..110CC
    Character::UnicodeScript::KAITHI,                   // 110CD
    Character::UnicodeScript::UNKNOWN,                  // 110CE..110CF
    Character::UnicodeScript::SORA_SOMPENG,             // 110D0..110E8
    Character::UnicodeScript::UNKNOWN,                  // 110E9..110EF
    Character::UnicodeScript::SORA_SOMPENG,             // 110F0..110F9
    Character::UnicodeScript::UNKNOWN,                  // 110FA..110FF
    Character::UnicodeScript::CHAKMA,                   // 11100..11134
    Character::UnicodeScript::UNKNOWN,                  // 11135
    Character::UnicodeScript::CHAKMA,                   // 11136..11147
    Character::UnicodeScript::UNKNOWN,                  // 11148..1114F
    Character::UnicodeScript::MAHAJANI,                 // 11150..11176
    Character::UnicodeScript::UNKNOWN,                  // 11177..1117F
    Character::UnicodeScript::SHARADA,                  // 11180..111DF
    Character::UnicodeScript::UNKNOWN,                  // 111E0
    Character::UnicodeScript::SINHALA,                  // 111E1..111F4
    Character::UnicodeScript::UNKNOWN,                  // 111F5..111FF
    Character::UnicodeScript::KHOJKI,                   // 11200..11211
    Character::UnicodeScript::UNKNOWN,                  // 11212
    Character::UnicodeScript::KHOJKI,                   // 11213..1123E
    Character::UnicodeScript::UNKNOWN,                  // 1123F..1127F
    Character::UnicodeScript::MULTANI,                  // 11280..11286
    Character::UnicodeScript::UNKNOWN,                  // 11287
    Character::UnicodeScript::MULTANI,                  // 11288
    Character::UnicodeScript::UNKNOWN,                  // 11289
    Character::UnicodeScript::MULTANI,                  // 1128A..1128D
    Character::UnicodeScript::UNKNOWN,                  // 1128E
    Character::UnicodeScript::MULTANI,                  // 1128F..1129D
    Character::UnicodeScript::UNKNOWN,                  // 1129E
    Character::UnicodeScript::MULTANI,                  // 1129F..112A9
    Character::UnicodeScript::UNKNOWN,                  // 112AA..112AF
    Character::UnicodeScript::KHUDAWADI,                // 112B0..112EA
    Character::UnicodeScript::UNKNOWN,                  // 112EB..112EF
    Character::UnicodeScript::KHUDAWADI,                // 112F0..112F9
    Character::UnicodeScript::UNKNOWN,                  // 112FA..112FF
    Character::UnicodeScript::GRANTHA,                  // 11300..11303
    Character::UnicodeScript::UNKNOWN,                  // 11304
    Character::UnicodeScript::GRANTHA,                  // 11305..1130C
    Character::UnicodeScript::UNKNOWN,                  // 1130D..1130E
    Character::UnicodeScript::GRANTHA,                  // 1130F..11310
    Character::UnicodeScript::UNKNOWN,                  // 11311..11312
    Character::UnicodeScript::GRANTHA,                  // 11313..11328
    Character::UnicodeScript::UNKNOWN,                  // 11329
    Character::UnicodeScript::GRANTHA,                  // 1132A..11330
    Character::UnicodeScript::UNKNOWN,                  // 11331
    Character::UnicodeScript::GRANTHA,                  // 11332..11333
    Character::UnicodeScript::UNKNOWN,                  // 11334
    Character::UnicodeScript::GRANTHA,                  // 11335..11339
    Character::UnicodeScript::UNKNOWN,                  // 1133A
    Character::UnicodeScript::INHERITED,                // 1133B
    Character::UnicodeScript::GRANTHA,                  // 1133C..11344
    Character::UnicodeScript::UNKNOWN,                  // 11345..11346
    Character::UnicodeScript::GRANTHA,                  // 11347..11348
    Character::UnicodeScript::UNKNOWN,                  // 11349..1134A
    Character::UnicodeScript::GRANTHA,                  // 1134B..1134D
    Character::UnicodeScript::UNKNOWN,                  // 1134E..1134F
    Character::UnicodeScript::GRANTHA,                  // 11350
    Character::UnicodeScript::UNKNOWN,                  // 11351..11356
    Character::UnicodeScript::GRANTHA,                  // 11357
    Character::UnicodeScript::UNKNOWN,                  // 11358..1135C
    Character::UnicodeScript::GRANTHA,                  // 1135D..11363
    Character::UnicodeScript::UNKNOWN,                  // 11364..11365
    Character::UnicodeScript::GRANTHA,                  // 11366..1136C
    Character::UnicodeScript::UNKNOWN,                  // 1136D..1136F
    Character::UnicodeScript::GRANTHA,                  // 11370..11374
    Character::UnicodeScript::UNKNOWN,                  // 11375..113FF
    Character::UnicodeScript::NEWA,                     // 11400..1145B
    Character::UnicodeScript::UNKNOWN,                  // 1145C
    Character::UnicodeScript::NEWA,                     // 1145D..11461
    Character::UnicodeScript::UNKNOWN,                  // 11462..1147F
    Character::UnicodeScript::TIRHUTA,                  // 11480..114C7
    Character::UnicodeScript::UNKNOWN,                  // 114C8..114CF
    Character::UnicodeScript::TIRHUTA,                  // 114D0..114D9
    Character::UnicodeScript::UNKNOWN,                  // 114DA..1157F
    Character::UnicodeScript::SIDDHAM,                  // 11580..115B5
    Character::UnicodeScript::UNKNOWN,                  // 115B6..115B7
    Character::UnicodeScript::SIDDHAM,                  // 115B8..115DD
    Character::UnicodeScript::UNKNOWN,                  // 115DE..115FF
    Character::UnicodeScript::MODI,                     // 11600..11644
    Character::UnicodeScript::UNKNOWN,                  // 11645..1164F
    Character::UnicodeScript::MODI,                     // 11650..11659
    Character::UnicodeScript::UNKNOWN,                  // 1165A..1165F
    Character::UnicodeScript::MONGOLIAN,                // 11660..1166C
    Character::UnicodeScript::UNKNOWN,                  // 1166D..1167F
    Character::UnicodeScript::TAKRI,                    // 11680..116B8
    Character::UnicodeScript::UNKNOWN,                  // 116B9..116BF
    Character::UnicodeScript::TAKRI,                    // 116C0..116C9
    Character::UnicodeScript::UNKNOWN,                  // 116CA..116FF
    Character::UnicodeScript::AHOM,                     // 11700..1171A
    Character::UnicodeScript::UNKNOWN,                  // 1171B..1171C
    Character::UnicodeScript::AHOM,                     // 1171D..1172B
    Character::UnicodeScript::UNKNOWN,                  // 1172C..1172F
    Character::UnicodeScript::AHOM,                     // 11730..1173F
    Character::UnicodeScript::UNKNOWN,                  // 11740..117FF
    Character::UnicodeScript::DOGRA,                    // 11800..1183B
    Character::UnicodeScript::UNKNOWN,                  // 1183C..1189F
    Character::UnicodeScript::WARANG_CITI,              // 118A0..118F2
    Character::UnicodeScript::UNKNOWN,                  // 118F3..118FE
    Character::UnicodeScript::WARANG_CITI,              // 118FF
    Character::UnicodeScript::DIVES_AKURU,              // 11900..11906
    Character::UnicodeScript::UNKNOWN,                  // 11907..11908
    Character::UnicodeScript::DIVES_AKURU,              // 11909
    Character::UnicodeScript::UNKNOWN,                  // 1190A..1190B
    Character::UnicodeScript::DIVES_AKURU,              // 1190C..11913
    Character::UnicodeScript::UNKNOWN,                  // 11914
    Character::UnicodeScript::DIVES_AKURU,              // 11915..11916
    Character::UnicodeScript::UNKNOWN,                  // 11917
    Character::UnicodeScript::DIVES_AKURU,              // 11918..11935
    Character::UnicodeScript::UNKNOWN,                  // 11936
    Character::UnicodeScript::DIVES_AKURU,              // 11937..11938
    Character::UnicodeScript::UNKNOWN,                  // 11939..1193A
    Character::UnicodeScript::DIVES_AKURU,              // 1193B..11946
    Character::UnicodeScript::UNKNOWN,                  // 11947..1194F
    Character::UnicodeScript::DIVES_AKURU,              // 11950..11959
    Character::UnicodeScript::UNKNOWN,                  // 1195A..1199F
    Character::UnicodeScript::NANDINAGARI,              // 119A0..119A7
    Character::UnicodeScript::UNKNOWN,                  // 119A8..119A9
    Character::UnicodeScript::NANDINAGARI,              // 119AA..119D7
    Character::UnicodeScript::UNKNOWN,                  // 119D8..119D9
    Character::UnicodeScript::NANDINAGARI,              // 119DA..119E4
    Character::UnicodeScript::UNKNOWN,                  // 119E5..119FF
    Character::UnicodeScript::ZANABAZAR_SQUARE,         // 11A00..11A47
    Character::UnicodeScript::UNKNOWN,                  // 11A48..11A4F
    Character::UnicodeScript::SOYOMBO,                  // 11A50..11AA2
    Character::UnicodeScript::UNKNOWN,                  // 11AA3..11ABF
    Character::UnicodeScript::PAU_CIN_HAU,              // 11AC0..11AF8
    Character::UnicodeScript::UNKNOWN,                  // 11AF9..11BFF
    Character::UnicodeScript::BHAIKSUKI,                // 11C00..11C08
    Character::UnicodeScript::UNKNOWN,                  // 11C09
    Character::UnicodeScript::BHAIKSUKI,                // 11C0A..11C36
    Character::UnicodeScript::UNKNOWN,                  // 11C37
    Character::UnicodeScript::BHAIKSUKI,                // 11C38..11C45
    Character::UnicodeScript::UNKNOWN,                  // 11C46..11C4F
    Character::UnicodeScript::BHAIKSUKI,                // 11C50..11C6C
    Character::UnicodeScript::UNKNOWN,                  // 11C6D..11C6F
    Character::UnicodeScript::MARCHEN,                  // 11C70..11C8F
    Character::UnicodeScript::UNKNOWN,                  // 11C90..11C91
    Character::UnicodeScript::MARCHEN,                  // 11C92..11CA7
    Character::UnicodeScript::UNKNOWN,                  // 11CA8
    Character::UnicodeScript::MARCHEN,                  // 11CA9..11CB6
    Character::UnicodeScript::UNKNOWN,                  // 11CB7..11CFF
    Character::UnicodeScript::MASARAM_GONDI,            // 11D00..11D06
    Character::UnicodeScript::UNKNOWN,                  // 11D07
    Character::UnicodeScript::MASARAM_GONDI,            // 11D08..11D09
    Character::UnicodeScript::UNKNOWN,                  // 11D0A
    Character::UnicodeScript::MASARAM_GONDI,            // 11D0B..11D36
    Character::UnicodeScript::UNKNOWN,                  // 11D37..11D39
    Character::UnicodeScript::MASARAM_GONDI,            // 11D3A
    Character::UnicodeScript::UNKNOWN,                  // 11D3B
    Character::UnicodeScript::MASARAM_GONDI,            // 11D3C..11D3D
    Character::UnicodeScript::UNKNOWN,                  // 11D3E
    Character::UnicodeScript::MASARAM_GONDI,            // 11D3F..11D47
    Character::UnicodeScript::UNKNOWN,                  // 11D48..11D4F
    Character::UnicodeScript::MASARAM_GONDI,            // 11D50..11D59
    Character::UnicodeScript::UNKNOWN,                  // 11D5A..11D5F
    Character::UnicodeScript::GUNJALA_GONDI,            // 11D60..11D65
    Character::UnicodeScript::UNKNOWN,                  // 11D66
    Character::UnicodeScript::GUNJALA_GONDI,            // 11D67..11D68
    Character::UnicodeScript::UNKNOWN,                  // 11D69
    Character::UnicodeScript::GUNJALA_GONDI,            // 11D6A..11D8E
    Character::UnicodeScript::UNKNOWN,                  // 11D8F
    Character::UnicodeScript::GUNJALA_GONDI,            // 11D90..11D91
    Character::UnicodeScript::UNKNOWN,                  // 11D92
    Character::UnicodeScript::GUNJALA_GONDI,            // 11D93..11D98
    Character::UnicodeScript::UNKNOWN,                  // 11D99..11D9F
    Character::UnicodeScript::GUNJALA_GONDI,            // 11DA0..11DA9
    Character::UnicodeScript::UNKNOWN,                  // 11DAA..11EDF
    Character::UnicodeScript::MAKASAR,                  // 11EE0..11EF8
    Character::UnicodeScript::UNKNOWN,                  // 11EF9..11FAF
    Character::UnicodeScript::LISU,                     // 11FB0
    Character::UnicodeScript::UNKNOWN,                  // 11FB1..11FBF
    Character::UnicodeScript::TAMIL,                    // 11FC0..11FF1
    Character::UnicodeScript::UNKNOWN,                  // 11FF2..11FFE
    Character::UnicodeScript::TAMIL,                    // 11FFF
    Character::UnicodeScript::CUNEIFORM,                // 12000..12399
    Character::UnicodeScript::UNKNOWN,                  // 1239A..123FF
    Character::UnicodeScript::CUNEIFORM,                // 12400..1246E
    Character::UnicodeScript::UNKNOWN,                  // 1246F
    Character::UnicodeScript::CUNEIFORM,                // 12470..12474
    Character::UnicodeScript::UNKNOWN,                  // 12475..1247F
    Character::UnicodeScript::CUNEIFORM,                // 12480..12543
    Character::UnicodeScript::UNKNOWN,                  // 12544..12FFF
    Character::UnicodeScript::EGYPTIAN_HIEROGLYPHS,     // 13000..1342E
    Character::UnicodeScript::UNKNOWN,                  // 1342F
    Character::UnicodeScript::EGYPTIAN_HIEROGLYPHS,     // 13430..13438
    Character::UnicodeScript::UNKNOWN,                  // 13439..143FF
    Character::UnicodeScript::ANATOLIAN_HIEROGLYPHS,    // 14400..14646
    Character::UnicodeScript::UNKNOWN,                  // 14647..167FF
    Character::UnicodeScript::BAMUM,                    // 16800..16A38
    Character::UnicodeScript::UNKNOWN,                  // 16A39..16A3F
    Character::UnicodeScript::MRO,                      // 16A40..16A5E
    Character::UnicodeScript::UNKNOWN,                  // 16A5F
    Character::UnicodeScript::MRO,                      // 16A60..16A69
    Character::UnicodeScript::UNKNOWN,                  // 16A6A..16A6D
    Character::UnicodeScript::MRO,                      // 16A6E..16A6F
    Character::UnicodeScript::UNKNOWN,                  // 16A70..16ACF
    Character::UnicodeScript::BASSA_VAH,                // 16AD0..16AED
    Character::UnicodeScript::UNKNOWN,                  // 16AEE..16AEF
    Character::UnicodeScript::BASSA_VAH,                // 16AF0..16AF5
    Character::UnicodeScript::UNKNOWN,                  // 16AF6..16AFF
    Character::UnicodeScript::PAHAWH_HMONG,             // 16B00..16B45
    Character::UnicodeScript::UNKNOWN,                  // 16B46..16B4F
    Character::UnicodeScript::PAHAWH_HMONG,             // 16B50..16B59
    Character::UnicodeScript::UNKNOWN,                  // 16B5A
    Character::UnicodeScript::PAHAWH_HMONG,             // 16B5B..16B61
    Character::UnicodeScript::UNKNOWN,                  // 16B62
    Character::UnicodeScript::PAHAWH_HMONG,             // 16B63..16B77
    Character::UnicodeScript::UNKNOWN,                  // 16B78..16B7C
    Character::UnicodeScript::PAHAWH_HMONG,             // 16B7D..16B8F
    Character::UnicodeScript::UNKNOWN,                  // 16B90..16E3F
    Character::UnicodeScript::MEDEFAIDRIN,              // 16E40..16E9A
    Character::UnicodeScript::UNKNOWN,                  // 16E9B..16EFF
    Character::UnicodeScript::MIAO,                     // 16F00..16F4A
    Character::UnicodeScript::UNKNOWN,                  // 16F4B..16F4E
    Character::UnicodeScript::MIAO,                     // 16F4F..16F87
    Character::UnicodeScript::UNKNOWN,                  // 16F88..16F8E
    Character::UnicodeScript::MIAO,                     // 16F8F..16F9F
    Character::UnicodeScript::UNKNOWN,                  // 16FA0..16FDF
    Character::UnicodeScript::TANGUT,                   // 16FE0
    Character::UnicodeScript::NUSHU,                    // 16FE1
    Character::UnicodeScript::COMMON,                   // 16FE2..16FE3
    Character::UnicodeScript::KHITAN_SMALL_SCRIPT,      // 16FE4
    Character::UnicodeScript::UNKNOWN,                  // 16FE5..16FEF
    Character::UnicodeScript::HAN,                      // 16FF0..16FF1
    Character::UnicodeScript::UNKNOWN,                  // 16FF2..16FFF
    Character::UnicodeScript::TANGUT,                   // 17000..187F7
    Character::UnicodeScript::UNKNOWN,                  // 187F8..187FF
    Character::UnicodeScript::TANGUT,                   // 18800..18AFF
    Character::UnicodeScript::KHITAN_SMALL_SCRIPT,      // 18B00..18CD5
    Character::UnicodeScript::UNKNOWN,                  // 18CD6..18CFF
    Character::UnicodeScript::TANGUT,                   // 18D00..18D08
    Character::UnicodeScript::UNKNOWN,                  // 18D09..1AFFF
    Character::UnicodeScript::KATAKANA,                 // 1B000
    Character::UnicodeScript::HIRAGANA,                 // 1B001..1B11E
    Character::UnicodeScript::UNKNOWN,                  // 1B11F..1B14F
    Character::UnicodeScript::HIRAGANA,                 // 1B150..1B152
    Character::UnicodeScript::UNKNOWN,                  // 1B153..1B163
    Character::UnicodeScript::KATAKANA,                 // 1B164..1B167
    Character::UnicodeScript::UNKNOWN,                  // 1B168..1B16F
    Character::UnicodeScript::NUSHU,                    // 1B170..1B2FB
    Character::UnicodeScript::UNKNOWN,                  // 1B2FC..1BBFF
    Character::UnicodeScript::DUPLOYAN,                 // 1BC00..1BC6A
    Character::UnicodeScript::UNKNOWN,                  // 1BC6B..1BC6F
    Character::UnicodeScript::DUPLOYAN,                 // 1BC70..1BC7C
    Character::UnicodeScript::UNKNOWN,                  // 1BC7D..1BC7F
    Character::UnicodeScript::DUPLOYAN,                 // 1BC80..1BC88
    Character::UnicodeScript::UNKNOWN,                  // 1BC89..1BC8F
    Character::UnicodeScript::DUPLOYAN,                 // 1BC90..1BC99
    Character::UnicodeScript::UNKNOWN,                  // 1BC9A..1BC9B
    Character::UnicodeScript::DUPLOYAN,                 // 1BC9C..1BC9F
    Character::UnicodeScript::COMMON,                   // 1BCA0..1BCA3
    Character::UnicodeScript::UNKNOWN,                  // 1BCA4..1CFFF
    Character::UnicodeScript::COMMON,                   // 1D000..1D0F5
    Character::UnicodeScript::UNKNOWN,                  // 1D0F6..1D0FF
    Character::UnicodeScript::COMMON,                   // 1D100..1D126
    Character::UnicodeScript::UNKNOWN,                  // 1D127..1D128
    Character::UnicodeScript::COMMON,                   // 1D129..1D166
    Character::UnicodeScript::INHERITED,                // 1D167..1D169
    Character::UnicodeScript::COMMON,                   // 1D16A..1D17A
    Character::UnicodeScript::INHERITED,                // 1D17B..1D182
    Character::UnicodeScript::COMMON,                   // 1D183..1D184
    Character::UnicodeScript::INHERITED,                // 1D185..1D18B
    Character::UnicodeScript::COMMON,                   // 1D18C..1D1A9
    Character::UnicodeScript::INHERITED,                // 1D1AA..1D1AD
    Character::UnicodeScript::COMMON,                   // 1D1AE..1D1E8
    Character::UnicodeScript::UNKNOWN,                  // 1D1E9..1D1FF
    Character::UnicodeScript::GREEK,                    // 1D200..1D245
    Character::UnicodeScript::UNKNOWN,                  // 1D246..1D2DF
    Character::UnicodeScript::COMMON,                   // 1D2E0..1D2F3
    Character::UnicodeScript::UNKNOWN,                  // 1D2F4..1D2FF
    Character::UnicodeScript::COMMON,                   // 1D300..1D356
    Character::UnicodeScript::UNKNOWN,                  // 1D357..1D35F
    Character::UnicodeScript::COMMON,                   // 1D360..1D378
    Character::UnicodeScript::UNKNOWN,                  // 1D379..1D3FF
    Character::UnicodeScript::COMMON,                   // 1D400..1D454
    Character::UnicodeScript::UNKNOWN,                  // 1D455
    Character::UnicodeScript::COMMON,                   // 1D456..1D49C
    Character::UnicodeScript::UNKNOWN,                  // 1D49D
    Character::UnicodeScript::COMMON,                   // 1D49E..1D49F
    Character::UnicodeScript::UNKNOWN,                  // 1D4A0..1D4A1
    Character::UnicodeScript::COMMON,                   // 1D4A2
    Character::UnicodeScript::UNKNOWN,                  // 1D4A3..1D4A4
    Character::UnicodeScript::COMMON,                   // 1D4A5..1D4A6
    Character::UnicodeScript::UNKNOWN,                  // 1D4A7..1D4A8
    Character::UnicodeScript::COMMON,                   // 1D4A9..1D4AC
    Character::UnicodeScript::UNKNOWN,                  // 1D4AD
    Character::UnicodeScript::COMMON,                   // 1D4AE..1D4B9
    Character::UnicodeScript::UNKNOWN,                  // 1D4BA
    Character::UnicodeScript::COMMON,                   // 1D4BB
    Character::UnicodeScript::UNKNOWN,                  // 1D4BC
    Character::UnicodeScript::COMMON,                   // 1D4BD..1D4C3
    Character::UnicodeScript::UNKNOWN,                  // 1D4C4
    Character::UnicodeScript::COMMON,                   // 1D4C5..1D505
    Character::UnicodeScript::UNKNOWN,                  // 1D506
    Character::UnicodeScript::COMMON,                   // 1D507..1D50A
    Character::UnicodeScript::UNKNOWN,                  // 1D50B..1D50C
    Character::UnicodeScript::COMMON,                   // 1D50D..1D514
    Character::UnicodeScript::UNKNOWN,                  // 1D515
    Character::UnicodeScript::COMMON,                   // 1D516..1D51C
    Character::UnicodeScript::UNKNOWN,                  // 1D51D
    Character::UnicodeScript::COMMON,                   // 1D51E..1D539
    Character::UnicodeScript::UNKNOWN,                  // 1D53A
    Character::UnicodeScript::COMMON,                   // 1D53B..1D53E
    Character::UnicodeScript::UNKNOWN,                  // 1D53F
    Character::UnicodeScript::COMMON,                   // 1D540..1D544
    Character::UnicodeScript::UNKNOWN,                  // 1D545
    Character::UnicodeScript::COMMON,                   // 1D546
    Character::UnicodeScript::UNKNOWN,                  // 1D547..1D549
    Character::UnicodeScript::COMMON,                   // 1D54A..1D550
    Character::UnicodeScript::UNKNOWN,                  // 1D551
    Character::UnicodeScript::COMMON,                   // 1D552..1D6A5
    Character::UnicodeScript::UNKNOWN,                  // 1D6A6..1D6A7
    Character::UnicodeScript::COMMON,                   // 1D6A8..1D7CB
    Character::UnicodeScript::UNKNOWN,                  // 1D7CC..1D7CD
    Character::UnicodeScript::COMMON,                   // 1D7CE..1D7FF
    Character::UnicodeScript::SIGNWRITING,              // 1D800..1DA8B
    Character::UnicodeScript::UNKNOWN,                  // 1DA8C..1DA9A
    Character::UnicodeScript::SIGNWRITING,              // 1DA9B..1DA9F
    Character::UnicodeScript::UNKNOWN,                  // 1DAA0
    Character::UnicodeScript::SIGNWRITING,              // 1DAA1..1DAAF
    Character::UnicodeScript::UNKNOWN,                  // 1DAB0..1DFFF
    Character::UnicodeScript::GLAGOLITIC,               // 1E000..1E006
    Character::UnicodeScript::UNKNOWN,                  // 1E007
    Character::UnicodeScript::GLAGOLITIC,               // 1E008..1E018
    Character::UnicodeScript::UNKNOWN,                  // 1E019..1E01A
    Character::UnicodeScript::GLAGOLITIC,               // 1E01B..1E021
    Character::UnicodeScript::UNKNOWN,                  // 1E022
    Character::UnicodeScript::GLAGOLITIC,               // 1E023..1E024
    Character::UnicodeScript::UNKNOWN,                  // 1E025
    Character::UnicodeScript::GLAGOLITIC,               // 1E026..1E02A
    Character::UnicodeScript::UNKNOWN,                  // 1E02B..1E0FF
    Character::UnicodeScript::NYIAKENG_PUACHUE_HMONG,   // 1E100..1E12C
    Character::UnicodeScript::UNKNOWN,                  // 1E12D..1E12F
    Character::UnicodeScript::NYIAKENG_PUACHUE_HMONG,   // 1E130..1E13D
    Character::UnicodeScript::UNKNOWN,                  // 1E13E..1E13F
    Character::UnicodeScript::NYIAKENG_PUACHUE_HMONG,   // 1E140..1E149
    Character::UnicodeScript::UNKNOWN,                  // 1E14A..1E14D
    Character::UnicodeScript::NYIAKENG_PUACHUE_HMONG,   // 1E14E..1E14F
    Character::UnicodeScript::UNKNOWN,                  // 1E150..1E2BF
    Character::UnicodeScript::WANCHO,                   // 1E2C0..1E2F9
    Character::UnicodeScript::UNKNOWN,                  // 1E2FA..1E2FE
    Character::UnicodeScript::WANCHO,                   // 1E2FF
    Character::UnicodeScript::UNKNOWN,                  // 1E300..1E7FF
    Character::UnicodeScript::MENDE_KIKAKUI,            // 1E800..1E8C4
    Character::UnicodeScript::UNKNOWN,                  // 1E8C5..1E8C6
    Character::UnicodeScript::MENDE_KIKAKUI,            // 1E8C7..1E8D6
    Character::UnicodeScript::UNKNOWN,                  // 1E8D7..1E8FF
    Character::UnicodeScript::ADLAM,                    // 1E900..1E94B
    Character::UnicodeScript::UNKNOWN,                  // 1E94C..1E94F
    Character::UnicodeScript::ADLAM,                    // 1E950..1E959
    Character::UnicodeScript::UNKNOWN,                  // 1E95A..1E95D
    Character::UnicodeScript::ADLAM,                    // 1E95E..1E95F
    Character::UnicodeScript::UNKNOWN,                  // 1E960..1EC70
    Character::UnicodeScript::COMMON,                   // 1EC71..1ECB4
    Character::UnicodeScript::UNKNOWN,                  // 1ECB5..1ED00
    Character::UnicodeScript::COMMON,                   // 1ED01..1ED3D
    Character::UnicodeScript::UNKNOWN,                  // 1ED3E..1EDFF
    Character::UnicodeScript::ARABIC,                   // 1EE00..1EE03
    Character::UnicodeScript::UNKNOWN,                  // 1EE04
    Character::UnicodeScript::ARABIC,                   // 1EE05..1EE1F
    Character::UnicodeScript::UNKNOWN,                  // 1EE20
    Character::UnicodeScript::ARABIC,                   // 1EE21..1EE22
    Character::UnicodeScript::UNKNOWN,                  // 1EE23
    Character::UnicodeScript::ARABIC,                   // 1EE24
    Character::UnicodeScript::UNKNOWN,                  // 1EE25..1EE26
    Character::UnicodeScript::ARABIC,                   // 1EE27
    Character::UnicodeScript::UNKNOWN,                  // 1EE28
    Character::UnicodeScript::ARABIC,                   // 1EE29..1EE32
    Character::UnicodeScript::UNKNOWN,                  // 1EE33
    Character::UnicodeScript::ARABIC,                   // 1EE34..1EE37
    Character::UnicodeScript::UNKNOWN,                  // 1EE38
    Character::UnicodeScript::ARABIC,                   // 1EE39
    Character::UnicodeScript::UNKNOWN,                  // 1EE3A
    Character::UnicodeScript::ARABIC,                   // 1EE3B
    Character::UnicodeScript::UNKNOWN,                  // 1EE3C..1EE41
    Character::UnicodeScript::ARABIC,                   // 1EE42
    Character::UnicodeScript::UNKNOWN,                  // 1EE43..1EE46
    Character::UnicodeScript::ARABIC,                   // 1EE47
    Character::UnicodeScript::UNKNOWN,                  // 1EE48
    Character::UnicodeScript::ARABIC,                   // 1EE49
    Character::UnicodeScript::UNKNOWN,                  // 1EE4A
    Character::UnicodeScript::ARABIC,                   // 1EE4B
    Character::UnicodeScript::UNKNOWN,                  // 1EE4C
    Character::UnicodeScript::ARABIC,                   // 1EE4D..1EE4F
    Character::UnicodeScript::UNKNOWN,                  // 1EE50
    Character::UnicodeScript::ARABIC,                   // 1EE51..1EE52
    Character::UnicodeScript::UNKNOWN,                  // 1EE53
    Character::UnicodeScript::ARABIC,                   // 1EE54
    Character::UnicodeScript::UNKNOWN,                  // 1EE55..1EE56
    Character::UnicodeScript::ARABIC,                   // 1EE57
    Character::UnicodeScript::UNKNOWN,                  // 1EE58
    Character::UnicodeScript::ARABIC,                   // 1EE59
    Character::UnicodeScript::UNKNOWN,                  // 1EE5A
    Character::UnicodeScript::ARABIC,                   // 1EE5B
    Character::UnicodeScript::UNKNOWN,                  // 1EE5C
    Character::UnicodeScript::ARABIC,                   // 1EE5D
    Character::UnicodeScript::UNKNOWN,                  // 1EE5E
    Character::UnicodeScript::ARABIC,                   // 1EE5F
    Character::UnicodeScript::UNKNOWN,                  // 1EE60
    Character::UnicodeScript::ARABIC,                   // 1EE61..1EE62
    Character::UnicodeScript::UNKNOWN,                  // 1EE63
    Character::UnicodeScript::ARABIC,                   // 1EE64
    Character::UnicodeScript::UNKNOWN,                  // 1EE65..1EE66
    Character::UnicodeScript::ARABIC,                   // 1EE67..1EE6A
    Character::UnicodeScript::UNKNOWN,                  // 1EE6B
    Character::UnicodeScript::ARABIC,                   // 1EE6C..1EE72
    Character::UnicodeScript::UNKNOWN,                  // 1EE73
    Character::UnicodeScript::ARABIC,                   // 1EE74..1EE77
    Character::UnicodeScript::UNKNOWN,                  // 1EE78
    Character::UnicodeScript::ARABIC,                   // 1EE79..1EE7C
    Character::UnicodeScript::UNKNOWN,                  // 1EE7D
    Character::UnicodeScript::ARABIC,                   // 1EE7E
    Character::UnicodeScript::UNKNOWN,                  // 1EE7F
    Character::UnicodeScript::ARABIC,                   // 1EE80..1EE89
    Character::UnicodeScript::UNKNOWN,                  // 1EE8A
    Character::UnicodeScript::ARABIC,                   // 1EE8B..1EE9B
    Character::UnicodeScript::UNKNOWN,                  // 1EE9C..1EEA0
    Character::UnicodeScript::ARABIC,                   // 1EEA1..1EEA3
    Character::UnicodeScript::UNKNOWN,                  // 1EEA4
    Character::UnicodeScript::ARABIC,                   // 1EEA5..1EEA9
    Character::UnicodeScript::UNKNOWN,                  // 1EEAA
    Character::UnicodeScript::ARABIC,                   // 1EEAB..1EEBB
    Character::UnicodeScript::UNKNOWN,                  // 1EEBC..1EEEF
    Character::UnicodeScript::ARABIC,                   // 1EEF0..1EEF1
    Character::UnicodeScript::UNKNOWN,                  // 1EEF2..1EFFF
    Character::UnicodeScript::COMMON,                   // 1F000..1F02B
    Character::UnicodeScript::UNKNOWN,                  // 1F02C..1F02F
    Character::UnicodeScript::COMMON,                   // 1F030..1F093
    Character::UnicodeScript::UNKNOWN,                  // 1F094..1F09F
    Character::UnicodeScript::COMMON,                   // 1F0A0..1F0AE
    Character::UnicodeScript::UNKNOWN,                  // 1F0AF..1F0B0
    Character::UnicodeScript::COMMON,                   // 1F0B1..1F0BF
    Character::UnicodeScript::UNKNOWN,                  // 1F0C0
    Character::UnicodeScript::COMMON,                   // 1F0C1..1F0CF
    Character::UnicodeScript::UNKNOWN,                  // 1F0D0
    Character::UnicodeScript::COMMON,                   // 1F0D1..1F0F5
    Character::UnicodeScript::UNKNOWN,                  // 1F0F6..1F0FF
    Character::UnicodeScript::COMMON,                   // 1F100..1F1AD
    Character::UnicodeScript::UNKNOWN,                  // 1F1AE..1F1E5
    Character::UnicodeScript::COMMON,                   // 1F1E6..1F1FF
    Character::UnicodeScript::HIRAGANA,                 // 1F200
    Character::UnicodeScript::COMMON,                   // 1F201..1F202
    Character::UnicodeScript::UNKNOWN,                  // 1F203..1F20F
    Character::UnicodeScript::COMMON,                   // 1F210..1F23B
    Character::UnicodeScript::UNKNOWN,                  // 1F23C..1F23F
    Character::UnicodeScript::COMMON,                   // 1F240..1F248
    Character::UnicodeScript::UNKNOWN,                  // 1F249..1F24F
    Character::UnicodeScript::COMMON,                   // 1F250..1F251
    Character::UnicodeScript::UNKNOWN,                  // 1F252..1F25F
    Character::UnicodeScript::COMMON,                   // 1F260..1F265
    Character::UnicodeScript::UNKNOWN,                  // 1F266..1F2FF
    Character::UnicodeScript::COMMON,                   // 1F300..1F6D7
    Character::UnicodeScript::UNKNOWN,                  // 1F6D8..1F6DF
    Character::UnicodeScript::COMMON,                   // 1F6E0..1F6EC
    Character::UnicodeScript::UNKNOWN,                  // 1F6ED..1F6EF
    Character::UnicodeScript::COMMON,                   // 1F6F0..1F6FC
    Character::UnicodeScript::UNKNOWN,                  // 1F6FD..1F6FF
    Character::UnicodeScript::COMMON,                   // 1F700..1F773
    Character::UnicodeScript::UNKNOWN,                  // 1F774..1F77F
    Character::UnicodeScript::COMMON,                   // 1F780..1F7D8
    Character::UnicodeScript::UNKNOWN,                  // 1F7D9..1F7DF
    Character::UnicodeScript::COMMON,                   // 1F7E0..1F7EB
    Character::UnicodeScript::UNKNOWN,                  // 1F7EC..1F7FF
    Character::UnicodeScript::COMMON,                   // 1F800..1F80B
    Character::UnicodeScript::UNKNOWN,                  // 1F80C..1F80F
    Character::UnicodeScript::COMMON,                   // 1F810..1F847
    Character::UnicodeScript::UNKNOWN,                  // 1F848..1F84F
    Character::UnicodeScript::COMMON,                   // 1F850..1F859
    Character::UnicodeScript::UNKNOWN,                  // 1F85A..1F85F
    Character::UnicodeScript::COMMON,                   // 1F860..1F887
    Character::UnicodeScript::UNKNOWN,                  // 1F888..1F88F
    Character::UnicodeScript::COMMON,                   // 1F890..1F8AD
    Character::UnicodeScript::UNKNOWN,                  // 1F8AE..1F8AF
    Character::UnicodeScript::COMMON,                   // 1F8B0..1F8B1
    Character::UnicodeScript::UNKNOWN,                  // 1F8B2..1F8FF
    Character::UnicodeScript::COMMON,                   // 1F900..1F978
    Character::UnicodeScript::UNKNOWN,                  // 1F979
    Character::UnicodeScript::COMMON,                   // 1F97A..1F9CB
    Character::UnicodeScript::UNKNOWN,                  // 1F9CC
    Character::UnicodeScript::COMMON,                   // 1F9CD..1FA53
    Character::UnicodeScript::UNKNOWN,                  // 1FA54..1FA5F
    Character::UnicodeScript::COMMON,                   // 1FA60..1FA6D
    Character::UnicodeScript::UNKNOWN,                  // 1FA6E..1FA6F
    Character::UnicodeScript::COMMON,                   // 1FA70..1FA74
    Character::UnicodeScript::UNKNOWN,                  // 1FA75..1FA77
    Character::UnicodeScript::COMMON,                   // 1FA78..1FA7A
    Character::UnicodeScript::UNKNOWN,                  // 1FA7B..1FA7F
    Character::UnicodeScript::COMMON,                   // 1FA80..1FA86
    Character::UnicodeScript::UNKNOWN,                  // 1FA87..1FA8F
    Character::UnicodeScript::COMMON,                   // 1FA90..1FAA8
    Character::UnicodeScript::UNKNOWN,                  // 1FAA9..1FAAF
    Character::UnicodeScript::COMMON,                   // 1FAB0..1FAB6
    Character::UnicodeScript::UNKNOWN,                  // 1FAB7..1FABF
    Character::UnicodeScript::COMMON,                   // 1FAC0..1FAC2
    Character::UnicodeScript::UNKNOWN,                  // 1FAC3..1FACF
    Character::UnicodeScript::COMMON,                   // 1FAD0..1FAD6
    Character::UnicodeScript::UNKNOWN,                  // 1FAD7..1FAFF
    Character::UnicodeScript::COMMON,                   // 1FB00..1FB92
    Character::UnicodeScript::UNKNOWN,                  // 1FB93
    Character::UnicodeScript::COMMON,                   // 1FB94..1FBCA
    Character::UnicodeScript::UNKNOWN,                  // 1FBCB..1FBEF
    Character::UnicodeScript::COMMON,                   // 1FBF0..1FBF9
    Character::UnicodeScript::UNKNOWN,                  // 1FBFA..1FFFF
    Character::UnicodeScript::HAN,                      // 20000..2A6DD
    Character::UnicodeScript::UNKNOWN,                  // 2A6DE..2A6FF
    Character::UnicodeScript::HAN,                      // 2A700..2B734
    Character::UnicodeScript::UNKNOWN,                  // 2B735..2B73F
    Character::UnicodeScript::HAN,                      // 2B740..2B81D
    Character::UnicodeScript::UNKNOWN,                  // 2B81E..2B81F
    Character::UnicodeScript::HAN,                      // 2B820..2CEA1
    Character::UnicodeScript::UNKNOWN,                  // 2CEA2..2CEAF
    Character::UnicodeScript::HAN,                      // 2CEB0..2EBE0
    Character::UnicodeScript::UNKNOWN,                  // 2EBE1..2F7FF
    Character::UnicodeScript::HAN,                      // 2F800..2FA1D
    Character::UnicodeScript::UNKNOWN,                  // 2FA1E..2FFFF
    Character::UnicodeScript::HAN,                      // 30000..3134A
    Character::UnicodeScript::UNKNOWN,                  // 3134B..E0000
    Character::UnicodeScript::COMMON,                   // E0001
    Character::UnicodeScript::UNKNOWN,                  // E0002..E001F
    Character::UnicodeScript::COMMON,                   // E0020..E007F
    Character::UnicodeScript::UNKNOWN,                  // E0080..E00FF
    Character::UnicodeScript::INHERITED,                // E0100..E01EF
    Character::UnicodeScript::UNKNOWN,                  // E01F0..10FFFF
};
const std::unordered_map<std::string, int>& Character::UnicodeScript::getAliases() {
    static const std::unordered_map<std::string, int> aliases_map = {
    {"ADLM", Character::UnicodeScript::ADLAM},
    {"AGHB", Character::UnicodeScript::CAUCASIAN_ALBANIAN},
    {"AHOM", Character::UnicodeScript::AHOM},
    {"ARAB", Character::UnicodeScript::ARABIC},
    {"ARMI", Character::UnicodeScript::IMPERIAL_ARAMAIC},
    {"ARMN", Character::UnicodeScript::ARMENIAN},
    {"AVST", Character::UnicodeScript::AVESTAN},
    {"BALI", Character::UnicodeScript::BALINESE},
    {"BAMU", Character::UnicodeScript::BAMUM},
    {"BASS", Character::UnicodeScript::BASSA_VAH},
    {"BATK", Character::UnicodeScript::BATAK},
    {"BENG", Character::UnicodeScript::BENGALI},
    {"BHKS", Character::UnicodeScript::BHAIKSUKI},
    {"BOPO", Character::UnicodeScript::BOPOMOFO},
    {"BRAH", Character::UnicodeScript::BRAHMI},
    {"BRAI", Character::UnicodeScript::BRAILLE},
    {"BUGI", Character::UnicodeScript::BUGINESE},
    {"BUHD", Character::UnicodeScript::BUHID},
    {"CAKM", Character::UnicodeScript::CHAKMA},
    {"CANS", Character::UnicodeScript::CANADIAN_ABORIGINAL},
    {"CARI", Character::UnicodeScript::CARIAN},
    {"CHAM", Character::UnicodeScript::CHAM},
    {"CHER", Character::UnicodeScript::CHEROKEE},
    {"CHRS", Character::UnicodeScript::CHORASMIAN},
    {"COPT", Character::UnicodeScript::COPTIC},
    {"CPRT", Character::UnicodeScript::CYPRIOT},
    {"CYRL", Character::UnicodeScript::CYRILLIC},
    {"DEVA", Character::UnicodeScript::DEVANAGARI},
    {"DIAK", Character::UnicodeScript::DIVES_AKURU},
    {"DOGR", Character::UnicodeScript::DOGRA},
    {"DSRT", Character::UnicodeScript::DESERET},
    {"DUPL", Character::UnicodeScript::DUPLOYAN},
    {"EGYP", Character::UnicodeScript::EGYPTIAN_HIEROGLYPHS},
    {"ELBA", Character::UnicodeScript::ELBASAN},
    {"ELYM", Character::UnicodeScript::ELYMAIC},
    {"ETHI", Character::UnicodeScript::ETHIOPIC},
    {"GEOR", Character::UnicodeScript::GEORGIAN},
    {"GLAG", Character::UnicodeScript::GLAGOLITIC},
    {"GONM", Character::UnicodeScript::MASARAM_GONDI},
    {"GOTH", Character::UnicodeScript::GOTHIC},
    {"GONG", Character::UnicodeScript::GUNJALA_GONDI},
    {"GRAN", Character::UnicodeScript::GRANTHA},
    {"GREK", Character::UnicodeScript::GREEK},
    {"GUJR", Character::UnicodeScript::GUJARATI},
    {"GURU", Character::UnicodeScript::GURMUKHI},
    {"HANG", Character::UnicodeScript::HANGUL},
    {"HANI", Character::UnicodeScript::HAN},
    {"HANO", Character::UnicodeScript::HANUNOO},
    {"HATR", Character::UnicodeScript::HATRAN},
    {"HEBR", Character::UnicodeScript::HEBREW},
    {"HIRA", Character::UnicodeScript::HIRAGANA},
    {"HLUW", Character::UnicodeScript::ANATOLIAN_HIEROGLYPHS},
    {"HMNG", Character::UnicodeScript::PAHAWH_HMONG},
    {"HMNP", Character::UnicodeScript::NYIAKENG_PUACHUE_HMONG},
    // it appears we don't have the KATAKANA_OR_HIRAGANA
    // {"HRKT", Character::UnicodeScript::KATAKANA_OR_HIRAGANA},
    {"HUNG", Character::UnicodeScript::OLD_HUNGARIAN},
    {"ITAL", Character::UnicodeScript::OLD_ITALIC},
    {"JAVA", Character::UnicodeScript::JAVANESE},
    {"KALI", Character::UnicodeScript::KAYAH_LI},
    {"KANA", Character::UnicodeScript::KATAKANA},
    {"KHAR", Character::UnicodeScript::KHAROSHTHI},
    {"KHMR", Character::UnicodeScript::KHMER},
    {"KHOJ", Character::UnicodeScript::KHOJKI},
    {"KITS", Character::UnicodeScript::KHITAN_SMALL_SCRIPT},
    {"KNDA", Character::UnicodeScript::KANNADA},
    {"KTHI", Character::UnicodeScript::KAITHI},
    {"LANA", Character::UnicodeScript::TAI_THAM},
    {"LAOO", Character::UnicodeScript::LAO},
    {"LATN", Character::UnicodeScript::LATIN},
    {"LEPC", Character::UnicodeScript::LEPCHA},
    {"LIMB", Character::UnicodeScript::LIMBU},
    {"LINA", Character::UnicodeScript::LINEAR_A},
    {"LINB", Character::UnicodeScript::LINEAR_B},
    {"LISU", Character::UnicodeScript::LISU},
    {"LYCI", Character::UnicodeScript::LYCIAN},
    {"LYDI", Character::UnicodeScript::LYDIAN},
    {"MAHJ", Character::UnicodeScript::MAHAJANI},
    {"MAKA", Character::UnicodeScript::MAKASAR},
    {"MARC", Character::UnicodeScript::MARCHEN},
    {"MAND", Character::UnicodeScript::MANDAIC},
    {"MANI", Character::UnicodeScript::MANICHAEAN},
    {"MEDF", Character::UnicodeScript::MEDEFAIDRIN},
    {"MEND", Character::UnicodeScript::MENDE_KIKAKUI},
    {"MERC", Character::UnicodeScript::MEROITIC_CURSIVE},
    {"MERO", Character::UnicodeScript::MEROITIC_HIEROGLYPHS},
    {"MLYM", Character::UnicodeScript::MALAYALAM},
    {"MODI", Character::UnicodeScript::MODI},
    {"MONG", Character::UnicodeScript::MONGOLIAN},
    {"MROO", Character::UnicodeScript::MRO},
    {"MTEI", Character::UnicodeScript::MEETEI_MAYEK},
    {"MULT", Character::UnicodeScript::MULTANI},
    {"MYMR", Character::UnicodeScript::MYANMAR},
    {"NAND", Character::UnicodeScript::NANDINAGARI},
    {"NARB", Character::UnicodeScript::OLD_NORTH_ARABIAN},
    {"NBAT", Character::UnicodeScript::NABATAEAN},
    {"NEWA", Character::UnicodeScript::NEWA},
    {"NKOO", Character::UnicodeScript::NKO},
    {"NSHU", Character::UnicodeScript::NUSHU},
    {"OGAM", Character::UnicodeScript::OGHAM},
    {"OLCK", Character::UnicodeScript::OL_CHIKI},
    {"ORKH", Character::UnicodeScript::OLD_TURKIC},
    {"ORYA", Character::UnicodeScript::ORIYA},
    {"OSGE", Character::UnicodeScript::OSAGE},
    {"OSMA", Character::UnicodeScript::OSMANYA},
    {"PALM", Character::UnicodeScript::PALMYRENE},
    {"PAUC", Character::UnicodeScript::PAU_CIN_HAU},
    {"PERM", Character::UnicodeScript::OLD_PERMIC},
    {"PHAG", Character::UnicodeScript::PHAGS_PA},
    {"PHLI", Character::UnicodeScript::INSCRIPTIONAL_PAHLAVI},
    {"PHLP", Character::UnicodeScript::PSALTER_PAHLAVI},
    {"PHNX", Character::UnicodeScript::PHOENICIAN},
    {"PLRD", Character::UnicodeScript::MIAO},
    {"PRTI", Character::UnicodeScript::INSCRIPTIONAL_PARTHIAN},
    {"RJNG", Character::UnicodeScript::REJANG},
    {"ROHG", Character::UnicodeScript::HANIFI_ROHINGYA},
    {"RUNR", Character::UnicodeScript::RUNIC},
    {"SAMR", Character::UnicodeScript::SAMARITAN},
    {"SARB", Character::UnicodeScript::OLD_SOUTH_ARABIAN},
    {"SAUR", Character::UnicodeScript::SAURASHTRA},
    {"SGNW", Character::UnicodeScript::SIGNWRITING},
    {"SHAW", Character::UnicodeScript::SHAVIAN},
    {"SHRD", Character::UnicodeScript::SHARADA},
    {"SIDD", Character::UnicodeScript::SIDDHAM},
    {"SIND", Character::UnicodeScript::KHUDAWADI},
    {"SINH", Character::UnicodeScript::SINHALA},
    {"SOGD", Character::UnicodeScript::SOGDIAN},
    {"SOGO", Character::UnicodeScript::OLD_SOGDIAN},
    {"SORA", Character::UnicodeScript::SORA_SOMPENG},
    {"SOYO", Character::UnicodeScript::SOYOMBO},
    {"SUND", Character::UnicodeScript::SUNDANESE},
    {"SYLO", Character::UnicodeScript::SYLOTI_NAGRI},
    {"SYRC", Character::UnicodeScript::SYRIAC},
    {"TAGB", Character::UnicodeScript::TAGBANWA},
    {"TAKR", Character::UnicodeScript::TAKRI},
    {"TALE", Character::UnicodeScript::TAI_LE},
    {"TALU", Character::UnicodeScript::NEW_TAI_LUE},
    {"TAML", Character::UnicodeScript::TAMIL},
    {"TANG", Character::UnicodeScript::TANGUT},
    {"TAVT", Character::UnicodeScript::TAI_VIET},
    {"TELU", Character::UnicodeScript::TELUGU},
    {"TFNG", Character::UnicodeScript::TIFINAGH},
    {"TGLG", Character::UnicodeScript::TAGALOG},
    {"THAA", Character::UnicodeScript::THAANA},
    {"THAI", Character::UnicodeScript::THAI},
    {"TIBT", Character::UnicodeScript::TIBETAN},
    {"TIRH", Character::UnicodeScript::TIRHUTA},
    {"UGAR", Character::UnicodeScript::UGARITIC},
    {"VAII", Character::UnicodeScript::VAI},
    {"WARA", Character::UnicodeScript::WARANG_CITI},
    {"WCHO", Character::UnicodeScript::WANCHO},
    {"XPEO", Character::UnicodeScript::OLD_PERSIAN},
    {"XSUX", Character::UnicodeScript::CUNEIFORM},
    {"YIII", Character::UnicodeScript::YI},
    {"YEZI", Character::UnicodeScript::YEZIDI},
    {"ZANB", Character::UnicodeScript::ZANABAZAR_SQUARE},
    {"ZINH", Character::UnicodeScript::INHERITED},
    {"ZYYY", Character::UnicodeScript::COMMON},
    {"ZZZZ", Character::UnicodeScript::UNKNOWN},
    };
    return aliases_map;
}

int Character::UnicodeScript::of(int codePoint) {
    if (!isValidCodePoint(codePoint)){
        //throw new IllegalArgumentException("Not a valid Unicode code point: 0x%X", codePoint);
    }
    int type = getType(codePoint);
    // leave SURROGATE and PRIVATE_USE for table lookup
    if (type == UNASSIGNED)
        return UNKNOWN;
    
    // 使用 C++ 的二分查找
    auto it = std::lower_bound(std::begin(scriptStarts), std::end(scriptStarts), codePoint);
    int index = std::distance(std::begin(scriptStarts), it);
    
    // 如果找到的值大于 codePoint，回退一个位置
    if (it == std::end(scriptStarts) || *it > codePoint) {
        index--;
    }
    
    return scripts[index];
}

int Character::UnicodeScript::forName(const std::string& scriptName) {
    //scriptName = scriptName.toUpperCase(Locale.ENGLISH);
    //                        //.replace(' ', '_'));
    auto& aliases=getAliases();
    auto it = aliases.find(scriptName);
    if (it != aliases.end())
        return it->second;
    return UNKNOWN;//valueOf(scriptName);
}
//////endof UnicodeScript/////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CharacterCache {
private:
    CharacterCache(){}
    static Character cache[];
    static Character archivedCache[128];

    /*static {
        int size = 127 + 1;
        // Load and use the archived cache if it exists
        // Android-removed: CDS is not used on Android.
        // CDS.initializeFromArchive(CharacterCache.class);
        if (archivedCache == null || archivedCache.length != size) {
            Character[] c = new Character[size];
            for (int i = 0; i < size; i++) {
                c[i] = new Character((char16_t) i);
            }
            archivedCache = c;
        }
        cache = archivedCache;
    }*/
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Character* Character::valueOf(char16_t c) {
    /*if (c <= 127) { // must cache
        return CharacterCache.cache[(int)c];
    }*/
    return new Character(c);
}

String Character::toString(char16_t c) {
    return String::valueOf(c);
}

String Character::toString(int codePoint) {
    if (isSupplementaryCodePoint(codePoint)) {
        char16_t buf[2];
        toChars(codePoint, buf, 0);
        return String(std::u16string(buf, 2));
    }
    return String::valueOf((char16_t)codePoint);
}

bool Character::isValidCodePoint(int codePoint) {
    // Optimized form of:
    //     codePoint >= MIN_CODE_POINT && codePoint <= MAX_CODE_POINT
    int plane = codePoint >> 16;
    return plane < ((MAX_CODE_POINT + 1) >> 16);
}

bool Character::isBmpCodePoint(int codePoint) {
    return codePoint >> 16 == 0;
    // Optimized form of:
    //     codePoint >= MIN_VALUE && codePoint <= MAX_VALUE
    // We consistently use logical shift (>>>) to facilitate
    // additional runtime optimizations.
}

bool Character::isSupplementaryCodePoint(int codePoint) {
    return codePoint >= MIN_SUPPLEMENTARY_CODE_POINT
        && codePoint <  MAX_CODE_POINT + 1;
}

bool Character::isHighSurrogate(char16_t ch) {
    // Help VM constant-fold; MAX_HIGH_SURROGATE + 1 == MIN_LOW_SURROGATE
    return ch >= MIN_HIGH_SURROGATE && ch < (MAX_HIGH_SURROGATE + 1);
}

bool Character::isLowSurrogate(char16_t ch) {
    return ch >= MIN_LOW_SURROGATE && ch < (MAX_LOW_SURROGATE + 1);
}

bool Character::isSurrogate(char16_t ch) {
    return ch >= MIN_SURROGATE && ch < (MAX_SURROGATE + 1);
}

bool Character::isSurrogatePair(char16_t high, char16_t low) {
    return isHighSurrogate(high) && isLowSurrogate(low);
}

int Character::charCount(int codePoint) {
    return codePoint >= MIN_SUPPLEMENTARY_CODE_POINT ? 2 : 1;
}

int Character::toCodePoint(char16_t high, char16_t low) {
    // Optimized form of:
    // return ((high - MIN_HIGH_SURROGATE) << 10)
    //         + (low - MIN_LOW_SURROGATE)
    //         + MIN_SUPPLEMENTARY_CODE_POINT;
    return ((high << 10) + low) + (MIN_SUPPLEMENTARY_CODE_POINT
                                   - (MIN_HIGH_SURROGATE << 10)
                                   - MIN_LOW_SURROGATE);
}

int Character::codePointAt(const CharSequence* seq, int index) {
    char16_t c1 = seq->charAt(index);
    if (isHighSurrogate(c1) && ++index < seq->length()) {
        char16_t c2 = seq->charAt(index);
        if (isLowSurrogate(c2)) {
            return toCodePoint(c1, c2);
        }
    }
    return c1;
}

int Character::codePointAt(const char16_t* a, int index){
    return codePointAtImpl(a, index, 2);
}

// throws ArrayIndexOutOfBoundsException if index out of bounds
int Character::codePointAtImpl(const char16_t* a, int index, int limit) {
    char16_t c1 = a[index];
    if (isHighSurrogate(c1) && ++index < limit) {
        char16_t c2 = a[index];
        if (isLowSurrogate(c2)) {
            return toCodePoint(c1, c2);
        }
    }
    return c1;
}

int Character::codePointBefore(const CharSequence* seq, int index) {
    char16_t c2 = seq->charAt(--index);
    if (isLowSurrogate(c2) && index > 0) {
        char16_t c1 = seq->charAt(--index);
        if (isHighSurrogate(c1)) {
            return toCodePoint(c1, c2);
        }
    }
    return c2;
}

int Character::codePointBefore(const char16_t* a, int index) {
    return codePointBeforeImpl(a, index, 0);
}

int Character::codePointBefore(const char16_t* a, int index, int start) {
    return codePointBeforeImpl(a, index, start);
}

// throws ArrayIndexOutOfBoundsException if index-1 out of bounds
int Character::codePointBeforeImpl(const char16_t* a, int index, int start) {
    char16_t c2 = a[--index];
    if (isLowSurrogate(c2) && index > start) {
        char16_t c1 = a[--index];
        if (isHighSurrogate(c1)) {
            return toCodePoint(c1, c2);
        }
    }
    return c2;
}

char16_t Character::highSurrogate(int codePoint) {
    return (char) ((codePoint >> 10)
        + (MIN_HIGH_SURROGATE - (MIN_SUPPLEMENTARY_CODE_POINT >> 10)));
}

char16_t Character::lowSurrogate(int codePoint) {
    return (char) ((codePoint & 0x3ff) + MIN_LOW_SURROGATE);
}

int Character::toChars(int codePoint, char16_t* dst, int dstIndex) {
    if (isBmpCodePoint(codePoint)) {
        dst[dstIndex] = (char) codePoint;
        return 1;
    } else if (isValidCodePoint(codePoint)) {
        toSurrogates(codePoint, dst, dstIndex);
        return 2;
    } else {
        //throw "Not a valid Unicode code point: 0x%X", codePoint);
        return 0;
    }
}


void Character::toSurrogates(int codePoint, char16_t* dst, int index) {
    // We write elements "backwards" to guarantee all-or-nothing
    dst[index+1] = lowSurrogate(codePoint);
    dst[index] = highSurrogate(codePoint);
}

int Character::codePointCount(const CharSequence* seq, int beginIndex, int endIndex) {
    int length = seq->length();
    if (beginIndex < 0 || endIndex > length || beginIndex > endIndex) {
        //throw new IndexOutOfBoundsException();
    }
    int n = endIndex - beginIndex;
    for (int i = beginIndex; i < endIndex; ) {
        if (isHighSurrogate(seq->charAt(i++)) && i < endIndex &&
            isLowSurrogate(seq->charAt(i))) {
            n--;
            i++;
        }
    }
    return n;
}

int Character::codePointCount(const char16_t* a, int offset, int count) {
    return codePointCountImpl(a, offset, count);
}

int Character::codePointCountImpl(const char16_t* a, int offset, int count) {
    int endIndex = offset + count;
    int n = count;
    for (int i = offset; i < endIndex; ) {
        if (isHighSurrogate(a[i++]) && i < endIndex &&
            isLowSurrogate(a[i])) {
            n--;
            i++;
        }
    }
    return n;
}

int Character::offsetByCodePoints(const CharSequence* seq, int index, int codePointOffset) {
    int length = seq->length();
    if (index < 0 || index > length) {
        //throw new IndexOutOfBoundsException();
    }

    int x = index;
    if (codePointOffset >= 0) {
        int i;
        for (i = 0; x < length && i < codePointOffset; i++) {
            if (isHighSurrogate(seq->charAt(x++)) && x < length &&
                isLowSurrogate(seq->charAt(x))) {
                x++;
            }
        }
        if (i < codePointOffset) {
            //throw new IndexOutOfBoundsException();
        }
    } else {
        int i;
        for (i = codePointOffset; x > 0 && i < 0; i++) {
            if (isLowSurrogate(seq->charAt(--x)) && x > 0 &&
                isHighSurrogate(seq->charAt(x-1))) {
                x--;
            }
        }
        if (i < 0) {
            //throw new IndexOutOfBoundsException();
        }
    }
    return x;
}

int Character::offsetByCodePoints(const char16_t* a, int start, int count, int index, int codePointOffset) {
    if (start < 0 || count < 0
        || index < start || index > start+count) {
        //throw new IndexOutOfBoundsException();
    }
    return offsetByCodePointsImpl(a, start, count, index, codePointOffset);
}

int Character::offsetByCodePointsImpl(const char16_t* a, int start, int count, int index, int codePointOffset) {
    int x = index;
    if (codePointOffset >= 0) {
        int limit = start + count;
        int i;
        for (i = 0; x < limit && i < codePointOffset; i++) {
            if (isHighSurrogate(a[x++]) && x < limit &&
                isLowSurrogate(a[x])) {
                x++;
            }
        }
        if (i < codePointOffset) {
            //throw new IndexOutOfBoundsException();
        }
    } else {
        int i;
        for (i = codePointOffset; x > start && i < 0; i++) {
            if (isLowSurrogate(a[--x]) && x > start &&
                isHighSurrogate(a[x-1])) {
                x--;
            }
        }
        if (i < 0) {
            //throw new IndexOutOfBoundsException();
        }
    }
    return x;
}

bool Character::isLowerCase(char16_t ch) {
    return isLowerCase((int)ch);
}

bool Character::isLowerCase(int codePoint) {
    return isLowerCaseImpl(codePoint);
}

// END Android-changed: Reimplement methods natively on top of ICU4C.

bool Character::isUpperCase(char16_t ch) {
    return isUpperCase((int)ch);
}

bool Character::isUpperCase(int codePoint) {
    return isUpperCaseImpl(codePoint);
}

static native bool isUpperCaseImpl(int codePoint);
// END Android-changed: Reimplement methods natively on top of ICU4C.

bool Character::isTitleCase(char16_t ch) {
    return isTitleCase((int)ch);
}

bool Character::isTitleCase(int codePoint) {
    return isTitleCaseImpl(codePoint);
}

static native bool isTitleCaseImpl(int codePoint);
// END Android-changed: Reimplement methods natively on top of ICU4C.

bool Character::isDigit(char16_t ch) {
    return isDigit((int)ch);
}

bool Character::isDigit(int codePoint) {
    return isDigitImpl(codePoint);
}

static native bool isDigitImpl(int codePoint);
// END Android-changed: Reimplement methods natively on top of ICU4C.

bool Character::isDefined(char16_t ch) {
    return isDefined((int)ch);
}

bool Character::isDefined(int codePoint) {
    return isDefinedImpl(codePoint);
}

static native bool isDefinedImpl(int codePoint);
// END Android-changed: Reimplement methods natively on top of ICU4C.

bool Character::isLetter(char16_t ch) {
    return isLetter((int)ch);
}

bool Character::isLetter(int codePoint) {
    return isLetterImpl(codePoint);
}

static native bool isLetterImpl(int codePoint);
// END Android-changed: Reimplement methods natively on top of ICU4C.

bool Character::isLetterOrDigit(char16_t ch) {
    return isLetterOrDigit((int)ch);
}

bool Character::isLetterOrDigit(int codePoint) {
    return isLetterOrDigitImpl(codePoint);
}

static native bool isLetterOrDigitImpl(int codePoint);
// END Android-changed: Reimplement methods natively on top of ICU4C.

bool Character::isJavaLetter(char16_t ch) {
    return isJavaIdentifierStart(ch);
}

bool Character::isJavaLetterOrDigit(char16_t ch) {
    return isJavaIdentifierPart(ch);
}

bool Character::isAlphabetic(int codePoint) {
    return isAlphabeticImpl(codePoint);
}

static native bool isAlphabeticImpl(int codePoint);
// END Android-changed: Reimplement methods natively on top of ICU4C.

bool Character::isIdeographic(int codePoint) {
    return isIdeographicImpl(codePoint);
}
static native bool isIdeographicImpl(int codePoint);
// END Android-changed: Reimplement methods natively on top of ICU4C.

// Android-changed: Removed @see tag (target does not exist on Android):
// @see     javax.lang.model.SourceVersion#isIdentifier(CharSequence)
bool Character::isJavaIdentifierStart(char16_t ch) {
    return isJavaIdentifierStart((int)ch);
}

// Android-changed: Removed @see tag (target does not exist on Android):
// @see     javax.lang.model.SourceVersion#isIdentifier(CharSequence)
bool Character::isJavaIdentifierStart(int codePoint) {
    // Use precomputed bitmasks to optimize the ASCII range.
    if (codePoint < 64) {
        return (codePoint == '$'); // There's only one character in this range.
    } else if (codePoint < 128) {
        return (0x7fffffe87fffffeL & (1L << (codePoint - 64))) != 0;
    }
    return ((1 << getType(codePoint))
            & ((1 << UPPERCASE_LETTER)
               | (1 << LOWERCASE_LETTER)
               | (1  << TITLECASE_LETTER)
               | (1  << MODIFIER_LETTER)
               | (1  << OTHER_LETTER)
               | (1  << CURRENCY_SYMBOL)
               | (1  << CONNECTOR_PUNCTUATION)
               | (1  << LETTER_NUMBER))) != 0;
}
// END Android-changed: Use ICU.

// Android-changed: Removed @see tag (target does not exist on Android):
bool Character::isJavaIdentifierPart(char16_t ch) {
    return isJavaIdentifierPart((int)ch);
}

bool Character::isJavaIdentifierPart(int codePoint) {
    // Use precomputed bitmasks to optimize the ASCII range.
    if (codePoint < 64) {
        return (0x3ff00100fffc1ffL & (1L << codePoint)) != 0;
    } else if (codePoint < 128) {
        return (0x87fffffe87fffffeL & (1L << (codePoint - 64))) != 0;
    }
    return ((1 << getType(codePoint))
            & ((1 << UPPERCASE_LETTER)
               | (1 << LOWERCASE_LETTER)
               | (1 << TITLECASE_LETTER)
               | (1 << MODIFIER_LETTER)
               | (1 << OTHER_LETTER)
               | (1 << CURRENCY_SYMBOL)
               | (1 << CONNECTOR_PUNCTUATION)
               | (1 << DECIMAL_DIGIT_NUMBER)
               | (1 << LETTER_NUMBER)
               | (1 << FORMAT)
               | (1 << COMBINING_SPACING_MARK)
               | (1 << NON_SPACING_MARK))) != 0
            || (codePoint >= 0 && codePoint <= 8) || (codePoint >= 0xe && codePoint <= 0x1b)
            || (codePoint >= 0x7f && codePoint <= 0x9f);
}
bool Character::isUnicodeIdentifierStart(char16_t ch) {
    return isUnicodeIdentifierStart((int)ch);
}

bool Character::isUnicodeIdentifierStart(int codePoint) {
    return isUnicodeIdentifierStartImpl(codePoint);
}

static native bool isUnicodeIdentifierStartImpl(int codePoint);
// END Android-changed: Reimplement methods natively on top of ICU4C.

bool Character::isUnicodeIdentifierPart(char16_t ch) {
    return isUnicodeIdentifierPart((int)ch);
}

bool Character::isUnicodeIdentifierPart(int codePoint) {
    return isUnicodeIdentifierPartImpl(codePoint);
}

static native bool isUnicodeIdentifierPartImpl(int codePoint);
// END Android-changed: Reimplement methods natively on top of ICU4C.

bool Character::isIdentifierIgnorable(char16_t ch) {
    return isIdentifierIgnorable((int)ch);
}

bool Character::isIdentifierIgnorable(int codePoint) {
    return isIdentifierIgnorableImpl(codePoint);
}

static native bool isIdentifierIgnorableImpl(int codePoint);
// END Android-changed: Reimplement methods natively on top of ICU4C.

char16_t Character::toLowerCase(char16_t ch) {
    return (char16_t)toLowerCase((int)ch);
}

int Character::toLowerCase(int codePoint) {
    if (codePoint >= 'A' && codePoint <= 'Z') {
        return codePoint + ('a' - 'A');
    }

    // All ASCII codepoints except the ones above remain unchanged.
    if (codePoint < 0x80) {
        return codePoint;
    }

    return u_tolower(codePoint);
}


char16_t Character::toUpperCase(char16_t ch) {
    return (char)toUpperCase((int)ch);
}

int Character::toUpperCase(int codePoint) {
    if (codePoint >= 'a' && codePoint <= 'z') {
        return codePoint - ('a' - 'A');
    }

    // All ASCII codepoints except the ones above remain unchanged.
    if (codePoint < 0x80) {
        return codePoint;
    }

    return u_toupper(codePoint);
}

static native int toUpperCaseImpl(int codePoint);
// END Android-changed: Reimplement methods natively on top of ICU4C.

char16_t Character::toTitleCase(char16_t ch) {
    return (char16_t)toTitleCase((int)ch);
}

int Character::toTitleCase(int codePoint) {
    return toTitleCaseImpl(codePoint);
}

static native int toTitleCaseImpl(int codePoint);
// END Android-changed: Reimplement methods natively on top of ICU4C.

int Character::digit(char16_t ch, int radix) {
    return digit((int)ch, radix);
}

int Character::digit(int codePoint, int radix) {
    if (radix < MIN_RADIX || radix > MAX_RADIX) {
        return -1;
    }
    if (codePoint < 128) {
        // Optimized for ASCII
        int result = -1;
        if ('0' <= codePoint && codePoint <= '9') {
            result = codePoint - '0';
        } else if ('a' <= codePoint && codePoint <= 'z') {
            result = 10 + (codePoint - 'a');
        } else if ('A' <= codePoint && codePoint <= 'Z') {
            result = 10 + (codePoint - 'A');
        }
        return result < radix ? result : -1;
    }
    return digitImpl(codePoint, radix);
}

native static int digitImpl(int codePoint, int radix);
// END Android-changed: Reimplement methods natively on top of ICU4C.

int Character::getNumericValue(char16_t ch) {
    return getNumericValue((int)ch);
}

int Character::getNumericValue(int codePoint) {
    if (codePoint < 128) {
        if (codePoint >= '0' && codePoint <= '9') {
            return codePoint - '0';
        }
        if (codePoint >= 'a' && codePoint <= 'z') {
            return codePoint - ('a' - 10);
        }
        if (codePoint >= 'A' && codePoint <= 'Z') {
            return codePoint - ('A' - 10);
        }
        return -1;
    }
    // Full-width uppercase A-Z.
    if (codePoint >= 0xff21 && codePoint <= 0xff3a) {
        return codePoint - 0xff17;
    }
    // Full-width lowercase a-z.
    if (codePoint >= 0xff41 && codePoint <= 0xff5a) {
        return codePoint - 0xff37;
    }
    return getNumericValueImpl(codePoint);
}

native static int getNumericValueImpl(int codePoint);
// END Android-changed: Reimplement methods natively on top of ICU4C.

bool Character::isSpace(char16_t ch) {
    return (ch <= 0x0020) &&
        (((((1L << 0x0009) |
        (1L << 0x000A) |
        (1L << 0x000C) |
        (1L << 0x000D) |
        (1L << 0x0020)) >> ch) & 1L) != 0);
}


bool Character::isSpaceChar(char16_t ch) {
    return isSpaceChar((int)ch);
}

bool Character::isSpaceChar(int codePoint) {
    // We don't just call into icu4c because of the JNI overhead. Ideally we'd fix that.
    // SPACE or NO-BREAK SPACE?
    if (codePoint == 0x20 || codePoint == 0xa0) {
        return true;
    }
    if (codePoint < 0x1000) {
        return false;
    }
    // OGHAM SPACE MARK or MONGOLIAN VOWEL SEPARATOR?
    if (codePoint == 0x1680 || codePoint == 0x180e) {
        return true;
    }
    if (codePoint < 0x2000) {
        return false;
    }
    if (codePoint <= 0xffff) {
        // Other whitespace from General Punctuation...
        return codePoint <= 0x200a || codePoint == 0x2028 || codePoint == 0x2029 || codePoint == 0x202f || codePoint == 0x205f ||
            codePoint == 0x3000; // ...or CJK Symbols and Punctuation?
    }
    // Let icu4c worry about non-BMP code points.
    return isSpaceCharImpl(codePoint);
}

static native bool isSpaceCharImpl(int codePoint);
// END Android-changed: Reimplement methods natively on top of ICU4C.

bool Character::isWhitespace(char16_t ch) {
    return isWhitespace((int)ch);
}

bool Character::isWhitespace(int codePoint) {
    // We don't just call into icu4c because of the JNI overhead. Ideally we'd fix that.
    // Any ASCII whitespace character?
    if ((codePoint >= 0x1c && codePoint <= 0x20) || (codePoint >= 0x09 && codePoint <= 0x0d)) {
        return true;
    }
    if (codePoint < 0x1000) {
        return false;
    }
    // OGHAM SPACE MARK or MONGOLIAN VOWEL SEPARATOR?
    if (codePoint == 0x1680 || codePoint == 0x180e) {
        return true;
    }
    if (codePoint < 0x2000) {
        return false;
    }
    // Exclude General Punctuation's non-breaking spaces (which includes FIGURE SPACE).
    if (codePoint == 0x2007 || codePoint == 0x202f) {
        return false;
    }
    if (codePoint <= 0xffff) {
        // Other whitespace from General Punctuation...
        return codePoint <= 0x200a || codePoint == 0x2028 || codePoint == 0x2029 || codePoint == 0x205f ||
            codePoint == 0x3000; // ...or CJK Symbols and Punctuation?
    }
    // Let icu4c worry about non-BMP code points.
    return isWhitespaceImpl(codePoint);
}

native static bool isWhitespaceImpl(int codePoint);
// END Android-changed: Reimplement methods natively on top of ICU4C.

bool Character::isISOControl(char16_t ch) {
    return isISOControl((int)ch);
}

bool Character::isISOControl(int codePoint) {
    // Optimized form of:
    //     (codePoint >= 0x00 && codePoint <= 0x1F) ||
    //     (codePoint >= 0x7F && codePoint <= 0x9F);
    return codePoint <= 0x9F &&
        (codePoint >= 0x7F || (codePoint >> 5 == 0));
}

int Character::getType(char16_t ch) {
    return getType((int)ch);
}

int Character::getType(int codePoint) {
    int type = getTypeImpl(codePoint);
    // The type values returned by ICU are not RI-compatible. The RI skips the value 17.
    if (type <= Character::FORMAT) {
        return type;
    }
    return (type + 1);
}

static native int getTypeImpl(int codePoint);
// END Android-changed: Reimplement methods natively on top of ICU4C.

char16_t Character::forDigit(int digit, int radix) {
    if ((digit >= radix) || (digit < 0)) {
        return '\0';
    }
    if ((radix < Character::MIN_RADIX) || (radix > Character::MAX_RADIX)) {
        return '\0';
    }
    if (digit < 10) {
        return (char)('0' + digit);
    }
    return (char)('a' - 10 + digit);
}

uint8_t Character::getDirectionality(char16_t ch) {
    return getDirectionality((int)ch);
}
static uint8_t getDirectionalityImpl(int codePoint){
    const UCharDirection dir = u_charDirection(codePoint);
    switch (dir) {
    case U_LEFT_TO_RIGHT:
        return Character::DIRECTIONALITY_LEFT_TO_RIGHT;
    case U_RIGHT_TO_LEFT:
        return Character::DIRECTIONALITY_RIGHT_TO_LEFT;
    case U_EUROPEAN_NUMBER:
        return Character::DIRECTIONALITY_EUROPEAN_NUMBER;
    case U_EUROPEAN_NUMBER_SEPARATOR:
        return Character::DIRECTIONALITY_EUROPEAN_NUMBER_SEPARATOR;
    case U_EUROPEAN_NUMBER_TERMINATOR:
        return Character::DIRECTIONALITY_EUROPEAN_NUMBER_TERMINATOR;
    case U_ARABIC_NUMBER:
        return Character::DIRECTIONALITY_ARABIC_NUMBER;
    case U_COMMON_NUMBER_SEPARATOR:
        return Character::DIRECTIONALITY_COMMON_NUMBER_SEPARATOR;
    case U_BLOCK_SEPARATOR:
        return Character::DIRECTIONALITY_PARAGRAPH_SEPARATOR;
    case U_SEGMENT_SEPARATOR:
        return Character::DIRECTIONALITY_SEGMENT_SEPARATOR;
    //case U_WHITE_SPACE_NEUTRAL: return Character::DIRECTIONALITY_WHITE_SPACE_NEUTRAL;
    case U_OTHER_NEUTRAL:   return Character::DIRECTIONALITY_OTHER_NEUTRALS;
    case U_LEFT_TO_RIGHT_EMBEDDING:
    case U_LEFT_TO_RIGHT_OVERRIDE:
        return Character::DIRECTIONALITY_LEFT_TO_RIGHT_EMBEDDING;
    case U_RIGHT_TO_LEFT_ARABIC:
        return Character::DIRECTIONALITY_RIGHT_TO_LEFT_ARABIC;
    case U_RIGHT_TO_LEFT_EMBEDDING:
    case U_RIGHT_TO_LEFT_OVERRIDE:
        return Character::DIRECTIONALITY_RIGHT_TO_LEFT_EMBEDDING;
    case U_POP_DIRECTIONAL_FORMAT:
        return Character::DIRECTIONALITY_POP_DIRECTIONAL_FORMAT;
    //case U_DIR_NON_SPACING_MARK:return Character::DIRECTIONALITY_NON_SPACING_MARK;
    case U_BOUNDARY_NEUTRAL: return Character::DIRECTIONALITY_BOUNDARY_NEUTRAL;
    default: return Character::DIRECTIONALITY_UNDEFINED;
    }
}
uint8_t Character::getDirectionality(int codePoint) {
    if (getType(codePoint) == Character::UNASSIGNED) {
        return Character::DIRECTIONALITY_UNDEFINED;
    }
    const uint8_t directionality = getDirectionalityImpl(codePoint);
    if (directionality >= 0 && directionality < sizeof(DIRECTIONALITY)/sizeof(DIRECTIONALITY[0])) {
        return DIRECTIONALITY[directionality];
    }
    return Character::DIRECTIONALITY_UNDEFINED;
}


// END Android-changed: Reimplement methods natively on top of ICU4C.

bool Character::isMirrored(char16_t ch) {
    return isMirrored((int)ch);
}

bool Character::isMirrored(int codePoint) {
    return u_isMirrored(codePoint);//isMirroredImpl(codePoint);
}

native static bool isMirroredImpl(int codePoint);
// END Android-changed: Reimplement methods natively on top of ICU4C.

int Character::compareTo(const Character* anotherCharacter) const{
    return compare(this->value, anotherCharacter->value);
}

// END Android-removed: Use ICU.

std::string Character::getName(int codePoint) {
    if (!isValidCodePoint(codePoint)) {
        //row new IllegalArgumentException("Not a valid Unicode code point: 0x%X", codePoint));
    }
    // Android-changed: Use ICU.
    // String name = CharacterName.get(codePoint);
    std::string name = getNameImpl(codePoint);
    if (!name.empty())
        return name;
    if (getType(codePoint) == UNASSIGNED)
        return "";
    const UnicodeBlock* block = UnicodeBlock::of(codePoint);
    if (block != nullptr) {
        std::string blockName = block->toString();
        std::replace(blockName.begin(), blockName.end(), '_', ' ');
        char hexBuffer[9];
        std::snprintf(hexBuffer, sizeof(hexBuffer), "%04X", codePoint);
        return blockName + " " + std::string(hexBuffer);
    }
    // should never come here
    char hexBuffer[9];
    std::snprintf(hexBuffer, sizeof(hexBuffer), "%04X", codePoint);
    return std::string(hexBuffer);
}

int Character::codePointOf(const std::string& name) {
    // Android-changed: Use ICU4C.
    // int cp = CharacterName.getInstance().getCodePoint(name);
    int cp = codePointOfImpl(name);
    if (cp != -1)
        return cp;
    auto off = name.find_last_of(' ');
    if (off != std::string::npos) {
        try {
            cp = std::stoi(name.substr(off + 1), nullptr, 16);
        } catch (...) {
            return 0;
        }
        if (isValidCodePoint(cp) && name == getName(cp))
            return cp;
    }
    return 0;
}
// END Android-removed: expose after CharacterName.getCodePoint() is imported.

// Android-added: Use ICU.
// Implement getNameImpl() and codePointOfImpl() natively.
//private static native String getNameImpl(int codePoint);
//private static native int codePointOfImpl(String name);

// ============================================================================
// Character 内部实现函数（使用基础实现）
// ============================================================================

bool Character::isLowerCaseImpl(int codePoint) {
    return u_charType(codePoint) == U_LOWERCASE_LETTER;
}

bool Character::isUpperCaseImpl(int codePoint) {
    return u_charType(codePoint) == U_UPPERCASE_LETTER;
}

bool Character::isTitleCaseImpl(int codePoint) {
    return u_charType(codePoint) == U_TITLECASE_LETTER;
}

bool Character::isDigitImpl(int codePoint) {
    return u_charType(codePoint) == U_DECIMAL_DIGIT_NUMBER;
}

bool Character::isDefinedImpl(int codePoint) {
    return u_charType(codePoint) != U_UNASSIGNED;
}

bool Character::isLetterImpl(int codePoint) {
    int8_t type = u_charType(codePoint);
    return type >= U_UPPERCASE_LETTER && type <= U_OTHER_LETTER;
}

bool Character::isLetterOrDigitImpl(int codePoint) {
    int8_t type = u_charType(codePoint);
    return (type >= U_UPPERCASE_LETTER && type <= U_OTHER_LETTER) ||
           (type == U_DECIMAL_DIGIT_NUMBER);
}

bool Character::isAlphabeticImpl(int codePoint) {
    int8_t type = u_charType(codePoint);
    return type >= U_UPPERCASE_LETTER && type <= U_OTHER_LETTER;
}

bool Character::isIdeographicImpl(int codePoint) {
    return codePoint >= 0x4E00 && codePoint <= 0x9FFF;
}

bool Character::isUnicodeIdentifierStartImpl(int codePoint) {
    int8_t type = u_charType(codePoint);
    return (type >= U_UPPERCASE_LETTER && type <= U_OTHER_LETTER) ||
           codePoint == '_';
}

bool Character::isUnicodeIdentifierPartImpl(int codePoint) {
    int8_t type = u_charType(codePoint);
    return (type >= U_UPPERCASE_LETTER && type <= U_OTHER_LETTER) ||
           (type >= U_DECIMAL_DIGIT_NUMBER && type <= U_OTHER_NUMBER) ||
           codePoint == '_';
}

bool Character::isIdentifierIgnorableImpl(int codePoint) {
    return false;
}

int Character::toLowerCaseImpl(int codePoint) {
    if (codePoint >= 'A' && codePoint <= 'Z') {
        return codePoint + ('a' - 'A');
    }
    return codePoint;
}

int Character::toUpperCaseImpl(int codePoint) {
    if (codePoint >= 'a' && codePoint <= 'z') {
        return codePoint - ('a' - 'A');
    }
    return codePoint;
}

int Character::toTitleCaseImpl(int codePoint) {
    if (codePoint >= 'a' && codePoint <= 'z') {
        return codePoint - ('a' - 'A');
    }
    return codePoint;
}

int Character::digitImpl(int codePoint, int radix) {
    if (radix < 2 || radix > 36) {
        return -1;
    }
    if (codePoint >= '0' && codePoint <= '9') {
        int digit = codePoint - '0';
        return (digit < radix) ? digit : -1;
    }
    if (codePoint >= 'A' && codePoint <= 'Z') {
        int digit = 10 + (codePoint - 'A');
        return (digit < radix) ? digit : -1;
    }
    if (codePoint >= 'a' && codePoint <= 'z') {
        int digit = 10 + (codePoint - 'a');
        return (digit < radix) ? digit : -1;
    }
    return -1;
}

int Character::getNumericValueImpl(int codePoint) {
    if (codePoint >= '0' && codePoint <= '9') {
        return codePoint - '0';
    }
    return -1;
}

bool Character::isSpaceCharImpl(int codePoint) {
    return codePoint == ' ' || codePoint == '\t' || 
           codePoint == '\n' || codePoint == '\r';
}

bool Character::isWhitespaceImpl(int codePoint) {
    return u_isWhiteSpace(codePoint);
}

int Character::getTypeImpl(int codePoint) {
    return u_charType(codePoint);
}

unsigned char Character::getDirectionalityImpl(int codePoint) {
    return u_charDirection(codePoint);
}

std::string Character::getNameImpl(int codePoint) {
    return "";
}

int Character::codePointOfImpl(const std::string& name) {
    return codePointOf(name);
}

}/*endof namespace*/
