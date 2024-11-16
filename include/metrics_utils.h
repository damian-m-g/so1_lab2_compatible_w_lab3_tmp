/**
 * @file metrics_utils.h
 * @brief "metrics" app utilities declaration.
 */

#ifndef METRICS_UTILS_H
#define METRICS_UTILS_H

#include <cjson/cJSON.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <limits.h>
#include <string.h>

//! \brief Lowest array index.
#define LOWEST_ARR_INDEX 0
//! \brief Total amount of key-value pairs in the JSON config file.
#define N_JSON_ENTRIES 6
/**
 * \brief JSON config file default values for key-value pairs.
 * - 0: update_interval (in seconds, only integer).
 * - 1: cpu (take or not metric).
 * - 2: mem (take or not metric).
 * - 3: hdd (take or not metric).
 * - 4: net (take or not metric).
 * - 5: procs (take or not metric).
 */
#define JSON_ENTRIES_DEF_VAL {1, 1, 1, 1, 1, 1}
//! \brief Default JSON configuration file output path
#define DEFAULT_JSON_CONFIG_FILE_OUTPUT_PATH "/tmp/metrics_config.json"

/**
 * @brief According to argv passed by the user of the shell, parses them and get the path to the "metrics" config file.
 * @param argv Typical argv passed to any program. Last element of the array must be NULL to mark its end.
 * @return The path to the JSON configuration file that shall be used when calling the "metrics" app.
 */
char* get_metrics_json_config_file_path(char** argv);

/**
 * @brief Creates the "metrics" JSON configuration file, that it's going to use.
 * @param data Filtered data, about the JSON config file entries.
 * @return 0 if everything went OK, -1 if some problem arised.
 */
int create_metrics_json_config_file(const unsigned char* data);

/**
 * @brief Deletes the owned (created by this app) "metrics" JSON configuration file, if exist.
 */
void delete_owned_metrics_json_config_file(void);

#endif
