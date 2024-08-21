#ifndef util_h
#define util_h

#include <stdio.h> // size_t
#include <stdbool.h> // bool
#include <string.h> //strlen
#include <stddef.h> // ptrdiff_t

typedef struct addrPair
{
    const void* start;
    ptrdiff_t endOffset;
} addrPair_t;

typedef enum builtin {
    NO_BUILTIN = 0,
    EXIT = 1,
    EXEC = 2,
    CD = 3
} builtin;

bool isSpace(char c)
{
    return (c == 0x20   // ' '  space
            //              or
         || c == 0x09   // '/t' tab
         || c == 0x0A   // '/v' vertical tab
         || c == 0x0B   // '/r' carriage return
         || c == 0x0C   // '/f' form feed
         || c == 0x0D); // '/n' new line
}

addrPair_t trimAlgorithm(const char* str, size_t len)
{   
    addrPair_t pair;
    const char* start = str;
    const char* end = str + len - 2; // end is length minus one (zero indexed) minus one more for null byte
    while (isSpace(*start)) { start++; } // move start ptr forward
    pair.start = start;
    pair.endOffset = end - start;
    if (*start == 0) { return pair; } // return if null
    while (end > start && isSpace(*end)) { end--; } // move end ptr backward
    pair.endOffset = end - start;
    return pair;
}

const char* trimWhitespace(const char* str, size_t len)
{   
    addrPair_t pair = trimAlgorithm(str, len);
    ((char* const)pair.start)[pair.endOffset + 1] = '\0';
    return (const char*)pair.start;
}

// this can only be run on trimmed strings!
int getTokenCount(const char* text, size_t len)
{
    if(*text == 0) { return 0; }
    int tokenCount = 1; // there is always atleast 1 token unless text is null
    for (size_t i = 0; text[i] != '\0' && i < len; i++) {
        if (text[i] == ' ') {
            while (i+1 <= len && (text[i+1] == ' ' || text[i+1] == '\0')) {
                i++; // scan until next non-null, non space byte
            }
            tokenCount++;
        }
    }
    return tokenCount;
}

char** splitTextToTokens(char* text, size_t len, char** tokens, int tokenCount)
{
    char** toks = tokens;
    toks[0] = text; // first ptr is the beginning of text, since the first token is at beginning of text
    size_t i_stop = 0;
    size_t text_i;
    for (size_t token_i = 1; token_i <= tokenCount; token_i++) { // continue til you run out of tokens
        for (text_i = i_stop; text[text_i] != '\0' && text_i < len; text_i++) { // continue til you run of out len or hit null byte (happens as you handle each token)
            if (text[text_i] == ' ') { // if current char is space
                toks[token_i] = &(text[text_i + 1]); // move ptr after space.
                if (toks[token_i][0] != ' ') { // if there's no more space after move
                    token_i++; // increment iterator
                    text[text_i] = '\0'; // add null byte to previous string
                }
            }
        }
        i_stop = text_i; // keep place of where you stopped
    }
    return toks;
}

#endif // util_h
