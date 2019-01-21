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
 * \brief AM824ZB ģ�幤��
 *
 *
 * \internal
 * \par Modification history
 * - 1.00 17-07-07  nwt, first implementation
 * \endinternal
 */

#include "ametal.h"
#include "am_vdebug.h"

extern void demo_zigbee_firmware_recive_entry();

/**
 * \brief AMetal Ӧ�ó������
 */
void am_main (void)
{
    AM_DBG_INFO("Start up successful!\r\n");

    demo_zigbee_firmware_recive_entry();

    while(1) {
    }
}

/* end of file */
