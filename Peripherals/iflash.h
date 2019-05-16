#ifndef __IFLASH_H
#define __IFLASH_H

#include "stm32f4xx.h"





#define FLASH_BASE_ADDR		((uint32_t)0x08000000)		/* Flash基地址 */
#define	FLASH_SIZE			((uint32_t)1024*1024)		/* Flash 容量 */



/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0		((uint32_t)0x08000000)	/* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1		((uint32_t)0x08004000)	/* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2		((uint32_t)0x08008000)	/* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3		((uint32_t)0x0800C000)	/* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4		((uint32_t)0x08010000)	/* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5		((uint32_t)0x08020000)	/* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6		((uint32_t)0x08040000)	/* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7		((uint32_t)0x08060000)	/* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_8		((uint32_t)0x08080000)	/* Base @ of Sector 8, 128 Kbytes */
#define ADDR_FLASH_SECTOR_9		((uint32_t)0x080A0000)	/* Base @ of Sector 9, 128 Kbytes */
#define ADDR_FLASH_SECTOR_10	((uint32_t)0x080C0000)	/* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11	((uint32_t)0x080E0000)	/* Base @ of Sector 11, 128 Kbytes */
#define FLASH_END_ADDR			((uint32_t)0x08100000)	/* 结束地址 */

#define FLASH_DATA_SAME		0	/* Flash内容和待写入的数据相等，不需要擦除和写操作 */
#define FLASH_REQ_WRITE		1	/* Flash不需要擦除，直接写 */
#define FLASH_REQ_ERASE		2	/* Flash需要先擦除,再写 */
#define FLASH_PARAM_ERR		3	/* 函数参数错误 */



uint32_t Flash_EnableReadProtection(void);
uint32_t Flash_DisableReadProtection(void);

uint32_t ulFlash_GetSectorStart(uint32_t ulAddr);
uint8_t ucFlash_Read(uint32_t ulFlashAddr, uint8_t *pucDst, uint32_t ulSize);
uint8_t ucFlash_Compare(uint32_t ulFlashAddr, uint8_t *pucBuf, uint32_t ulSize);
uint8_t ucFlash_Write(uint32_t ulFlashAddr, uint8_t *pucSrc, uint32_t ulSize);




#endif
