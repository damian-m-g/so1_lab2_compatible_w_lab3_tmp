/**
 * @file shell.c
 * @brief Custom shell related logic definition
 */

#include "shell.h"

// WIP: ...
void start_shell()
{
    // Get the 3 parts that conform the command line prompt
    const char *user = getenv(ENV_USER_KEY);
    char host[HOST_NAME_MAX + 1];
    gethostname(host, sizeof(host));
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));

    // Main loop
    while (true)
    {
        printf("%s@%s:%s$ ", user, host, cwd);
        // Buffer for the input
        static char input[ARG_MAX];
        fgets(input, sizeof(input), stdin);
    }
}
