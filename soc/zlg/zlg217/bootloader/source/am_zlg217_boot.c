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
 * \brief bootloader drivers implementation
 *
 * \internal
 * \par Modification history
 * - 1.00 18-11-15  yrh, first implementation
 * \endinternal
 */
#include "am_zlg217_boot.h"
#include "am_zlg217.h"

/**
 * \brief �ж�Ӧ�ô����Ƿ��ִ��
 */
static am_bool_t __boot_app_is_ready(void *p_drv, uint32_t app_start_addr)
{
    am_zlg217_boot_dev_t *p_dev = (am_zlg217_boot_dev_t *)p_drv;
    if ((!app_start_addr) || (app_start_addr == 0xffffffff)) {
        return AM_FALSE;
    }
    uint32_t flash_start_addr = p_dev->p_devinfo->flash_start_addr;
    uint32_t flash_end_adrr   = p_dev->p_devinfo->flash_start_addr + \
        p_dev->p_devinfo->flash_size - 1;
    uint32_t ram_start_addr = p_dev->p_devinfo->ram_start_addr;
    uint32_t ram_size    = p_dev->p_devinfo->ram_size;


    if(app_start_addr < flash_start_addr || app_start_addr > flash_end_adrr) {
        return AM_FALSE;
    }
    else {
        if((*(uint32_t *)app_start_addr) > (ram_start_addr + ram_size )||
           (*(uint32_t *)app_start_addr) < ram_start_addr) {
            return AM_FALSE;
        }
    }
    return AM_TRUE;
}
/**
 * \brief ��ת��Ӧ�ô���
 */
static int __boot_go_application(void  *p_drv, uint32_t app_start_addr)
{
    //am_zlg217_boot_dev_t *p_dev = (am_zlg217_boot_dev_t *)p_drv;

    uint32_t stack_pointer = *(uint32_t *)app_start_addr;

    void (*farewell_bootloader)(void);

    if(AM_TRUE != __boot_app_is_ready(p_drv, app_start_addr)) {
        return AM_ERROR;
    }

    farewell_bootloader = (void (*)(void))(*(uint32_t *)(app_start_addr + 4));

    /* ����ջָ�� */
    __set_MSP(stack_pointer);
    __set_PSP(stack_pointer);

    /* ��ת��Ӧ�ô��� */
    farewell_bootloader();

    /*���������ת�����벻��ִ�е�����*/
    return AM_ERROR;
}

/**
 * \brief ϵͳ����
 */
static void __boot_sys_reset()
{
    NVIC_SystemReset();
}

/**
 * \brief BootLoader��׼��ӿ�ʵ��
 */
static struct am_boot_drv_funcs __g_boot_drv_funcs = {
    __boot_go_application,
    __boot_sys_reset,
    __boot_app_is_ready,
};

/**
 * \brief BootLoader��ʼ������
 */
am_boot_handle_t am_zlg217_boot_init(am_zlg217_boot_dev_t     *p_dev,
                                     am_zlg217_boot_devinfo_t *p_devinfo)
{
    if(p_devinfo == NULL || p_dev == NULL) {
        return NULL;
    }

    p_dev->p_devinfo          = p_devinfo;

    p_dev->boot_serv.p_funcs  = &__g_boot_drv_funcs;
    p_dev->boot_serv.p_drv    = p_dev;
    p_dev->boot_serv.mem_info.flash_size       = p_devinfo->flash_size;
    p_dev->boot_serv.mem_info.flash_start_addr = p_devinfo->flash_start_addr;
    p_dev->boot_serv.mem_info.ram_size         = p_devinfo->ram_start_addr;
    p_dev->boot_serv.mem_info.ram_size         = p_devinfo->ram_size;
    return &p_dev->boot_serv;
}
/**
 * \brief BootLoader���ʼ������
 */
void am_zlg217_boot_deinit(am_zlg217_boot_dev_t     *p_dev)
{

}

/* end of file */
