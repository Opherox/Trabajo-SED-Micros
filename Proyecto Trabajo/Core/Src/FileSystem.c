/*
 * FileSystem.c
 *
 *  Created on: Jan 20, 2024
 *      Author: Opherox
 */

#include <FileSystem.h>
#include <string.h>

char *songPlaying = NULL;
static FATFS FS_fs;
static DIR FS_dir;
static FILINFO FS_info;
static FRESULT FS_status = FR_OK;


FRESULT FS_getStatus(void)
{
  return FS_status;
}

void FS_mount(void)
{
  FS_status = f_mount(&FS_fs, "", 0); //montamos el sistema de archivos, 0:no forzar montaje
  if (FS_status != FR_OK)
    Error_Handler();
}

const FILINFO *FS_nextMP3(bool reset)
{
  if (reset)
  {
    f_closedir(&FS_dir);
    FS_status = f_opendir(&FS_dir, "/");
  }

  int noMatch = 1;
  while (FS_status == FR_OK && noMatch)
  {
    FS_status = f_readdir(&FS_dir, &FS_info);
    if (FS_status != FR_OK || FS_info.fname[0] == '\0')
      break;

    char *ext = strrchr(FS_info.fname, '.');
    if (ext)
      noMatch = strcmp(ext + 1, "mp3");
  }

  if (FS_status != FR_OK)
    return NULL;

  /* If no match... there are really no MP3's or all scanned.
   * Try to restart scanning if not doing it yet.
   */
  if (noMatch)
    return reset ? NULL : FS_nextMP3(true);

  return &FS_info; /* Match!. Return it now */
}

void FS_unmount(void)
{
  f_closedir(&FS_dir);
  f_mount(NULL, "", 0);
}
