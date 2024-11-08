/**
 * @file main.c
 * @brief Entry point of the system
 */

// STD headers
#include <stdlib.h>

// lib or deps headers
// TODO: For later usage.
// #include <cjson/cJSON.h>

// source headers
#include "shell.h"

//! \brief Main function of the program.
int main(int argc, char* argv[])
{

    if (argc > 2)
    {
        // Make the user know that this shell accept 0 or 1 argument
        _wstderr("ERROR: This shell only takes 1 arg (path to a batch file), or 0 (start shell).\n", false);
    }
    else if (argc == 2)
    {
        // An argument was passed
        execute_batch_file(argv[1]);
    }
    else
    {
        // No argument passed, start main loop
        start_shell_ml();
    }

    return EXIT_SUCCESS;
}
