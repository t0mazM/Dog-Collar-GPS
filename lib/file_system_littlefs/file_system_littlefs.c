#include "file_system_littlefs.h"


static const char *LFS_TAG = "LFS_INTEGRATION";

// LittleFS configuration structure
struct lfs_config cfg;
// LittleFS filesystem object
lfs_t lfs;

// Variables for caching and lookahead buffer
// These must be defined here as they are used by the LittleFS library internally.
// Their sizes are defined in file_system_littlefs.h
static uint8_t lfs_read_buffer[LFS_READ_SIZE];
static uint8_t lfs_prog_buffer[LFS_PROG_SIZE];
static uint8_t lfs_lookahead_buffer[LFS_LOOKAHEAD_SIZE];


// ---------- LittleFS private Callback Functions (using basic low-level functions from ext_flash.h) --------

static int lfs_read(const struct lfs_config *c, lfs_block_t block,
                     lfs_off_t off, void *buffer, lfs_size_t size) {
    ESP_LOGD(LFS_TAG, "lfs_read: block=%lu, off=%lu, size=%lu", block, off, size);
    uint32_t address = (block * LFS_BLOCK_SIZE) + off;
    esp_err_t ret = ext_flash_read(address, (uint8_t *)buffer, size);
    if (ret != ESP_OK) {
        ESP_LOGE(LFS_TAG, "LFS Read Error: Failed to read from 0x%06lX, size %lu, ret %d",
                 address, size, ret);
        return LFS_ERR_IO; 
    }
    return LFS_ERR_OK; 
}


static int lfs_prog(const struct lfs_config *c, lfs_block_t block,
                    lfs_off_t off, const void *buffer, lfs_size_t size) {
    ESP_LOGD(LFS_TAG, "lfs_prog: block=%lu, off=%lu, size=%lu", block, off, size);
    uint32_t address = (block * LFS_BLOCK_SIZE) + off;

    esp_err_t ret = ext_flash_write(address, (const uint8_t *)buffer, size); //Size is checked in ext_flash_write
    if (ret != ESP_OK) {
        ESP_LOGE(LFS_TAG, "LFS Program Error: Failed to write to 0x%06lX, size %lu, ret %d",
                 address, size, ret);
        return LFS_ERR_IO;
    }
    return LFS_ERR_OK;
}


static int lfs_erase(const struct lfs_config *c, lfs_block_t block) {
    ESP_LOGD(LFS_TAG, "lfs_erase: block=%lu", block);

    uint32_t address = block * LFS_BLOCK_SIZE;
    esp_err_t ret = ext_flash_erase_sector(address);

    if (ret != ESP_OK) {
        ESP_LOGE(LFS_TAG, "LFS Erase Error: Failed to erase block at 0x%06lX, ret %d",
                 address, ret);
        return LFS_ERR_IO;
    }
    return LFS_ERR_OK;
}

// Synchronize the flash memory (ensure all pending operations are complete)
static int lfs_sync(const struct lfs_config *c) {

    esp_err_t ret = ext_flash_wait_for_idle(5000); 
    if (ret != ESP_OK) {
        ESP_LOGE(LFS_TAG, "LFS Sync Error: Flash not idle, ret %d", ret);
        return LFS_ERR_IO;
    }
    return LFS_ERR_OK;
}

// ----------------- Public LittleFS Integration Functions ---------------------

esp_err_t lfs_mount_filesystem(bool format_if_fail) {
    ESP_LOGI(LFS_TAG, "Initializing LittleFS configuration...");

    // Add LittleFS callback functions (using basic low-level functions from ext_flash.h)
    cfg.read = lfs_read;
    cfg.prog = lfs_prog;
    cfg.erase = lfs_erase;
    cfg.sync = lfs_sync;

    cfg.read_size = LFS_READ_SIZE;
    cfg.prog_size = LFS_PROG_SIZE;
    cfg.block_size = LFS_BLOCK_SIZE;
    cfg.block_count = LFS_BLOCK_COUNT;
    cfg.block_cycles = LFS_BLOCK_CYCLES;
    cfg.cache_size = LFS_CACHE_SIZE;
    cfg.lookahead_size = LFS_LOOKAHEAD_SIZE;

    // Assign internal buffers for LittleFS operations
    cfg.read_buffer = lfs_read_buffer;
    cfg.prog_buffer = lfs_prog_buffer;
    cfg.lookahead_buffer = lfs_lookahead_buffer;
    cfg.name_max = 64; //TODO: Adjust it later
    cfg.file_max = 0;  // No limit on file size
    cfg.attr_max = 0;  // No limit on file attributes

    ESP_LOGI(LFS_TAG, "Attempting to mount LittleFS...");
    int err = lfs_mount(&lfs, &cfg);

    if (err) {
        ESP_LOGW(LFS_TAG, "Failed to mount LittleFS (%d).", err);
        if (format_if_fail) {
            ESP_LOGI(LFS_TAG, "Formatting LittleFS partition...");
            err = lfs_format(&lfs, &cfg);
            ESP_LOGI(LFS_TAG, "lfs_format returned: %d", err);
            if (err) {
                ESP_LOGE(LFS_TAG, "Failed to format LittleFS partition (%d).", err);
                return ESP_FAIL;
            }
            ESP_LOGI(LFS_TAG, "Formatting successful. Attempting to mount again...");
            err = lfs_mount(&lfs, &cfg);
            ESP_LOGI(LFS_TAG, "lfs_mount (after format) returned: %d", err);
            if (err) {
                ESP_LOGE(LFS_TAG, "Failed to mount LittleFS after formatting (%d).", err);
                return ESP_FAIL;
            }
        } else {
            ESP_LOGE(LFS_TAG, "LittleFS mount failed and format_if_fail is false. Returning error.");
            return ESP_FAIL;
        }
    }
    ESP_LOGI(LFS_TAG, "LittleFS mounted successfully.");
    return ESP_OK;
}

// Helper function to print directory contents
void lfs_list_directory(const char *path) {
    lfs_dir_t dir;
    struct lfs_info info;

    int err = lfs_dir_open(&lfs, &dir, path);
    if (err) {
        ESP_LOGE(LFS_TAG, "Failed to open directory %s (%d)", path, err);
        return;
    }

    ESP_LOGI(LFS_TAG, "Listing directory: %s", path);
    while ((err = lfs_dir_read(&lfs, &dir, &info)) != 0 && info.name[0] != '\0') {
        if (info.type == LFS_TYPE_REG) {
            ESP_LOGI(LFS_TAG, "  File: %s, Size: %lu bytes", info.name, info.size);
        } else if (info.type == LFS_TYPE_DIR) {
            ESP_LOGI(LFS_TAG, "  Dir: %s", info.name);
        }
    }
    if (err < 0) {
        ESP_LOGE(LFS_TAG, "Error reading directory: %d", err);
    }
    lfs_dir_close(&lfs, &dir);
}

void file_system_test(void){
// Mount LittleFS. Set 'true' to format if mounting fails (e.g., first boot)
    if (lfs_mount_filesystem(true) != ESP_OK) {
        ESP_LOGE(LFS_TAG, "Failed to mount LittleFS. Aborting application.");
        return;
    }


    lfs_file_t file;
    int err;

    const char *test_filename = "data.txt";
    const char *write_data_1 = "This is the first line.\n";
    const char *write_data_2 = "This is the second line.";
    char read_buffer[100];
    memset(read_buffer, 0, sizeof(read_buffer));

    // 1. Write to a file
    ESP_LOGI(LFS_TAG, "Attempting to create and write to file: %s", test_filename);
    err = lfs_file_open(&lfs, &file, test_filename, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
    if (err) {
        ESP_LOGE(LFS_TAG, "Failed to open file %s for writing (%d)", test_filename, err);
    } else {
        lfs_ssize_t bytes_written = lfs_file_write(&lfs, &file, write_data_1, strlen(write_data_1));
        if (bytes_written < 0) {
            ESP_LOGE(LFS_TAG, "Failed to write data_1 to file %s (%d)", test_filename, (int)bytes_written);
        } else {
            ESP_LOGI(LFS_TAG, "Successfully wrote %ld bytes of data_1 to %s", bytes_written, test_filename);
        }
        lfs_file_close(&lfs, &file); // Close the file
    }

    // 2. Append to the same file
    ESP_LOGI(LFS_TAG, "Attempting to append to file: %s", test_filename);
    err = lfs_file_open(&lfs, &file, test_filename, LFS_O_WRONLY | LFS_O_APPEND);
    if (err) {
        ESP_LOGE(LFS_TAG, "Failed to open file %s for appending (%d)", test_filename, err);
    } else {
        lfs_ssize_t bytes_written = lfs_file_write(&lfs, &file, write_data_2, strlen(write_data_2));
        if (bytes_written < 0) {
            ESP_LOGE(LFS_TAG, "Failed to write data_2 to file %s (%d)", test_filename, (int)bytes_written);
        } else {
            ESP_LOGI(LFS_TAG, "Successfully appended %ld bytes of data_2 to %s", bytes_written, test_filename);
        }
        lfs_file_close(&lfs, &file); // Close the file
    }

    // 3. Read the file back
    ESP_LOGI(LFS_TAG, "Attempting to read from file: %s", test_filename);
    err = lfs_file_open(&lfs, &file, test_filename, LFS_O_RDONLY);
    if (err) {
        ESP_LOGE(LFS_TAG, "Failed to open file %s for reading (%d)", test_filename, err);
    } else {
        lfs_ssize_t bytes_read = lfs_file_read(&lfs, &file, read_buffer, sizeof(read_buffer) - 1);
        if (bytes_read < 0) {
            ESP_LOGE(LFS_TAG, "Failed to read from file %s (%d)", test_filename, (int)bytes_read);
        } else {
            read_buffer[bytes_read] = '\0'; // Null-terminate the string
            ESP_LOGI(LFS_TAG, "Content of %s: '%s'", test_filename, read_buffer);
        }
        lfs_file_close(&lfs, &file); // Close the file
    }

    // 4. List directory contents
    lfs_list_directory("/"); // List root directory

    // 5. Example of removing a file
    ESP_LOGI(LFS_TAG, "Attempting to remove file: %s", test_filename);
    err = lfs_remove(&lfs, test_filename);
    if (err) {
        ESP_LOGE(LFS_TAG, "Failed to remove file %s (%d)", test_filename, err);
    } else {
        ESP_LOGI(LFS_TAG, "Successfully removed file %s", test_filename);
    }
    
    lfs_list_directory("/"); // Should show an empty directory if no other files exist

    ESP_LOGI(LFS_TAG, "Application finished.");
}

esp_err_t lfs_append_to_file(const char* data, const char* filename){
    lfs_file_t file;

    ESP_LOGI(LFS_TAG, "Attempting to append to file: %s", filename);

    ESP_RETURN_ON_ERROR(lfs_file_open(&lfs, &file, filename, LFS_O_WRONLY | LFS_O_APPEND), 
                        LFS_TAG,
                        "Failed to open file %s for appending", filename);

    lfs_ssize_t bytes_written = lfs_file_write(&lfs, &file, data, strlen(data));

    // Check if the write operation was successful (NEG values indicate an error)
    if (bytes_written < 0) {
        ESP_LOGE(LFS_TAG, "Failed to write data to file %s (%d)", filename, (int)bytes_written);
        return ESP_FAIL;
    }

    ESP_LOGI(LFS_TAG, "Successfully appended %ld bytes to %s", bytes_written, filename);
    
    lfs_file_close(&lfs, &file);
    return ESP_OK;
}

static esp_err_t lfs_create_new_file_name(const char* prefix, const char* suffix, char* filename, size_t filename_size) {

    //Time from with the eps32 system started (in seconds)
    // TODO: get current date from gps module. Save it somwhere in flash
    time_t current_time = time(NULL);
    // Format: prefix + current_time(for randomness) + suffix (.csv)
    int err = snprintf(filename, filename_size, "%s_%lu%s", prefix, (unsigned long)current_time, suffix);

    /* 
    Check if snprintf was successful
    it returns the number of characters written, or a negative value on error
    But all that matters (to me) is that it returns a value less than buffer_size
    */
    if(err < 0) {
        ESP_LOGE(LFS_TAG, "Failed to create new file name");
        return ESP_FAIL;
    }
    ESP_LOGI(LFS_TAG, "Generated filename: '%s' , from prefix: %s , current time: %lu , suffix: %s ", filename, prefix, (unsigned long)current_time, suffix);
    return ESP_OK;
}

static bool lfs_file_exsists(const char* filename) {
    // Check if file already exists
    struct lfs_info info;
    int stat_result = lfs_stat(&lfs, filename, &info);
    
    if (stat_result == LFS_ERR_OK) {
        ESP_LOGE(LFS_TAG, "File %s already exists. Cannot create.", filename);
        return true;  // File exists
    } else if (stat_result != LFS_ERR_NOENT) {
        ESP_LOGE(LFS_TAG, "Error checking file %s existence (%d)", filename, stat_result);
        return false;
    }
    ESP_LOGI(LFS_TAG, "File %s does not exist", filename);
    return false;
}

esp_err_t lfs_create_new_csv_file(void) {
    lfs_file_t file;
    char filename[LFS_MAX_FILE_NAME_SIZE]; // Buffer for the new file name
    const char* file_prefix = "dog_run";
    const char* file_suffix = ".csv";
    const char* header = "timestamp,latitude,longitude,altitude,speed\n";
    int counter = 0;

    // 1) Create a new file name with the current time
    ESP_RETURN_ON_ERROR(
        lfs_create_new_file_name(file_prefix, file_suffix, filename, sizeof(filename)),
        LFS_TAG,
        "Failed to create new file name"
    );

    // 2) Check if the file already exists, add _1, _2 if it does
    while(lfs_file_exsists(filename) ) {
        counter++;
        // If file already exists add _1, _2 to the file name (normally you should not have to do this)
        char modified_prefix[LFS_MAX_FILE_NAME_SIZE]; 

        // Create new prefix with counter: "dog_run_" becomes "dog_run_1_", "dog_run_2_", etc.
        snprintf(modified_prefix, sizeof(modified_prefix), "%s%d_", file_prefix, counter);

        // Create new file name
        ESP_RETURN_ON_ERROR(
            lfs_create_new_file_name(modified_prefix, file_suffix, filename, sizeof(filename)),
            LFS_TAG,
            "Failed to create new file name"
        );
        
        if(counter > 1000) { // Safety limit: prevent infinite loop if filesystem has issues
            ESP_LOGE(LFS_TAG, "Too many files with the same name (%s).", filename);
            return ESP_FAIL;
        }
    }
    // 3) Create new file

    ESP_RETURN_ON_ERROR(
    lfs_file_open(&lfs, &file, filename, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC),
        LFS_TAG,
        "Failed to open/create file %s ", filename
    );

    // 4) Write the header to the file
    lfs_ssize_t bytes_written = lfs_file_write(&lfs, &file, header, strlen(header));
    if (bytes_written < 0) {
        ESP_LOGE(LFS_TAG, "Failed to write header to file %s (%d)", filename, (int)bytes_written);
        lfs_file_close(&lfs, &file);
        return ESP_FAIL;
    }

    // 5) Close the file
    lfs_file_close(&lfs, &file);
    ESP_LOGI(LFS_TAG, "Successfully created CSV file %s with header '%s'", filename, header);
    
    return ESP_OK;
}

esp_err_t lfs_delete_file(const char* filename) {
    ESP_LOGI(LFS_TAG, "Attempting to delete file: %s", filename);
    
    ESP_RETURN_ON_ERROR(
        lfs_remove(&lfs, filename), 
        LFS_TAG, "Failed to remove file %s", filename
    );
    
    ESP_LOGI(LFS_TAG, "Successfully removed file %s", filename);
    return ESP_OK;
}