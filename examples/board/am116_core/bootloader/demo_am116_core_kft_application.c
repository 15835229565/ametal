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
 * \brief bootloader kboot KinetisFlashTool ���̣���demo����ΪӦ�ó���
 *
 * - �������裺
 *   �ο�AMmetal-AM116-Core-bootloader�����ֲ�
 *
 *
 * \par Դ����
 * \snippet demo_am116_core_hw_adc_int.c src_am116_core_hw_adc_int
 *
 * \internal
 * \par Modification History
 * - 1.00 18-12-18  yrh, first implementation
 * \endinternal
 */
#include "ametal.h"
#include "am_board.h"
#include "am_vdebug.h"
void demo_am116_core_kft_application_entry (void)
{
    AM_DBG_INFO("application : zlg116_bootloader_kft_application start up successful!\r\n");

    while (1) {

        am_led_toggle(LED0);

        am_mdelay(1000);
    }
}
