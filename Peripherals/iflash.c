#include "iflash.h"



/****************************************************************
 * Function:    Flash_EnableReadProtection
 * Description: Enable the read protection of user flash area.
 * Input:
 * Output:
 * Return:      1: Read Protection successfully enable
 *              2: Error: Flash read unprotection failed
*****************************************************************/
uint32_t Flash_EnableReadProtection(void)
{
	/* Returns the FLASH Read Protection level. */
	if( FLASH_OB_GetRDP() == RESET )
	{
		/* Unlock the Option Bytes */
		FLASH_OB_Unlock();
		/* Sets the read protection level. */
		FLASH_OB_RDPConfig(OB_RDP_Level_1);
		/* Start the Option Bytes programming process. */
		if (FLASH_OB_Launch() != FLASH_COMPLETE)
		{
			/* Disable the Flash option control register access (recommended to protect
			   the option Bytes against possible unwanted operations) */
			FLASH_OB_Lock();
			/* Error: Flash read unprotection failed */
			return (1);
		}
		/* Disable the Flash option control register access (recommended to protect
		   the option Bytes against possible unwanted operations) */
		FLASH_OB_Lock();
		/* Read Protection successfully enable */
		return (0);
	}
	/* Read Protection successfully enable */
	return (0);
}

/****************************************************************
 * Function:    Flash_DisableReadProtection
 * Description: Disable the read protection of user flash area.
 * Input:
 * Output:
 * Return:      1: Read Protection successfully disable
 *              2: Error: Flash read unprotection failed
*****************************************************************/
uint32_t Flash_DisableReadProtection(void)
{
	/* Returns the FLASH Read Protection level. */
	if( FLASH_OB_GetRDP() != RESET )
	{
		/* Unlock the Option Bytes */
		FLASH_OB_Unlock();
		/* Sets the read protection level. */
		FLASH_OB_RDPConfig(OB_RDP_Level_0);
		/* Start the Option Bytes programming process. */
		if (FLASH_OB_Launch() != FLASH_COMPLETE)
		{
			/* Disable the Flash option control register access (recommended to protect
			   the option Bytes against possible unwanted operations) */
			FLASH_OB_Lock();
			/* Error: Flash read unprotection failed */
			return (1);
		}
		/* Disable the Flash option control register access (recommended to protect
		   the option Bytes against possible unwanted operations) */
		FLASH_OB_Lock();
		/* Read Protection successfully disable */
		return (0);
	}
	/* Read Protection successfully disable */
	return (0);
}

/*
 * 函数说明: 根据总线地址获取这块Flash的Sector的起始地址
 * 参    数: 总线地址
 * 返 回 值: 给定地址的sector
*/
uint32_t ulFlash_GetSectorStart(uint32_t ulAddr)
{
	uint32_t sector = 0;
	if ((ulAddr >= ADDR_FLASH_SECTOR_0) && \
		(ulAddr < ADDR_FLASH_SECTOR_1))
	{
		sector = FLASH_Sector_0;
	}
	else if ((ulAddr >= ADDR_FLASH_SECTOR_1) && \
			(ulAddr < ADDR_FLASH_SECTOR_2))
	{
		sector = FLASH_Sector_1;
	}
	else if ((ulAddr >= ADDR_FLASH_SECTOR_2) && \
			(ulAddr < ADDR_FLASH_SECTOR_3)) 
	{
		sector = FLASH_Sector_2;
	}
	else if ((ulAddr >= ADDR_FLASH_SECTOR_3) && \
			(ulAddr < ADDR_FLASH_SECTOR_4))
	{
		sector = FLASH_Sector_3;
	}
	else if ((ulAddr >= ADDR_FLASH_SECTOR_4) && \
			(ulAddr < ADDR_FLASH_SECTOR_5))
	{
		sector = FLASH_Sector_4;
	}
	else if ((ulAddr >= ADDR_FLASH_SECTOR_5) && \
			(ulAddr < ADDR_FLASH_SECTOR_6))
	{
		sector = FLASH_Sector_5;
	}
	else if ((ulAddr >= ADDR_FLASH_SECTOR_6) && \
			(ulAddr < ADDR_FLASH_SECTOR_7))
	{
		sector = FLASH_Sector_6;
	}
	else if ((ulAddr >= ADDR_FLASH_SECTOR_7) && \
			(ulAddr < ADDR_FLASH_SECTOR_8))
	{
		sector = FLASH_Sector_7;
	}
	else if ((ulAddr >= ADDR_FLASH_SECTOR_8) && \
			(ulAddr < ADDR_FLASH_SECTOR_9))
	{
		sector = FLASH_Sector_8;
	}
	else if ((ulAddr >= ADDR_FLASH_SECTOR_9) && \
			(ulAddr < ADDR_FLASH_SECTOR_10))
	{
		sector = FLASH_Sector_9;
	}
	else if ((ulAddr >= ADDR_FLASH_SECTOR_10) && \
			(ulAddr < ADDR_FLASH_SECTOR_11))
	{
		sector = FLASH_Sector_10;
	}
	else if ((ulAddr >= ADDR_FLASH_SECTOR_11) && \
			(ulAddr < FLASH_END_ADDR))
	{
		sector = FLASH_Sector_11;
	}
	return sector;
}

/*
 * 函数说明: 读取内部Flash的内容
 * 参    数：pucDst: 目标缓冲区
 *			 ulFlashAddr: 起始地址
 *			 ulSize: 数据大小（单位是字节）
 * 返 回 值: 0=成功，1=失败
*/
uint8_t ucFlash_Read(uint32_t ulFlashAddr, uint8_t *pucDst, uint32_t ulSize)
{
	/* 如果偏移地址超过芯片容量，则不改写输出缓冲区 */
	if (ulFlashAddr + ulSize > FLASH_BASE_ADDR + FLASH_SIZE)
	{
		return 1;
	}
	/*长度为0时不继续操作,否则起始地址为奇地址会出错*/
	if (ulSize == 0)
	{
		return 1;
	}
	for (uint32_t i=0; i<ulSize; i++)
	{
		*pucDst++ = *(uint8_t *)ulFlashAddr++;
	}
	return 0;
}


/*
 * 函数说明: 比较Flash指定地址的数据.
 * 参    数: ulFlashAddr : Flash地址
 *			 pucBuf: 数据缓冲区
 *			 ulSize: 数据大小（单位是字节）
 * 返 回 值:
 *			 FLASH_DATA_SAME	0	Flash内容和待写入的数据相等，不需要擦除和写操作
 *			 FLASH_REQ_WRITE	1	Flash不需要擦除，直接写
 *			 FLASH_REQ_ERASE	2	Flash需要先擦除,再写
 *			 FLASH_PARAM_ERR	3	函数参数错误
*/
uint8_t ucFlash_Compare(uint32_t ulFlashAddr, uint8_t *pucBuf, uint32_t ulSize)
{
	uint8_t ucSame = 0x00;		/*不相等标志*/

	/*如果偏移地址超过芯片容量，则不改写输出缓冲区*/
	if (ulFlashAddr + ulSize > FLASH_BASE_ADDR + FLASH_SIZE)
	{
		return FLASH_PARAM_ERR;
	}
	/*长度为0时返回正确*/
	if (ulSize == 0x00)
	{
		return FLASH_DATA_SAME;
	}
	for (uint32_t i=0; i<ulSize; i++)
	{
		if (*pucBuf != *(uint8_t *)ulFlashAddr)
		{
			if (*(uint8_t *)ulFlashAddr != 0xFF)
			{
				return FLASH_REQ_ERASE;
			}
			else
			{
				ucSame = 0x01;
				break;
			}
		}
		ulFlashAddr++;
		pucBuf++;
	}
	if (ucSame == 0x00)
	{
		return FLASH_DATA_SAME;
	}
	else
	{
		return FLASH_REQ_WRITE;
	}
}

/*
 * 功能说明: 写数据到Flash。
 * 形    参: ulFlashAddr: Flash地址
 *			 pucSrc: 数据缓冲区
 *			 ulSize: 数据大小（单位是字节Byte）
 * 返 回 值: 0=成功，1=数据长度或地址溢出，2=写Flash出错(这块Flash已经GG了)
*/
uint8_t ucFlash_Write(uint32_t ulFlashAddr, uint8_t *pucSrc, uint32_t ulSize)
{
	uint8_t ucRet = 0x00;
	
	/*如果偏移地址超过芯片容量，则不改写输出缓冲区*/
	if (ulFlashAddr + ulSize > FLASH_BASE_ADDR + FLASH_SIZE)
	{
		return 1;
	}
	/*长度为0时不继续操作*/
	if (ulSize == 0)
	{
		return 0;
	}
	ucRet = ucFlash_Compare(ulFlashAddr, pucSrc, ulSize);
	/*如果内容相同，就不写了*/
	if (ucRet == FLASH_DATA_SAME)
	{
		return 0;
	}
	__set_PRIMASK(1);		/*关中断，以防止系统调度或者被其他东西打断*/

	/*FLASH 解锁*/
	FLASH_Unlock();
	/*Clear pending flags (if any)*/
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
					FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);
	/*需要擦除*/
	if (ucRet == FLASH_REQ_ERASE)
	{
		FLASH_EraseSector(ulFlash_GetSectorStart(ulFlashAddr), VoltageRange_3);
	}
	/* 按字节模式编程（为提高效率，可以按字编程，一次写入4字节） */
	for (uint32_t i=0; i<ulSize; i++)
	{
		FLASH_ProgramByte(ulFlashAddr++, *pucSrc++);
	}
	/*Flash 上锁，禁止写Flash控制寄存器*/
	FLASH_Lock();
	__set_PRIMASK(0);		/*开中断*/
	return 0;
}



