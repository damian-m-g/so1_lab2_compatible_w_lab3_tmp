/**
 * @file main.c
 * @brief Entry point of the system
 */

// STD headers
#include <stdlib.h>

// source headers
#include "shell.h"

//! \brief This app maximum argc value.
#define APP_MAX_ARGC 2
//! \brief This app first argument index, among argv.
#define ARGV_FIRST_APP_ARG_I 1

//! \brief Main function of the program.
int main(int argc, char* argv[])
{

    if (argc > APP_MAX_ARGC)
    {
        // Make the user know that this shell accept 0 or 1 argument
        wstderr("ERROR: This shell only takes 1 arg (path to a batch file), or 0 (start shell).\n", false);
    }
    else if (argc == APP_MAX_ARGC)
    {
        // An argument was passed
        execute_batch_file(argv[ARGV_FIRST_APP_ARG_I]);
    }
    else
    {
        // No argument passed, start main loop
        start_shell_ml();
    }

    return EXIT_SUCCESS;
}
