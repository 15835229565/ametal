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
 * \brief bootloader ͨ�û�����׼�ӿ�
 *
 * \internal
 * \par Modification History
 * - 1.00 18-11-13  yrh, first implementation.
 * \endinternal
 */
 
 #ifndef __AM_BOOT_H
 #define __AM_BOOT_H
 
#ifdef __cplusplus
extern "C" {
#endif
#include "ametal.h"

typedef struct am_boot_mem_info {
    uint32_t  flash_start_addr;    /**< \brief flash����ʼ��ַ*/
    uint32_t  flash_size;          /**< \brief flash�Ĵ�С */

    uint32_t  ram_start_addr;      /**< \brief ram��ʼ��ַ */
    uint32_t  ram_size;            /**< \brief ram�Ĵ�С */
}am_boot_mem_info_t;


/**
 * \brief �ж�Ӧ�ô����Ƿ������ת��ȥִ��
 * 
 * \param[in] app_start_addr : Ӧ�ô����׵�ַ
 * 
 * \retval  ��
 */ 
am_bool_t am_boot_app_is_ready(uint32_t app_start_addr);

/**
 * \brief ��ת��Ӧ�ô������
 *
 * \param[in] app_start_addr : Ӧ�ô����׵�ַ
 *
 * \retval AM_ERROR ��ת������߲������벻�Ϸ�
 */
int am_boot_go_application(uint32_t app_start_addr);

/**
 * \brief ϵͳ�������
 *
 * \retval ��
 */
void am_boot_reset(void);

/**
 * \brief ��ȡ�������ڴ���Ϣ����Ϣ
 * \param[in] mem_info : �����Ż�ȡ�����Ϣ

 * \retval ��
 */
void am_boot_base_mem_info_get(am_boot_mem_info_t **pp_mem_info);

/**
 * \brief �ͷ�ϵͳ��Դ
 *
 * \note bootloader����ת��Ӧ�ô���ǰ��������ô˽ӿڣ�
 *       ��bootloader���������Դ���߳�ʼ����ĳЩ���趼Ӧ�����ͷźͽ��ʼ���������Ӧ�ó������Ӱ�졣

 * \retval AM_OK : �ɹ�
 */
int am_boot_source_release(void);

#ifdef __cplusplus
}
#endif

 #endif /* __AM_BOOTS_H */
 
 /* end of file */
