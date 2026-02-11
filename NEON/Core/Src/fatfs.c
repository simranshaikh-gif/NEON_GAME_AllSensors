#include "fatfs.h"

uint8_t retUSER;  /* Return value for USER */
char USERPath[4]; /* USER logical drive path */
FATFS USERFatFS;  /* File system object for USER logical drive */
FIL USERFile;     /* File object for USER */

void MX_FATFS_Init(void) {
  /*## FatFS: Link the USER driver ###########################*/
  retUSER = FATFS_LinkDriver(&USER_Driver, USERPath);
}

/**
 * @brief  Gets Time from RTC (Stubbed as we don't have RTC enabled)
 */
DWORD get_fattime(void) { return 0; }
