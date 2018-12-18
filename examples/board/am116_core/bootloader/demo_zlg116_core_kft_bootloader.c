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
 * \brief bootloader kboot KinetisFlashTool ���̣���demo����Ϊbootloader��
 *
 * - �������裺
 *   �ο�AMmetal-AM116-Core-bootloader�����ֲ�
 *
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
#include "zlg116_pin.h"
#include "am_boot.h"
#include "am_boot_kft.h"
#include "am_zlg116_inst_init.h"

/** \brief �̼��������� */
#define __ENABLE_PIN          (PIOA_8)
/** \brief Ӧ�ô�����ʼ��ַ  */
#define __APP_START_ADDR      0x08007000

void demo_am116_core_kft_bootloader_entry (void)
{
    am_gpio_pin_cfg(__ENABLE_PIN, AM_GPIO_INPUT | AM_GPIO_PULLUP);

    am_zlg116_std_boot_inst_init();

    if (am_gpio_get(__ENABLE_PIN)) {

        am_boot_source_release();
        am_gpio_pin_cfg(__ENABLE_PIN, AM_GPIO_INPUT | AM_GPIO_FLOAT);
        am_kprintf("bootloadr : go to application...\r\n");
        if(AM_OK != am_boot_go_application(__APP_START_ADDR)){
            while(1);
        }
    }

    am_zlg116_boot_kft_inst_init();
    am_kprintf("bootloader : running......\r\n");

    am_kprintf("bootloader : start command pump\r\n");
    while(1) {
        am_boot_kft_command_pump();
    }
}
