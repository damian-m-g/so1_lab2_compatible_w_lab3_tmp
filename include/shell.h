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

//! \brief Environment variable key to retrieve the computer current user
#define ENV_USER_KEY "USER"

/**
 * @brief Starts the custom shell.
 */
void start_shell(void);
