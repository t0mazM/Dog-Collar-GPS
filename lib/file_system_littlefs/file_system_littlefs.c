#include "file_system_littlefs.h"
#include "ext_flash.h" // Your existing flash driver header
#include "esp_log.h"
#include <string.h>

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

