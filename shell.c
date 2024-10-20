#include <stdio.h> // printf, size_t
#include <errno.h> // errno
#include <stdlib.h> // malloc
#include <unistd.h> // getcwd
#include <pwd.h> // getpwuid_r
#include "util.h"

#define BUFSIZE 1024
#define NULLCHECK(PTR)    if (PTR  == NULL)  {printf("\nerror: null ptr exception\n"); exit(EXIT_FAILURE);}
#define ERREQZERO(CODE)   do {int code = CODE; if (code == 0) {printf("\nerror: function exception\n"); printf("%s\n", strerror(errno)); exit(code);}} while (false);
#define ERRNEQZERO(CODE)  do {int code = CODE; if (code != 0) {printf("\nerror: function exception\n"); printf("%s\n", strerror(errno)); exit(code);}} while (false);
#define PRTCHECK(CODE)    do {int code = CODE; if (code <  0) {printf("\nerror: printf exception\n"); exit(code);}} while (false);
#define BUFFALLOC         (char*)malloc(sizeof(char) * BUFSIZE)

char* getSysInfo(struct passwd* pw, char* host, char* cwd, size_t bufsiz, char* buf, struct passwd** result);
builtin getBuiltin(const char* cmd);
void logTextAndTokens(char* text, char* toks[], size_t count);
int runBuiltin(builtin bi, char* argv[], int argc);
bool commandExists(char* cmd, char** path_buf);
int runCommand(const char* cmd, char* argv[], int argc);

/// @brief Main function for the shell.
/// @param argc The number of arguments.
/// @param argv The arguments.
/// @return The exit code of the shell.
int main(int argc, char* argv[])
{
    // allocate/init memory
    char** tokens = NULL;
    size_t count = 0;
    struct passwd pw = {0};
    struct passwd *result = NULL;
    char* pw_buf  = BUFFALLOC;     NULLCHECK(pw_buf);
    char* host    = BUFFALLOC;     NULLCHECK(host);
    char* cwd     = BUFFALLOC;     NULLCHECK(cwd);
    char* buf     = BUFFALLOC;     NULLCHECK(buf);

    // main loop
    do
    {   // get info/print prompts
        char* wrkdir = getSysInfo(&pw, host, cwd, BUFSIZE, pw_buf, &result);
        NULLCHECK(result);
        PRTCHECK(printf("%s@%s:%s $ ", result->pw_name, host, wrkdir));

        // Get string
        char* text = fgets(buf, BUFSIZE, stdin);
        NULLCHECK(text);

        // get tokens
        count = getTokenCount(text, ' ', strlen(text));
        count = (count == 0) ? 1 : count;
        size_t args_c = count - 1;
        tokens = (char**)malloc(sizeof(char*) * count);
        NULLCHECK(tokens);
        char** toks = splitTextToTokens(text, ' ', tokens, count);
        char* cmd = toks[0];
        char** args = &(toks[1]);
        char* path_buf = BUFFALLOC;
        NULLCHECK(path_buf);

        // process tokens/commands
        builtin bi = getBuiltin(cmd);
        if (bi) {
            ERRNEQZERO(runBuiltin(bi, args, args_c));
        }
        else if (commandExists(cmd, &path_buf)) {
            ERRNEQZERO(runCommand(path_buf, args, args_c));
        } else {
            PRTCHECK(printf("\nshell: %s: command not found\n", cmd));
        }
        
        free(tokens); free(path_buf);
    } while (tokens != NULL);
    
    // free memory
    free(buf); free(pw_buf); free(host); free(cwd);
    return EXIT_SUCCESS;
}

/// @brief Gets the user, host, and current working directory of the system for the prompt.
/// @param pw Pointer to a string to store the user information.
/// @param host Pointer to a string to store the host information.
/// @param cwd  Pointer to a string to store the current working directory information.
/// @param bufsiz Size of the buffer which can store information.
/// @return Pointer to the current working directory.
char* getSysInfo(struct passwd* pw, char* host, char* cwd, size_t bufsiz, char* buf, struct passwd** result)
{
    uid_t uid = getuid();
    ERRNEQZERO(getpwuid_r(uid, pw, buf, bufsiz, result));
    ERRNEQZERO(gethostname(host, bufsiz));
    char* workingdir = getcwd(cwd, bufsiz);
    NULLCHECK(workingdir);
    return workingdir;
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

void logTextAndTokens(char* text, char* toks[], size_t count) {
    printf("text: %s\n", text);
    printf("token count: %zu\n", count);
    for (size_t i = 0; i < count; i++)
    {
        printf("token %zu: %s\n", i, toks[i]);
    }
}

/// @brief Function which runs a builtin command.
/// @param bi Builtin command to run. 
/// @param argv Arguments to the builtin command.
/// @param argc Number of arguments to the builtin command.
/// @return Exit code of the builtin command.
int runBuiltin(builtin bi, char* argv[], int argc)
{
    switch (bi)
    {
    case EXEC:
        if (argc < 1) {
            printf("error: no arguments provided to exec, when at least one is required.");
            return -1;
        } else {
            const char* exec_cmd = argv[0];
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
bool commandExists(char* cmd, char** path_buf)
{
    if (strchr(cmd, '/') != NULL && access(cmd, F_OK) == 0) {
        *path_buf = cmd;
        return true;
    }
    char** tokens = NULL;
    char* path = getenv("PATH");
    NULLCHECK(path);
    size_t count = getTokenCount(path, ':', strlen(path));
    count = (count == 0) ? 1 : count;
    tokens = (char**)malloc(sizeof(char*) * count);
    NULLCHECK(tokens);
    char** toks = splitTextToTokens(path, ':', tokens, count);
    NULLCHECK(toks);
    for (size_t i = 0; i < count; i++)
    {
        snprintf(*path_buf, BUFSIZ, "%s/%s", toks[i], cmd);
        if (access(*path_buf, F_OK) == 0) {
            free(tokens);
            return true;
        }
    }
    free(tokens);
    return false;
}

/// @brief Function which runs a command.
/// @param cmd Command to run.
/// @param argv Arguments to the command.
/// @param argc Number of arguments to the command.
/// @return Exit code of the command.
int runCommand(const char* cmd, char* argv[], int argc)
{
    pid_t childpid = fork();
    ERRNEQZERO(childpid);
    
    if(childpid == 0) {
        // Child process
        if (argc < 1) {
            printf("error: no arguments provided to command, when at least one is required.");
            return -1;
        } else {
            return execv(cmd, argv);
        }
    } else {
        // Parent process
        int status = 0;
        ERREQZERO(waitpid(childpid, &status, WEXITED));
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            printf("error: command exited improperly.\n");
            return -1;
        }
    }
}
