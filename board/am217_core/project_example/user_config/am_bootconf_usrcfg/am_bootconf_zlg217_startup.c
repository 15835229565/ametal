/*
 * am_bootloader.c
 *
 *  Created on: 2018��11��1��
 *      Author: luoguangzhi
 */
#include "am_bootconf_zlg217_all_inst_init.h"
#include "am_boot_startup.h"
#include "am_zlg217_boot_serial_uart.h"

static am_boot_startup_dev_info_t  __g_zlg217_boot_startup_dev_info = {
    0x0800C400,     /**< \brief Ӧ�ô�����ʼ��ַ */
    NULL,
    NULL,
};

static am_boot_startup_dev_t __g_zlg217_boot_startup_dev;

int am_zlg217_boot_startup_inst_init(am_boot_handle_t boot_std_handle)
{
    am_boot_flash_handle_t       flash_handle = am_zlg217_boot_flash_inst_init();

    am_boot_autobaud_handle_t autobaud_handle = am_zlg217_boot_autobaud_inst_init();

    am_boot_serial_handle_t    serial_handle  = am_zlg217_boot_serial_uart_init(autobaud_handle);
    return am_boot_startup_init(&__g_zlg217_boot_startup_dev,
                                &__g_zlg217_boot_startup_dev_info,
                                 flash_handle,
                                 serial_handle,
                                 boot_std_handle);
}
/**
 * \brief bootloader ����ʵ�����ʼ��
 *
 * \param ��
 *
 * \return ��
 */
void am_zlg217_boot_startup_inst_deinit(void)
{
    am_boot_startup_deinit();
}
