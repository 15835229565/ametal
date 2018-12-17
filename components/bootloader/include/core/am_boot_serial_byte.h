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
 * \brief bootloader �������ݲ���ͨ�ò����ӿ�
 *
 *      ������Ҫ������BootLoader���ڴ������ݲ����ģ����ڳ����࣬���ݲ�ͬ��������ʽ��������ʵ�ֶ��������
 *   ���磬ʹ��uart�������ڴ������ʹ��i2c�����ڴ������
 *
 *
 * \par ʹ��ʾ��
 * \code
 *
 *
 * \endcode
 *
 * \internal
 * \par modification history:
 * - 1.00 18-12-4  yrh, first implementation.
 * \endinternal
 */
#ifndef __AM_BOOT_SERIAL_BYTE_H
#define __AM_BOOT_SERIAL_BYTE_H

#include "am_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*serial_byte_receive_func_t)(uint8_t);

struct am_boot_serial_byte_funcs {
    int (*pfn_serial_byte_send)(void *p_arg, const uint8_t *p_buffer, uint32_t byte_count);
    int (*pfn_serial_byte_receive)(void *p_arg, uint8_t *buffer, uint32_t requested_bytes);
    int (*pfn_serial_int_callback_enable)(void *p_arg, serial_byte_receive_func_t callback_fun);
};

/**
 * \brief bootloader �������ݴ��� ��׼����ṹ��
 */
typedef struct am_boot_serial_byte_serv {
    const struct am_boot_serial_byte_funcs *p_funcs;  /**< \brief �豸��������     */
    void                                   *p_drv;    /**< \brief �豸������������ */
} am_boot_serial_byte_serv_t;

/** \brief bootloader �������ݲ�����׼�������������� */
typedef am_boot_serial_byte_serv_t  *am_boot_serial_handle_t;

/**
 * \brief ͨ�������豸�ӿڷ�����
 *
 * \param[in] handle     : ��׼������
 * \param[in] buffer     : ���͵�����
 * \param[in] byte_count : �������ݵĳ���
 *
 * \retval ���͵��ֽ���
 */
am_static_inline
int am_boot_serial_byte_send(am_boot_serial_handle_t handle,
                             const uint8_t          *buffer,
                             uint32_t                byte_count)
{
    if(handle && handle->p_funcs && handle->p_funcs->pfn_serial_byte_send) {
        return handle->p_funcs->pfn_serial_byte_send(handle->p_drv,
                                                     buffer,
                                                     byte_count);
    }
    return -AM_EINVAL;
}

/**
 * \brief ͨ�������豸�ӿ�������
 *
 * \param[in] handle     : ��׼������
 * \param[in] buffer     : ���յ�����
 * \param[in] byte_count : �������ݵĳ���
 *
 * \retval �յ����ֽ���
 */
am_static_inline
int am_boot_serial_byte_receive(am_boot_serial_handle_t handle,
                                uint8_t                *buffer,
                                uint32_t                requested_bytes)
{
    if(handle && handle->p_funcs && handle->p_funcs->pfn_serial_byte_receive) {
        return handle->p_funcs->pfn_serial_byte_receive(handle->p_drv,
                                                        buffer,
                                                        requested_bytes);
    }
    return -AM_EINVAL;
}

/**
 * \brief �����жϽ����û�����Ļص�����
 *
 * \param[in] handle       : ��׼������
 * \param[in] callback_fun : �û��Ļص�������,serial_byte_receive_func_t��һ������ָ������
 *                           ���жϽ��յ���һ���ֽڣ��û������ڻص��������洦������ֽ�
 *
 * \retval AM_OK : �ɹ�
 */
am_static_inline
int am_boot_serial_int_recev_callback_enable(am_boot_serial_handle_t    handle,
                                             serial_byte_receive_func_t callback_fun)
{
    if(handle && handle->p_funcs && handle->p_funcs->pfn_serial_int_callback_enable) {
        return handle->p_funcs->pfn_serial_int_callback_enable(handle->p_drv,
                                                               callback_fun);
    }
    return -AM_EINVAL;
}

#ifdef __cplusplus
}
#endif

#endif /* __AM_BOOT_SERIAL_BYTE_H */
/* end of file */
