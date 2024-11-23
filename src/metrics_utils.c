/**
 * @file metrics_utils.h
 * @brief "metrics" app utilities definition.
 */

#include "metrics_utils.h"

char* get_metrics_json_config_file_path(char** argv)
{
    unsigned char data[N_JSON_ENTRIES] = JSON_ENTRIES_DEF_VAL;
    // First arg is "start_monitor", known
    for (int i = LOWEST_ARR_INDEX + 1; argv[i] != NULL; i++)
    {
        // Inquire for each possible parameter; if `--config` was passed, ignore all the others
        const char* arg = argv[i];
        if ((strlen(arg) > CONFIG_OPL) && (strncmp(arg, "--config=", CONFIG_OPL) == 0))
        {
            // Return the path to the config file
            return argv[i] + CONFIG_OPL;
        }
        // `--update_interval` check
        else if ((strlen(arg) > UPDATE_I_OPL) && (strncmp(arg, "--update_interval=", UPDATE_I_OPL) == 0))
        {
            int seconds = atoi(arg + UPDATE_I_OPL);
            if (seconds <= UPDATE_I_MINV || seconds > UPDATE_I_MAXV)
            {
                char* error_s = "ERROR: `--update_interval` value must be a recognizable int between 1 and 255.";
                fwrite(error_s, sizeof(char), strlen(error_s), stderr);
                return NULL;
            }
            data[UPDATE_I_I] = seconds;
        }
        // `--cpu` check
        else if ((strlen(arg) > CPU_OPL) && (strncmp(arg, "--cpu=", CPU_OPL) == 0))
        {
            if (strcmp(arg + CPU_OPL, "true") == 0)
            {
                data[CPU_I] = true;
            }
            else if (strcmp(arg + CPU_OPL, "false") == 0)
            {
                data[CPU_I] = false;
            }
            else
            {
                char* error_s = "ERROR: `--cpu` value must be either \"true\" or \"false\".";
                fwrite(error_s, sizeof(char), strlen(error_s), stderr);
                return NULL;
            }
        }
        // `--mem` check
        else if ((strlen(arg) > MEM_OPL) && (strncmp(arg, "--mem=", MEM_OPL) == 0))
        {
            if (strcmp(arg + MEM_OPL, "true") == 0)
            {
                data[MEM_I] = true;
            }
            else if (strcmp(arg + MEM_OPL, "false") == 0)
            {
                data[MEM_I] = false;
            }
            else
            {
                char* error_s = "ERROR: `--mem` value must be either \"true\" or \"false\".";
                fwrite(error_s, sizeof(char), strlen(error_s), stderr);
                return NULL;
            }
        }
        // `--hdd` check
        else if ((strlen(arg) > HDD_OPL) && (strncmp(arg, "--hdd=", HDD_OPL) == 0))
        {
            if (strcmp(arg + HDD_OPL, "true") == 0)
            {
                data[HDD_I] = true;
            }
            else if (strcmp(arg + HDD_OPL, "false") == 0)
            {
                data[HDD_I] = false;
            }
            else
            {
                char* error_s = "ERROR: `--hdd` value must be either \"true\" or \"false\".";
                fwrite(error_s, sizeof(char), strlen(error_s), stderr);
                return NULL;
            }
        }
        // `--net` check
        else if ((strlen(arg) > NET_OPL) && (strncmp(arg, "--net=", NET_OPL) == 0))
        {
            if (strcmp(arg + NET_OPL, "true") == 0)
            {
                data[NET_I] = true;
            }
            else if (strcmp(arg + NET_OPL, "false") == 0)
            {
                data[NET_I] = false;
            }
            else
            {
                char* error_s = "ERROR: `--net` value must be either \"true\" or \"false\".";
                fwrite(error_s, sizeof(char), strlen(error_s), stderr);
                return NULL;
            }
        }
        // `--procs` check
        else if ((strlen(arg) > PROCS_OPL) && (strncmp(arg, "--procs=", PROCS_OPL) == 0))
        {
            if (strcmp(arg + PROCS_OPL, "true") == 0)
            {
                data[PROCS_I] = true;
            }
            else if (strcmp(arg + PROCS_OPL, "false") == 0)
            {
                data[PROCS_I] = false;
            }
            else
            {
                char* error_s = "ERROR: `--procs` value must be either \"true\" or \"false\".";
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
    cJSON_AddNumberToObject(root, "update_interval", data[UPDATE_I_I]);
    cJSON* metrics = cJSON_AddObjectToObject(root, "metrics");
    cJSON_AddBoolToObject(metrics, "cpu", data[CPU_I]);
    cJSON_AddBoolToObject(metrics, "mem", data[MEM_I]);
    cJSON_AddBoolToObject(metrics, "hdd", data[HDD_I]);
    cJSON_AddBoolToObject(metrics, "net", data[NET_I]);
    cJSON_AddBoolToObject(metrics, "procs", data[PROCS_I]);
    // Transform it to a string
    char* json_string = cJSON_Print(root);
    // Write that to a file
    FILE* file = fopen(DEFAULT_JSON_CONFIG_FILE_OUTPUT_PATH, "w");
    if (file == NULL)
    {
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
