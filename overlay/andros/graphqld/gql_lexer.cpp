#include <cctype>
#include <cstring>

#include "gql.h"

namespace andros {

bool Tokenize(const std::string& src, std::vector<Token>* out, std::string* error) {
    size_t i = 0;
    while (i < src.size()) {
        char c = src[i];
        if (isspace((unsigned char)c) || c == ',') { ++i; continue; }
        if (c == '#') { while (i < src.size() && src[i] != '\n') ++i; continue; }
        Token t;
        if (isalpha((unsigned char)c) || c == '_') {
            t.type = Token::kName;
            while (i < src.size() && (isalnum((unsigned char)src[i]) || src[i] == '_'))
                t.text += src[i++];
        } else if (isdigit((unsigned char)c) || c == '-' || c == '.') {
            t.type = Token::kNum;
            while (i < src.size() && strchr("0123456789+-.eE", src[i])) t.text += src[i++];
        } else if (c == '"') {
            t.type = Token::kStr;
            for (++i; i < src.size() && src[i] != '"'; ++i) {
                if (src[i] == '\\' && i + 1 < src.size()) ++i;
                t.text += src[i];
            }
            if (i >= src.size()) { *error = "unterminated string"; return false; }
            ++i;
        } else if (strchr("{}()[]:$!=@", c)) {
            t.type = Token::kPunct;
            t.text = std::string(1, c);
            ++i;
        } else {
            *error = std::string("unexpected character '") + c + "'";
            return false;
        }
        out->push_back(std::move(t));
    }
    out->push_back(Token());
    return true;
}

}  // namespace andros
