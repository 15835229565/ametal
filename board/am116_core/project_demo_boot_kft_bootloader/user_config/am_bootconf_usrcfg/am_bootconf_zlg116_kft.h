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
 * \brief ZLG116 kboot KinetisFlashTool �û������ļ�
 *
 *
 *
 * \internal
 * \par Modification history
 * - 1.00 18-12-13  yrh, first implementation
 * \endinternal
 */

#include "am_boot_autobaud.h"

/** \brief �Զ�������ʵ�����ʼ�� */
void am_zlg116_boot_autobaud_inst_deinit (am_boot_autobaud_handle_t handle);

/** \brief ʵ����ʼ��������Զ������ʷ����� */
am_boot_autobaud_handle_t am_zlg116_boot_autobaud_inst_init (void);


/**
 * \brief bootloader ��׼ʵ����ʼ��
 *
 *
 * \return ��Ϊ AM_OK��������ʼ���ɹ�
 */
int am_zlg116_std_boot_inst_init();

/**
 * \brief bootloader kboot KinetisFlashTool ʵ����ʼ��
 *
 * \return ��Ϊ AM_OK��������ʼ���ɹ�
 */
int am_zlg116_boot_kft_inst_init();
