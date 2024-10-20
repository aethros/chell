#ifndef util_h
#define util_h

#include <stdio.h> // size_t
#include <stdbool.h> // bool
#include <string.h> // strlen
#include <stddef.h> // ptrdiff_t

/// @brief Enum to represent built-in commands.
typedef enum builtin {
    NO_BUILTIN = 0, // No builtin command
    EXIT = 1, // Exit the shell
    EXEC = 2, // Execute a command
    CD = 3  // Change directory
} builtin;

/// @brief Function which counts the number of tokens in a c-string.
/// NOTE: This can only be run on trimmed strings!
/// @param text Text to count tokens in.
/// @param delim Delimiter to split text on.
/// @param len Length of the text.
/// @return Number of tokens in the text.
size_t getTokenCount(const char* text, char delim, size_t len)
{
    const char* start = text;
    if(*text == 0) { return 0; }
    size_t count = (*start != delim) ? 1 : 0;
    while((text = strchr(text, delim)) != NULL && text < start + len) {
        if (*(++text) != delim && *text != '\0') { count++; }
    }
    return count;
}

/// @brief Takes a c-string and splits it into tokens based on spaces.
/// @param text Input text to split.
/// @param delim Delimiter to split text on.
/// @param tokens Pre-allocated array of pointers to store tokens.
/// @param tokenCount Predefined number of tokens to split text into.
/// @return Pointer to the array of token pointers.
char** splitTextToTokens(char* text, char delim, char** tokens, size_t count)
{
    char** toks = tokens;
    char* saveptr = NULL;
    toks[0] = strtok_r(text, &delim, &saveptr);
    for (size_t i = 1; i < count; i++)
    {
        toks[i] = strtok_r(NULL, &delim, &saveptr);
        if (toks[i] == NULL) { break; }
    }
    return toks;
}

#endif // util_h
