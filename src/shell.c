/**
 * @file shell.c
 * @brief Custom shell related logic definition.
 */

#include "shell.h"

// Global variables
//! \brief Status from "metrics" handled. Gets set to true when the "metrics" app signals with its status data.
static bool sfmh = false;

void start_shell_ml()
{
    // The shell itself mustn't answer to certain signals
    for (int i = LOWEST_ARR_INDEX; i < N_SINGALS_TO_HANDLE; i++)
    {
        signal(signals[i], SIG_IGN);
    }

    // Get the 3 parts that conform the command line prompt
    const char* user = getenv(ENV_USER_KEY);
    char host[HOST_NAME_MAX + 1];
    gethostname(host, (HOST_NAME_MAX + 1));
    char cwd[PATH_MAX];
    if (getcwd(cwd, PATH_MAX) == NULL)
    {
        wstderr("ERROR: cwd can't be retrieved", true);
        exit(EXIT_FAILURE);
    }

    // Main loop
    while (true)
    {
        printf("%s@%s:%s$ ", user, host, cwd);
        // Buffer for the input
        static char input[ARG_MAX];
        if (fgets(input, ARG_MAX, stdin) != NULL)
        {
            execute_command(input, cwd);
        }
    }
}

void execute_command(char* input, char* cwd)
{
    // Initialize job counter
    static unsigned long long int job_id = 0;
    // There're custom commands that work with the "metrics" app (lab 1); keep track of some data
    static int metrics_pid = PID_UNASSIGNED;
    // Cleanse the newline added at the end, if exist
    cleanse_newline(input);
    // Let's dup this value to a helper, for strtok() usage; the original one'll be useful as pristine later
    static char input_h[ARG_MAX];
    strcpy(input_h, input);

    // Tokenize single commands, per pipe
    char* single_commands[MAX_SINGLE_COMMANDS];
    // Will remain with this value if empty input was submitted; # of SC
    int sc_n = LOWEST_ARR_INDEX;
    char* token = strtok(input_h, SC_TOKEN_SEPARATOR);
    while (token != NULL && sc_n < MAX_SINGLE_COMMANDS)
    {
        single_commands[sc_n++] = token;
        token = strtok(NULL, SC_TOKEN_SEPARATOR);
    }

    // How many single commands (separated by |) were submitted: none, one or multiple?
    if (sc_n == 0)
        return;
    // One single command was submitted
    if (sc_n == 1)
    {
        // One single command submitted; update job id
        job_id++;
        // Explicit freed of memory isn't done, relies on the OS when exit() gets called
        char** sc_tokens = tokenize_single_command(single_commands[LOWEST_ARR_INDEX]);
        // Check if "&" appears, to see if it requires background execution
        bool background_execution = is_background_exec(sc_tokens);
        // Redirections implementation; check for "<" appearance
        char* file_name = get_possible_redirection(sc_tokens, true);
        int original_stdin = redirect_stdin(file_name);
        // Check for ">" appearance; re-use file_name, as it is a buffer
        file_name = get_possible_redirection(sc_tokens, false);
        int original_stdout = redirect_stdout(file_name);
        // cleanse single command tokens & the string itself from redirection tokens
        cleanse_redirections_on_argv(sc_tokens);
        cleanse_redirections_on_sc(input);
        // Internal commands, when called solo, are always executed in the foreground, as they are quick
        if (strcmp(sc_tokens[LOWEST_ARR_INDEX], "cd") == 0)
        {
            execute_cd(input, sc_tokens, background_execution, cwd);
        }
        else if (strcmp(sc_tokens[LOWEST_ARR_INDEX], "clr") == 0)
        {
            printf(CLR_ANSI_EC);
            fflush(stdout);
        }
        else if (strcmp(sc_tokens[LOWEST_ARR_INDEX], "echo") == 0)
        {
            execute_echo(input, sc_tokens, background_execution);
        }
        else if (strcmp(sc_tokens[LOWEST_ARR_INDEX], "quit") == 0)
        {
            // Try to end zombie processes
            while (waitpid(-1, NULL, WNOHANG) > 0)
            {
            }
            // In case the JSON config file for "metrics" was created, try its deletion
            delete_owned_metrics_json_config_file();
            // do exit
            exit(EXIT_SUCCESS);
        }
        else if (strcmp(sc_tokens[LOWEST_ARR_INDEX], "stop_monitor") == 0)
        {
            execute_stop_monitor(&metrics_pid);
        }
        else if (strcmp(sc_tokens[LOWEST_ARR_INDEX], "status_monitor") == 0)
        {
            execute_status_monitor(&metrics_pid);
        }
        else if (strcmp(sc_tokens[LOWEST_ARR_INDEX], "explore_filesystem") == 0)
        {
            execute_explore_filesystem(sc_tokens);
        }
        else
        {
            // Potential external or monitor-related command invocation
            const pid_t pid_child = fork();
            if (pid_child == -1)
            {
                wstderr("ERROR: Forking of current process failed", true);
                return;
            }
            else if (pid_child == 0)
            {
                if (strcmp(sc_tokens[LOWEST_ARR_INDEX], "start_monitor") == 0)
                {
                    char* metrics_json_config_file_path = get_metrics_json_config_file_path(sc_tokens);
                    char* argv[METRICS_MAX_ARGC] = {METRICS_APP_PATH, metrics_json_config_file_path};
                    execute_external_cmd(argv, background_execution);
                }
                else
                {
                    execute_external_cmd(sc_tokens, background_execution);
                }
            }
            // Save the child pid if "start_monitor" command was called
            if (strcmp(sc_tokens[LOWEST_ARR_INDEX], "start_monitor") == 0)
            {
                metrics_pid = pid_child;
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
                // Non concurrent execution, wait for child process to finish
                if (waitpid(pid_child, NULL, 0) == -1)
                {
                    wstderr("ERROR: waitpid() failed", true);
                }
            }
        }
        // Restore stdin and stdout if were modified
        restore_stdio(original_stdin, STDIN_FILENO);
        restore_stdio(original_stdout, STDOUT_FILENO);
    }
    else
    {
        // Multiple single command (sc_n > 1) submitted, pipe implementation called
        int pipesfd[2 * (sc_n - 1)];
        // Pipes creation
        for (int i = LOWEST_ARR_INDEX; i < sc_n; i++)
        {
            if (pipe(pipesfd + i * 2) == -1)
            {
                wstderr("ERROR: On pipe creation", true);
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
            char** sc_tokens = tokenize_single_command(sc_h);
            // Check if "&" appears, to see if it requires background execution
            bool background_execution = is_background_exec(sc_tokens);
            // Fork main process
            const pid_t pid_child = fork();
            if (pid_child == -1)
            {
                wstderr("ERROR: Forking of current process failed", true);
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
                        wstderr("ERROR: dup2() failed", true);
                        exit(EXIT_FAILURE);
                    }
                }
                // Last single command doesn't need its stdout set to be certain end of a pipe
                if (i < (sc_n - 1))
                {
                    // Set the stdout of this process to be certain end of a pipe (write end)
                    if (dup2(pipesfd[i * 2 + 1], STDOUT_FILENO) == -1)
                    {
                        wstderr("ERROR: dup2() failed", true);
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
                    const char* file_name = get_possible_redirection(sc_tokens, true);
                    redirect_stdin(file_name);
                }
                else if (i == (sc_n - 1))
                {
                    // Last single command, accepts ">"; check for its appearance
                    const char* file_name = get_possible_redirection(sc_tokens, false);
                    redirect_stdout(file_name);
                }
                // cleanse single command tokens & the string itself from redirection tokens
                cleanse_redirections_on_argv(sc_tokens);
                cleanse_redirections_on_sc(single_commands[i]);
                // Watch out if the command called is internal or external
                if (strcmp(sc_tokens[LOWEST_ARR_INDEX], "cd") == 0)
                {
                    execute_cd(single_commands[i], sc_tokens, background_execution, cwd);
                }
                else if (strcmp(sc_tokens[LOWEST_ARR_INDEX], "clr") == 0)
                {
                    printf(CLR_ANSI_EC);
                    fflush(stdout);
                }
                else if (strcmp(sc_tokens[LOWEST_ARR_INDEX], "echo") == 0)
                {
                    execute_echo(single_commands[i], sc_tokens, background_execution);
                }
                else if (strcmp(sc_tokens[LOWEST_ARR_INDEX], "quit") == 0)
                {
                    // As a coupled command, does nothing
                }
                else if (strcmp(sc_tokens[LOWEST_ARR_INDEX], "status_monitor") == 0)
                {
                    execute_status_monitor(&metrics_pid);
                }
                else if (strcmp(sc_tokens[LOWEST_ARR_INDEX], "explore_filesystem") == 0)
                {
                    execute_explore_filesystem(sc_tokens);
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
                wstderr("ERROR: waitpid() failed", true);
            }
        }
    }
}

char** tokenize_single_command(char* sc)
{
    char** argv = malloc((MAX_TOKENS_PER_COMMAND + 1) * sizeof(char*));
    if (argv == NULL)
    {
        wstderr("ERROR: Failed to allocate memory", true);
        exit(EXIT_FAILURE);
    }
    int argc = LOWEST_ARR_INDEX;
    char* token = strtok(sc, TOKEN_SEPARATOR);
    while (token != NULL && argc < MAX_TOKENS_PER_COMMAND)
    {
        argv[argc++] = malloc(strlen(token) + 1);
        if (argv[argc - 1] == NULL)
        {
            wstderr("ERROR: Failed to allocate memory", true);
            free_recursively((void**)argv, -1);
            exit(EXIT_FAILURE);
        }
        strcpy(argv[argc - 1], token);
        token = strtok(NULL, TOKEN_SEPARATOR);
    }
    // Check if max amount of tokens were reached and more of them are available to consume
    if (argc == MAX_TOKENS_PER_COMMAND && token != NULL)
    {
        wstderr("ERROR: You surpassed the arguments limit for a command.\n", false);
        free_recursively((void**)argv, argc);
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
    if (getcwd(cwd, PATH_MAX) == NULL)
    {
        wstderr("ERROR: cwd can't be retrieved", true);
        exit(EXIT_FAILURE);
    }
    // Open file
    FILE* file = fopen(path, "r");
    if (file == NULL)
    {
        wstderr("ERROR: Opening batch file", true);
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
            wstderr("ERROR: Failed to open input file", true);
            exit(EXIT_FAILURE);
        }
        // duplicate the original file descriptor number to return if everything goes well
        int original_stdin = dup(STDIN_FILENO);
        if (dup2(input_fd, STDIN_FILENO) == -1)
        {
            wstderr("ERROR: Failed to redirect stdin", true);
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
            wstderr("ERROR: Failed to open output file", true);
            exit(EXIT_FAILURE);
        }
        // duplicate the original file descriptor number to return if everything goes well
        int original_stdin = dup(STDOUT_FILENO);
        if (dup2(output_fd, STDOUT_FILENO) == -1)
        {
            wstderr("ERROR: Failed to redirect stdout", true);
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
            wstderr("ERROR: Failed to restore stdio", true);
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
    if (sc_tokens[SC_FIRST_ARG_I])
    {
        // Argument provided
        if (strcmp(sc_tokens[SC_FIRST_ARG_I], "-") == 0 && sc_tokens[SC_SECOND_ARG_I] == NULL)
        {
            // Return to old current directory (if exist)
            const char* old_cwd = getenv(ENV_OLDPWD_KEY);
            if (old_cwd == NULL)
            {
                errno = ENOENT;
                wstderr("ERROR: No such thing as 'OLDPWD' for now", true);
            }
            else
            {
                // Old current directory exist, head to it if possible and update env
                if (chdir(old_cwd) == -1)
                {
                    wstderr("ERROR: Issue changing cwd to the old one", true);
                }
                else
                {
                    // Success
                    setenv(ENV_OLDPWD_KEY, cwd, true);
                    if (getcwd(cwd, PATH_MAX) == NULL)
                    {
                        wstderr("ERROR: cwd can't be retrieved", true);
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
        else
        {
            // Try to move to a new current directory
            if (chdir(&input[CD_ARG_START_I]) == -1)
            {
                // The argument is wrong or another problem appeared
                wstderr("ERROR: Can't change current working directory", true);
            }
            else
            {
                // The arguments is right, proceed with the normal procedure
                setenv(ENV_OLDPWD_KEY, cwd, true);
                if (getcwd(cwd, PATH_MAX) == NULL)
                {
                    wstderr("ERROR: cwd can't be retrieved", true);
                    exit(EXIT_FAILURE);
                }
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
    if (sc_tokens[SC_FIRST_ARG_I])
    {
        // Arg provided; check if starts with '$', in which case it's referencing an env var
        if (sc_tokens[SC_FIRST_ARG_I][LOWEST_ARR_INDEX] == '$')
        {
            const char* env_val = getenv(&input[6]);
            if (env_val == NULL)
            {
                // No env var with thy name or any other issue; errno doesn't get set
                wstderr("ERROR: Unexistent env var or empty arg provided.\n", false);
            }
            else
            {
                puts(env_val);
            }
        }
        else
        {
            puts(&input[ECHO_ARG_START_I]);
        }
    }
    else
    {
        // No argument; a newline gets printed
        puts("");
    }
}

void execute_stop_monitor(int* metrics_pid)
{
    if (*metrics_pid != -1)
    {
        if (kill(*metrics_pid, SIGTERM) == -1)
        {
            wstderr("ERROR: \"metrics\" child process can't be killed", true);
        }
        else
        {
            puts("metrics successfully stopped.");
        }
        *metrics_pid = -1;
    }
}

void execute_status_monitor(const int* metrics_pid)
{
    if (*metrics_pid != -1)
    {
        // Only if the monitor was started, get its status; subscribe to listening to a response
        struct sigaction sa;
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = handle_sigusr1;
        sigemptyset(&sa.sa_mask);
        sa.sa_restorer = NULL;
        if (sigaction(SIGUSR1, &sa, NULL) == -1)
        {
            wstderr("ERROR: Signal subscription failed", true);
        }
        else
        {
            // Successfully subscribed to catch response from the child process; send signal
            union sigval directive;
            directive.sival_int = METRICS_GET_STATUS_CODE;
            if (sigqueue(*metrics_pid, SIGUSR1, directive) == -1)
            {
                wstderr("ERROR: Signaling \"status_monitor\"", true);
            }
            else
            {
                // Signal successfully sent, await response
                time_t l_start_t, l_current_t;
                time(&l_start_t);
                while (true)
                {
                    time(&l_current_t);
                    if (sfmh)
                    {
                        // Signal from metrics with its status successfully handled, reset it
                        sfmh = false;
                        break;
                    }
                    else if (difftime(l_current_t, l_start_t) >= WAIT_T_FOR_METRICS_RESPONSE)
                    {
                        wstderr("ERROR: Timeout reached. No response from \"metrics\" app.\n", false);
                        break;
                    }
                }
            }
            // Unsubscribe to SIGUSR1
            sa.sa_flags = 0;
            sa.sa_handler = SIG_DFL;
            if (sigaction(SIGUSR1, &sa, NULL) == -1)
            {
                wstderr("ERROR: Signal unsubscription failed", true);
            }
        }
    }
    else
    {
        puts("WARNING: metrics app not initialized, or not tracked by this Shell.");
    }
}

void execute_explore_filesystem(char** sc_tokens)
{
    // The first arg of the cmd is only taken into account and must exist
    if (!sc_tokens[SC_FIRST_ARG_I])
    {
        wstderr("ERROR: This command needs a path to a dir as a lone arg.\n", false);
        return;
    }
    // Argument provided
    const char* path = sc_tokens[SC_FIRST_ARG_I];
    struct stat stat_buffer;
    // Check path existence, and if it's in fact a dir, which is what's expected
    if (stat(path, &stat_buffer) == 0)
    {
        if (S_ISDIR(stat_buffer.st_mode))
        {
            // Path exist and it's a dir
            traverse_directory(path);
        }
        else
        {
            wstderr("ERROR: The path provided isn't a dir.\n", false);
        }
    }
    else
    {
        wstderr("ERROR: Provided path to a potential dir doesn't exist.\n", false);
    }
}

void execute_external_cmd(char** sc_tokens, bool background_execution)
{
    // If this child proc is being executed in the foreground, certain signals must respond
    if (!background_execution)
    {
        for (int i = LOWEST_ARR_INDEX; i < N_SINGALS_TO_HANDLE; i++)
        {
            // Revert these signals managment to their default behavior
            signal(signals[i], SIG_DFL);
        }
    }
    // Execute this child, pass the torch of the proc to another program
    if (execvp(sc_tokens[LOWEST_ARR_INDEX], sc_tokens) == -1)
    {
        // Something went wrong
        wstderr("ERROR: Command couldn't be executed", true);
        exit(EXIT_FAILURE);
    }
}

void handle_sigusr1(int sig, siginfo_t* info, void* context)
{
    // Args ignored
    (void)context;
    (void)sig;

    // "monitor" app status encoded
    int e_status = info->si_value.sival_int;
    // Decode
    unsigned char d_status[G_STATUS_N_METRICS_TRACKED] = {0};
    /** Index meaning (LSB to MSB):
     * 0 - cpu_usage_percentage
     * 1 - memory_used_percentage
     * 2 - sectors_read_rate
     * 3 - sectors_written_rate
     */
    d_status[D_STATUS_CPU_I] = (unsigned char)((e_status >> LSBIT_CPU_EMSD) & LSBYTE_MASK);
    d_status[D_STATUS_RAM_I] = (unsigned char)((e_status >> LSBIT_RAM_EMSD) & LSBYTE_MASK);
    d_status[D_STATUS_HDDR_I] = (unsigned char)((e_status >> LSBIT_HDDR_EMSD) & LSBYTE_MASK);
    d_status[D_STATUS_HDDW_I] = (unsigned char)((e_status >> LSBIT_HDDW_EMSD) & LSBYTE_MASK);
    // print data to stdout
    printf("metrics app (working: OK) data\n"
           "------------------------------\n"
           "CPU usage: %u %%\n"
           "RAM usage: %u %%\n"
           "HDD Sectors (512 KB each) read/s: %u\n"
           "HDD Sectors (512 KB each) written/s: %u\n",
           d_status[D_STATUS_CPU_I], d_status[D_STATUS_RAM_I], d_status[D_STATUS_HDDR_I], d_status[D_STATUS_HDDW_I]);

    // set the flag as "monitor" status was received
    sfmh = true;
}

void wstderr(const char* s, bool use_perror)
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

void free_recursively(void** arr, int limit)
{
    // free memory prev allocated and get out
    for (int i = LOWEST_ARR_INDEX; limit == -1 ? arr[i] != NULL : i < limit; i++)
    {
        free(arr[i]);
    }
    free(arr);
}
