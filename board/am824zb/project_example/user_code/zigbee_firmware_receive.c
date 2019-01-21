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
 * - �������裺
 *   1. ���Ա� Demo ��Ҫʹ������ AM824ZB �壬�������Ҫ���ز�ͬ�ĳ���
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
 * \snippet demo_zm516x.c src_zm516x
 *
 * \internal
 * \par Modification History
 * - 1.01 18-01-16  pea, simplify demo, enable display head
 * - 1.00 17-05-26  mex, first implementation
 * \endinternal
 */

/**
 * \addtogroup demo_if_zm516x
 * \copydoc demo_zm516x.c
 */

/** [src_zm516x] */
#include "ametal.h"
#include "am_zm516x.h"
#include "am_led.h"
#include "am_delay.h"
#include "am_vdebug.h"
#include "am_board.h"
#include <string.h>
#include "am_int.h"

#include "am_softimer.h"
#include "am_lpc82x_inst_init.h"
#include "hw/amhw_lpc82x_iap.h"

#define RECV_BUF_SIZE     64
#define SELF_ADDR_SET     0

static void __timer (void *p_arg)
{
	 uint8_t *p_flog = (uint8_t *)p_arg;
	
	 (*p_flog) ++;
}

static uint8_t __attribute__((aligned(256))) recv_buf[RECV_BUF_SIZE] = {0};

static int __bootloader_to_app (uint32_t addr)
{
    void (*app_func) (void);

    app_func = (void (*)(void))(*(uint32_t *)(addr + 4));

	  __disable_irq();
	
    __set_MSP(*(uint32_t *)addr);

    app_func();
		
	return AM_OK;
}

static void __self_addr_set (am_zm516x_handle_t zm516x_handle, uint16_t addr)
{
    uint8_t              dst_addr[2] = {0x00};
		
	  am_zm516x_cfg_info_t zm516x_cfg_info;
	
    /* ��ȡ ZigBee ģ���������Ϣ���������D1�� */
    if (am_zm516x_cfg_info_get(zm516x_handle, &zm516x_cfg_info) != AM_OK) {
        AM_DBG_INFO("am_zm516x_cfg_info_get failed\r\n");
    }

    zm516x_cfg_info.my_addr[0] = (uint8_t)(addr >> 8);
    zm516x_cfg_info.my_addr[1] = (uint8_t)(addr & 0xff);
    zm516x_cfg_info.dst_addr[0] = dst_addr[0];
    zm516x_cfg_info.dst_addr[1] = dst_addr[1];
    zm516x_cfg_info.panid[0] = 0x10;
    zm516x_cfg_info.panid[1] = 0x01;

    /* �޸� ZigBee ģ���������Ϣ���������D6�������óɹ��踴λ */
    if (am_zm516x_cfg_info_set(zm516x_handle, &zm516x_cfg_info) != AM_OK) {
        AM_DBG_INFO("am_zm516x_cfg_info_set failed\r\n");
    }

    /* ʹ ZigBee ģ�鸴λ���������D9�� */
    am_zm516x_reset(zm516x_handle);
    am_mdelay(10);
		
		AM_DBG_INFO("\r\n set scr addr 0x%04x\r\n", addr);
}


/**
 * \brief �������
 */
void demo_zigbee_firmware_recive_entry ()
{
		am_zm516x_handle_t   zm516x_handle    = am_zm516x_inst_init();    

	  __self_addr_set(zm516x_handle, 0x2002);
		
    am_led_on(LED0);

	  int           key;
	  uint32_t      count = 0;
		am_softimer_t timer;
		uint8_t       flog = 0;
		uint32_t      ret = 0;
		uint8_t       offset = 0; 
		
		key = am_int_cpu_lock();
		amhw_lpc82x_iap_prepare(14, 31);
		amhw_lpc82x_iap_erase_sector(14, 31);
		am_int_cpu_unlock(key);
		
		am_softimer_init(&timer, __timer, &flog);
		am_softimer_start(&timer, 10);
		
    AM_FOREVER {
			
//        memset(recv_buf, 0xff, sizeof(recv_buf));
        /* am_zm516x_receive �����Ķ���ʱΪ 10ms */
        ret = am_zm516x_receive(zm516x_handle, recv_buf, sizeof(recv_buf));
			  
        if (ret > 0) {
					
					key = am_int_cpu_lock();
					amhw_lpc82x_iap_prepare(14, 31);
          amhw_lpc82x_iap_copy(0x3800 + count, (uint32_t )recv_buf, 64);
					am_int_cpu_unlock(key);
					
					count += ret;
          flog = 0;
					
          if (ret < sizeof(recv_buf))	{
						  offset = ret;
					}
          
        }
				
				if (flog > 50 && count > 0) {
					
					count -= 6;

					if (offset == 0) {
						offset = sizeof(recv_buf);
					}
					
					if (count == ((uint32_t)recv_buf[offset - 3] << 24 | 
						            (uint32_t)recv_buf[offset - 4] << 16 | 
					              (uint16_t)recv_buf[offset - 5] << 8  | 
					              (uint8_t)recv_buf[offset - 6] )) {
						  AM_DBG_INFO("\r\nsum checkout successul !!!\r\n");

#if SELF_ADDR_SET													
							__self_addr_set(zm516x_handle, (uint16_t)recv_buf[offset - 1] << 16 | 
													                   (uint8_t)recv_buf[offset - 2]);
							am_zm516x_send(zm516x_handle, "recvOK", sizeof("recvOK"));
#endif
							//break;
					}
					AM_DBG_INFO("\r\ncheckout error size = %d bytes\r\n", count);
					count = 0;
					
				}		
    }
		
		extern am_timer_handle_t g_mrt_handle;
		
		/* ����Ӧ�ô���ǰ��ر�bootloader��������Ӳ�� */
		am_softimer_stop(&timer);
		am_zm516x_deinit(zm516x_handle);
		am_lpc82x_mrt_inst_deinit(g_mrt_handle);
		
		__bootloader_to_app(0X3800);
}
/** [src_zm516x] */

/* end of file */
