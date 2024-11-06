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
        // Get first and potential second token to inquire if this is an internal command
        static char *first_token, *second_token;
        first_token = strtok(input, TOKEN_SEPARATOR);
        // If the input is null, do nothing. Iterate again
        if (first_token == NULL) continue;
        // Potential second token grabs the rest of the input; could be null
        second_token = strtok(NULL, "\n");
        if (strcmp(first_token, "cd") == 0)
        {
            // Check existence of argument
            if (second_token != NULL)
            {
                // Argument provided
                if (strcmp(second_token, "-") == 0)
                {
                    // Return to old current directory (if exist)
                    const char *old_cwd = getenv(ENV_OLDPWD_KEY);
                    if (old_cwd == NULL)
                    {
                        // No old current directory, show error message
                        errno = ENOENT;
                        _perror("ERROR: No such thing as 'OLDPWD' for now");
                    }
                    else
                    {
                        // Old current directory exist, head to it if possible and update env
                        if (chdir(old_cwd) == -1)
                        {
                            // Some problem commited and the current working dir change can't be done
                            _perror("ERROR: Issue changing cwd to the old one");
                        }
                        else
                        {
                            // Success
                            setenv(ENV_OLDPWD_KEY, cwd, 1);
                            setenv(ENV_PWD_KEY, old_cwd, 1);
                            getcwd(cwd, sizeof(cwd));
                        }
                    }
                }
                else
                {
                    // Try to move to a new current directory
                    if (chdir(second_token) == -1)
                    {
                        // The argument is wrong or another problem appeared
                        _perror("ERROR: Can't change current working directory");
                    }
                    else
                    {
                        // The arguments is right, proceed with the normal procedure
                        setenv(ENV_OLDPWD_KEY, cwd, 1);
                        setenv(ENV_PWD_KEY, second_token, 1);
                        getcwd(cwd, sizeof(cwd));
                    }
                }
            }
            else
            {
                // No argument; the current directory shall be provided
                printf("%s\n", cwd);
            }
        }
        else if (strcmp(first_token, "clr") == 0)
        {
            printf(CLR_ANSI_EC);
            fflush(stdout);
        }
        else if (strcmp(first_token, "echo") == 0)
        {
            // Check existence of argument
            if(second_token != NULL)
            {
                // Argument provided; check if it starts with '$', in which case it's referencing a env var
                if (second_token[0] == '$')
                {
                    const char *env_val = getenv(&second_token[1]);
                    if (env_val == NULL)
                    {
                        // No env var with thy name or any other issue; errno doesn't get set, so puts()
                        puts("ERROR: Unexistent env var or empty arg provided.");
                    }
                    else
                    {
                        puts(env_val);
                    }
                }
                else
                {
                    puts(second_token);
                }
            }
            else
            {
                // No argument; a newline gets printed
                puts("");
            }
        }
        else if (strcmp(first_token, "quit") == 0)
        {
            return;
        }
    }
}

void _perror(const char* s)
{
    perror(s);
    // Sadly some IDEs and shells don't flush stderr right away, even with fflush()
    fflush(stderr);
    usleep(TERMINAL_FLUSH_DELAY);
}
