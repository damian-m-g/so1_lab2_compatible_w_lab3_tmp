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

//! \brief Environment variable key to retrieve the computer current user
#define ENV_USER_KEY "USER"
//! \brief Maximum amount of smybols/tokens/words on a shell command
#define MAX_TOKENS 32
//! \brief Char or group of chars that define the separation of smybols/tokens/words on a shell command
#define TOKEN_SEPARATOR " "
//! \brief Lowest array index
#define LOWEST_ARR_INDEX 0
//! \brief Environment variable key to retrieve the last ("old") current working directory
#define ENV_OLDPWD_KEY "OLDPWD"
//! \brief Environment variable key to retrieve the current working directory
#define ENV_PWD_KEY "PWD"
//! \brief Average delay that some terminals (in IDEs, Shells, etc.) take to flush stdout/stderr, in microseconds.
#define TERMINAL_FLUSH_DELAY 30000

/**
 * @brief Starts the custom shell.
 */
void start_shell(void);

/**
 * @brief Monkeypatch implementation of perror. Needed due to "bad" management of some IDE/Shell terminals.
 * @param s String to write to stderr.
 * @return NULL
 */
void _perror(const char* s);
