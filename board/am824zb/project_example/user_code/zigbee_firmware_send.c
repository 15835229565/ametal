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
 * \brief ZM516X ģ����ʾ���̣�ͨ����׼�ӿ�ʵ��
 *
 *
 * \note
 *    1. LED0 ��Ҫ�̽� J9 ����ñ�����ܱ� PIO0_8 ���ƣ�
 *    2. ʹ�ð���������Ҫ�� J14 �� KEY �� PIO0_1 �̽���һ��
 *    3. ����۲촮�ڴ�ӡ�ĵ�����Ϣ����Ҫ�� PIO0_0 �������� PC ���ڵ� TXD��
 *       PIO0_4 �������� PC ���ڵ� RXD��
 *    4. ���Ա� Demo ������ am_prj_config.h �ڽ� AM_CFG_KEY_GPIO_ENABLE ����Ϊ 1
 *       ���ú��Ѿ�Ĭ������Ϊ 1�� �û������ٴ����ã�
 *    5. ZigBee ģ�������ӹ�ϵ���£�
 * <pre>
 *           PIO0_26  <-->  ZigBee_TX
 *           PIO0_27  <-->  ZigBee_RX
 *           PIO0_28  <-->  ZigBee_RST
 * </pre>
 *        �����Ҫʹ�� ZigBee����Щ IO �ڲ�������������;��
 *
 * \par Դ����
 * \snippet demo_lpc82x_std_zm516x.c src_lpc28x_std_zm516x
 *
 * \internal
 * \par Modification History
 * - 1.01 18-01-16  pea, simplify demo, enable display head
 * - 1.00 17-05-26  mex, first implementation
 * \endinternal
 */

/**
 * \addtogroup demo_if_lpc82x_std_zm516x
 * \copydoc demo_lpc82x_std_zm516x.c
 */

/** [src_lpc28x_std_zm516x] */
#include "ametal.h"
#include "am_zm516x.h"
#include "am_led.h"
#include "am_delay.h"
#include "am_vdebug.h"
#include "am_board.h"
#include <string.h>

#include "am_uart.h"
#include "am_lpc82x_inst_init.h"


#define RNGBUF_SIZE   1024   
#define SEND_BUF_SIZE 32

/**
 * \brief ��˸ LED0
 */
static void flash_led (void)
{
    while (1) {
        am_led_on(LED0);
        am_mdelay(50);

        am_led_off(LED0);
        am_mdelay(500);
    }
}

static void __timer (void *p_arg)
{
	 uint8_t *p_flog = (uint8_t *)p_arg;
	
	 (*p_flog) ++;
}

static uint8_t uart_rx_buf[RNGBUF_SIZE];

static am_uart_rngbuf_dev_t __g_rngbuf_dev;

static uint8_t  buf[64] = {0};

/**
 * \brief �������
 */
void demo_zigbee_firmware_send_entry (void)
{

    uint8_t              src_addr[2] = {0x00, 0x00};
    uint8_t              dst_addr[2] = {0x20, 0x02};

    am_zm516x_cfg_info_t zm516x_cfg_info;
    am_zm516x_handle_t   zm516x_handle    = am_zm516x_inst_init();
		
		am_uart_handle_t        uart_handle;
		am_uart_rngbuf_handle_t rngbuf_handle = NULL;

    /* ��ȡ ZigBee ģ���������Ϣ���������D1�� */
    if (am_zm516x_cfg_info_get(zm516x_handle, &zm516x_cfg_info) != AM_OK) {
        AM_DBG_INFO("am_zm516x_cfg_info_get failed\r\n");
        flash_led();
    }

    zm516x_cfg_info.my_addr[0] = src_addr[0];
    zm516x_cfg_info.my_addr[1] = src_addr[1];
    zm516x_cfg_info.dst_addr[0] = dst_addr[0];
    zm516x_cfg_info.dst_addr[1] = dst_addr[1];
    zm516x_cfg_info.panid[0] = 0x10;
    zm516x_cfg_info.panid[1] = 0x01;

    /* �޸� ZigBee ģ���������Ϣ���������D6�������óɹ��踴λ */
    if (am_zm516x_cfg_info_set(zm516x_handle, &zm516x_cfg_info) != AM_OK) {
        AM_DBG_INFO("am_zm516x_cfg_info_set failed\r\n");
        flash_led();
    }

    /* ʹ ZigBee ģ�鸴λ���������D9�� */
    am_zm516x_reset(zm516x_handle);
    am_mdelay(10);


    /**
     * ���� ZigBee ģ�����˯��ģʽ����ʱ���D8������Ҫ�������ԣ�ZM516X ˯�ߺ�
     * ֻ�ܸ�λģ����ģ��� WAKE ��������������ģ��
     */
//    am_zm516x_enter_sleep(zm516x_handle);

		uint8_t  dat;
		static uint16_t dst_addr_set = 0x0001;
		
    /* ���в���ͨ�������� LED0 */
    am_led_on(LED0);
		
    am_softimer_t timer;
		uint8_t       flog       = 0;
		uint8_t       recv_ok[10];
		
		uint32_t num = 0; 
		uint32_t count = 0;
		
		uart_handle = am_lpc82x_usart0_inst_init();
    am_uart_ioctl(uart_handle, AM_UART_MODE_SET, (void *)AM_UART_MODE_POLL);
		am_uart_ioctl(uart_handle, AM_UART_BAUD_SET, (void *)38400);
		
		rngbuf_handle = am_uart_rngbuf_init(&__g_rngbuf_dev,
		                                    uart_handle,
		                                    uart_rx_buf,
		                                    sizeof(uart_rx_buf),
																				&dat,
																				1);
																				
		am_uart_rngbuf_ioctl(rngbuf_handle, AM_UART_RNGBUF_TIMEOUT, (void *)20);
		am_softimer_init(&timer, __timer, &flog);
		am_softimer_start(&timer, 10);
		
    AM_FOREVER {

			  num = am_uart_rngbuf_receive(rngbuf_handle, buf, sizeof(buf));
			
			  if (num > 0) {
					
					  flog = 0;
					  count += num;
						
					  am_zm516x_send(zm516x_handle, buf, num);
					
					  memset(uart_rx_buf, 0xff, sizeof(buf));
					           					
				}
				
				if (am_zm516x_receive(zm516x_handle, recv_ok, sizeof(recv_ok)) > 0) {
					  if (strstr((const char *)recv_ok, "recvOK") != NULL) {
							  AM_DBG_INFO("set addr success, next dst_addr is 0x%04x\r\n", ++dst_addr_set);
							  memset(recv_ok, 0, sizeof(recv_ok));
						}
				}
				
				if (flog >= 30 && count > 0) {
					
					am_zm516x_send(zm516x_handle, &count, sizeof(count));
					
					AM_DBG_INFO("\r\n set now dst addr 0x%04x\r\n", dst_addr_set);
					am_zm516x_send(zm516x_handle, &dst_addr_set, sizeof(dst_addr_set));
					
					AM_DBG_INFO("\r\nsuccessul size = %d bytes\r\n", count);
					
					count = 0;
				}				
    }
}
/** [src_lpc28x_std_zm516x] */

/* end of file */
