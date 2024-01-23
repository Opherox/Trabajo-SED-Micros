/*
 * FileSystem.h
 *
 *  Created on: Jan 20, 2024
 *      Author: Opherox
 */

#ifndef INC_FILESYSTEM_H_
#define INC_FILESYSTEM_H_

#include <stm32f4xx_hal.h>
#include <ff.h>

#include <stdbool.h>

FRESULT FS_getStatus(void);
void FS_mount(void);
const FILINFO *FS_nextMP3(bool reset);
void FS_unmount(void);

#endif /* INC_FILESYSTEM_H_ */
