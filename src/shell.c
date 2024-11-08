/**
 * @file shell.c
 * @brief Custom shell related logic definition
 */

#include "shell.h"

void start_shell_ml()
{
    // The shell itself mustn't answer to certain signals
    for (int i = LOWEST_ARR_INDEX; i < N_SINGALS_TO_HANDLE; i++)
    {
        signal(signals[i], SIG_IGN);
    }

    // Get the 3 parts that conform the command line prompt
    const char *user = getenv(ENV_USER_KEY);
    char host[HOST_NAME_MAX + 1];
    gethostname(host, (HOST_NAME_MAX + 1));
    char cwd[PATH_MAX];
    getcwd(cwd, PATH_MAX);

    // Main loop
    while (true)
    {
        printf("%s@%s:%s$ ", user, host, cwd);
        // Buffer for the input
        static char input[ARG_MAX];
        fgets(input, ARG_MAX, stdin);
        execute_command(input, cwd);
    }
}

void execute_command(char* command, char* cwd)
{
    // Prepare job counter
    static unsigned long long int job_id = 0;
    job_id++;
    // Get first and potential second token to inquire if this is an internal command
    static char *first_token, *second_token;
    first_token = strtok(command, TOKEN_SEPARATOR);
    // If the input is null, do nothing. Iterate again
    if (first_token == NULL) return;
    // Potential second token grabs the rest of the input; could be null
    second_token = strtok(NULL, "\n");
    if (strcmp(first_token, "cd") == 0)
    {
        // Ignore " &" if appears, as this is an internal command
        is_background_exec(second_token);
        // Check existence of argument
        if (second_token != NULL && second_token[0] != '\0')
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
                    _wstderr("ERROR: No such thing as 'OLDPWD' for now", true);
                }
                else
                {
                    // Old current directory exist, head to it if possible and update env
                    if (chdir(old_cwd) == -1)
                    {
                        // Some problem commited and the current working dir change can't be done
                        _wstderr("ERROR: Issue changing cwd to the old one", true);
                    }
                    else
                    {
                        // Success
                        setenv(ENV_OLDPWD_KEY, cwd, 1);
                        getcwd(cwd, PATH_MAX);
                    }
                }
            }
            else
            {
                // Try to move to a new current directory
                if (chdir(second_token) == -1)
                {
                    // The argument is wrong or another problem appeared
                    _wstderr("ERROR: Can't change current working directory", true);
                }
                else
                {
                    // The arguments is right, proceed with the normal procedure
                    setenv(ENV_OLDPWD_KEY, cwd, 1);
                    getcwd(cwd, PATH_MAX);
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
        // Ignore " &" if appears, as this is an internal command
        is_background_exec(second_token);
        // Check existence of argument
        if(second_token != NULL && second_token[0] != '\0')
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
        exit(EXIT_SUCCESS);
    }
    else
    {
        // Check if " &" appears, in which case the command must be executed in the background
        const bool background_execution = is_background_exec(second_token);
        // Potential external command invocation
        const pid_t pid_child = fork();
        if (pid_child == -1)
        {
            _wstderr("ERROR: forking of current process failed", true);
            return;
        }
        else if (pid_child == 0)
        {
            // If this child process is being executed in the foreground, certain signals must be absorved by it
            if (!background_execution)
            {
                for (int i = LOWEST_ARR_INDEX; i < N_SINGALS_TO_HANDLE; i++)
                {
                    // Revert these signals managment to their default behavior
                    signal(signals[i], SIG_DFL);
                }
            }
            // Child process, needs to take apart through exec; *second_token* must be subtokenized
            char *argv[MAX_TOKENS_PER_COMMAND + 1];
            argv[LOWEST_ARR_INDEX] = first_token;
            int argc = LOWEST_ARR_INDEX + 1;
            char *token = strtok(second_token, TOKEN_SEPARATOR);
            while (token != NULL && argc < MAX_TOKENS_PER_COMMAND) {
                argv[argc++] = token;
                token = strtok(NULL, TOKEN_SEPARATOR);
            }
            // Check if max amount of tokens were reached and more of them are available to consume
            if (argc == MAX_TOKENS_PER_COMMAND && token != NULL)
            {
                _wstderr("ERROR: You surpassed the arguments limit for a command.", true);
                exit(EXIT_FAILURE);
            }
            // Reached this line, the limits of argument were respected; "close" the argv list
            argv[argc] = NULL;
            if (execvp(first_token, argv) == -1)
            {
                // Something went wrong
                _wstderr("ERROR: Command couldn't be executed", true);
                exit(EXIT_FAILURE);
            }
        }
        // Parent process (child never reaches next lines); wait for child to finish only if not a background proc
        if (background_execution)
        {
            // Concurrent execution
            printf("[%llu] %d\n", job_id, (int)pid_child);
            // Try that this output goes out first
            fflush(stdout);
        }
        else
        {
            // Non concurrent execution, wait for child processto finish
            int status;
            if (waitpid(pid_child, &status, 0) == -1)
            {
                _wstderr("ERROR: waitpid() failed", true);
            }
        }
    }
}

void execute_batch_file(const char* path)
{
    // Get current working directory
    char cwd[PATH_MAX];
    getcwd(cwd, PATH_MAX);
    // Open file
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        _wstderr("ERROR: Opening batch file", true);
        return;
    }
    char command[ARG_MAX];
    while (fgets(command, ARG_MAX, file))
    {
        // If there's a forced exit, the file gets closed automatically
        execute_command(command, cwd);
    }
    // Close the file cleanly
    fclose(file);
}

void _wstderr(const char* s, bool use_perror)
{
    if (use_perror)
    {
        perror(s);   
    }
    else
    {
        fwrite(s, sizeof(char), strlen(s), stderr);
    }
    // Sadly some IDEs and shells don't flush stderr right away, even with fflush()
    fflush(stderr);
    usleep(TERMINAL_FLUSH_DELAY);
}
