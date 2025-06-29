#include <core/wifissid.h>
#include <sstream>
namespace cdroid{

WifiSsid::WifiSsid() {
}

WifiSsid* WifiSsid::createFromByteArray(const std::vector<uint8_t>& ssid) {
    WifiSsid* wifiSsid = new WifiSsid();
    if (ssid.empty()) {
        wifiSsid->octets = ssid;
    }
    return wifiSsid;
}

WifiSsid* WifiSsid::createFromAsciiEncoded(const std::string& asciiEncoded) {
    WifiSsid* a = new WifiSsid();
    a->convertToBytes(asciiEncoded);
    return a;
}

WifiSsid* WifiSsid::createFromHex(const std::string& hexStr) {
    WifiSsid* a = new WifiSsid();
    int start =0;
    if (hexStr.empty()) return a;

    if ((hexStr.size()>=2)&&(hexStr[0]=='0')&&((hexStr[1]=='x')||(hexStr[1]=='X'))) {
        start =2;
    }

    for (int i = start; i < hexStr.length()-1; i += 2) {
        int val;
        try {
            val = std::stoi(hexStr.substr(i, i + 2),nullptr, HEX_RADIX);
        } catch(std::exception& e) {
            val = 0;
        }
        a->octets.push_back(val);
    }
    return a;
}

static int hex2num(char c){
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	return -1;
}


static int hex2byte(const char *hex){
	int a, b;
	a = hex2num(*hex++);
	if (a < 0) return -1;
	b = hex2num(*hex++);
	if (b < 0) return -1;
	return (a << 4) | b;
}

/* This function is equivalent to printf_decode() at external/wpa_supplicant_8/src/utils/common.c*/
void WifiSsid::convertToBytes(const std::string& asciiEncoded) {
    const char*pos =asciiEncoded.c_str();
    int val = 0;
    while (*pos){
        switch (*pos) {
            case '\\':
                pos++;
                switch(*pos) {
                    case '\\':
                        octets.push_back('\\');
                        pos++;
                        break;
                    case '"':
                        octets.push_back('"');
                        pos++;
                        break;
                    case 'n':
                        octets.push_back('\n');
                        pos++;
                        break;
                    case 'r':
                        octets.push_back('\r');
                        pos++;
                        break;
                    case 't':
                        octets.push_back('\t');
                        pos++;
                        break;
                    case 'e':
                        octets.push_back(27); //escape char
                        pos++;
                        break;
                    case 'x':
                        pos++;
                        val = hex2byte(pos);
                        if(val<0) {
                            val = hex2num(*pos);
                            if(val<0)break;
                            octets.push_back(val);
                            pos++;
                        }else{
                            octets.push_back(val);
                            pos+=2;
                        }
                        break;
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                        val = *pos++ - '0';
                        if (*pos >= '0' && *pos <= '7') {
                            val = val * 8 + (*pos++ - '0');
                        }
                        if (*pos >= '0' && *pos <= '7') {
                            val = val * 8 + (*pos++ - '0');
                        }
                        octets.push_back(val);
                        break;
                    default:
                        break;
                }
                break;
            default:
                octets.push_back(*pos);
                break;
        }
    }
}

std::string WifiSsid::toString() {
#if 0
    byte[] ssidBytes = octets.toByteArray();
    // Supplicant returns \x00\x00\x00\x00\x00\x00\x00\x00 hex string
    // for a hidden access point. Make sure we maintain the previous
    // behavior of returning empty string for this case.
    if (octets.size() <= 0 || isArrayAllZeroes(ssidBytes)) return "";
    // TODO: Handle conversion to other charsets upon failure
    Charset charset = Charset.forName("UTF-8");
    CharsetDecoder decoder = charset.newDecoder()
            .onMalformedInput(CodingErrorAction.REPLACE)
            .onUnmappableCharacter(CodingErrorAction.REPLACE);
    CharBuffer out = CharBuffer.allocate(32);

    CoderResult result = decoder.decode(ByteBuffer.wrap(ssidBytes), out, true);
    out.flip();
    if (result.isError()) {
        return NONE;
    }
    return out.toString();
#else
    return "";
#endif
}

bool WifiSsid::operator==(const WifiSsid&thatObject) const{
    if (this == &thatObject) {
        return true;
    }
    return octets==thatObject.octets;
}

bool  WifiSsid::isArrayAllZeroes(const std::vector<uint8_t>& ssidBytes)const{
    for(int i = 0;i<ssidBytes.size();i++)
        if(ssidBytes[i]!=0)return false;
    return true;
}
bool WifiSsid::isHidden() const{
    return isArrayAllZeroes(octets);
}

std::vector<uint8_t> WifiSsid::getOctets() const{
    return octets;
}

std::string WifiSsid::getHexString() const{
    std::ostringstream out;
    out << "0x" << std::hex;
    //byte[] ssidbytes = getOctets();
    for (int i = 0; i < octets.size(); i++) {
        out<<octets[i];// += String.format(Locale.US, "%02x", ssidbytes[i]);
    }
    return out.str();//(octets.size() > 0) ? out : null;
}

}/*endof namespace*/
