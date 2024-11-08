/**
 * @file cmd_utils.h
 * @brief Commands utilities declaration
 */

#ifndef CMD_UTILS_H
#define CMD_UTILS_H

#include <string.h>
#include <stdbool.h>

/**
 * @brief Checks for " &" existence at the end of the command, removing it from the token.
 * @param arg Argument of the internal command.
 * @return true if a background execution was called, false otherwise.
 */
bool is_background_exec(char *arg);

#endif
