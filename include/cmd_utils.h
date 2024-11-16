/**
 * @file cmd_utils.h
 * @brief Commands utilities declaration.
 */

#ifndef CMD_UTILS_H
#define CMD_UTILS_H

#include <string.h>
#include <stdbool.h>

//! \brief Lowest array index.
#define LOWEST_ARR_INDEX 0

/**
 * @brief Checks for "&" existence at the end of the command, "removing" it from the tokens array.
 * @param tokens Array where elements are each token of a single command. After the last useful token, must be a
 * NULL value.
 * @return true if a background execution was called, false otherwise.
 */
bool is_background_exec(char** tokens);

/**
 * @brief If the arg has a newline at its end, it gets wiped. Modifies the arg.
 * @param input String to cleanse.
 */
void cleanse_newline(char* input);

/**
 * @brief This function doesn't check for valid arg, it expects a valid string with " &" at the end, aka
 * is_background_exec() returning true.
 * @param input String to cleanse.
 */
void cleanse_ampersand(char* input);

/**
 * @brief From a list of tokenized argv strings, check if stdin redirection is found and if it does, return the file.
 * @param argv Tokenized list of strings.
 * @param _stdin Pass true if you're looking for a stdin redirection, false for a stdout redirection.
 * @return The file name, if found; otherwise returns NULL.
 */
char* get_possible_redirection(char** argv, bool _stdin);

/**
 * @brief As redirections are expected at the end of the single command, clean them from argv.
 * @param argv Tokenized list of strings. Could get modified.
 */
void cleanse_redirections_on_argv(char** argv);

/**
 * @brief Cleanse redirection on single command, non-tokenized.
 * @param sc Raw single command. Could modify it.
 */
void cleanse_redirections_on_sc(char* sc);

#endif
