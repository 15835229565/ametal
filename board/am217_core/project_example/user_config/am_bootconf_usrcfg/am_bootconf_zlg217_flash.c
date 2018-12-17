/*******************************************************************************
*                                 AMetal
*                       ----------------------------
*                       innovating embedded platform
*
* Copyright (c) 2001-2018 Guangzhou ZHIYUAN Electronics Co., Ltd.
* All rights reserved.
*
* Contact information:
* web site:    http://www.zlg.cn/
*******************************************************************************/

/**
 * \file
 * \brief ZLG217 bootloader flash �û������ļ�
 * \sa am_bootconf_zlg217_flash.c
 *
 * \internal
 * \par Modification history
 * - 1.00 18-11-21  yrh, first implementation
 * \endinternal
 */

#include "am_zlg217_boot_flash.h"
#include "am_boot_flash.h"
#include "am_zlg_flash.h"
#include "am_zlg217.h"
#include "zlg217_regbase.h"

static am_zlg217_boot_flash_devinfo_t __g_zlg217_flash_devinfo = {
    /** \brief flash����ʼ��ַ */
    0x08000000,
    /** \brief flash���ܵĴ�С */
    128 * 1024,
    /** \brief flash������С */
    1024,
    /** \brief flash������ */
    128,
    /** \brief flash�Ĵ����Ļ���ַ */
    ZLG217_FLASH_BASE,
    /** \brief ƽ̨��ʼ������ */
    NULL,
    /** \brief ƽ̨����ʼ������ */
    NULL,
};

static am_zlg217_boot_flash_dev_t __g_zlg217_flash_dev;

am_boot_flash_handle_t am_zlg217_boot_flash_inst_init()
{
    return am_boot_flash_init(&__g_zlg217_flash_dev, &__g_zlg217_flash_devinfo);
}
/* end of file */
