#include <stdio.h> // printf, size_t
#include <stdlib.h> // malloc
#include <sys/types.h> // waitpid
#include <sys/wait.h> // waitpid
#include "util.h"

#if defined(DEBUG)
    void logTextAndTokens(char* text, char* toks[], size_t count);
#endif // DEBUG
void printPrompt(struct passwd pw, const char* const pw_buf, const char *host, const char *cwd, size_t bufsiz, const struct passwd *result);
const char* getInput(char* const buf, size_t sz);
const char** getTokens(const char* text, const char** tokens, char delim, size_t* count);
builtin getBuiltin(const char* cmd);
int runBuiltin(builtin bi, char* const argv[], int argc);
bool commandExists(const char* cmd, const char** path_buf);
int runCommand(const char* cmd, char* const argv[], int argc);

/// @brief Main function for the shell.
/// @param argc The number of arguments.
/// @param argv The arguments.
/// @return The exit code of the shell.
int main(void)
{
    // allocate/init memory
    const char** tokens = NULL;
    size_t count = 0;
    struct passwd pw = {0};
    const struct passwd* result = NULL;
    char* const pw_buf  = BUFFALLOC();     NULLCHECK(pw_buf);
    const char* host    = BUFFALLOC();     NULLCHECK(host);
    const char* cwd     = BUFFALLOC();     NULLCHECK(cwd);
    char* const buf     = BUFFALLOC();     NULLCHECK(buf);

    // main loop
    do
    {   // get info/print prompts
        printPrompt(pw, pw_buf, host, cwd, BUFSIZ, result);

        // Get string
        const char* text = getInput(buf, BUFSIZE);

        // get tokens
        const char** toks = getTokens(text, tokens, ' ', &count);
        #if defined(DEBUG)
            logTextAndTokens((char*)text, (char**)toks, count);
        #endif // DEBUG

        // setup cmd and args
        size_t args_c = count - 1;
        const char* cmd = toks[0];
        const char* const* args = &(toks[1]);
        const char* path_buf = BUFFALLOC();
        NULLCHECK(path_buf);
        #if defined(DEBUG)
            printf("cmd: %s\n", cmd);
            printf("args count: %zu\n", args_c);
            fflush(stdout);
        #endif // DEBUG

        // process tokens/commands
        builtin bi = getBuiltin(cmd);
        if (bi) {
            ERRNEQZERO(runBuiltin(bi, (char *const *)args, args_c), "runBuiltin");
        } else if (commandExists(cmd, &path_buf)) {
            // checks if the command exists in the PATH
            // if it does, path_buf will contain the full path to the command
            ERRLEQZERO(runCommand(path_buf, (char *const *)args, args_c), "runCommand"); // sends the full path buffer to runCommand
        } else {
            ERRLEQZERO(printf("\nshell: %s: command not found\n", cmd), "printf");
            fflush(stdout);
        }
        
        free((void*)tokens); free((void*)path_buf);
    } while (tokens != NULL);
    
    // free memory
    free((void*)buf); free((void*)pw_buf); free((void*)host); free((void*)cwd);
    return EXIT_SUCCESS;
}

/// @brief Function which logs the text and tokens in a c-string.
#if defined(DEBUG)
    void logTextAndTokens(char* text, char* toks[], size_t count) {
        printf("text: %s\n", text);
        printf("token count: %zu\n", count);
        for (size_t i = 0; i < count; i++)
        {
            printf("token %zu: %s\n", i, toks[i]);
        }
    }
    fflush(stdout);
#endif // DEBUG

/// @brief Prints the prompt for the shell.
/// @param pw The password struct.
/// @param pw_buf The buffer to store the password.
/// @param host The host name buffer.
/// @param cwd The current working directory buffer.
/// @param bufsiz The size of the buffer.
/// @param result The result of the getpwuid_r function.
/// @return void
void printPrompt(struct passwd pw, const char* const pw_buf, const char *host, const char *cwd, size_t bufsiz, const struct passwd *result) {
    const char* wrkdir = getSysInfo(((const struct passwd* const)(&pw)), host, cwd, bufsiz, pw_buf, &result);
    NULLCHECK(result);
    ERRLEQZERO(printf(PROMPT_STRING, result->pw_name, host, wrkdir), "printf");
    fflush(stdout);
}

/// @brief Gets input from the user.
/// @param buf Buffer to store the input. 
/// @param sz Size of the buffer.
/// @return Pointer to the input.
const char* getInput(char* const buf, size_t sz) {
    char* text = fgets(buf, (int)sz, stdin);
    NULLCHECK(text);
    text[strcspn(text, "\n")] = 0; // remove trailing \n from input (it will throw errors if present)
    return text;
}

const char** getTokens(const char* text, const char** tokens, char delim, size_t* count) {
    *count = getTokenCount(text, delim, strlen(text));
    tokens = generateTokenArray(*count);
    const char** const toks = splitTextToTokens(text, delim, tokens, *count);
    return toks;
}

/// @brief Parses a string to determine if it is a builtin command.
/// @param cmd String to parse.
/// @return Builtin command if the string is a builtin command, NO_BUILTIN otherwise.
builtin getBuiltin(const char* cmd)
{
    if (strncmp("exit", cmd, 5) == 0) {
        return EXIT;
    }
    else if (strncmp("exec", cmd, 5) == 0) {
        return EXEC;
    }
    else if (strncmp("cd", cmd, 3) == 0) {
        return CD;
    } else {
        return NO_BUILTIN;
    }
}

/// @brief Function which runs a builtin command.
/// @param bi Builtin command to run. 
/// @param argv Arguments to the builtin command.
/// @param argc Number of arguments to the builtin command.
/// @return Exit code of the builtin command.
int runBuiltin(builtin bi, char* const argv[], int argc)
{
    switch (bi)
    {
    case EXEC:
        if (argc < 1) {
            printf("error: no arguments provided to exec, when at least one is required.");
            return -1;
        } else {
            const char* exec_cmd = argv[0];
            #if defined(DEBUG)
                printf("executing: %s\n", exec_cmd);
                printf("args count: %d\n", argc);
                for (int i = 0; i < argc; i++)
                {
                    printf("arg %d: %s\n", i, argv[i]);
                }
                fflush(stdout);
            #endif // DEBUG
            return execv(exec_cmd, argv);
        }
    case EXIT:
        if (argc > 0) {
            printf("error: arguments provided to exit, when none are allowed.");
            return -1;
        } else {
            exit(0);
        }
    case CD:
        if (argc < 1) {
            printf("error: no arguments provided to cd, when at least one is required.");
            return -1;
        } else {
            return chdir(argv[0]);
        }
    default:
        return -1;
    }
}

/// @brief Function which checks if a command exists within the PATH.
/// @param cmd Command to check.
/// @param path_buf Buffer to store the path of the command.
/// @return True if the command exists, false otherwise.
bool commandExists(const char* cmd, const char** path_buf)
{
    // if the command is a full path, check if it exists
    if (strchr(cmd, '/') != NULL && access(cmd, F_OK) == 0) {
        *path_buf = cmd;
        return true; // return true if the full path'd command exists
    }

    // Get the PATH
    const char** tokens = NULL;
    const char* path = getenv("PATH");
    NULLCHECK(path);
    size_t count = 0;
    const char** toks = getTokens(path, tokens, ':', &count);
    NULLCHECK(toks);
    #if defined(DEBUG)
        logTextAndTokens((char*)path, (char**)toks, count);
    #endif // DEBUG

    // Check if the command exists in the PATH
    for (size_t i = 0; i < count; i++)
    {
        snprintf((char*)(*path_buf), BUFSIZ, "%s/%s", toks[i], cmd);
        #if defined(DEBUG)
            printf("checking: %s\n", *path_buf);
            fflush(stdout);
        #endif // DEBUG
        if (access(*path_buf, F_OK) == 0) {
            free((void*)tokens);
            return true;
        }
    }
    free((void*)tokens);
    return false;
}

/// @brief Function which runs a command.
/// @param cmd Command to run.
/// @param argv Arguments to the command.
/// @param argc Number of arguments to the command.
/// @return Exit code of the command.
int runCommand(const char* cmd, char* const argv[], int argc)
{
    pid_t childpid = fork();
    ERRLEQZERO(childpid, "fork");
    
    if(childpid == 0) {
        // Child process
        if (argc < 1) {
            printf("error: no arguments provided to command, when at least one is required.");
            return -1;
        } else {
            #if defined(DEBUG)
                printf("executing: %s\n", cmd);
                printf("args count: %d\n", argc);
                for (int i = 0; i < argc; i++)
                {
                    printf("arg %d: %s\n", i, argv[i]);
                }
                fflush(stdout);
            #endif // DEBUG
            
            return execv(cmd, argv);
        }
    } else {
        // Parent process
        int status = 0;
        ERR_EQZERO(waitpid(childpid, &status, WEXITED), "waitpid");
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            printf("error: command exited improperly.\n");
            return -1;
        }
    }
}
