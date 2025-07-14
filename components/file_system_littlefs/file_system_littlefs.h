#ifndef LITTLEFS_H
#define LITTLEFS_H  


#include "../../drivers/littlefs/lfs.h"    
#include "ext_flash.h"    
#include "esp_err.h" 
#include "esp_check.h"
#include "esp_log.h"
#include <string.h>
#include <time.h>

// LittleFS specific defines (adjust these based on your flash characteristics and desired LittleFS configuration)
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
#define LFS_MAX_FILE_NAME_SIZE      64      // Maximum file name size 

// Public functions for LittleFS integration
esp_err_t lfs_mount_filesystem(bool format_if_fail);
esp_err_t lfs_unmount_filesystem(void);
esp_err_t lfs_list_directory(const char *path, char *file_name_buffer, size_t buffer_size);
esp_err_t wifi_get_file_list_as_html(char *buffer, size_t buffer_size);
void file_system_test(void);
esp_err_t lfs_append_to_file(const char* data, const char* filename);
esp_err_t lfs_create_new_csv_file(void);
esp_err_t lfs_delete_file(const char* filename);


extern lfs_t lfs;
extern struct lfs_config cfg;


#endif // LITTLEFS_H