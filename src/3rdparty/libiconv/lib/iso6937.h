#define CAS9_V6 1
#define U16_TO_UNICODE(x) (x)
#define BYTES_PER_CHAR 2
#define UINT16 unsigned short
struct u8u16{
    unsigned char u8;
    unsigned short u16;
};
struct lookup_table{
    struct u8u16* table;
    unsigned int count;
};
static unsigned short iso6937_map_a8_bf[] =
{
    0x00A4,     /*A8    CURRENCY SIGN */
    0x2018,     /*A9    LEFT SINGLE QUOTATION MARK */
    0x201C,     /*AA    LEFT DOUBLE QUOTATION MARK */
    0x00AB,     /*AB    LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */
    0x2190,     /*AC    LEFTWARDS ARROW */
    0x2191,     /*AD    UPWARDS ARROW */
    0x2192,     /*AE    RIGHTWARDS ARROW */
    0x2193,     /*AF    DOWNWARDS ARROW */
    0x00B0,     /*B0    DEGREE SIGN */
    0x00B1,     /*B1    PLUS-MINUS SIGN */
    0x00B2,     /*B2    SUPERSCRIPT TWO */
    0x00B3,     /*B3    SUPERSCRIPT THREE */
    0x00D7,     /*B4    MULTIPLICATION SIGN */
    0x00B5,     /*B5    MICRO SIGN */
    0x00B6,     /*B6    PILCROW SIGN */
    0x00B7,     /*B7    MIDDLE DOT */
    0x00F7,     /*B8    DIVISION SIGN */
    0x2019,     /*B9    RIGHT SINGLE QUOTATION MARK */
    0x201D,     /*BA    RIGHT DOUBLE QUOTATION MARK */
    0x00BB,     /*BB    RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */
    0x00BC,     /*BC    VULGAR FRACTION ONE QUARTER */
    0x00BD,     /*BD    VULGAR FRACTION ONE HALF */
    0x00BE,     /*BE    VULGAR FRACTION THREE QUARTERS */
    0x00BF,     /*BF    INVERTED QUESTION MARK */
};
static unsigned short iso6937_map_d0_ff[] =
{
    0x2014,     /*D0    EM DASH */
    0x00B9,     /*D1    SUPERSCRIPT ONE */
    0x00AE,     /*D2    REGISTERED SIGN */
    0x00A9,     /*D3    COPYRIGHT SIGN */
    0x2122,     /*D4    TRADE MARK SIGN */
    0x266A,     /*D5    EIGHTH NOTE */
    0x00AC,     /*D6    NOT SIGN */
    0x00A6,     /*D7    BROKEN BAR */
    0xFFFF,     /*D8    INVALID */
    0xFFFF,     /*D9    INVALID */
    0xFFFF,     /*DA    INVALID */
    0xFFFF,     /*DB    INVALID */
    0x215B,     /*DC    VULGAR FRACTION ONE EIGHTH */
    0x215C,     /*DD    VULGAR FRACTION THREE EIGHTHS */
    0x215D,     /*DE    VULGAR FRACTION FIVE EIGHTHS */
    0x215E,     /*DF    VULGAR FRACTION SEVEN EIGHTHS */
    0x2126,     /*E0    OHM SIGN */
    0x00C6,     /*E1    LATIN CAPITAL LETTER AE */
    0x00D0,     /*E2    LATIN CAPITAL LETTER ETH (Icelandic) */
    0x00AA,     /*E3    FEMININE ORDINAL INDICATOR */
    0x0126,     /*E4    LATIN CAPITAL LETTER H WITH STROKE */
    0xFFFF,     /*E5    INVALID */
    0x0132,     /*E6    LATIN CAPITAL LIGATURE IJ */
    0x013F,     /*E7    LATIN CAPITAL LETTER L WITH MIDDLE DOT */
    0x0141,     /*E8    LATIN CAPITAL LETTER L WITH STROKE */
    0x00D8,     /*E9    LATIN CAPITAL LETTER O WITH STROKE */
    0x0152,     /*EA    LATIN CAPITAL LIGATURE OE */
    0x00BA,     /*EB    MASCULINE ORDINAL INDICATOR */
    0x00DE,     /*EC    LATIN CAPITAL LETTER THORN (Icelandic) */
    0x0166,     /*ED    LATIN CAPITAL LETTER T WITH STROKE */
    0x014A,     /*EE    LATIN CAPITAL LETTER ENG (Sami) */
    0x0149,     /*EF    LATIN SMALL LETTER N PRECEDED BY APOSTROPHE */
    0x0138,     /*F0    LATIN SMALL LETTER KRA (Greenlandic) */
    0x00E6,     /*F1    LATIN SMALL LETTER AE */
    0x0111,     /*F2    LATIN SMALL LETTER D WITH STROKE */
    0x00F0,     /*F3    LATIN SMALL LETTER ETH (Icelandic) */
    0x0127,     /*F4    LATIN SMALL LETTER H WITH STROKE */
    0x0131,     /*F5    LATIN SMALL LETTER DOTLESS I */
    0x0133,     /*F6    LATIN SMALL LIGATURE IJ */
    0x0140,     /*F7    LATIN SMALL LETTER L WITH MIDDLE DOT */
    0x0142,     /*F8    LATIN SMALL LETTER L WITH STROKE */
    0x00F8,     /*F9    LATIN SMALL LETTER O WITH STROKE */
    0x0153,     /*FA    LATIN SMALL LIGATURE OE */
    0x00DF,     /*FB    LATIN SMALL LETTER SHARP S (German) */
    0x00FE,     /*FC    LATIN SMALL LETTER THORN (Icelandic) */
    0x0167,     /*FD    LATIN SMALL LETTER T WITH STROKE */
    0x014B,     /*FE    LATIN SMALL LETTER ENG (Sami) */
    0x00AD,     /*FF    SOFT HYPHEN */
};
static struct u8u16 iso6937_c2[] =
{
    {0x20, 0x00B4},    /* ACUTE ACCEN */
    {0x41, 0x00C1},    /* LATIN CAPITAL LETTER A WITH ACUT */
    {0x43, 0x0106},    /* LATIN CAPITAL LETTER C WITH ACUT */
    {0x45, 0x00C9},    /* LATIN CAPITAL LETTER E WITH ACUT */
    {0x49, 0x00CD},    /* LATIN CAPITAL LETTER I WITH ACUT */
    {0x4C, 0x0139},    /* LATIN CAPITAL LETTER L WITH ACUT */
    {0x4E, 0x0143},    /* LATIN CAPITAL LETTER N WITH ACUT */
    {0x4F, 0x00D3},    /* LATIN CAPITAL LETTER O WITH ACUT */
    {0x52, 0x0154},    /* LATIN CAPITAL LETTER R WITH ACUT */
    {0x53, 0x015A},    /* LATIN CAPITAL LETTER S WITH ACUT */
    {0x55, 0x00DA},    /* LATIN CAPITAL LETTER U WITH ACUT */
    {0x59, 0x00DD},    /* LATIN CAPITAL LETTER Y WITH ACUT */
    {0x5A, 0x0179},    /* LATIN CAPITAL LETTER Z WITH ACUT */
    {0x61, 0x00E1},    /* LATIN SMALL LETTER A WITH ACUT */
    {0x63, 0x0107},    /* LATIN SMALL LETTER C WITH ACUT */
    {0x65, 0x00E9},    /* LATIN SMALL LETTER E WITH ACUT */
#ifdef CAS9_V6 //char_selected
    {0x67, 0x01F5},    /* LATIN SMALL LETTER G WITH ACUT */
#endif
    {0x69, 0x00ED},    /* LATIN SMALL LETTER I WITH ACUT */
    {0x6C, 0x013A},    /* LATIN SMALL LETTER L WITH ACUT */
    {0x6E, 0x0144},    /* LATIN SMALL LETTER N WITH ACUT */
    {0x6F, 0x00F3},    /* LATIN SMALL LETTER O WITH ACUT */
    {0x72, 0x0155},    /* LATIN SMALL LETTER R WITH ACUT */
    {0x73, 0x015B},    /* LATIN SMALL LETTER S WITH ACUT */
    {0x75, 0x00FA},    /* LATIN SMALL LETTER U WITH ACUT */
    {0x79, 0x00FD},    /* LATIN SMALL LETTER Y WITH ACUT */
    {0x7A, 0x017A},    /* LATIN SMALL LETTER Z WITH ACUT */
};
static struct u8u16 iso6937_c6[] =
{
    {0x20, 0x02D8},    /* BREV */
    {0x41, 0x0102},    /* LATIN CAPITAL LETTER A WITH BREV */
    {0x47, 0x011E},    /* LATIN CAPITAL LETTER G WITH BREV */
    {0x55, 0x016C},    /* LATIN CAPITAL LETTER U WITH BREV */
    {0x61, 0x0103},    /* LATIN SMALL LETTER A WITH BREV */
    {0x67, 0x011F},    /* LATIN SMALL LETTER G WITH BREV */
    {0x75, 0x016D},    /* LATIN SMALL LETTER U WITH BREV */
};
static struct u8u16 iso6937_cf[] =
{
    {0x20, 0x02C7},    /* CARON (Mandarin Chinese third tone */
    {0x43, 0x010C},    /* LATIN CAPITAL LETTER C WITH CARO */
    {0x44, 0x010E},    /* LATIN CAPITAL LETTER D WITH CARO */
    {0x45, 0x011A},    /* LATIN CAPITAL LETTER E WITH CARO */
    {0x4C, 0x013D},    /* LATIN CAPITAL LETTER L WITH CARO */
    {0x4E, 0x0147},    /* LATIN CAPITAL LETTER N WITH CARO */
    {0x52, 0x0158},    /* LATIN CAPITAL LETTER R WITH CARO */
    {0x53, 0x0160},    /* LATIN CAPITAL LETTER S WITH CARO */
    {0x54, 0x0164},    /* LATIN CAPITAL LETTER T WITH CARO */
    {0x5A, 0x017D},    /* LATIN CAPITAL LETTER Z WITH CARO */
    {0x63, 0x010D},    /* LATIN SMALL LETTER C WITH CARO */
    {0x64, 0x010F},    /* LATIN SMALL LETTER D WITH CARO */
    {0x65, 0x011B},    /* LATIN SMALL LETTER E WITH CARO */
    {0x6C, 0x013E},    /* LATIN SMALL LETTER L WITH CARO */
    {0x6E, 0x0148},    /* LATIN SMALL LETTER N WITH CARO */
    {0x72, 0x0159},    /* LATIN SMALL LETTER R WITH CARO */
    {0x73, 0x0161},    /* LATIN SMALL LETTER S WITH CARO */
    {0x74, 0x0165},    /* LATIN SMALL LETTER T WITH CARO */
    {0x7A, 0x017E},    /* LATIN SMALL LETTER Z WITH CARO */
};
static struct u8u16 iso6937_cb[] =
{
    {0x20, 0x00B8},    /* CEDILL */
    {0x43, 0x00C7},    /* LATIN CAPITAL LETTER C WITH CEDILL */
    {0x47, 0x0122},    /* LATIN CAPITAL LETTER G WITH CEDILL */
    {0x4B, 0x0136},    /* LATIN CAPITAL LETTER K WITH CEDILL */
    {0x4C, 0x013B},    /* LATIN CAPITAL LETTER L WITH CEDILL */
    {0x4E, 0x0145},    /* LATIN CAPITAL LETTER N WITH CEDILL */
    {0x52, 0x0156},    /* LATIN CAPITAL LETTER R WITH CEDILL */
    {0x53, 0x015E},    /* LATIN CAPITAL LETTER S WITH CEDILL */
    {0x54, 0x0162},    /* LATIN CAPITAL LETTER T WITH CEDILL */
    {0x63, 0x00E7},    /* LATIN SMALL LETTER C WITH CEDILL */
    {0x67, 0x0123},    /* LATIN SMALL LETTER G WITH CEDILL */
    {0x6B, 0x0137},    /* LATIN SMALL LETTER K WITH CEDILL */
    {0x6C, 0x013C},    /* LATIN SMALL LETTER L WITH CEDILL */
    {0x6E, 0x0146},    /* LATIN SMALL LETTER N WITH CEDILL */
    {0x72, 0x0157},    /* LATIN SMALL LETTER R WITH CEDILL */
    {0x73, 0x015F},    /* LATIN SMALL LETTER S WITH CEDILL */
    {0x74, 0x0163},    /* LATIN SMALL LETTER T WITH CEDILL */
};
static struct u8u16 iso6937_c3[] =
{
    {0x41, 0x00C2},    /* LATIN CAPITAL LETTER A WITH CIRCUMFLE */
    {0x43, 0x0108},    /* LATIN CAPITAL LETTER C WITH CIRCUMFLE */
    {0x45, 0x00CA},    /* LATIN CAPITAL LETTER E WITH CIRCUMFLE */
    {0x47, 0x011C},    /* LATIN CAPITAL LETTER G WITH CIRCUMFLE */
    {0x48, 0x0124},    /* LATIN CAPITAL LETTER H WITH CIRCUMFLE */
    {0x49, 0x00CE},    /* LATIN CAPITAL LETTER I WITH CIRCUMFLE */
    {0x4A, 0x0134},    /* LATIN CAPITAL LETTER J WITH CIRCUMFLE */
    {0x4F, 0x00D4},    /* LATIN CAPITAL LETTER O WITH CIRCUMFLE */
    {0x53, 0x015C},    /* LATIN CAPITAL LETTER S WITH CIRCUMFLE */
    {0x55, 0x00DB},    /* LATIN CAPITAL LETTER U WITH CIRCUMFLE */
    {0x57, 0x0174},    /* LATIN CAPITAL LETTER W WITH CIRCUMFLE */
    {0x59, 0x0176},    /* LATIN CAPITAL LETTER Y WITH CIRCUMFLE */
    {0x61, 0x00E2},    /* LATIN SMALL LETTER A WITH CIRCUMFLE */
    {0x63, 0x0109},    /* LATIN SMALL LETTER C WITH CIRCUMFLE */
    {0x65, 0x00EA},    /* LATIN SMALL LETTER E WITH CIRCUMFLE */
    {0x67, 0x011D},    /* LATIN SMALL LETTER G WITH CIRCUMFLE */
    {0x68, 0x0125},    /* LATIN SMALL LETTER H WITH CIRCUMFLE */
    {0x69, 0x00EE},    /* LATIN SMALL LETTER I WITH CIRCUMFLE */
    {0x6A, 0x0135},    /* LATIN SMALL LETTER J WITH CIRCUMFLE */
    {0x6F, 0x00F4},    /* LATIN SMALL LETTER O WITH CIRCUMFLE */
    {0x73, 0x015D},    /* LATIN SMALL LETTER S WITH CIRCUMFLE */
    {0x75, 0x00FB},    /* LATIN SMALL LETTER U WITH CIRCUMFLE */
    {0x77, 0x0175},    /* LATIN SMALL LETTER W WITH CIRCUMFLE */
    {0x79, 0x0177},    /* LATIN SMALL LETTER Y WITH CIRCUMFLE */
};
static struct u8u16 iso6937_c8[] =
{
    {0x20, 0x00A8},    /* DIAERESI */
    {0x41, 0x00C4},    /* LATIN CAPITAL LETTER A WITH DIAERESI */
    {0x45, 0x00CB},    /* LATIN CAPITAL LETTER E WITH DIAERESI */
    {0x49, 0x00CF},    /* LATIN CAPITAL LETTER I WITH DIAERESI */
    {0x4F, 0x00D6},    /* LATIN CAPITAL LETTER O WITH DIAERESI */
    {0x55, 0x00DC},    /* LATIN CAPITAL LETTER U WITH DIAERESI */
    {0x59, 0x0178},    /* LATIN CAPITAL LETTER Y WITH DIAERESI */
    {0x61, 0x00E4},    /* LATIN SMALL LETTER A WITH DIAERESI */
    {0x65, 0x00EB},    /* LATIN SMALL LETTER E WITH DIAERESI */
    {0x69, 0x00EF},    /* LATIN SMALL LETTER I WITH DIAERESI */
    {0x6F, 0x00F6},    /* LATIN SMALL LETTER O WITH DIAERESI */
    {0x75, 0x00FC},    /* LATIN SMALL LETTER U WITH DIAERESI */
    {0x79, 0x00FF},    /* LATIN SMALL LETTER Y WITH DIAERESI */
};
static struct u8u16 iso6937_c7[] =
{
    {0x20, 0x02D9},    /* DOT ABOVE (Mandarin Chinese light tone */
    {0x43, 0x010A},    /* LATIN CAPITAL LETTER C WITH DOT ABOV */
    {0x45, 0x0116},    /* LATIN CAPITAL LETTER E WITH DOT ABOV */
    {0x47, 0x0120},    /* LATIN CAPITAL LETTER G WITH DOT ABOV */
    {0x49, 0x0130},    /* LATIN CAPITAL LETTER I WITH DOT ABOV */
    {0x5A, 0x017B},    /* LATIN CAPITAL LETTER Z WITH DOT ABOV */
    {0x63, 0x010B},    /* LATIN SMALL LETTER C WITH DOT ABOV */
    {0x65, 0x0117},    /* LATIN SMALL LETTER E WITH DOT ABOV */
    {0x67, 0x0121},    /* LATIN SMALL LETTER G WITH DOT ABOV */
#ifdef CAS9_V6 //char_selected
    {0x69, 0x0131},    /* LATIN SMALL LETTER DOTLESS I */
#endif
    {0x7A, 0x017C},    /* LATIN SMALL LETTER Z WITH DOT ABOV */
};
static struct u8u16 iso6937_cd[] =
{
    {0x20, 0x02DD},    /* DOUBLE ACUTE ACCEN */
    {0x4F, 0x0150},    /* LATIN CAPITAL LETTER O WITH DOUBLE ACUT */
    {0x55, 0x0170},    /* LATIN CAPITAL LETTER U WITH DOUBLE ACUT */
    {0x6F, 0x0151},    /* LATIN SMALL LETTER O WITH DOUBLE ACUT */
    {0x75, 0x0171},    /* LATIN SMALL LETTER U WITH DOUBLE ACUT */
};

static struct u8u16 iso6937_c1[] =
{
    {0x41, 0x00C0},    /* LATIN CAPITAL LETTER A WITH GRAV */
    {0x45, 0x00C8},    /* LATIN CAPITAL LETTER E WITH GRAV */
    {0x49, 0x00CC},    /* LATIN CAPITAL LETTER I WITH GRAV */
    {0x4F, 0x00D2},    /* LATIN CAPITAL LETTER O WITH GRAV */
    {0x55, 0x00D9},    /* LATIN CAPITAL LETTER U WITH GRAV */
    {0x61, 0x00E0},    /* LATIN SMALL LETTER A WITH GRAV */
    {0x65, 0x00E8},    /* LATIN SMALL LETTER E WITH GRAV */
    {0x69, 0x00EC},    /* LATIN SMALL LETTER I WITH GRAV */
    {0x6F, 0x00F2},    /* LATIN SMALL LETTER O WITH GRAV */
    {0x75, 0x00F9},    /* LATIN SMALL LETTER U WITH GRAV */
};
static struct u8u16 iso6937_c5[] =
{
    {0x20, 0x00AF},    /* MACRO */
    {0x41, 0x0100},    /* LATIN CAPITAL LETTER A WITH MACRO */
    {0x45, 0x0112},    /* LATIN CAPITAL LETTER E WITH MACRO */
    {0x49, 0x012A},    /* LATIN CAPITAL LETTER I WITH MACRO */
    {0x4F, 0x014C},    /* LATIN CAPITAL LETTER O WITH MACRO */
    {0x55, 0x016A},    /* LATIN CAPITAL LETTER U WITH MACRO */
    {0x61, 0x0101},    /* LATIN SMALL LETTER A WITH MACRO */
    {0x65, 0x0113},    /* LATIN SMALL LETTER E WITH MACRO */
    {0x69, 0x012B},    /* LATIN SMALL LETTER I WITH MACRO */
    {0x6F, 0x014D},    /* LATIN SMALL LETTER O WITH MACRO */
    {0x75, 0x016B},    /* LATIN SMALL LETTER U WITH MACRO */
};
static struct u8u16 iso6937_ce[] =
{
    {0x20, 0x02DB},    /* OGONE */
    {0x41, 0x0104},    /* LATIN CAPITAL LETTER A WITH OGONE */
    {0x45, 0x0118},    /* LATIN CAPITAL LETTER E WITH OGONE */
    {0x49, 0x012E},    /* LATIN CAPITAL LETTER I WITH OGONE */
    {0x55, 0x0172},    /* LATIN CAPITAL LETTER U WITH OGONE */
    {0x61, 0x0105},    /* LATIN SMALL LETTER A WITH OGONE */
    {0x65, 0x0119},    /* LATIN SMALL LETTER E WITH OGONE */
    {0x69, 0x012F},    /* LATIN SMALL LETTER I WITH OGONE */
    {0x75, 0x0173},    /* LATIN SMALL LETTER U WITH OGONE */
};
static struct u8u16 iso6937_ca[] =
{
    {0x20, 0x02DA},    /* RING ABOV */
    {0x41, 0x00C5},    /* LATIN CAPITAL LETTER A WITH RING ABOV */
    {0x55, 0x016E},    /* LATIN CAPITAL LETTER U WITH RING ABOV */
    {0x61, 0x00E5},    /* LATIN SMALL LETTER A WITH RING ABOV */
    {0x75, 0x016F},    /* LATIN SMALL LETTER U WITH RING ABOV */
};
static struct u8u16 iso6937_c4[] =
{
    {0x41, 0x00C3},    /* LATIN CAPITAL LETTER A WITH TILD */
    {0x49, 0x0128},    /* LATIN CAPITAL LETTER I WITH TILD */
    {0x4E, 0x00D1},    /* LATIN CAPITAL LETTER N WITH TILD */
    {0x4F, 0x00D5},    /* LATIN CAPITAL LETTER O WITH TILD */
    {0x55, 0x0168},    /* LATIN CAPITAL LETTER U WITH TILD */
    {0x61, 0x00E3},    /* LATIN SMALL LETTER A WITH TILD */
    {0x69, 0x0129},    /* LATIN SMALL LETTER I WITH TILD */
    {0x6E, 0x00F1},    /* LATIN SMALL LETTER N WITH TILD */
    {0x6F, 0x00F5},    /* LATIN SMALL LETTER O WITH TILD */
    {0x75, 0x0169},    /* LATIN SMALL LETTER U WITH TILD */
};
static struct lookup_table  iso6937_lookup_table[] =
{
    { iso6937_c1, 10 },
#ifdef CAS9_V6  //char_selected
    { iso6937_c2, 26 },
#else
    { iso6937_c2, 25 },
#endif
    { iso6937_c3, 24 },
    { iso6937_c4, 10 },
    { iso6937_c5, 11 },
    { iso6937_c6, 7 },
#ifdef CAS9_V6  //char_selected
    { iso6937_c7, 11 },
#else
    { iso6937_c7, 10 },
#endif
    { iso6937_c8, 13 },
    { NULL, 0 },
    { iso6937_ca, 5 },
    { iso6937_cb, 17 },
    { NULL, 0 },
    { iso6937_cd, 5 },
    { iso6937_ce, 9 },
    { iso6937_cf, 19 },
};
static UINT16   iso6937_map_c0_cf[] =
{
    0xFFFF,     /*C0    INVALID */
    0x0060,     /*C1    Grave */
    0x00B4,     /*C2    Acute */
    0x005E,     /*C3    Circumflex */
    0x007E,     /*C4    Tilde */
    0x00AF,     /*C5    Macron */
    0x02D8,     /*C6    Breve */
    0x02D9,     /*C7    Dot */
    0x00A8,     /*C8    Umlaut(DIAERESIS) */
    0xFFFF,     /*C9    INVALID */
    0x02DA,     /*CA    Ring */
    0x00B8,     /*CB    Cedilla */
    0xFFFF,     /*CC    INVALID */
    0x02DD,     /*CD    DoubleAcute */
    0x02DB,     /*CE    Ogonek */
    0x02C7,     /*CF    Caron */
};
static unsigned int u8u16_lookup(unsigned short ch,const struct u8u16*table,unsigned int count){
    int i;
    for(i=0;i<count;i++)
       if(table[i].u8==ch)return table[i].u16;
    return 0xFFFF;
}
static int
iso6937_mbtowc (conv_t conv, ucs4_t *pwc, const unsigned char *iso6937, int length)
{
    unsigned int si=0;
    unsigned int di=0;
    unsigned short src,tmp,value;
    struct lookup_table *entry = NULL;
    ucs4_t* unicode=pwc;
    src = *iso6937;
    if ((src < 0xA8)&&(src>0)){
        if ((0x8A == src) || ('\r' == src) || ('\n' == src)){
             unicode[di] = U16_TO_UNICODE((UINT16) '\n');
             di++;
        }else if (0xA4 == src){
             //Euro currency mark ??
             unicode[di] = U16_TO_UNICODE((UINT16) 0x20AC);
             di++;
        }else if (0xA0 == src){
             /* NBSP*/
             unicode[di] = U16_TO_UNICODE((UINT16) 0x20);
             di++;
        }else if (((src >= 0x20) && (src < 0x80)) || (src > 0x9F)){
             unicode[di] = U16_TO_UNICODE((UINT16) iso6937[si]);
             di++;
        }
    }else if (src <= 0xBF){       /* A8-BF table */
        unicode[di] = U16_TO_UNICODE(iso6937_map_a8_bf[src - 0xA8]);
        di++;
    }else if (src < 0xD0){       /* C0-CF table */
        if ((length - si) < BYTES_PER_CHAR){
            //071206, last one char
            unicode[di] = U16_TO_UNICODE(iso6937_map_c0_cf[src - 0xC0]);
            di++;
            return 1;
        }else if ((src != 0xC0) && (src != 0xC9) && (src != 0xCC)){
            entry = &iso6937_lookup_table[src - 0xC1];
            ++si;
            src = iso6937[si];
            value = u8u16_lookup(src, entry->table, entry->count);
            if (0xFFFF == value){
                tmp = iso6937_map_c0_cf[iso6937[si - 1] - 0xC0];
                unicode[di] = U16_TO_UNICODE(tmp);
                di++;
                si--;           //continue to process next one char
            }else{
                unicode[di] = U16_TO_UNICODE(value);
                di++;
            }
        }
   }
        //"SHY" need to special process, UI will process it according to
        // the display situation, so don't convert it to hyphen character
   else if (0xFF == src){
       unicode[di] = 0xFFFF; //U16_TO_UNICODE((UINT16)0x0C);
       di++;
   }else{   /* D0-FF table */
       unicode[di] = U16_TO_UNICODE(iso6937_map_d0_ff[src - 0xD0]);
       di++;
   }
   si++;
   return si;
}

static int
iso6937_wctomb (conv_t conv, unsigned char *r, ucs4_t wc, int n){
   printf("=====%s is TODO=====\r\n",__FUNCTION__);
   return 1;
}
