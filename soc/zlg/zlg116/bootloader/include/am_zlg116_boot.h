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
 * \brief bootloader drivers for standard interface
 *
 * \internal
 * \par Modification history
 * - 1.00 18-11-15  yrh, first implementation
 * \endinternal
 */

#ifndef __AM_ZLG116_BOOT_H
#define __AM_ZLG116_BOOT_H

#include "am_boot.h"
#include "amhw_zlg_flash.h"

typedef struct am_zlg116_boot_devinfo {
    uint32_t  flash_start_addr;    /**< \brief flash����ʼ��ַ*/
    uint32_t  flash_size;          /**< \brief flash�Ĵ�С */

    uint32_t  ram_start_addr;      /**< \brief ram��ʼ��ַ */
    uint32_t  ram_size;            /**< \brief ram�Ĵ�С */

    void (*pfn_plfm_init)(void);   /**< \brief ƽ̨��ʼ������ */

    void (*pfn_plfm_deinit)(void); /**< \brief ƽ̨ȥ��ʼ������ */
}am_zlg116_boot_devinfo_t;

typedef struct am_zlg116_boot_dev {
    am_zlg116_boot_devinfo_t *p_devinfo;
    am_boot_mem_info_t        mem_info;
    amhw_zlg_flash_t          amhw_zlg_flash;   /**< \brief flash�Ĵ����ṹ�� */

}am_zlg116_boot_dev_t;

/**
 * \brief ��ʼ��BootLoader������BootLoader��׼����������
 *
 * \param[in] p_dev     : ָ��BootLoader�豸��ָ��
 * \param[in] p_devinfo : ָ��BootLoader�豸��Ϣ������ָ��
 *
 * \return BootLoader��׼������������ֵΪNULLʱ������ʼ��ʧ��
 */
int am_zlg116_boot_init(am_zlg116_boot_dev_t      *p_dev,
                        am_zlg116_boot_devinfo_t *p_devinfo);

/**
 * \brief ��ʹ��BootLoaderʱ�����ʼ��BootLoader���ͷ������Դ
 */
void am_zlg116_boot_deinit();

#endif /* __AM_ZLG116_BOOT_H */
