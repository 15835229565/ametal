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
 * \brief ZLG217 bootloader ʵ����ʼ����������
 *
 *
 * \internal
 * \par Modification history
 * - 1.00 18-11-21 yrh, first implementation
 * \endinternal
 */

#ifndef __AM_BOOTCONF_ZLG217_ALL_INST_INIT_H
#define __AM_BOOTCONF_ZLG217_ALL_INST_INIT_H
#include "am_boot_autobaud.h"
#include "am_boot.h"
#include "am_boot_flash.h"
#include "am_boot_firmware.h"
#include "am_boot_startup.h"

/**
 * \brief bootloader ��׼�ӿ�ʵ����ʼ����������׼������
 *
 * \param ��
 *
 * \return bootloader ��׼�ӿڱ�׼����������Ϊ NULL��������ʼ��ʧ��
 */
am_boot_handle_t          am_zlg217_std_boot_inst_init();

/**
 * \brief bootloader flashʵ����ʼ����������׼������
 *
 * \param ��
 *
 * \return bootloader flash��׼����������Ϊ NULL��������ʼ��ʧ��
 */
am_boot_flash_handle_t    am_zlg217_boot_flash_inst_init();

/**
 * \brief bootloader �̼��洢ʵ����ʼ��(flash��ʽ)��������׼������
 *
 * \param ��
 *
 * \return bootloader �̼��洢��׼����������Ϊ NULL��������ʼ��ʧ��
 */
am_boot_firmware_handle_t am_zlg217_boot_firmware_flash();

/**
 * \brief bootloader ����ʵ����ʼ����������׼������
 *
 * \param boot_std_handle �� BootLoader��׼�ӿ�ʵ�����
 *
 * \return ��Ϊ AM_OK��������ʼ���ɹ�
 */
int                       am_zlg217_boot_startup_inst_init(am_boot_handle_t boot_std_handle);
/**
 * \brief bootloader ����ʵ�����ʼ��
 *
 * \param ��
 *
 * \return ��
 */
void                      am_zlg217_boot_startup_inst_deinit(void);

/**
 * \brief bootloader �Զ������ʼ��ʵ����ʼ����������׼������
 *
 * \param ��
 *
 * \return bootloader �Զ������ʼ���׼����������Ϊ NULL��������ʼ��ʧ��
 */
am_boot_autobaud_handle_t am_zlg217_boot_autobaud_inst_init (void);



#endif /* __AM_BOOTCONF_ZLG217_ALL_INST_INIT_H */
/* end of file */
