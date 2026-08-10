int pxlenght(int lenght, int offset=0){return (int)((float)(lenght+offset)*scale);}
int rpxlenght(int lenght, int offset=0){return (int)((float)(lenght+offset)/scale);}
std::vector<std::string> utf8_split_graphemes(const std::string& s) {
    std::vector<std::string> out;
    size_t i = 0;
    while (i < s.size()) {
        unsigned char c = (unsigned char)s[i];
        size_t len = 1;
        if ((c & 0x80) == 0) len = 1;             // ASCII
        else if ((c & 0xE0) == 0xC0) len = 2;    // 2-byte
        else if ((c & 0xF0) == 0xE0) len = 3;    // 3-byte
        else if ((c & 0xF8) == 0xF0) len = 4;    // 4-byte
        else {
            // Neplatný UTF-8, můžeme volit, jak naložit
            len = 1;
        }
        out.push_back(s.substr(i, len));
        i += len;
    }
    return out;
}
int stringlen(std::string strrr){return utf8_split_graphemes(strrr).size();}
bool isNumber(const std::string& str) {
    for (char c : str) {
        if (!isdigit(c)) return false;
    }
    return !str.empty(); // aby "" nevrátilo true
}
int stringToInt(const std::string& str) {
    try {
        return std::stoi(str); // použije C++ funkci, vyhodí výjimku pokud failne
    } catch (...) {
        return 0; // fallback, když to není platné číslo
    }
}
SDL_Color hexStringToColor(std::string* hexStr) {
    std::string hex = *hexStr;

    auto isValidHexChar = [](char c) -> bool {
        return std::isxdigit(static_cast<unsigned char>(c));
    };

    auto hexToInt = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
        if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
        return 0;
    };

    if (hex.size() != 7 || hex[0] != '#' || !std::all_of(hex.begin() + 1, hex.end(), isValidHexChar)) {
        printf("[hexStringToColor] Invalid hex code: \"%s\"\n", hex.c_str());
        *hexStr = "#FFAA44";
        return SDL_Color{255, 200, 100, 255};
    }

    int r = hexToInt(hex[1]) * 16 + hexToInt(hex[2]);
    int g = hexToInt(hex[3]) * 16 + hexToInt(hex[4]);
    int b = hexToInt(hex[5]) * 16 + hexToInt(hex[6]);

    return SDL_Color{Uint8(r), Uint8(g), Uint8(b), 255};
}
bool tobool(int val){
    if (val==1){return true;}
    if (val==0){return false;}
    return false;
}
std::string colorToHexString(const SDL_Color& color) {
    std::ostringstream ss;
    ss << "#" 
       << std::uppercase << std::setfill('0') << std::hex
       << std::setw(2) << int(color.r)
       << std::setw(2) << int(color.g)
       << std::setw(2) << int(color.b);
    return ss.str();
}
int booltoint(bool val){
    if (val){return 1;}
    return 0;
}
void emptyfunc(){return;}
float max(float number){
    if (number>0.0f){
        return number;
    }
    return -number;
}
int max(int number){
    if (number>0){
        return number;
    }
    return -number;
}
void replace_text_range(std::string* inputptr, int start, int end, const std::string& replacement) {
    auto chars = utf8_split_graphemes(*inputptr);
    if (start < 0) start = 0;
    if (end > (int)chars.size()) end = (int)chars.size();
    if (start > end) std::swap(start, end);

    std::string result;
    for (int i = 0; i < start; ++i) result += chars[i];
    result += replacement;
    for (int i = end; i < (int)chars.size(); ++i) result += chars[i];

    *inputptr = result;
}