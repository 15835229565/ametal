/*******************************************************************************
*                                 AMetal
*                       ----------------------------
*                       innovating embedded platform
*
* Copyright (c) 2001-2018 Guangzhou ZHIYUAN Electronics Stock Co., Ltd.
* All rights reserved.
*
* Contact information:
* web site:    http://www.zlg.cn/
* e-mail:      ametal.support@zlg.cn
*******************************************************************************/

/**
 * \file
 * \brief usbd msc ���̣�ͨ����׼�ӿ�ʵ��
 *
 * - ʵ������
 *
 * \note
 *
 * \par Դ����
 * \snippet demo_usbd_msc.c src_usbd_msc
 *
 * \internal
 * \par Modification History
 * - 1.00 18-01-15  pea, first implementation
 * \endinternal
 */
/**
 * \brief �������
 */
#include "ametal.h"
#include "am_int.h"
#include "am_delay.h"
#include "am_zlg217_usbd.h"
#include "am_usbd_msc.h"
#include "am_zlg217_inst_init.h"

/**
 * \brief �������
 */
void demo_zlg217_usbd_msc_entry (void)
{
    uint32_t  key = 0;

    am_usbd_msc_handle usbd_handle;

    AM_DBG_INFO("Start up successful!\r\n");
    /* ��λ����ʱһ��ʱ�䣬ģ��USB�豸�γ��Ķ��� */
    am_mdelay(3000);

    usbd_handle = am_zlg217_usb_msc_inst_init();

    while (1) {

        /** ����Ӧ���жϴ������� ��λ��־λ������������while(1) ����ѯ����־λ��Ȼ�������Ӧ�Ĵ���
         *  ��Ҫ���ж���ֱ�Ӵ����������/���봦����Ϊ��������/������������ϴ�ִ�е�ʱ��ϳ������һ�������
         *  �ж���ѯִ�У����׵������ݷ�����ȥ���ж�ִ��ʱ��ϳ����³������г���
         */
        if (usbd_handle->int_status_out == AM_TRUE) {

            /* �Ƚ�ֹ�жϣ���ִ����������/�������������ʧЧ��־λ�������ж� */
            key = am_int_cpu_lock();

            am_zlg217_usb_msc_enpoint2_bulk_out(usbd_handle);        // ���ݴ���

            usbd_handle->int_status_out = AM_FALSE;                  // ����жϱ�־λ

            am_int_cpu_unlock(key);
        }

        if (usbd_handle->int_status_in == AM_TRUE) {

            key = am_int_cpu_lock();
            am_zlg217_usb_msc_enpoint1_bulk_in(usbd_handle);

            usbd_handle->int_status_in = AM_FALSE;

            am_int_cpu_unlock(key);
        }
    }
}
/**
 * \addtogroup demo_if_usbd_msc
 * \copydoc demo_usbd_msc.c
 */


/** [src_usbd_msc] */

/* end of file */
