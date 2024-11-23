/**
 * @file cmd_utils.c
 * @brief Commands utilities definition.
 */

#include "cmd_utils.h"

bool is_background_exec(char** tokens)
{
    // Ignore " &" if appears, as this is an internal command
    if (tokens != NULL)
    {
        // Seek for the NULL token
        int i;
        for (i = LOWEST_ARR_INDEX; tokens[i] != NULL; i++)
        {
        }
        if (strcmp(tokens[i - 1], "&") == 0)
        {
            tokens[i - 1] = NULL;
            return true;
        }
    }
    return false;
}

void cleanse_newline(char* input)
{
    const size_t input_len = strlen(input);
    if (input[input_len - 1] == '\n')
    {
        input[input_len - 1] = STR_NULL_TERMINATOR;
    }
}

void cleanse_ampersand(char* input)
{
    const size_t input_len = strlen(input);
    input[input_len - 2] = STR_NULL_TERMINATOR;
}

char* get_possible_redirection(char** argv, bool _stdin)
{
    for (int i = LOWEST_ARR_INDEX; argv[i] != NULL; i++)
    {
        if (strcmp(argv[i], _stdin ? "<" : ">") == 0)
        {
            // This could even be NULL
            return argv[i + 1];
        }
    }
    return NULL;
}

void cleanse_redirections_on_argv(char** argv)
{
    for (int i = LOWEST_ARR_INDEX; argv[i] != NULL; i++)
    {
        if (strcmp(argv[i], ">") == 0 || strcmp(argv[i], "<") == 0)
        {
            argv[i] = NULL;
            return;
        }
    }
}

void cleanse_redirections_on_sc(char* sc)
{
    const size_t sc_len = strlen(sc);
    int first_char_redirecting = -1;
    // No cmd is permitted redirection at the leftmost, on a char index less than AMP_MIN_INDEX_APPEARANCE
    for (int i = sc_len - 1; i >= AMP_MIN_INDEX_APPEARANCE; i--)
    {
        if (sc[i] == '<' || sc[i] == '>')
        {
            first_char_redirecting = i;
        }
    }
    if (first_char_redirecting != -1)
    {
        // cleanse prefix space if exist
        if (sc[first_char_redirecting - 1] == ' ')
        {
            sc[first_char_redirecting - 1] = STR_NULL_TERMINATOR;
        }
        else
        {
            sc[first_char_redirecting] = STR_NULL_TERMINATOR;
        }
    }
    else if (sc[sc_len - 1] == ' ')
    {
        // cleanse trailing space
        sc[sc_len - 1] = STR_NULL_TERMINATOR;
    }
}
