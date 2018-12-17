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
 * \brief KL26 �Զ��������û������ļ���ʵ��ʹ��ʱ��Ҫ��������������Ϊ���ڵĽ�������
 * \sa am_hwconf_auto_baudrate.c
 *
 * \internal
 * \par Modification history
 * - 1.00 16-09-13  ipk, first implementation.
 * \endinternal
 */

#include "am_boot_autobaud.h"
#include "ametal.h"
#include "am_zlg217.h"
#include "am_gpio.h"
#include "am_zlg217_inst_init.h"
#include "am_zlg_tim_cap.h"

/**
 * \addtogroup am_if_hwconf_auto_baudrate
 * \copydoc am_hwconf_auto_baudrate.c
 * @{
 */

/** \brief ��Ҫ�õ��Ķ�ʱ��λ�� */
#define     TIMER_WIDTH           16

/** \brief �Զ������ʵ�ƽ̨��ʼ�� */
void __zlg217_plfm_autobaud_init (void)
{
    amhw_zlg_tim_prescale_set((amhw_zlg_tim_t *)ZLG217_TIM1_BASE, 8);

}

/** \brief �Զ������ʵ�ƽ̨���ʼ��  */
void __zlg217_plfm_autobaud_deinit (void)
{

}

/** \brief ƽ̨����ʱ�������־��ȡ  */
int __fsl_plfm_stat_flag_get (void *parg)
{
    return 0;
}

/** \brief �Զ������ʵ��豸��Ϣʵ�� */
static am_boot_autobaud_devinfo_t  __g_zlg217_boot_autobaud_devinfo = {

    3,                          /**< \brief CAP����ͨ���� */
    TIMER_WIDTH,                /**< \brief TIMER��ʱ��λ�� */
    10,                         /**< \brief ����һ�����ݵĳ�ʱʱ��(ms)*/

    __zlg217_plfm_autobaud_init,  /**< \brief ƽ̨��ʼ������ */
    __zlg217_plfm_autobaud_deinit,/**< \brief ƽ̨���ʼ������ */

    __fsl_plfm_stat_flag_get
};

/** \brief �Զ������ʹ��ܵ��豸ʵ�� */
am_boot_autobaud_dev_t  __g_zlg217_boot_autobaud_dev;

/** \brief ʵ����ʼ��������Զ������ʷ����� */
am_boot_autobaud_handle_t am_zlg217_boot_autobaud_inst_init (void)
{
    am_cap_handle_t   cap_handle   = am_zlg217_tim2_cap_inst_init();

    am_zlg_tim_cap_dev_t *p_cap_dev = (am_zlg_tim_cap_dev_t *)cap_handle;

    __g_zlg217_boot_autobaud_dev.cap_pin =
            p_cap_dev->p_devinfo->p_ioinfo[__g_zlg217_boot_autobaud_devinfo.cap_chanel].gpio;

    return am_boot_autobaud_init(&__g_zlg217_boot_autobaud_dev,
                                 &__g_zlg217_boot_autobaud_devinfo,
                                  cap_handle);
}

/** \brief �Զ�������ʵ�����ʼ�� */
void am_zlg217_boot_autobaud_inst_deinit (am_boot_autobaud_handle_t handle)
{
    am_zlg217_tim1_cap_inst_deinit(handle->cap_handle);

    am_boot_autobaud_deinit(handle);
}

/**
 * @}
 */

/* end of file */




