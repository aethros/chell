#ifndef util_h
#define util_h

#include <stdio.h> // size_t
#include <stdbool.h> // bool
#include <string.h> // strlen, strtok_r
#include <stddef.h> // ptrdiff_t
#include <stdlib.h> // exit
#include <errno.h> // errno
#include <pwd.h> // getpwuid_r
#include <unistd.h> // getcwd

#define PROMPT_STRING "%s@%s:%s $ "
#define BUFSIZE 1024
#define NULLCHECK(PTR)           if (PTR == NULL)  {printf("\nerror: null ptr exception\n"); exit(EXIT_FAILURE);}
#define ERR_EQZERO(CODE, NAME)   do {int code = CODE; if (code == 0) {printf("\nerror: function exception - %s\n", NAME); printf("%s\n", strerror(errno)); exit(code);}} while (false);
#define ERRNEQZERO(CODE, NAME)   do {int code = CODE; if (code != 0) {printf("\nerror: function exception - %s\n", NAME); printf("%s\n", strerror(errno)); exit(code);}} while (false);
#define ERRLEQZERO(CODE, NAME)   do {int code = CODE; if (code <  0) {printf("\nerror: function exception - %s\n", NAME); printf("%s\n", strerror(errno)); exit(code);}} while (false);
#define BUFFALLOC()              (char*)malloc(sizeof(char) * BUFSIZE)

/// @brief Enum to represent built-in commands.
typedef enum builtin {
    NO_BUILTIN = 0, // No builtin command
    EXIT = 1, // Exit the shell
    EXEC = 2, // Execute a command
    CD = 3  // Change directory
} builtin;

/// @brief Gets the user, host, and current working directory of the system for the prompt.
/// @param pw Pointer to a string to store the user information.
/// @param host Pointer to a string to store the host information.
/// @param cwd  Pointer to a string to store the current working directory information.
/// @param bufsiz Size of the buffer which can store information.
/// @return Pointer to the current working directory.
const char* getSysInfo(const struct passwd* const pw, const char* host, const char* cwd, size_t bufsiz, const char* pw_buf, const struct passwd** result)
{
    uid_t uid = getuid();
    ERRNEQZERO(getpwuid_r(uid, (struct passwd*)pw, (char*)pw_buf, bufsiz, (struct passwd**)result), "getpwuid_r");
    ERRNEQZERO(gethostname((char*)host, bufsiz), "gethostname");
    const char* workingdir = getcwd((char*)cwd, bufsiz);
    NULLCHECK(workingdir);
    return workingdir;
}

/// @brief Generates an array of token pointers.
/// @param count Number of token pointers to generate.
/// @return Pointer to the array of token pointers.
const char** generateTokenArray(size_t count) {
    const char** tokens = (const char**)malloc(sizeof(const char**) * (count + 1));  // add one to count for null terminator
    NULLCHECK(tokens);
    return tokens;
}

/// @brief Function which counts the number of tokens in a c-string.
/// NOTE: This can only be run on trimmed strings!
/// @param text Text to count tokens in.
/// @param delim Delimiter to split text on.
/// @param len Length of the text.
/// @return Number of tokens in the text.
size_t getTokenCount(const char* text, char delim, size_t len)
{
    const char* start = text;
    if(text[0] == '\0') { return 1; } // ensure count is atleast 1
    size_t count = (start[0] != delim) ? 1 : 0;
    while((text = strchr(text, delim)) != NULL && text < start + len) {
        if (*(++text) != delim && *text != '\0') { count++; }
    }
    count = (count == 0) ? 1 : count; // ensure count is atleast 1
    return count;
}

/// @brief Takes a c-string and splits it into tokens based on spaces.
/// @param text Input text to split.
/// @param delim Delimiter to split text on.
/// @param tokens Pre-allocated array of pointers to store tokens.
/// @param tokenCount Predefined number of tokens to split text into.
/// @return Pointer to the array of token pointers.
const char** splitTextToTokens(const char* const text, const char delim, const char** tokens, size_t count)
{
    const char** toks = tokens;
    char* saveptr = NULL;
    toks[0] = strtok_r((char*)text, &delim, &saveptr);
    if (toks[0] == NULL) {  toks[0] = text; }
    
    for (size_t i = 1; i < count; i++)
    {
        toks[i] = strtok_r(NULL, &delim, &saveptr);
        if (toks[i] == NULL) { break; }
    }
    toks[count] = NULL;
    return toks;
}

#endif // util_h
