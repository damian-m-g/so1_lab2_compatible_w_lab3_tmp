/**
 * @file shell.h
 * @brief Custom shell related logic declaration.
 */

#ifndef SHELL_H
#define SHELL_H

#include "cmd_utils.h"
#include "metrics_utils.h"
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <wait.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>

//! \brief Environment variable key to retrieve the computer current user.
#define ENV_USER_KEY "USER"
//! \brief First layer separator of smybols/tokens/words on a shell command, that creates single commands.
#define SC_TOKEN_SEPARATOR "|"
//! \brief Char or group of chars that define the separation of smybols/tokens/words on a single command.
#define TOKEN_SEPARATOR " "
//! \brief Lowest array index.
#define LOWEST_ARR_INDEX 0
//! \brief Environment variable key to retrieve the last ("old") current working directory.
#define ENV_OLDPWD_KEY "OLDPWD"
//! \brief Environment variable key to retrieve the current working directory.
#define ENV_PWD_KEY "PWD"
//! \brief Average delay that some terminals (in IDEs, Shells, etc.) take to flush stdout/stderr, in microseconds.
#define TERMINAL_FLUSH_DELAY 30000
//! \brief ANSI escape codes that moves the cursor to the home position and clears the screen.
#define CLR_ANSI_EC "\033[H\033[J"
//! \brief This shell has a max limit of tokens per command.
#define MAX_TOKENS_PER_COMMAND 32
//! \brief Amount of signals to handle, has direct relationship with the signals array.
#define N_SINGALS_TO_HANDLE 4
//! \brief Signals handled or not, depending on the process currently executed.
static const int signals[N_SINGALS_TO_HANDLE] = {SIGINT, SIGTERM, SIGTSTP, SIGQUIT};
//! \brief Maximum number of single commands to execute, given the max length allowed for an absolute command.
#define MAX_SINGLE_COMMANDS (ARG_MAX / 4)
//! \brief Path to the metrics (lab #1) app.
#define METRICS_APP_PATH "/opt/metrics"
//! \brief A code, that the "metrics" app understands as "get status".
# define METRICS_GET_STATUS_CODE 7
//! \brief Seconds awaited to receive response from metrics app after a "get status" req, when executed in foreground.
# define WAIT_T_FOR_METRICS_RESPONSE 3.0
//! \brief Number of metrics returned by a "get status" by the metrics app.
#define G_STATUS_N_METRICS_TRACKED 4

/**
 * @brief Starts the custom shell main loop.
 */
void start_shell_ml(void);

/**
 * @brief Passed a single command, tokenize it, and return an array with each token.
 * @param sc Single command.
 * @return Array of tokens. Last "useful" element is always NULL. Must be manually freed after being used.
 */
char** tokenize_single_command(char* sc);

/**
 * @brief Executes certain command, could be an internal one, external one, or unexistent.
 * @param input Command input.
 * @param cwd Current working directory. This variable could be updated inside.
 */
void execute_command(char* input, char* cwd);

/**
 * @brief Tries to execute a certain (no comments, one line per command) batch file.
 * @param path Path to the batch file.
 */
void execute_batch_file(const char* path);

/**
 * @brief Redirects the stdin to a specific existent (hopefully) file.
 * @param file_name Relative or absolute path to the file to which the stdin will be redirected.
 * @return -1 if no redirection was performed; otherwise returns the original file descriptor id.
 */
int redirect_stdin(const char* file_name);

/**
 * @brief Redirects the stdout to a specific existent or inexistent (will get created) file.
 * @param file_name Relative or absolute path to the file to which the stdout will be redirected. If it doesn't exist,
 * will get created.
 * @return -1 if no redirection was performed; otherwise returns the original file descriptor id.
 */
int redirect_stdout(const char* file_name);

/**
 * @brief Restores stdin to its original fd for the current process.
 * @param original_stdio File descriptor id of the original stdin, or -1 (in which case this function does nothing)
 * @param target_fd Target file descriptor to set.
 */
void restore_stdio(int original_stdio, int target_fd);

/**
 * @brief "Change directory" internal command.
 * @param input Single command input.
 * @param sc_tokens Single command tokens.
 * @param background_execution Should be executed in the background?
 * @param cwd Current working directory. Used too as a buffer, where the new cd (if) will be saved.
 */
void execute_cd(char* input, char** sc_tokens, bool background_execution, char* cwd);

/**
 * @brief "Echo" internal command.
 * @param input Single command input.
 * @param sc_tokens Single command tokens.
 * @param background_execution Should be executed in the background?
 */
void execute_echo(char* input, char** sc_tokens, bool background_execution);

/**
 * @brief Executes the "stop_monitor" command, which stops the "metrics" app, if it was init by this Shell.
 * @param metrics_pid The "metrics" app process id.
 */
void execute_stop_monitor(int *metrics_pid);

/**
 * @brief Executes the "status_monitor" command, that shows the "metrics" app, if it was init by this Shell.
 * @param metrics_pid The "metrics" app process id.
 */
void execute_status_monitor(const int *metrics_pid);

/**
 * @brief Potential external command execution.
 * @param sc_tokens Single command tokens.
 * @param background_execution Should be executed in the background?
 */
void execute_external_cmd(char** sc_tokens, bool background_execution);

/**
 * @brief You don't call this function directly, this is a handler.
 * @param sig Not used inside the function.
 * @param info Metadata of the signal.
 * @param context Not used inside the function.
 */
void handle_sigusr1(int sig, siginfo_t *info, void *context);

/**
 * @brief Monkeypatch of perror and fprintf(stderr, ...). Needed due to "bad" management of some IDE/Shell terminals.
 * @param s String to write to stderr.
 * @param use_perror Pass true to use perror(), pass false to use fwrite() to stderr. Do not pass true if errno
 * wasn't set by the code throwing an error.
 */
void _wstderr(const char* s, bool use_perror);

/**
 * @brief Auxiliary function to free dinamycally allocated memory recursively.
 * @param arr Array of array. Could be for example an array of strings (char array).
 * @param limit The limit at which stop to free elements of the upmost array. Pass -1 as the limit to free until
 * a NULL value is found.
 */
void _free_recursively(void** arr, int limit);

#endif
