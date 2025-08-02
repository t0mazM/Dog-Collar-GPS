/*
 * Copyright © 2025 Tomaz Miklavcic
 *
 * Use this code for whatever you want. No restrictions, no warranty.
 * Attribution appreciated but not required.
 */

#ifndef LITTLEFS_H
#define LITTLEFS_H  


#include "../../drivers/littlefs/lfs.h"    
#include "external_flash/ext_flash.h" 
#include "../gps_l96/gps_l96.h" 
#include "esp_err.h" 
#include "esp_check.h"
#include "esp_log.h"
#include <string.h>
#include <time.h>


extern lfs_t lfs;
extern struct lfs_config cfg;

// LittleFS specifically defines (adjust these based on your flash characteristics and desired LittleFS configuration)
// These are crucial for defining how LittleFS sees your flash memory.
// LFS_READ_SIZE and LFS_PROG_SIZE should be at least 1 and ideally match your flash's page size if performing full page ops.
// LFS_BLOCK_SIZE MUST be your erase unit size (sector size for NOR flash).
#define LFS_READ_SIZE           W25Q128JV_PAGE_SIZE // Should be 256 bytes for W25Q128JV
#define LFS_PROG_SIZE           W25Q128JV_PAGE_SIZE // Should be 256 bytes for W25Q128JV
#define LFS_BLOCK_SIZE          W25Q128JV_SECTOR_SIZE // 4KB sector for W25Q128JV
#define LFS_BLOCK_COUNT         (W25Q128JV_TOTAL_SIZE_BYTES / LFS_BLOCK_SIZE) // Total number of 4KB blocks
#define LFS_BLOCK_CYCLES        500     // LittleFS will prioritize blocks with fewer erase cycles for wear leveling (not a hard limit, more like a differential wear leveling strategy)
#define LFS_CACHE_SIZE          256     // Cache size for read/write/erase operations (multiple of LFS_PROG_SIZE). For me program crashes if I even try to multiply it, but since my writes are not so frequent and only a couple of bytes the min value of 256 is more than enough sufficient
#define LFS_LOOKAHEAD_SIZE      16      // Size of the lookahead buffer (in bytes), multiple of 8. 32 is widely used, but 16 is sufficient for my use case
#define LFS_MAX_FILE_NAME_SIZE  64      // Maximum file name size 

// Public functions for LittleFS integration
/**
 * @brief Initializes the LittleFS filesystem.
 * 
 * This function mounts the LittleFS filesystem - just a function wrapper around lfs_mount_filesystem.
 *
 * @return esp_err_t ESP_OK on success, or an error code on failure.
 */
esp_err_t lfs_init(void);

/**
 * @brief Mounts the LittleFS filesystem.
 * 
 * This function mounts the LittleFS filesystem, formatting it if necessary.
 * It initializes the LittleFS configuration and sets up the read, program, erase, and sync callbacks.
 *
 * @param format_if_fail If true, formats the filesystem if mounting fails.
 * @return esp_err_t ESP_OK on success, or an error code on failure.
 */
esp_err_t lfs_mount_filesystem(bool format_if_fail);

/**
 * @brief Unmounts the LittleFS filesystem.
 * 
 * This function unmounts the LittleFS filesystem, ensuring all data is written to flash.
 *
 * @return esp_err_t ESP_OK on success, or an error code on failure.
 */
esp_err_t lfs_unmount_filesystem(void);

/**
 * @brief Lists the contents of a directory.
 *
 * This function returns the names of files in the external flash filesystem directory.
 *
 * @param path The path of the directory to list.
 * @param file_name_buffer A buffer to store the names of the files found.
 * @param buffer_size The size of the file name buffer.
 * @return esp_err_t ESP_OK on success, or an error code on failure.
 */
esp_err_t lfs_list_directory(const char *path, char *file_name_buffer, size_t buffer_size);

/**
 * @brief Gets the list of files in the filesystem as an HTML formatted string.
 *
 * This function generates an HTML list of files in the LittleFS filesystem.
 * The generated HTML can be used to display the file list in a web interface.
 *
 * @param buffer Pointer to a buffer where the HTML will be stored.
 * @param buffer_size Size of the buffer.
 * @return esp_err_t ESP_OK on success, or an error code on failure.
 */
esp_err_t wifi_get_file_list_as_html(char *buffer, size_t buffer_size);

/**
 * @brief A test function for the LittleFS filesystem.
 */
void file_system_test(void);

/**
 * @brief Appends data to a file in the LittleFS filesystem.
 *
 * This function opens a file  and appends the provided data to it.
 * If the file does not exist, it will be created.
 *
 * @param data The data to append to the file.
 * @param filename The name of the file to append to.
 * @return esp_err_t ESP_OK on success, or an error code on failure.
 */
esp_err_t lfs_append_to_file(const char* data, const char* filename);

/**
 * @brief Creates a new CSV file with a unique random with random numbers
 *  Generates random file name and cheks if it alredy excist, add -1, -2, etc. if it does.
 *  Creates file and adds cvs header to it -> "timestamp,latitude,longitude,altitude,speed\n"
 * 
 * @param filename Pointer to a buffer where the new file name will be stored. The buffer should be at least LFS_MAX_FILE_NAME_SIZE bytes long.
 */
esp_err_t lfs_create_new_csv_file(char* filename, size_t filename_size);

/**
 * @brief Deletes a file from the LittleFS filesystem.
 * 
 * @param filename The name of the file to delete.
 * @return esp_err_t ESP_OK on success, or an error code on failure.
 */
esp_err_t lfs_delete_file(const char* filename);


#endif // LITTLEFS_H