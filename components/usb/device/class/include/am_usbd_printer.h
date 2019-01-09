/*******************************************************************************
*                                 AMetal
*                       ----------------------------
*                       innovating embedded platform
*
* Copyright (c) 2001-2015 Guangzhou ZHIYUAN Electronics Stock Co., Ltd.
* All rights reserved.
*
* Contact information:
* web site:    http://www.zlg.cn/
* e-mail:      ametal.support@zlg.cn
*******************************************************************************/

/**
 * \file
 * \brief USBD PRINTER
 *
 * \internal
 * \par Modification history
 * - 1.00 16-9-27  bob, first implementation.
 * \endinternal
 */

/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __AM_USBD_PRINTER_H
#define __AM_USBD_PRINTER_H

#include "am_usb.h"
#include "am_usb_dci.h"


/*!
 * @addtogroup am_usbd_printer_drv
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/** \brief ��ӡ�����豸�����ʶ  */
#define AM_USBD_CONFIG_PRINTER_CLASS_CODE               (0x07)

/** \brief USB printer �豸������*/
#define AM_USBD_PRINTER_SUBCLASS                        (0x01U)

/** \brief usb printerЭ����*/
#define AM_USBD_PRINTER_PROTOCOL                        (0x02U)

/** \brief ��ȡ�豸id���� */
#define AM_USBD_PRINTER_GET_DEVICE_ID                   (0x00U)
/** \brief ��ȡ�˿�״̬���� */
#define AM_USBD_PRINTER_GET_PORT_STATUS                 (0x01U)
/** \brief ����������� */
#define AM_USBD_PRINTER_SOFT_RESET                      (0x02U)

/** \brief ��ȡ�˿�״̬��ֽ��λ���� */
#define AM_USBD_PRINTER_PORT_STATUS_PAPER_EMPTRY_MASK   (0x20U)
/** \brief ��ȡ�˿�״̬ */
#define AM_USBD_PRINTER_PORT_STATUS_SELECT_MASK         (0x10U)
/** \brief ��ȡ�˿�״̬�Ĵ���λ���� */
#define AM_USBD_PRINTER_PORT_STATUS_NOT_ERROR_MASK      (0x08U)

#define AM_USBD_PRINTER_PORT_STATUS_DEFAULT_VALUE \
    (AM_USBD_PRINTER_PORT_STATUS_SELECT_MASK | AM_USBD_PRINTER_PORT_STATUS_NOT_ERROR_MASK)

/*******************************************************************************
 * API
 ******************************************************************************/

#define AM_USBD_PRINTER_BULK_EP_IN                      (1U)
#define AM_USBD_PRINTER_BULK_EP_OUT                     (2U)
#define AM_USBD_PRINTER_ENDPOINT_COUNT                  (2U)


#if defined(__cplusplus)
extern "C" {
#endif

/** \brief ��ӡ����Ϣ�ṹ�� */
typedef struct am_usbd_printer_info {
    const uint8_t   *p_printer_id;      /**< \brief ��ӡ���豸id*/
    uint8_t          p_printer_id_len;  /**< \brief ��ӡ���豸id, ����*/
    uint8_t         *p_printer_buff;    /**< \brief ��ӡ������buff*/
}am_usbd_printer_info_t;


/** \brief ��ӡ����������ص��������� */
typedef void (*am_printer_recv_cb_t)(void *p_arg, uint8_t *p_buf, uint8_t len);

/** \brief ��ӡ����������ص���������*/
typedef void (*am_printer_send_cb_t)(void *p_arg);

/** \brief usb device printer struct */
typedef struct _usbd_printer {
    am_usbd_dev_t                *p_dev;          /**< \brief ����usb�豸��ָ��*/
    uint8_t                      *p_printer_buff; /**< \brief ��ӡ��buff */
    uint8_t                       port_state;     /**< \brief ��ӡ��״̬. */

    am_printer_send_cb_t          pfn_send_cb;    /**< \brief ��ӡ����������ص�����*/
    void                         *p_send_arg;     /**< \brief ��ӡ����������ص���������*/

    am_printer_recv_cb_t          pfn_recv_cb;    /**< \brief ��ӡ����������ص�����*/
    void                         *p_recv_arg;     /**< \brief ��ӡ����������ص���������*/
    const am_usbd_printer_info_t *p_info;         /**< \brief ��ӡ���豸��Ϣ*/
} am_usbd_printer_t;


typedef am_usbd_printer_t  *am_usbd_printer_handle;



/**
 * \brief ��ӡ����������ص���������
 * \param[in] handle : usb device ��ӡ��handle
 * \param[in] pfn_cb : ��ӡ����������ص�����
 * \param[in] p_arg  : ��ӡ����������ص���������
 *
 * \retval AM_USB_STATUS_SUCCESS         ���óɹ�
 *         AM_USB_STATUS_INVALID_HANDLE  �Ƿ�handle
 */
am_usb_status_t am_usbd_printer_recv_request_callback(am_usbd_printer_handle  handle,
                                                      am_printer_recv_cb_t    pfn_cb,
                                                      void                    *p_arg);

/**
 * \brief ��ӡ����������ص���������
 * \param[in] handle : usb device ��ӡ�����
 * \param[in] pfn_cb : ��ӡ����������ص�����
 * \param[in] p_arg  : ��ӡ����������ص���������
 *
 * \retval AM_USB_STATUS_SUCCESS         ���óɹ�
 *         AM_USB_STATUS_INVALID_HANDLE  ��Ч�ľ��
 */
am_usb_status_t am_usbd_printer_send_request_callback(am_usbd_printer_handle handle,
                                                      am_printer_send_cb_t   pfn_cb,
                                                      void                  *p_arg);


/**
 * \brief ��ӡ����������ص���������
 * \param[in] handle : usb device ��ӡ�����
 * \param[in] pfn_cb : ��ӡ����������ص�����
 * \param[in] p_arg  : ��ӡ����������ص���������
 *
 * \retval AM_USB_STATUS_SUCCESS         ���óɹ�
 *         AM_USB_STATUS_INVALID_HANDLE  ��Ч�ľ��
 */
am_usb_status_t am_usbd_printer_vendor_request_callback(am_usbd_printer_handle handle,
                                                        am_vendor_request_t    pfn_cb,
                                                        void                  *p_arg);
/**
 * \brief ��ӡ�����ͺ���
 *
 * \param[in] handle    : ��ӡ��handle
 * \param[in] p_buff    : �����͵�buff
 * \param[in] length    : �����͵����ݳ���
 *
 * \retval  AM_USB_STATUS_SUCCESS               ֪ͨ�ɹ�
 * \retval  AM_USB_STATUS_ERROR                 ֪ͨʧ��
 * \retval  AM_USB_STATUS_INVALID_REQUEST       USB ��Ӧ����
 * \retval  AM_USB_STATUS_INVALID_HANDLE        ��Ч�ľ��
 * \retval  AM_USB_STATUS_INVALID_PARAMETER     ��������
 */
am_usb_status_t am_usbd_printer_send(am_usbd_printer_handle handle,
                                     uint8_t               *p_buff,
                                     uint32_t               length);

/**
 * \brief ��ʼ��USB
 *
 * \param[in] p_dev     : ָ��USB�豸
 * \param[in] p_info    : ָ��USB�豸��Ϣ
 *
 * \return USB��׼���������������Ϊ NULL��������ʼ��ʧ�ܡ�
 */
am_usbd_printer_handle am_usbd_printer_init (am_usbd_printer_t            *p_dev,
                                             const am_usbd_printer_info_t *p_info,
                                             am_usbd_dev_t                *p_usbd);


/**
 * \brief USB Device ȥ��ʼ��
 *
 * \param[in] p_dev : ָ��USB�豸ʵ��
 */
void am_usbd_printer_deinit (am_usbd_printer_t *p_dev);



#if defined(__cplusplus)
}
#endif


#endif /* __AM_USBD_PRINTER_H */

/* end of file */
