/**
  * GreenPois0n Cynanide - iPhone3,1/8C148/iBSS/offsets.h
  * Copyright (C) 2010 Chronic-Dev Team
  * Copyright (C) 2010 Joshua Hill
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#ifndef OFFSETS_H
#define OFFSETS_H

// Base address
#define TARGET_BASEADDR                   (0x84000000)

// Standard offsets
#define TARGET_FREE                       (0x11A4c+1)
#define TARGET_MALLOC                     ((0x124A4)+1)
#define TARGET_JUMP_TO                    (0x16080+1)
#define TARGET_PRINTF                     (0x1B984+1)
#define TARGET_VPRINTF                    (0x1B934+1)

// Command offsets
#define TARGET_CMD_LIST_BEGIN             (0x24004)
#define TARGET_CMD_LIST_END               (0x2402C)

// Task functions
#define TARGET_TASK_YIELD                 (0x1697c+1)
#define TARGET_TASK_RUNNING               (0x24A1C)
#define TARGET_TASK_LIST                  (0x24A94)

// AES offsets
#define TARGET_AES_CRYPTO_CMD             (0x17DB8+1)

// BDev offsets
#define TARGET_BDEV_LIST                  (0x28580)

// Image offsets
#define TARGET_IMAGE_LIST                 (0x1DE40)

// Filesystem offsets
//#define TARGET_FS_MOUNT                   (0x0)
//#define TARGET_FS_UNMOUNT                 (0x0)
#define TARGET_FS_LOAD_FILE               (0x118E5)

// Kernel offsets
#define TARGET_KERNEL_LOAD                (0x13667+1)
#define TARGET_KERNEL_PHYMEM              (0x28AC0)
#define TARGET_KERNEL_BOOTARGS            (0x20EDD)

// NVRAM offsets
#define TARGET_NVRAM_LIST                 (0x24AB8)

#endif // OFFSETS_H
