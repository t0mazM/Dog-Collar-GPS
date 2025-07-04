#ifndef LITTLEFS_H
#define LITTLEFS_H  

#include "esp_err.h" 
#include "lfs.h"  // Should now work with the component
#include "ext_flash.h"    // Include for W25Q128JV constants


// LittleFS specific defines (adjust these based on your flash characteristics and desired LittleFS configuration)
// These are crucial for defining how LittleFS sees your flash memory.
// LFS_READ_SIZE and LFS_PROG_SIZE should be at least 1 and ideally match your flash's page size if performing full page ops.
// LFS_BLOCK_SIZE MUST be your erase unit size (sector size for NOR flash).
#define LFS_READ_SIZE           W25Q128JV_PAGE_SIZE // Should be 256 bytes for W25Q128JV
#define LFS_PROG_SIZE           W25Q128JV_PAGE_SIZE // Should be 256 bytes for W25Q128JV
#define LFS_BLOCK_SIZE          W25Q128JV_SECTOR_SIZE // 4KB sector for W25Q128JV
#define LFS_BLOCK_COUNT         (W25Q128JV_TOTAL_SIZE_BYTES / LFS_BLOCK_SIZE) // Total number of 4KB blocks
#define LFS_BLOCK_CYCLES        500     // Number of erase cycles before moving block (wear leveling)
#define LFS_CACHE_SIZE          512     // Cache size for read/write/erase operations (multiple of LFS_PROG_SIZE)
#define LFS_LOOKAHEAD_SIZE      16      // Size of the lookahead buffer (in bytes), multiple of 8

// Public functions for LittleFS integration
esp_err_t lfs_mount_filesystem(bool format_if_fail);
esp_err_t lfs_unmount_filesystem(void);
void lfs_list_directory(const char *path);
void file_system_test(void);

// Expose the lfs object and config for direct LittleFS API calls if needed
extern lfs_t lfs;
extern struct lfs_config cfg;


#endif // LITTLEFS_H