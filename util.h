#ifndef util_h
#define util_h

#include <errno.h>    // errno
#include <pwd.h>      // getpwuid_r, struct passwd, uid_t
#include <stdbool.h>  // bool, false
#include <stdio.h>    // fprintf, printf, stdout, size_t, fgets, fflush, snprintf
#include <stdlib.h>   // exit, malloc, free, EXIT_FAILURE, exit, getenv, strndup
#include <string.h>   // strlen, strtok_r, strerror, strchr, strrchr, strndup
#include <unistd.h>   // getcwd, getuid, gethostname

#define PROMPT_STRING "%s@%s:%s $ "
#define BUFSIZE 8192
#define BUFFALLOC() (char *)malloc(sizeof(char) * BUFSIZE)
#define LOG(MSG, ...) fprintf(stdout, MSG __VA_OPT__(,) __VA_ARGS__);

#define __FILENAME__                                            \
    (strrchr(__FILE__, '/')                                     \
        ? strrchr(__FILE__, '/') + 1                            \
        : __FILE__)

#define ERRNEQZERO(CODE, NAME)                                  \
    do {                                                        \
        int code = CODE;                                        \
        if (code != 0) {                                        \
            printf("\nerror: function exception - %s\n", NAME); \
            printf("%s\n", strerror(errno));                    \
            exit(code);                                         \
        }                                                       \
    } while (false);

#define ERRLEQZERO(CODE, NAME)                                  \
    do {                                                        \
        int code = CODE;                                        \
        if (code <  0) {                                        \
            printf("\nerror: function exception - %s\n", NAME); \
            printf("%s\n", strerror(errno));                    \
            exit(code);                                         \
        }                                                       \
    } while (false);

#define NULLCHECK(PTR)                                          \
    do {                                                        \
        if (PTR == NULL) {                                      \
            printf("\nerror: null ptr exception\n");            \
            exit(EXIT_FAILURE);                                 \
        }                                                       \
    } while (false);

#define LOGDBG(MSG, ...)                                        \
    do {                                                        \
        LOG("(%10s:%3d) %20s : ",                               \
            __FILENAME__, __LINE__, __FUNCTION__);              \
        LOG(MSG __VA_OPT__(,) __VA_ARGS__);                     \
    } while (false);

/// @brief Enum to represent built-in commands.
typedef enum builtin {
    NO_BUILTIN = 0, // No builtin command
    EXIT = 1, // Exit the shell
    EXEC = 2, // Execute a command
    CD = 3  // Change directory
} builtin;

/// @brief Generates an array of token pointers.
/// @param count Number of token pointers to generate.
/// @return Pointer to the array of token pointers.
const char** generate_token_array(size_t count) {
    const char** tokens = (const char**)malloc(sizeof(const char**) * (count + 1));
    // add one to count for null terminator
    NULLCHECK(tokens);
    // Initialize all elements to NULL to avoid garbage values
    for (size_t i = 0; i <= count; i++) {
        tokens[i] = NULL;
    }
    return tokens;
}

/// @brief Frees an array of tokens and their individual strings.
/// @param tokens Array of token pointers to free.
/// @param count Number of tokens in the array.
/// @return `void`
void free_token_array(const char** tokens, size_t count) {
    if (tokens == NULL) return;
    for (size_t i = 0; i < count && tokens[i] != NULL; i++) {
        free((void*)tokens[i]);
    }
    free((void*)tokens);
}

/// @brief Function which counts the number of tokens in a c-string.
/// NOTE: This can only be run on trimmed strings!
/// @param text Text to count tokens in.
/// @param delim Delimiter to split text on.
/// @param len Length of the text.
/// @return Number of tokens in the text.
size_t get_token_cnt(const char* text, char delim, size_t len)
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

/// @brief Gets the user, host, and current working directory of the system for the prompt.
/// @param pw Pointer to struct passwd to store the user information.
/// @param host Pointer to a string to store the host information.
/// @param cwd  Pointer to a string to store the current working directory information.
/// @param bufsiz Size of the buffer which can store information.
/// @param pw_buf Buffer for getpwuid_r to use.
/// @param result Pointer to store the result of getpwuid_r.
/// @return Pointer to the current working directory.
const char* get_sysinfo(
    struct passwd* pw,
    const char* host,
    const char* cwd,
    size_t bufsiz,
    char* pw_buf,
    struct passwd** result)
{
    uid_t uid = getuid();
    ERRNEQZERO(getpwuid_r(uid, pw, pw_buf, bufsiz, result), "getpwuid_r");
    ERRNEQZERO(gethostname((char*)host, bufsiz), "gethostname");
    const char* workingdir = getcwd((char*)cwd, bufsiz);
    NULLCHECK(workingdir);
    return workingdir;
}

/// @brief Takes a c-string and splits it into tokens based on spaces.
/// @param text Input text to split.
/// @param delim Delimiter to split text on.
/// @param tokens Pre-allocated array of pointers to store tokens.
/// @param strln Length of the input text.
/// @param count Predefined number of tokens to split text into.
/// @return Pointer to the array of token pointers.
const char** split_to_tokens(
    const char* const text,
    const char delim,
    const char** tokens,
    size_t strln,
    size_t count)
{
    const char** toks = tokens;
    char* saveptr = NULL;
    char* txcpy = strndup(text, strln);
    NULLCHECK(txcpy);

    // Create delimiter string from char
    char delim_str[2] = {delim, '\0'};

    // Get first token
    const char* token = strtok_r(txcpy, delim_str, &saveptr);
    if (token == NULL) {
        toks[0] = strndup(text, strln); 
    } else {
        toks[0] = strndup(token, strlen(token));
    }

    // Get subsequent tokens
    for (size_t i = 1; i < count; i++)
    {
        token = strtok_r(NULL, delim_str, &saveptr);
        if (token == NULL) { break; }
        toks[i] = strndup(token, strlen(token));
    }

    // Null-terminate tokens
    toks[count] = NULL;
    free(txcpy);
    return toks;
}

#endif // util_h
