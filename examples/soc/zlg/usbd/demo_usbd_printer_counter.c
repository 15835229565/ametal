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
 * \brief ZM516X ģ���������̣�ͨ����׼�ӿ�ʵ��
 *
 * - ʵ������
 *
 *
 * \par Դ����
 * \snippet demo_usbd_printer.c src_usbd_printer
 *
 * \internal
 * \par Modification History
 * - 1.00 18-01-15  pea, first implementation
 * \endinternal
 */

/**
 * \addtogroup demo_if_usbd_printer
 * \copydoc demo_usbd_printer.c
 */

/** [src_usbd_printer] */
#include "ametal.h"
#include "am_softimer.h"
#include "am_wait.h"
#include "am_int.h"
#include "am_delay.h"
#include "am_zlg217_usbd.h"
#include "am_usbd_printer.h"
#include "am_vdebug.h"
#include "am_zlg217_inst_init.h"
#include "am_rngbuf.h"
#include "am_int.h"
#include "string.h"

#define __RNG_BUFF_SIZE    256

static char __g_rng_buff[__RNG_BUFF_SIZE] = {0};

static char __g_buff[__RNG_BUFF_SIZE] = {0};

static struct am_rngbuf __g_rngbuff;  // ����һ�����λ�����ʵ��

static uint32_t __g_data_len  = 0;  /**< \brief һ���������͵ĵ����ݳ���. */
static uint32_t __g_timeout   = 0;  /**< \brief �жϳ�ʱ״̬*/
static uint32_t __g_clock     = 0;  /**< \brief ��ʱ������ֵ,ms����. */
static uint8_t  __g_int_state = 0;  /**< \brief �ж�״̬*/

/**
 * \brief ��ӡ�������ж�����ص�����
 *
 * param[in] p_arg  : �ص���������
 * param[in] p_data : ��������buff
 * param[in] len    : �������ݵ���Ч���ȣ���������
 */
static void __printer_recv_callback(void *p_arg, uint8_t *p_data, uint8_t len)
{
    __g_data_len += len;  /* ����. */
    __g_timeout   = 0;    /* ���㳬ʱʱ��*/
    __g_int_state = 1;    /* �ж�״̬��־*/
	  am_rngbuf_put(&__g_rngbuff, (char *)p_data, len);
    //am_kprintf("%s", p_data);  
}

/**
 * \brief ��ӡ�������ж�����ص�����
 *
 * param[in] p_arg  : �ص���������
 */
static void __printer_send_callback(void *p_arg)
{
//    uint8_t data[] = "ZLG printer demo test string";
//    am_usbd_printer_handle handle = (am_usbd_printer_handle)p_arg;

//    am_usbd_printer_send(handle, data, sizeof(data));
}

/**
 * \brief �����ʱ���жϻص�����
 */
static void  __softimer_callback_func(void * p_arg)
{
    static uint32_t time = 0;

    if (__g_int_state == 1) {       /* �ж��Ƿ�����ж�ʱ��*/
        __g_timeout++;              /* ��ʱʱ���ۼӣ���������ж�״̬����ճ�ʱʱ��*/
        __g_clock++;                /* ���η�������ʱ���ۼ�*/

        if (__g_clock > 1000) {     /* ����ۼ�ʱ�����1s,���㣬��time��ֵ�ۼ�1 */
            __g_clock = 0;
            time++;
        }

        if (__g_timeout  > 1000) {  /* �����ʱʱ�����1s,����Ĭ�ϱ�ʾΪ����һ�����ݷ������*/
            am_kprintf("\n total data: %d byte" \
                       "time-consuming: %d.%03d s\n", __g_data_len,
                                                      time - 1,
                                                      __g_clock);
            /* ���״̬*/
            __g_data_len  = 0;
            __g_timeout   = 0;
            __g_int_state = 0;
            __g_clock     = 0;
            time          = 0;
        }
    }
}

/**
 * \brief �������
 */
void demo_zlg217_usbd_printer_counter_entry (void)
{
		uint32_t key = 0;
    am_kprintf("printer counter demo\n");

    am_mdelay(3000);                                  /* ģ��USB�豸�γ��Ķ��� */
    am_softimer_t soft_time;                          /* �����ʱ��handle*/
    am_usbd_printer_handle handle = NULL;

		  /* ��ʼ�����λ�����*/
	  am_rngbuf_init(&__g_rngbuff, __g_rng_buff, __RNG_BUFF_SIZE);
	
    /* ��ʼ�������ʱ��. */
    am_softimer_init(&soft_time, __softimer_callback_func, handle);
    am_softimer_start(&soft_time, 1);                 /* ��ʼ�����ʱ������ 1ms*/

    handle = am_zlg227_usbd_printer_inst_init();      /* usb��ӡ��ʵ����ʼ��*/

    /* ���������ʱ�����պͷ�������ص�����*/
    am_usbd_printer_recv_request_callback(handle, __printer_recv_callback, handle);
    am_usbd_printer_send_request_callback(handle, __printer_send_callback, handle);

    while (1) {
				if (!am_rngbuf_isempty(&__g_rngbuff)) {
						key = am_int_cpu_lock();
						am_rngbuf_get(&__g_rngbuff, __g_buff, __RNG_BUFF_SIZE);
						
						am_kprintf("%s", __g_buff);
						
						memset(__g_buff, 0, __RNG_BUFF_SIZE);
						am_int_cpu_unlock(key);
					}			
    }
}
/** [src_usbd_printer] */

/* end of file */
