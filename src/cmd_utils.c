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

void traverse_directory(const char* dir_path)
{
    // Open the directory file, to interpret its metadata
    DIR* dir = opendir(dir_path);
    struct dirent* entry;
    char path[PATH_MAX];
    // Sanity check
    if (!dir)
    {
        perror("ERROR: Couldn't open the dir provided");
        return;
    }
    printf("Explorando el dir: \"%s\" en busca de archivos *.{config|json} ...\n", dir_path);
    // Actually traverse all its content
    while ((entry = readdir(dir)) != NULL)
    {
        // Skip "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }
        // Build the full path
        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);
        // Check if it's a directory
        struct stat stat_buffer;
        if (stat(path, &stat_buffer) == 0 && S_ISDIR(stat_buffer.st_mode))
        {
            // If it's a directory, recurse into it
            traverse_directory(path);
        }
        // Check if it's a regular file and has a valid extension
        else if (S_ISREG(stat_buffer.st_mode) && is_config_file(entry->d_name))
        {
            printf("Archivo de configuración encontrado: \"%s\".\n", path);
            // Open the file
            FILE* file = fopen(path, "r");
            if (file)
            {
                // Print the file content to stdout
                char buffer[READING_BUFFER];
                size_t bytes_read;
                printf("Contenido de \"%s\":\n", path);
                while ((bytes_read = fread((void*)buffer, sizeof(char), sizeof(buffer), file)) > 0)
                {
                    fwrite((void*)buffer, sizeof(char), bytes_read, stdout);
                }
                fclose(file);
                printf("\n");
                // Force the stdout flush
                fflush(stderr);
            }
            else
            {
                perror("ERROR: On config file opening for reading purpose.");
                // Doesn't exist, just continues with the next entry ...
            }
        }
    }
    // Close the dir file opened
    closedir(dir);
}

bool is_config_file(const char* filename)
{
    const char* ext = strrchr(filename, '.');
    return ext && ((strcmp(ext, ".json") == 0 || strcmp(ext, ".config") == 0));
}
