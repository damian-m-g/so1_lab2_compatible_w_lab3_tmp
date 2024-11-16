/**
 * @file shell_project.test.c
 * @brief Main testing file.
 */

#include "unity.h"
#include "metrics_utils.h"

/* PROTOTYPES */
void test_get_metrics_json_config_file_path_valid(void);
void test_get_metrics_json_config_file_path_invalid_update_interval(void);
void test_get_metrics_json_config_file_path_invalid_cpu(void);
void test_delete_owned_metrics_json_config_file(void);

// Mock data for testing
char *argv_valid[] = {
    "start_monitor",
    "--config=test_config.json",
    "--update_interval=5",
    "--cpu=true",
    "--mem=false",
    "--hdd=true",
    "--net=false",
    "--procs=true",
    NULL
};

char *argv_invalid_update_interval[] = {
    "start_monitor",
    "--update_interval=300", // Invalid (greater than 255)
    NULL
};

char *argv_invalid_cpu[] = {
    "start_monitor",
    "--cpu=maybe", // Invalid value
    NULL
};

//! \brief Helper function to remove the config file before each test.
void setUp(void) {
    delete_owned_metrics_json_config_file();
}

//! \brief Helper function to clean up after tests.
void tearDown(void) {
    delete_owned_metrics_json_config_file();
}

//! \brief Test for get_metrics_json_config_file_path() with valid arguments.
void test_get_metrics_json_config_file_path_valid(void) {
    char *result = get_metrics_json_config_file_path(argv_valid);
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_STRING("test_config.json", result);
}

//! \brief Test for get_metrics_json_config_file_path() with invalid update interval.
void test_get_metrics_json_config_file_path_invalid_update_interval(void) {
    char *result = get_metrics_json_config_file_path(argv_invalid_update_interval);
    TEST_ASSERT_NULL(result);
}

//! \brief Test for get_metrics_json_config_file_path() with invalid CPU argument.
void test_get_metrics_json_config_file_path_invalid_cpu(void) {
    char *result = get_metrics_json_config_file_path(argv_invalid_cpu);
    TEST_ASSERT_NULL(result);
}

//! \brief Test for delete_owned_metrics_json_config_file() to ensure it removes the file.
void test_delete_owned_metrics_json_config_file(void) {
    // Create the file first
    unsigned char data[N_JSON_ENTRIES] = {5, 1, 0, 1, 0, 1};
    create_metrics_json_config_file(data);
    
    // Ensure the file exists before deletion
    FILE *file = fopen(DEFAULT_JSON_CONFIG_FILE_OUTPUT_PATH, "r");
    TEST_ASSERT_NOT_NULL(file);
    fclose(file);

    // Now delete the file
    delete_owned_metrics_json_config_file();

    // Check that the file no longer exists
    file = fopen(DEFAULT_JSON_CONFIG_FILE_OUTPUT_PATH, "r");
    TEST_ASSERT_NULL(file); // File should not exist anymore
}

//! \brief Main function for testing.
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_get_metrics_json_config_file_path_valid);
    RUN_TEST(test_get_metrics_json_config_file_path_invalid_update_interval);
    RUN_TEST(test_get_metrics_json_config_file_path_invalid_cpu);
    RUN_TEST(test_delete_owned_metrics_json_config_file);
    return UNITY_END();
}
