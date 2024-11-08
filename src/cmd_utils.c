/**
 * @file cmd_utils.c
 * @brief Commands utilities definition
 */

#include "cmd_utils.h"

bool is_background_exec(char *arg)
{
    // Ignore " &" if appears, as this is an internal command
    if (arg != NULL)
    {
        const unsigned st_len = (unsigned)strlen(arg);
        if ((st_len == 1) && (strcmp(arg, "&") == 0))
        {
            arg[0] = '\0';
            return true;
        }
        else if ((st_len > 1) && (strcmp(&(arg[st_len - 2]), " &") == 0))
        {
            arg[st_len - 2] = '\0';
            return true;
        }
    }
    return false;
}
