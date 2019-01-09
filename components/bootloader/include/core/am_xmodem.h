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
 * \brief  XmodemЭ�� Ӧ�ýӿ��ļ�
 *
 * \internal
 * \par Modification History
 * - 1.00 18-8-31 , xgg, first implementation.
 * \endinternal
 */
#ifndef __AM_XMODEM_H
#define __AM_XMODEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "am_uart_rngbuf.h"
#include "am_softimer.h"

/**
 * @addtogroup am_if_am_xmodem
 * @copydoc am_xmodem.h
 * @{
 */

#define AM_XMODEM_1K_MODE     0    /**< \brief ���ݶ���1K�ֽڵĹ���ģʽ*/
#define AM_XMODEM_128_MODE    1    /**< \brief ���ݶ���128�ֽڵĹ���ģʽ*/

#define AM_XMODEM_CRC_MODE    0    /**< \brief ����CRCУ�� */
#define AM_XMODEM_SUM_MODE    1    /**< \brief �����ۻ���У�� */

#define AM_DATA_ERR           5    /**< \brief ���ܵ������ݴ��� */
#define AM_DATA_CAN           15   /**< \brief ���ͷ�ȡ������*/
#define AM_DATA_SUC           20   /**< \brief �ļ����ճɹ� */

#define AM_XMODEM_REC         3    /**< \brief Xmodem����ģʽ */
#define AM_XMODEM_TRA         4    /**< \brief Xmodem����ģʽ */


#define AM_XMODEM_SOH             0x01 /**< \brief ����λΪ128�ֽڹ���ģʽ�������ʼλ*/
#define AM_XMODEM_STX             0x02 /**< \brief ����λΪ1K�ֽڹ���ģʽ�������ʼλ*/
#define AM_XMODEM_EOT             0x04 /**< \brief �ļ�������ϵĽ����ź� */
#define AM_XMODEM_ACK             0x06 /**< \brief ���շ���ȷ���źţ���ʾһ֡���ݽ������ */
#define AM_XMODEM_NAK             0x15 /**< \brief ��һ��ʹ���Ǿ�������ģʽ������ʹ��Ϊ�ظ����� */
#define AM_XMODEM_CAN             0x18 /**< \brief ����ȡ������ */
#define AM_XMODEM_CTRLZ           0x1A /**< \brief ���һ֡���ݲ���ʱ�ô���� */
#define AM_XMODEM_1k              'C'  /**< \brief ����Xmodem״̬�ַ�*/

#define AM_XMODEM_NAK_TIME              0x40 /**< \brief �Ѵﵽ����ط����� */
#define AM_XMODEM_MOU_SUC               0x41 /**< \brief �ļ�������� */

/**
 * \breif �û����ջص�����
 * \param[in] p_arg    �û���������ص������Ĳ��������û����ڻص�������
 *                     ʹ��Xmodem������ɽ��������˲�����
 * \param[in] p_frames Xmodem���յ���һ֡���ݱ���ĵ�ַ���û���ֱ��
 *                     ʹ�øõ�ַ��ȡ���յ���һ֡����
 * \param[in] result   ���ж�resultֵ��������������0����һ֡���ݽ���
 *                     �ɹ���result������ǽ��յ����ֽ�������result
 *                     ֵΪ���������ʧ�ܣ��ɸ���reslut��ֵ�ж�������
 *                     ���ִ���
 *                     -AM_ETIME:    ��һ֡���ݳ�ʱδ���յ�
 *                     -AM_EBADMSG:  δ��ʱ���ط���κ�������Ȼ����
 *                     -AM_DATA_CAN: ���ͷ�����ȡ�����͵��´������
 */
typedef void (*am_xmodem_user_rx_t) (void    *p_arg,
                                     void    *frames,
                                     int      result);

/**
 * \brief �û����ͻص�����
 *
 * \param[in] p_arg  �û���Ҫ����Ĳ���,���Լ�����
 * \param[in] event  �ص����ص��¼����û��ɸ����¼�������������һ������
 *                   ��event == AM_XMODEM_NAKʱ,Xmodem������128�ֽڹ���ģʽ��
 *                   ��event == AM_XMODEM_1Kʱ,Xmodem������1K�ֽڹ���ģʽ��
 *                   ��event == AM_XMODEM_NAK_TIMEʱ,�ط������ﵽ�涨ֵ(�ڲ���ȡ������);
 *                   ��event == AM_XMODEM_MOU_SUCʱ,�ļ��������;
 *                   ��event == AM_XMODEM_CANʱ,�������շ�ȡ���˴���;
 *                   ��event == AM_XMODEM_ACKʱ��һ֡���ݷ������;
 *                   ��event == -AM_ETIMEʱ,���շ����յ�һ֡���ݺ�δ�ڹ涨��ʱ���ڻ�Ӧ;
 */
typedef void (*am_xmodem_user_tx_t) (void *p_arg, int event);

/** \brief xmodemЭ������豸ʵ�� */
typedef struct am_xmodem_rec_dev_info {
    char          *frames_info;                 /**< \brief ����Xmodemһ֡��Ϣ������ָ�� */
    uint8_t        nak_max_times;               /**< \brief �ط������� */
    uint32_t       frames_bytes;                /**< \brief һ֡���ݵ��ֽ��� */
    uint32_t       data_mode;                   /**< \brief ����ģʽ  */
    uint32_t       parity_mode;                 /**< \brief У��ģʽ */
    uint32_t       rx_timeout;                  /**< \brief ���ճ�ʱʱ�� */
} am_xmodem_rec_dev_info_t;


/** \brief XmodemЭ�鷢���豸ʵ��*/
typedef struct am_xmodem_tx_dev_info {
    uint32_t   tx_timeout;                       /**< \brief ���ͳ�ʱʱ��*/
    uint8_t    nak_times_max;                    /**< \brief ���������ط�����*/
} am_xmodem_tx_dev_info_t;

/**
 * \brief xmodem�����豸�ṹ��
 */
typedef struct am_xmodem_rec_dev {
    int                             rx_bytes;       /**< \brief ��ǰ���յ����ֽ��� */
    void                           *p_arg;          /**< \brief �û��ص��������� */
    void                           *state_handle;   /**< \brief ״̬���������*/
    char                            fra_sum_parity; /**< \brief �����ۻ���У��λ */
    uint8_t                         state_rx_char;  /**< \brief ״̬��־λ */
    uint8_t                         nak_state;      /**< \brief ��ǰ�ط�״̬ */
    uint16_t                        fra_crc_pry;    /**< \brief ����CRCУ���� */
    uint32_t                        frames_num;     /**< \brief ��ǰ��һ֡���ݵ����к� */
    uint32_t                        tx_bytes;       /**< \brief ��ǰһ֡�ѷ��Ͷ��� */
    am_softimer_t                   softimer;       /**< \brief ���ճ�ʱ��ʱ�� */
    am_uart_handle_t                uart_handle;    /**< \brief ���ھ�� */
    am_xmodem_user_rx_t             pfn_rx_callback;/**< \brief ע���û����ܻص����� */
    const am_xmodem_rec_dev_info_t *p_rec_devinfo;  /**< \brief �����豸��Ϣ�ṹ��ָ�� */
} am_xmodem_rec_dev_t;

/** \brief xmodem��׼�������������Ͷ��� */
typedef am_xmodem_rec_dev_t *am_xmodem_rec_handle_t;

typedef struct am_xmodem_tx_dev {
    void                          *p_arg;          /**< \brief �û��ص��������� */
    void                          *state_handle;   /**< \brief ״̬���������*/
    char                          *p_tx_arg;       /**< \brief �ļ�ָ�� */
    char                           fra_sum_parity; /**< \brief �����ۻ���У��λ */
    uint8_t                        nake_state;     /**< \brief ��ǰ�ط�״̬ */
    uint8_t                        state_flag;     /**< \brief ���ڷֱ����ģʽ�ַ�*/
    uint8_t                        state_rx_char;  /**< \brief ״̬��־λ */
    int16_t                        fra_crc_pry;    /**< \brief ����CRCУ���� */
    uint32_t                       frames_num;     /**< \brief ��ǰ��һ֡���ݵ����к� */
    uint32_t                       tx_bytes;       /**< \brief ��ǰһ֡�ѷ��Ͷ��� */
    uint32_t                       frame_tx_bytes; /**< \brief ���͵�һ֡���ݴ�С */
    uint32_t                       doc_bytes;      /**< \brief �ļ���С */
    am_softimer_t                  softimer;       /**< \brief ���ճ�ʱ��ʱ�� */
    const am_xmodem_tx_dev_info_t *p_tra_devinfo;  /**< \brief �����豸��Ϣ�ṹ��ָ��*/
    am_xmodem_user_tx_t            pfn_tx_callback;/**< \brief ע���û����ͻص����� */
    am_uart_handle_t               uart_handle;    /**< \brief ���ھ�� */
}am_xmodem_tx_dev_t;

typedef am_xmodem_tx_dev_t *am_xmodem_tx_handle_t;


/**
 * \brief �豸��ʼ������
 *
 * \param[in] p_dev       xmodem�豸���
 * \param[in] p_decinfo   �豸��Ϣ�ṹ��
 * \param[in] uart_handle ���ھ��
 *
 * \return Xmodem�������
 */
am_xmodem_rec_handle_t  am_xmodem_rec_init (
    am_xmodem_rec_dev_t            *p_dev,
    const am_xmodem_rec_dev_info_t *p_devinfo,
    am_uart_handle_t                uart_handle);

/**
 * \brief xmodem���ջص�����ע��
 *
 * \param[in] p_dev      xmodem�豸���
 *            pfn_rec_cb �û��ص�����
 *            p_arg      �û�����Ĳ���
 *
 * \retval AM_TRUE  ִ�гɹ�
 *         AM_FALSE ִ��ʧ��
 */
am_bool_t am_xmodem_rec_cb_reg(am_xmodem_rec_dev_t  *p_dev,
                               am_xmodem_user_rx_t   pfn_rec_cb,
                               void                 *p_arg);

/**
 * \brief ��ʼ���ܺ���
 *
 * \param[in] p_dev xmodem�豸���
 *
 * \retval AM_TRUE  ִ�гɹ�
 *         AM_FALSE ִ��ʧ��
 */
am_bool_t am_xmodem_rec_start (am_xmodem_rec_dev_t *p_dev);
/**
 * \brief �������պ���
 *
 * \param[in] p_dev xmodem�豸���
 *
 * \retval AM_TRUE    ִ�гɹ�
 *         -AM_EINVAL ��������
 */
am_err_t am_xmodem_rec_ack_set (am_xmodem_rec_dev_t   *p_dev);
/**
 * \brief ȡ�����亯��
 *
 * \param[in] p_dev xmodem�豸���
 *
 * \retval AM_TRUE    ִ�гɹ�
 *         -AM_EINVAL ��������
 */
am_err_t am_xmodem_rec_can_set (am_xmodem_rec_dev_t  *p_dev);

/*******************************************************************************/
/**
 * \brief �����豸��ʼ������
 *
 * \param[in] p_dev         Xmodem�����豸
 * \param[in] p_tra_devinfo Xmodem�����豸��Ϣ�ṹ��
 * \param[in] uart_handle   ���ھ��
 *
 * \return Xmodem���;��
 *
 */
am_xmodem_tx_handle_t  am_xmodem_tx_init (
                              am_xmodem_tx_dev_t            *p_dev,
                              const am_xmodem_tx_dev_info_t *p_tra_devinfo,
                              am_uart_handle_t               uart_handle);

/**
 * \brief ��ʼ���ͺ���
 *
 * \param[in] p_dev     Xmodem�����豸
 * \param[in] p_doc     �û���Ҫ���͵������ָ��
 * \param[in] pack_size ��Ҫ���͵����鳤�ȣ����ص�����������¼�ΪAM_XMODEM_NAK,��pack_size
 *                      �����ֵΪ128,���ص�����������¼�ΪAM_XMODEM_1K,��pack_size�����
 *                      ֵΪ1024������ļ����һ֡���ݲ�����128����1024��pack_size��ֵ�û�����
 *                      �Լ����壬������Ϊ�ļ�ʣ�µ��ֽ�����
 *
 * \retval AM_TRUE    ִ�гɹ�
 *         -AM_EINVAL ���Ϊ�գ���������
 */
am_err_t am_xmodem_tx_pack (am_xmodem_tx_dev_t *p_dev,
                            char               *p_doc,
                            uint32_t            pack_size);

/**
 * \brief xmodem���ͻص�����ע��
 *
 * \param[in] p_dev      Xmodem�����豸
 * \param[in] pfn_tx_cb  �û��ص�����ָ��
 * \param[in] p_arg      �û������豸����
 *
 * \retval AM_TRUE    ִ�гɹ�
 *         -AM_EINVAL ���Ϊ�գ���������
 */
am_err_t am_xmodem_tx_cb_reg (am_xmodem_tx_dev_t  *p_dev,
                              am_xmodem_user_tx_t  pfn_tx_cb,
                              void                *p_arg);

/**
 * \brief �ļ����ͽ�������
 *
 * \param[in] p_dev Xmodem�����豸
 *
 * \retval AM_TRUE    ִ�гɹ�
 *         -AM_EINVAL ��������
 */
am_err_t am_xmodem_tx_over (am_xmodem_tx_dev_t *p_dev);

/**
 * \brief �û�ȡ�����ͺ���
 *
 * \param[in] p_dev Xmodem�����豸
 *
 * \retval AM_TRUE    ִ�гɹ�
 *         -AM_EINVAL ��������
 */
am_err_t am_xmodem_tx_can_set (am_xmodem_tx_dev_t *p_dev);
/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __AM_XMODEM_H */

/* end of file */
