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

void execute_command(char* input, char* cwd)
{
    // Initialize job counter
    static unsigned long long int job_id = 0;
    // Cleanse the newline added at the end, if exist
    cleanse_newline(input);
    // Let's dup this value to a helper, for strtok() usage; the original one'll be useful as pristine later
    static char input_h[ARG_MAX];
    strcpy(input_h, input);

    // Tokenize single commands, per pipe
    char *single_commands[MAX_SINGLE_COMMANDS];
    int sc_n = LOWEST_ARR_INDEX; // will remain with this value if empty input was submitted; # of SC
    char *token = strtok(input_h, SC_TOKEN_SEPARATOR);
    while (token != NULL && sc_n < MAX_SINGLE_COMMANDS)
    {
        single_commands[sc_n++] = token;
        token = strtok(NULL, SC_TOKEN_SEPARATOR);
    }

    // How many single commands (separated by |) were submitted: none, one or multiple?
    if (sc_n == 0) return;
    // One single command was submitted
    if (sc_n == 1)
    {
        // One single command submitted; update job id
        job_id++;
        // Explicit freed of memory isn't done, relies on the OS when exit() gets called
        char **sc_tokens = tokenize_single_command(single_commands[0]);
        // Check if "&" appears, to see if it requires background execution
        bool background_execution = is_background_exec(sc_tokens);
        // Redirections implementation; check for "<" appearance
        char *file_name = get_possible_redirection(sc_tokens, true);
        int original_stdin = redirect_stdin(file_name);
        // Check for ">" appearance; re-use file_name, as it is a buffer
        file_name = get_possible_redirection(sc_tokens, false);
        int original_stdout = redirect_stdout(file_name);
        // cleanse single command tokens & the string itself from redirection tokens
        cleanse_redirections_on_argv(sc_tokens);
        cleanse_redirections_on_sc(input);
        // Internal commands, when called solo, are always executed in the foreground, as they are quick
        if (strcmp(sc_tokens[0], "cd") == 0)
        {
            execute_cd(input, sc_tokens, background_execution, cwd);
        }
        else if (strcmp(sc_tokens[0], "clr") == 0)
        {
            printf(CLR_ANSI_EC);
            fflush(stdout);
        }
        else if (strcmp(sc_tokens[0], "echo") == 0)
        {
            execute_echo(input, sc_tokens, background_execution);
        }
        else if (strcmp(sc_tokens[0], "quit") == 0)
        {
            // Try to end zombies processes
            while (waitpid(-1, NULL, WNOHANG) > 0) {}
            exit(EXIT_SUCCESS);
        }
        else
        {
            // Potential external command invocation
            const pid_t pid_child = fork();
            if (pid_child == -1)
            {
                _wstderr("ERROR: forking of current process failed", true);
                return;
            }
            else if (pid_child == 0)
            {
                execute_external_cmd(sc_tokens, background_execution);
            }
            // Parent process; wait for child to finish only if not a background proc
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
                if (waitpid(pid_child, NULL, 0) == -1)
                {
                    _wstderr("ERROR: waitpid() failed", true);
                }
            }
        }
        // Restore stdin and stdout if were modified
        restore_stdio(original_stdin, STDIN_FILENO);
        restore_stdio(original_stdout, STDOUT_FILENO);
    }
    else
    {
        // Multiple single command submitted, pipe implementation called
        int pipesfd[2 * (sc_n - 1)];
        // Pipes creation
        for (int i = LOWEST_ARR_INDEX; i < sc_n; i++)
        {
            if (pipe(pipesfd + i * 2) == -1)
            {
                _wstderr("ERROR: On pipe creation", true);
                return;
            }
        }
        // Track child processes that need to be waited (running in foreground)
        pid_t ch_procs_to_wait[MAX_SINGLE_COMMANDS];
        unsigned ch_proc_to_wait_n = 0;
        // Fork a process per single command
        for (int i = LOWEST_ARR_INDEX; i < sc_n; i++)
        {
            // Update job id
            job_id++;
            // Tokenize the single cmd; use a copy of the command as tokenize_single_command() modifies it
            static char sc_h[ARG_MAX];
            strcpy(sc_h, single_commands[i]);
            // Explicit freed of memory isn't done, relies on the OS when exit() gets called
            char **sc_tokens = tokenize_single_command(sc_h);
            // Check if "&" appears, to see if it requires background execution
            bool background_execution = is_background_exec(sc_tokens);
            // Fork main process
            const pid_t pid_child = fork();
            if (pid_child == -1)
            {
                _wstderr("ERROR: forking of current process failed", true);
                return;
            }
            else if (pid_child == 0)
            {
                // This is a child process; first one doesn't need read end set
                if (i > 0)
                {
                    // Set the stdin of this process to be certain end of a pipe (read end)
                    if (dup2(pipesfd[(i - 1) * 2], STDIN_FILENO) == -1)
                    {
                        _wstderr("ERROR: dup2() failed", true);
                        exit(EXIT_FAILURE);
                    }
                }
                // Last single command doesn't need its stdout set to be certain end of a pipe
                if (i < (sc_n - 1))
                {
                    // Set the stdout of this process to be certain end of a pipe (write end)
                    if (dup2(pipesfd[i * 2 + 1], STDOUT_FILENO) == -1)
                    {
                        _wstderr("ERROR: dup2() failed", true);
                        exit(EXIT_FAILURE);
                    }
                }
                // Once pipes file descriptors were copied to its respective stdin & stdout, close them
                for (int j = LOWEST_ARR_INDEX; j < 2 * (sc_n - 1); j++)
                {
                    close(pipesfd[j]);
                }
                // Redirections implementation
                if (i == 0)
                {
                    // First single command, accepts "<"; check for its appearance
                    const char *file_name = get_possible_redirection(sc_tokens, true);
                    redirect_stdin(file_name);
                }
                else if (i == (sc_n - 1))
                {
                    // Last single command, accepts ">"; check for its appearance
                    const char *file_name = get_possible_redirection(sc_tokens, false);
                    redirect_stdout(file_name);
                }
                // cleanse single command tokens & the string itself from redirection tokens
                cleanse_redirections_on_argv(sc_tokens);
                cleanse_redirections_on_sc(single_commands[i]);
                // Watch out if the command called is internal or external
                if (strcmp(sc_tokens[0], "cd") == 0)
                {
                    execute_cd(single_commands[i], sc_tokens, background_execution, cwd);
                }
                else if (strcmp(sc_tokens[0], "clr") == 0)
                {
                    printf(CLR_ANSI_EC);
                    fflush(stdout);
                }
                else if (strcmp(sc_tokens[0], "echo") == 0)
                {
                    execute_echo(single_commands[i], sc_tokens, background_execution);
                }
                else if (strcmp(sc_tokens[0], "quit") == 0)
                {
                    // As a coupled command, does nothing
                }
                else
                {
                    execute_external_cmd(sc_tokens, background_execution);
                }
                // End this child process successfully
                exit(EXIT_SUCCESS);
            }
            // Parent process; save child pid, if we need to wait for it to finish
            if (background_execution)
            {
                // Concurrent execution
                printf("[%llu] %d\n", job_id, (int)pid_child);
                // Try that this output goes out first
                fflush(stdout);
            }
            else
            {
                // Keep track of the just created child, as it has to be waited to finish
                ch_procs_to_wait[ch_proc_to_wait_n++] = pid_child;
            }
        }
        // Parent process closes all pipe file descriptors as it makes no use of them
        for (int i = LOWEST_ARR_INDEX; i < 2 * (sc_n - 1); i++)
        {
            close(pipesfd[i]);
        }
        // Hold for all child processes that needs to be awaited to finish
        for (unsigned i = LOWEST_ARR_INDEX; i < ch_proc_to_wait_n; i++)
        {
            if (waitpid(ch_procs_to_wait[i], NULL, 0) == -1)
            {
                _wstderr("ERROR: waitpid() failed", true);
            }
        }
    }
}

char** tokenize_single_command(char* sc)
{
    char **argv = malloc((MAX_TOKENS_PER_COMMAND + 1) * sizeof(char *));
    if (argv == NULL)
    {
        _wstderr("ERROR: Failed to allocate memory", true);
        exit(EXIT_FAILURE);
    }
    int argc = LOWEST_ARR_INDEX;
    char *token = strtok(sc, TOKEN_SEPARATOR);
    while (token != NULL && argc < MAX_TOKENS_PER_COMMAND)
    {
        argv[argc++] = malloc(strlen(token) + 1);
        if(argv[argc - 1] == NULL)
        {
            _wstderr("ERROR: Failed to allocate memory", true);
            exit(EXIT_FAILURE);
        }
        strcpy(argv[argc - 1], token);
        token = strtok(NULL, TOKEN_SEPARATOR);
    }
    // Check if max amount of tokens were reached and more of them are available to consume
    if (argc == MAX_TOKENS_PER_COMMAND && token != NULL)
    {
        _wstderr("ERROR: You surpassed the arguments limit for a command.\n", false);
        exit(EXIT_FAILURE);
    }
    // Reached this line, the limits of argument were respected; "close" the argv list
    argv[argc] = NULL;

    return argv;
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
    char input[ARG_MAX];
    while (fgets(input, ARG_MAX, file))
    {
        // If there's a forced exit, the file gets closed automatically
        execute_command(input, cwd);
    }
    // Close the file cleanly
    fclose(file);
}

int redirect_stdin(const char* file_name)
{
    if (file_name != NULL)
    {
        // Redirect stdin
        int input_fd = open(file_name, O_RDONLY);
        if (input_fd == -1)
        {
            _wstderr("ERROR: Failed to open input file", true);
            exit(EXIT_FAILURE);
        }
        // duplicate the original file descriptor number to return if everything goes well
        int original_stdin = dup(STDIN_FILENO);
        if (dup2(input_fd, STDIN_FILENO) == -1)
        {
            _wstderr("ERROR: Failed to redirect stdin", true);
            close(input_fd);
            exit(EXIT_FAILURE);
        }
        close(input_fd);
        return original_stdin;
    }
    return -1;
}

int redirect_stdout(const char* file_name)
{
    if (file_name != NULL)
    {
        // Redirect stdout
        int output_fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (output_fd == -1)
        {
            _wstderr("ERROR: Failed to open output file", true);
            exit(EXIT_FAILURE);
        }
        // duplicate the original file descriptor number to return if everything goes well
        int original_stdin = dup(STDOUT_FILENO);
        if (dup2(output_fd, STDOUT_FILENO) == -1)
        {
            _wstderr("ERROR: Failed to redirect stdout", true);
            close(output_fd);
            exit(EXIT_FAILURE);
        }
        close(output_fd);
        return original_stdin;
    }
    return -1;
}

void restore_stdio(int original_stdio, int target_fd)
{
    if (original_stdio != -1)
    {
        if (dup2(original_stdio, target_fd) == -1)
        {
            _wstderr("ERROR: Failed to restore stdio", true);
            close(original_stdio);
            exit(EXIT_FAILURE);
        }
        close(original_stdio);
    }
}

void execute_cd(char* input, char** sc_tokens, bool background_execution, char* cwd)
{
    // Ignore "&" if appears, as this is an internal command
    if (background_execution)
    {
        // Cleanse "&"
        cleanse_ampersand(input);
    }
    // Check existence of argument
    if (sc_tokens[1])
    {
        // Argument provided
        if (strcmp(sc_tokens[1], "-") == 0 && sc_tokens[2] == NULL)
        {
            // Return to old current directory (if exist)
            const char *old_cwd = getenv(ENV_OLDPWD_KEY);
            if (old_cwd == NULL)
            {
                errno = ENOENT;
                _wstderr("ERROR: No such thing as 'OLDPWD' for now", true);
            }
            else
            {
                // Old current directory exist, head to it if possible and update env
                if (chdir(old_cwd) == -1)
                {
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
            if (chdir(&input[3]) == -1)
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

void execute_echo(char* input, char** sc_tokens, bool background_execution)
{
    // Ignore "&" if appears, as this is an internal command
    if (background_execution)
    {
        // Cleanse "&"
        cleanse_ampersand(input);
    }
    // Check existence of argument
    if(sc_tokens[1])
    {
        // Arg provided; check if starts with '$', in which case it's referencing an env var
        if (sc_tokens[1][0] == '$')
        {
            const char *env_val = getenv(&input[6]);
            if (env_val == NULL)
            {
                // No env var with thy name or any other issue; errno doesn't get set
                _wstderr("ERROR: Unexistent env var or empty arg provided.\n", false);
            }
            else
            {
                puts(env_val);
            }
        }
        else
        {
            puts(&input[5]);
        }
    }
    else
    {
        // No argument; a newline gets printed
        puts("");
    }
}

void execute_external_cmd(char** sc_tokens, bool background_execution)
{
    // If this child proc is being executed in the foreground, certain signals must respond
    if (!background_execution)
    {
        for (int j = LOWEST_ARR_INDEX; j < N_SINGALS_TO_HANDLE; j++)
        {
            // Revert these signals managment to their default behavior
            signal(signals[j], SIG_DFL);
        }
    }
    // Execute this child, pass the torch of the proc to another program
    if (execvp(sc_tokens[0], sc_tokens) == -1)
    {
        // Something went wrong
        _wstderr("ERROR: Command couldn't be executed", true);
        exit(EXIT_FAILURE);
    }
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

void _free_recursively(void** arr, int limit)
{
    // free memory prev allocated and get out
    for (int i = LOWEST_ARR_INDEX; limit == -1 ? arr[i] != NULL : i < limit ; i++)
    {
        free(arr[i]);
    }
    free(arr);
}
