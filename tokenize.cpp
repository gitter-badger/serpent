#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include "util.h"

// These appear as independent tokens even if inside a stream of symbols
const std::string atoms[] = { "#", "-", "//", "(", ")", "[", "]", "{", "}" };
const int numAtoms = 9;

// Is the char alphanumeric, a space, a bracket, a quote, a symbol?
int chartype(char c) {
    if (c >= '0' && c <= '9') return ALPHANUM;
    else if (c >= 'a' && c <= 'z') return ALPHANUM;
    else if (c >= 'A' && c <= 'Z') return ALPHANUM;
    else if (std::string("._$").find(c) != -1) return ALPHANUM;
    else if (c == '\t' || c == ' ' || c == '\n') return SPACE;
    else if (std::string("()[]{}").find(c) != -1) return BRACK;
    else if (c == '"') return DQUOTE;
    else if (c == '\'') return SQUOTE;
    else return SYMB;
}

// "y = f(45,124)/3" -> [ "y", "f", "(", "45", ",", "124", ")", "/", "3"]
std::vector<Node> tokenize(std::string inp, Metadata metadata) {
    int curtype = SPACE;
    int pos = 0;
    metadata.ch = 0;
    std::string cur;
    std::vector<Node> out;

    inp += " ";
    while (pos < inp.length()) {
        int headtype = chartype(inp[pos]);
        // Are we inside a quote?
        if (curtype == SQUOTE || curtype == DQUOTE) {
            // Close quote
            if (headtype == curtype) {
                cur += inp[pos];
                out.push_back(token(cur, metadata));
                cur = "";
                metadata.ch = pos;
                curtype = SPACE;
            }
            // eg. \xc3
            else if (inp.length() >= pos + 4 && inp.substr(pos, 2) == "\\x") {
                cur += (std::string("0123456789abcdef").find(inp[pos+2]) * 16
                        + std::string("0123456789abcdef").find(inp[pos+3]));
                pos += 4;
            }
            // Newline
            else if (inp.substr(pos, 2) == "\\n") {
                cur += '\n';
                pos += 2;
            }
            // Backslash escape
            else if (inp.length() >= pos + 2 && inp[pos] == '\\') {
                cur += inp[pos + 1];
                pos += 2;
            }
            // Normal character
            else {
                cur += inp[pos];
                pos += 1;
            }
        }
        else {
            // Handle atoms ( '//', '#', '-', brackets )
            for (int i = 0; i < numAtoms; i++) {
                int split = cur.length() - atoms[i].length();
                if (split >= 0 && cur.substr(split) == atoms[i]) {
                    if (split > 0) {
                        out.push_back(token(cur.substr(0, split), metadata));
                    }
                    metadata.ch += split;
                    out.push_back(token(cur.substr(split), metadata));
                    metadata.ch = pos;
                    cur = "";
                    curtype = SPACE;
                }
            }
            // Boundary between different char types
            if (headtype != curtype) {
                if (curtype != SPACE && cur != "") {
                    out.push_back(token(cur, metadata));
                }
                metadata.ch = pos;
                cur = "";
            }
            cur += inp[pos];
            curtype = headtype;
            pos += 1;
        }
    }
    return out;
}

