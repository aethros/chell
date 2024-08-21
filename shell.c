#include <stdio.h> // printf, size_t
#include <stdlib.h> // malloc
#include <unistd.h> // getcwd
#include "util.h"

#define BUFSIZE 1024
#define NULLCHECK(PTR)    if (PTR  == NULL)  { printf("\nnull ptr exception\n"); exit(EXIT_FAILURE); }
#define ERRCHECK(CODE)    if (CODE != 0)     { printf("\nfunction exception\n"); exit(EXIT_FAILURE); }
#define PRTCHECK(CODE)    if (CODE <  0)     { printf("\n printf  exception\n"); exit(EXIT_FAILURE); }
#define BUFFALLOC         (char*)malloc(sizeof(char) * BUFSIZE)

char* getSysInfo(char* user, char* host, char* cwd, size_t bufsiz);
builtin getBuiltin(const char* command);
void logTextAndTokens(char* text, char* toks[], int tokenCount);
int runBuiltin(builtin bi, char* commandArgs[], int commandArgsCount);
bool commandExists(const char* command);
int runCommand(char* commandArgs[]);

int main(int argc, char* argv[])
{
    //// allocate/init memory
    char** tokens;
    int tokenCount;
    char* user    = BUFFALLOC;     NULLCHECK(user);
    char* host    = BUFFALLOC;     NULLCHECK(host);
    char* cwd     = BUFFALLOC;     NULLCHECK(cwd);
    char* buf     = BUFFALLOC;     NULLCHECK(buf);

    //// main loop
    do
    {   //// get info/print prompt
        char* wrkdir = getSysInfo(user, host, cwd, BUFSIZE);
        PRTCHECK(printf("%s@%s:%s $ ", user, host, wrkdir));

        //// Get string
        char* text = fgets(buf, BUFSIZE, stdin);
        NULLCHECK(text);

        //// clean text
        size_t len = strnlen(buf, BUFSIZE);
        text = (char*)trimWhitespace(buf, len);
        len = strnlen(text, len);

        //// get tokens
        tokenCount = getTokenCount(text, len);
        tokenCount = (tokenCount == 0) ? 1 : tokenCount;
        tokens = (char**)malloc(sizeof(char*) * tokenCount);
        NULLCHECK(tokens);
        char** toks = splitTextToTokens(text, len, tokens, tokenCount);

        //// process tokens/commands
        builtin bi = getBuiltin(toks[0]);
        if (bi) {
            ERRCHECK(runBuiltin(bi, &(toks[1]), tokenCount - 1));
        }
        else if (commandExists(toks[0])) {
            ERRCHECK(runCommand(toks));
        } else {
            PRTCHECK(printf("\nshell: %s: command not found\n", toks[0]));
        }
        
        free(tokens);
    } while (tokens != NULL);
    
    //// free memory
    free(buf); free(user); free(host); free(cwd);
    return EXIT_SUCCESS;
}

char* getSysInfo(char* user, char* host, char* cwd, size_t bufsiz)
{
    ERRCHECK(getlogin_r(user, bufsiz));
    ERRCHECK(gethostname(host, bufsiz));
    char* workingdir = getcwd(cwd, bufsiz);
    NULLCHECK(workingdir);
    return workingdir;
}


builtin getBuiltin(const char* command)
{
    if (strncmp("exit", command, 5) == 0) {
        return EXIT;
    }
    else if (strncmp("exec", command, 5) == 0) {
        return EXEC;
    }
    else if (strncmp("cd", command, 3) == 0) {
        return CD;
    } else {
        return NO_BUILTIN;
    }
}

void logTextAndTokens(char* text, char* toks[], int tokenCount) {
    printf("text: %s\n", text);
    printf("token count: %d\n", tokenCount);
    for (size_t i = 0; i < tokenCount; i++)
    {
        printf("token %zu: %s\n", i, toks[i]);
    }
}

int runBuiltin(builtin bi, char* commandArgs[], int commandArgsCount)
{
    switch (bi)
    {
    case EXEC:
        if (commandArgsCount < 1) {
            printf("error: no arguments provided to exec, when at least one is required.");
            return -1;
        } else {
            return execv(commandArgs[0], commandArgs);
        }
    case EXIT:
        if (commandArgsCount > 0) {
            printf("error: arguments provided to exit, when none are allowed.");
            return -1;
        } else {
            exit(0);
        }
    case CD:
        if (commandArgsCount < 1) {
            printf("error: no arguments provided to cd, when at least one is required.");
            return -1;
        } else {
            return chdir(commandArgs[0]);
        }
    default:
        return -1;
    }
}

bool commandExists(const char* command)
{
    return false;
}

int runCommand(char* commandArgs[])
{
    return -1;
}