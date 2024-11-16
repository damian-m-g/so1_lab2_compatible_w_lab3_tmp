/**
 * @file metrics_utils.h
 * @brief "metrics" app utilities definition
 */

#include "metrics_utils.h"

char* get_metrics_json_config_file_path(char** argv)
{
    unsigned char data[N_JSON_ENTRIES] = JSON_ENTRIES_DEF_VAL;
    // First arg is "start_monitor", known
    for (int i = LOWEST_ARR_INDEX + 1; argv[i] != NULL; i++)
    {
        // Inquire for each possible parameter; if `--config` was passed, ignore all the others
        const char *arg = argv[i];
        if ((strlen(arg) > 9) && (strncmp(arg, "--config=", 9) == 0))
        {
            // Return the path to the config file
            return argv[i] + 9;
        }
        // `--update_interval` check
        else if ((strlen(arg) > 18) && (strncmp(arg, "--update_interval=", 18) == 0))
        {
            int seconds = atoi(arg + 18);
            if (seconds <= 0 || seconds > 255)
            {
                char *error_s = "ERROR: `--update_interval` value must be a recognizable int between 1 and 255.";
                fwrite(error_s, sizeof(char), strlen(error_s), stderr);
                return NULL;
            }
            data[0] = seconds;
        }
        // `--cpu` check
        else if ((strlen(arg) > 6) && (strncmp(arg, "--cpu=", 6) == 0))
        {
            if (strcmp(arg + 6, "true") == 0)
            {
                data[1] = 1;
            }
            else if (strcmp(arg + 6, "false") == 0)
            {
                data[1] = 0;
            }
            else
            {
                char *error_s = "ERROR: `--cpu` value must be either \"true\" or \"false\".";
                fwrite(error_s, sizeof(char), strlen(error_s), stderr);
                return NULL;
            }
        }
        // `--mem` check
        else if ((strlen(arg) > 6) && (strncmp(arg, "--mem=", 6) == 0))
        {
            if (strcmp(arg + 6, "true") == 0)
            {
                data[2] = 1;
            }
            else if (strcmp(arg + 6, "false") == 0)
            {
                data[2] = 0;
            }
            else
            {
                char *error_s = "ERROR: `--mem` value must be either \"true\" or \"false\".";
                fwrite(error_s, sizeof(char), strlen(error_s), stderr);
                return NULL;
            }
        }
        // `--hdd` check
        else if ((strlen(arg) > 6) && (strncmp(arg, "--hdd=", 6) == 0))
        {
            if (strcmp(arg + 6, "true") == 0)
            {
                data[3] = 1;
            }
            else if (strcmp(arg + 6, "false") == 0)
            {
                data[3] = 0;
            }
            else
            {
                char *error_s = "ERROR: `--hdd` value must be either \"true\" or \"false\".";
                fwrite(error_s, sizeof(char), strlen(error_s), stderr);
                return NULL;
            }
        }
        // `--net` check
        else if ((strlen(arg) > 6) && (strncmp(arg, "--net=", 6) == 0))
        {
            if (strcmp(arg + 6, "true") == 0)
            {
                data[4] = 1;
            }
            else if (strcmp(arg + 6, "false") == 0)
            {
                data[4] = 0;
            }
            else
            {
                char *error_s = "ERROR: `--net` value must be either \"true\" or \"false\".";
                fwrite(error_s, sizeof(char), strlen(error_s), stderr);
                return NULL;
            }
        }
        // `--procs` check
        else if ((strlen(arg) > 8) && (strncmp(arg, "--procs=", 8) == 0))
        {
            if (strcmp(arg + 8, "true") == 0)
            {
                data[5] = 1;
            }
            else if (strcmp(arg + 8, "false") == 0)
            {
                data[5] = 0;
            }
            else
            {
                char *error_s = "ERROR: `--procs` value must be either \"true\" or \"false\".";
                fwrite(error_s, sizeof(char), strlen(error_s), stderr);
                return NULL;
            }
        }
    }
    // JSON config file must be created
    if (create_metrics_json_config_file(data) == 0)
    {
        // Created successfully, now call the "metrics" app
        return DEFAULT_JSON_CONFIG_FILE_OUTPUT_PATH;
    }
    else
    {
        return NULL;
    }
}

int create_metrics_json_config_file(const unsigned char* data)
{
    // Create JSON as a net of structs
    cJSON* root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "update_interval", data[0]);
    cJSON* metrics = cJSON_AddObjectToObject(root, "metrics");
    cJSON_AddBoolToObject(metrics, "cpu", data[1]);
    cJSON_AddBoolToObject(metrics, "mem", data[2]);
    cJSON_AddBoolToObject(metrics, "hdd", data[3]);
    cJSON_AddBoolToObject(metrics, "net", data[4]);
    cJSON_AddBoolToObject(metrics, "procs", data[5]);
    // Transform it to a string
    char *json_string = cJSON_Print(root);
    // Write that to a file
    FILE *file = fopen(DEFAULT_JSON_CONFIG_FILE_OUTPUT_PATH, "w");
    if (file == NULL) {
        perror("ERROR: Failed to open file for writing");
        cJSON_Delete(root);
        free(json_string);
        return -1;
    }
    fprintf(file, "%s\n", json_string);
    // Close the file
    fclose(file);
    // Clean up
    cJSON_Delete(root);
    free(json_string);
    return 0;
}

void delete_owned_metrics_json_config_file(void)
{
    // If doesn't exist, nothing bad happens
    remove(DEFAULT_JSON_CONFIG_FILE_OUTPUT_PATH);
}
