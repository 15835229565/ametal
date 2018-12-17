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
 * \brief ZLG217 bootloader �û������ļ�
 * \sa am_bootconf_zlg217.c
 *
 * \internal
 * \par Modification history
 * - 1.00 18-11-21  yrh, first implementation
 * \endinternal
 */

#include "am_zlg217_boot.h"

#include "am_boot.h"

static am_zlg217_boot_devinfo_t __g_zlg217_boot_devinfo = {
    /** \brief flash��ʼ��ַ*/
    0x08000000,
    /** \brief flash��С */
    128 * 1024,
    /** \brief ram��ʼ��ַ */
    0x20000000,
    /** \brief ram������ַ */
    20 * 1024,
    /** \brief ƽ̨��ʼ������ */
    NULL,
    /** \brief ƽ̨���ʼ������ */
    NULL,
};

static am_zlg217_boot_dev_t __g_zlg217_boot_dev;

am_boot_handle_t am_zlg217_std_boot_inst_init()
{
    return am_zlg217_boot_init(&__g_zlg217_boot_dev,
                               &__g_zlg217_boot_devinfo);
}

/* end of file */
