/**
 * @file shell.h
 * @brief Custom shell related logic declaration
 */

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

//! \brief Environment variable key to retrieve the computer current user
#define ENV_USER_KEY "USER"
//! \brief Char or group of chars that define the separation of smybols/tokens/words on a shell command
#define TOKEN_SEPARATOR " \n"
//! \brief Lowest array index
#define LOWEST_ARR_INDEX 0
//! \brief Environment variable key to retrieve the last ("old") current working directory
#define ENV_OLDPWD_KEY "OLDPWD"
//! \brief Environment variable key to retrieve the current working directory
#define ENV_PWD_KEY "PWD"
//! \brief Average delay that some terminals (in IDEs, Shells, etc.) take to flush stdout/stderr, in microseconds.
#define TERMINAL_FLUSH_DELAY 30000
//! \brief ANSI escape codes that moves the cursor to the home position and clears the screen
#define CLR_ANSI_EC "\033[H\033[J"
//! \brief This shell has a max limit of tokens per command
#define MAX_TOKENS_PER_COMMAND 32

/**
 * @brief Starts the custom shell.
 */
void start_shell(void);

/**
 * @brief Monkeypatch of perror and fprintf(stderr, ...). Needed due to "bad" management of some IDE/Shell terminals.
 * @param s String to write to stderr.
 * @param use_perror Pass true to use perror(), pass false to use fwrite() to stderr. Do not pass true if errno
 * wasn't set by the code throwing an error.
 * @return NULL
 */
void _wstderr(const char* s, bool use_perror);
