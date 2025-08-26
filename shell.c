#include <sys/types.h> // waitpid, pid_t
#include <sys/wait.h> // waitpid, WIFEXITED, WEXITSTATUS
#include "util.h"

#if defined(DEBUG)
    void log_text_tokens(char* text, char* toks[], size_t count);
#endif // DEBUG
void print_prompt(
    struct passwd pw,
    char* const pw_buf,
    const char *host,
    const char *cwd,
    size_t bufsiz,
    struct passwd *result);

const char** get_tokens(
    const char* text,
    const char** tokens,
    char delim,
    size_t strln,
    size_t* count);

size_t set_cmd(
    size_t count,
    const char** toks,
    char** cmd,
    const char* const** args,
    char** path_buf);

const char* get_input(char* const buf, size_t sz);
builtin get_builtin(const char* cmd);
int run_builtin(builtin bi, char* const argv[], int argc);
bool command_exists(char* cmd, char** path_buf, size_t bufsiz);
int run_command(const char* cmd, char* const argv[], int argc);

/// @brief Main function for the shell.
/// @param argc The number of arguments.
/// @param argv The arguments.
/// @return The exit code of the shell.
int main(void)
{
    // allocate/init memory
    const char** tokens         = NULL;
    char* cmd                   = NULL;
    const char* const* args     = NULL;
    char* path_buf              = NULL;
    size_t count                = 0;
    struct passwd pw            = {0};
    struct passwd* result       = NULL;
    char* const pw_buf          = BUFFALLOC();     NULLCHECK(pw_buf);
    const char* host            = BUFFALLOC();     NULLCHECK(host);
    const char* cwd             = BUFFALLOC();     NULLCHECK(cwd);
    char* const buf             = BUFFALLOC();     NULLCHECK(buf);

    // main loop
    do
    {   // get info/print prompts
        print_prompt(pw, pw_buf, host, cwd, BUFSIZE, result);

        // Get string
        const char* text = get_input(buf, BUFSIZE);

        // get tokens
        const char** toks = get_tokens(text, tokens, ' ', BUFSIZE, &count);
        #if defined(DEBUG)
            log_text_tokens((char*)text, (char**)toks, count);
        #endif // DEBUG

        // setup cmd and args
        size_t args_c = set_cmd(count, toks, &cmd, &args, &path_buf);
        NULLCHECK(cmd);
        NULLCHECK(args);
        NULLCHECK(path_buf);
        #if defined(DEBUG)
            LOGDBG("cmd: %s\n", cmd);
            LOGDBG("args count: %zu\n", args_c);
            fflush(stdout);
        #endif // DEBUG

        // process tokens/commands
        builtin bi = get_builtin(cmd);
        if (bi) {
            ERRNEQZERO(run_builtin(bi, (char *const *)args, args_c), "run_builtin");
        } else if (command_exists(cmd, &path_buf, BUFSIZE)) {
            // checks if the command exists in the PATH
            // if it does, path_buf will contain the full path to the command
            ERRLEQZERO(run_command(path_buf, (char *const *)args, args_c), "run_command");
            // sends the full path buffer to run_command (e.g.:/usr/bin/ls)
        } else {
            // command not found
            ERRLEQZERO(printf("\nshell: %s: command not found\n", cmd), "printf");
            fflush(stdout);
        }
        
        // free temp memory
        free((void*)cmd); 
        free_token_array(toks, count);
        free((void*)path_buf);
    } while (true);
    
    // free memory
    free((void*)buf); free((void*)pw_buf); free((void*)host); free((void*)cwd);
    return EXIT_SUCCESS;
}

#if defined(DEBUG)
    /// @brief Function which logs the text and tokens in a given c-string.
    void log_text_tokens(char* text, char* toks[], size_t count) {
        LOGDBG("text: %s\n", text);
        LOGDBG("token count: %zu\n", count);
        for (size_t i = 0; i < count; i++) {
            LOGDBG("token %zu: %s\n", i, toks[i]);
        }
        fflush(stdout);
    }
#endif // DEBUG

/// @brief Prints the prompt for the shell.
/// @param pw The password struct.
/// @param pw_buf The buffer to store the password.
/// @param host The host name buffer.
/// @param cwd The current working directory buffer.
/// @param bufsiz The size of the buffer.
/// @param result The result of the getpwuid_r function.
/// @return `void`
void print_prompt(
    struct passwd pw,
    char* const pw_buf,
    const char *host,
    const char *cwd,
    size_t bufsiz,
    struct passwd *result)
{
    const char* wrkdir = get_sysinfo(&pw, host, cwd, bufsiz, pw_buf, &result);
    NULLCHECK(result);
    ERRLEQZERO(printf(PROMPT_STRING, result->pw_name, host, wrkdir), "printf");
    fflush(stdout);
}

/// @brief Function which gets tokens from a c-string.
/// @param text The input c-string.
/// @param tokens Array of token pointers.
/// @param delim Delimiter to split the text on.
/// @param strln Length of the string.
/// @param count The number of tokens.
/// @return Array of token pointers.
const char** get_tokens(
    const char* text,
    const char** tokens,
    char delim,
    size_t strln,
    size_t* count)
{
    *count = get_token_cnt(text, delim, strlen(text));
    tokens = generate_token_array(*count);
    return split_to_tokens(text, delim, tokens, strln, *count);
}

/// @brief Sets up the command and arguments for execution.
/// @param count Number of tokens.
/// @param toks Array of token pointers.
/// @param cmd Pointer to the command string.
/// @param args Pointer to the arguments array.
/// @param path_buf Pointer to the path buffer.
/// @return Number of arguments.
size_t set_cmd(
    size_t count,
    const char** toks,
    char** cmd,
    const char* const** args,
    char** path_buf)
{
    *cmd = strndup(toks[0], BUFSIZE);
    *args = &(toks[0]);
    *path_buf = BUFFALLOC();
    return count;
}

/// @brief Gets input from the user.
/// @param buf Buffer to store the input. 
/// @param sz Size of the buffer.
/// @return Pointer to the input.
const char* get_input(char* const buf, size_t sz) {
    const char* text = fgets(buf, (int)sz, stdin);
    NULLCHECK(text);

    // remove trailing \n from input (it will throw errors if present)
    char* newline_ptr = strchr(text, '\n');
    if (newline_ptr != NULL) {
        newline_ptr[0] = 0;
    }
    return text;
}

/// @brief Parses a string to determine if it is a builtin command.
/// @param cmd String to parse.
/// @return Builtin enum if the string is a builtin command, NO_BUILTIN otherwise.
builtin get_builtin(const char* cmd)
{
    if (strncmp("exit", cmd, 5) == 0) {
        return EXIT;
    } else if (strncmp("exec", cmd, 5) == 0) {
        return EXEC;
    } else if (strncmp("cd", cmd, 3) == 0) {
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
int run_builtin(builtin bi, char* const argv[], int argc) {
    switch (bi) {
        case EXEC: {
            if (argc < 2) {
                printf("error: no arguments provided to exec, when at least one is required.");
                return -1;
            } else {
                const char* exec_cmd = argv[1];
                #if defined(DEBUG)
                    LOGDBG("executing: %s\n", exec_cmd);
                    LOGDBG("args count: %d\n", argc);
                    for (int i = 0; i < argc; i++)
                    {
                        LOGDBG("arg %d: %s\n", i, argv[i]);
                    }
                    fflush(stdout);
                #endif // DEBUG
                return execv(exec_cmd, &argv[1]);
            }
        }
        case EXIT: {
            if (argc > 1) {
                printf("error: arguments provided to exit, when none are allowed.");
                return -1;
            } else {
                exit(0);
            }
        }
        case CD: {
            if (argc < 2) {
                printf("error: no arguments provided to cd, when at least one is required.");
                return -1;
            } else {
                return chdir(argv[1]);
            }
        }
        default: {
            return -1;
        }
    }
}

/// @brief Function which checks if a command exists within the PATH.
/// @param cmd Command to check.
/// @param path_buf Buffer to store the path of the command.
/// @return True if the command exists, false otherwise.
bool command_exists(char* cmd, char** path_buf, size_t bufsiz) {
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
    const char** toks = get_tokens(path, tokens, ':', bufsiz, &count);
    NULLCHECK(toks);
    #if defined(DEBUG)
        log_text_tokens((char*)path, (char**)toks, count);
    #endif // DEBUG

    // Check if the command exists in the PATH
    for (size_t i = 0; i < count; i++) {
        snprintf(*path_buf, BUFSIZE, "%s/%s", toks[i], cmd);
        #if defined(DEBUG)
            LOGDBG("checking: %s\n", *path_buf);
            fflush(stdout);
        #endif // DEBUG
        if (access(*path_buf, F_OK) == 0) {
            free_token_array(toks, count);
            return true;
        }
    }
    free_token_array(toks, count);
    return false;
}

/// @brief Function which runs a command.
/// @param cmd Command to run.
/// @param argv Arguments to the command.
/// @param argc Number of arguments to the command.
/// @return Exit code of the command.
int run_command(const char* cmd, char* const argv[], int argc)
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
                LOGDBG("executing: %s\n", cmd);
                LOGDBG("args count: %d\n", argc);
                for (int i = 0; i < argc; i++) {
                    LOGDBG("arg %d: %s\n", i, argv[i]);
                }
                fflush(stdout);
            #endif // DEBUG
            return execv(cmd, argv);
        }
    } else {
        // Parent process
        int status = 0;
        ERRLEQZERO(waitpid(childpid, &status, 0), "waitpid");
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            printf("error: command exited improperly.\n");
            return -1;
        }
    }
}
