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
 * \brief  XmodemЭ�� ����
 *
 * \internal
 * \par Modification History
 * - 1.00 18-8-31 , xgg, first implementation.
 * \endinternal
 */

#include "ametal.h"
#include "am_types.h"
#include "am_uart.h"
#include "string.h"
#include "am_errno.h"
#include "am_xmodem.h"
#include "am_softimer.h"
#include "string.h"


#define __AM_XMODEM_DATA_EOT         0x50   /**< \brief �ļ����һ֡����������*/
#define __AM_XMODEM_DOC_EOT          0x51   /**< \brief �ļ�������� */
#define AM_XMODEM_EOT_ACK            0x52   /**< \brief �ļ������ź��ѷ��Ͳ�Ӧ��*/
/** \brief ����Xmodem����״̬������ָ������ */
typedef am_bool_t (*am_xmodem_rec_pfn_handle)(void *p_arg, char inchar);

/** \brief ����Xmodem����״̬������ָ������ */
typedef am_bool_t (*am_xmodem_tra_pfn_handle)(void *p_arg, char *outchar);


/**
 * \brief ���½�����һ֡����
 */
am_static_inline
void __xmodem_rx_nak_set (am_xmodem_rec_dev_t  *p_dev)
{
    /* ���ͷ�����NAK�ط��ź�*/
    p_dev->p_rec_devinfo->frames_info[0] = AM_XMODEM_NAK;
    p_dev->state_rx_char = 1;
    /* ��������*/
    am_uart_tx_startup(p_dev->uart_handle);
    /*���������ʱ�� */
    am_softimer_start(&p_dev->softimer, p_dev->p_rec_devinfo->rx_timeout);
    /* �����ط����� */
    ++p_dev->nak_state;
    if (p_dev->nak_state == p_dev->p_rec_devinfo->nak_max_times) {
        /* Ҫ�����������ط���� */
        p_dev->nak_state = AM_OK;
        p_dev->rx_bytes  = -AM_EBADMSG;
        /* δ�ܽ��ܵ���ȷ�����ݣ������û��ص�����֪ͨ�û����ݴ��� */
        p_dev->pfn_rx_callback(p_dev->p_arg,
                            p_dev->p_rec_devinfo->frames_info,
                            p_dev->rx_bytes);
    }
}
/**
 * \brief ���ͷ�����ȡ�����ͺ���
 */
am_static_inline
void __xmodem_rec_can (am_xmodem_rec_dev_t *p_dev, char inchar)
{
    p_dev->rx_bytes = -AM_DATA_CAN;
    /* �ص�֪ͨ�û� ���ͷ�����ȡ������*/
    p_dev->pfn_rx_callback(p_dev->p_arg,
                        p_dev->p_rec_devinfo->frames_info,
                        p_dev->rx_bytes);
}
/**
 * \brief ����CRCУ���뺯��
 *
 * \param[in]  ptr    ������ݵ������ָ��
 * \param[in]  count  ������ݵ�������ֽ���
 *
 * \return crc У����
 */
am_static_inline
int16_t __xmodem_check_mode_crc (char  *ptr, int16_t count)
{
    int16_t  crc = 0;
    char i;

    crc = 0;
    while (--count >= 0) {
        crc = crc ^ (((int)*ptr++) << 8); /*ÿ��������Ҫ�����������򣬼�������ָ����һ������*/
        i = 8;
        do
        {
           if (crc & 0x8000) {            /*�ж�CRC�ĵ�16λ�Ƿ�Ϊ0*/
              /* ��λΪ1������һλ�����*/
              crc = (crc << 1) ^ 0x1021;
           }
           else {
              /* ����λ��Ϊ1������һλ���ж���һλ�Ƿ�Ϊ1*/
              crc = crc << 1;
           }
         } while (--i);
      }
    /*���ص�crc��һ��16λ��*/
    return (crc);
}
/**
 * \brief ���շ������жϺ���
 */
am_static_inline
int __xmodem_data_check(am_xmodem_rec_dev_t *p_dev)
{
    if (p_dev->p_rec_devinfo->parity_mode == AM_XMODEM_CRC_MODE)
    {
        /* ��ȡcrcУ����*/
        int16_t crc = __xmodem_check_mode_crc(p_dev->p_rec_devinfo->frames_info,
                                              p_dev->p_rec_devinfo->frames_bytes);
        int16_t tcrc = p_dev->fra_crc_pry;
        /* ���Լ������У����ʹ���õ���У������бȽ�*/
        if (crc == tcrc)
            return AM_TRUE;
    }
    else
    {
        int i;
        char cks = 0;
        /* �������ۼ����*/
        for (i = 0; i < p_dev->p_rec_devinfo->frames_bytes; i++)
        {
            cks += p_dev->p_rec_devinfo->frames_info[i];
        }
        /* �����м�����ۼӺ��봫��������ۼӺͽ��бȽ�*/
        if (cks == p_dev->fra_sum_parity)
            return AM_TRUE;
    }
    return AM_FALSE;
}

/**
 * \brief ���յ�EOT��������
 */
am_static_inline
am_bool_t __xmodem_rec_eot (am_xmodem_rec_dev_t *p_dev, char inchar)
{
    /* ���ܵ������ַ����Ӧ���ͷ� */
    p_dev->p_rec_devinfo->frames_info[0] = AM_XMODEM_ACK;
    p_dev->state_rx_char = 1;
    /* ʹ���ж�*/
    am_uart_tx_startup(p_dev->uart_handle);
    p_dev->state_rx_char = 2;
    p_dev->rx_bytes = -AM_DATA_SUC;
    /* �ص�֪ͨ�û� �������*/
    p_dev->pfn_rx_callback(p_dev->p_arg,
                        p_dev->p_rec_devinfo->frames_info,
                        p_dev->rx_bytes);
    return AM_TRUE;
}
/**
 * \brief ���ݴ���״̬����
 */
am_static_inline
am_bool_t __xmodem_rec_data_err (am_xmodem_rec_dev_t *p_dev, char inchar)
{
    /* �жϴ�����Ϣ�������ͷ�����ȡ�����ͣ���ص�֪ͨ�û�*/
    if (inchar == AM_XMODEM_CAN) {
        __xmodem_rec_can(p_dev, inchar);
        return AM_TRUE;
    }
    if (inchar == AM_XMODEM_EOT) {
        __xmodem_rec_eot(p_dev, inchar);
        return AM_TRUE;
    }
    if (inchar == AM_XMODEM_ACK) {
        return AM_TRUE;
    }
    __xmodem_rx_nak_set(p_dev);
    return AM_FALSE;
}

/**
 * \brief SUMУ�麯��
 */
am_static_inline
am_bool_t __xmodem_rec_sum (am_xmodem_rec_dev_t *p_dev, char inchar)
{
    p_dev->fra_sum_parity = inchar;
    if (AM_TRUE == __xmodem_data_check(p_dev))
       {
          if (p_dev->pfn_rx_callback != NULL) {
              p_dev->pfn_rx_callback(p_dev->p_arg,
                                     p_dev->p_rec_devinfo->frames_info,
                                     p_dev->rx_bytes);
              p_dev->rx_bytes = 0;
              p_dev->frames_num++;
              return AM_TRUE;
          }
       }
    return AM_FALSE;
}
/**
 * \brief CRCУ�麯��
 */
am_static_inline
am_bool_t __xmodem__rec_crc (am_xmodem_rec_dev_t *p_dev, char inchar)
{
    uint8_t u_inchar = (uint8_t)inchar;
    if (p_dev->rx_bytes == p_dev->p_rec_devinfo->frames_bytes) {
        p_dev->fra_crc_pry |= (u_inchar << 8);
        p_dev->rx_bytes ++;
        return AM_TRUE;
    }
    if (p_dev->rx_bytes == p_dev->p_rec_devinfo->frames_bytes + 1) {
        p_dev->fra_crc_pry |= u_inchar;
        if (AM_TRUE == __xmodem_data_check(p_dev)) {
            p_dev->pfn_rx_callback(p_dev->p_arg,
                                p_dev->p_rec_devinfo->frames_info,
                                p_dev->p_rec_devinfo->frames_bytes);
            p_dev->rx_bytes = 0;
            p_dev->frames_num++;
            p_dev->fra_crc_pry = 0;
            return AM_TRUE;
        }
    }
    return AM_FALSE;
}

/*****************************************************************************/

/**
 * \brief ���ݶα��溯��
 */
am_static_inline
am_bool_t __xmodem_rec_data_rec (am_xmodem_rec_dev_t *p_dev, char inchar)
{
    if (p_dev->rx_bytes < p_dev->p_rec_devinfo->frames_bytes) {
        p_dev->p_rec_devinfo->frames_info[p_dev->rx_bytes] = inchar;
        p_dev->rx_bytes++;
        return AM_TRUE;
    }
    if (AM_XMODEM_SUM_MODE == p_dev->p_rec_devinfo->parity_mode) {
        __xmodem_rec_sum(p_dev, inchar);
        return AM_TRUE;
    }
    if (AM_XMODEM_CRC_MODE == p_dev->p_rec_devinfo->parity_mode) {
    	__xmodem__rec_crc(p_dev, inchar);
        return AM_TRUE;
    }
    return AM_FALSE;
}
/**
 * \brief ���кŷ����ж�
 */
am_static_inline
am_bool_t __xmodem_rec_rad (am_xmodem_rec_dev_t *p_dev, char inchar)
{
    if (inchar == ~p_dev->frames_num) {
        /* ���ĵ���һ������״̬*/
        p_dev->state_handle = (am_xmodem_rec_pfn_handle)__xmodem_rec_data_rec;
        return AM_TRUE;
    }
    return AM_FALSE;
}

/**
 * \brief ���кŽ��ܺ���
 */
am_static_inline
am_bool_t __xmodem_rec_pack (am_xmodem_rec_dev_t *p_dev, char inchar)
{
    p_dev->frames_num = inchar;
    p_dev->state_handle = (am_xmodem_rec_pfn_handle)__xmodem_rec_rad;
    return AM_TRUE;
}

/**
 * \brief ״̬����ʼ���亯��
 */
am_bool_t am_xmodem_rec_frames (am_xmodem_rec_dev_t *p_dev, char inchar)
{
    if (inchar == AM_XMODEM_SOH || inchar == AM_XMODEM_STX) {
        p_dev->state_handle = (am_xmodem_rec_pfn_handle)__xmodem_rec_pack;
        return AM_TRUE;
    }
    return AM_FALSE;
}

/*****************************************************************************/

/**
 * \brief ȡ�����ͺ���
 */
am_err_t am_xmodem_rec_can_set (am_xmodem_rec_dev_t  *p_dev)
{
   int i = 0;
   if (p_dev == NULL) {
      return -AM_EINVAL;
   }
   /* ȡ����������������η�ֹ���ͷ�δ�ܽӵ�*/
   for (i = 0; i < 3; i++) {
      p_dev->p_rec_devinfo->frames_info[0] = AM_XMODEM_CAN;
      p_dev->state_rx_char = 1;
      /* ���������ж� */
      am_uart_tx_startup(p_dev->uart_handle);
      p_dev->state_rx_char = 2;
   }
   return AM_TRUE;
}

/**
 * \brief �������պ���
 */
am_err_t am_xmodem_rec_ack_set (am_xmodem_rec_dev_t *p_dev)
{
    if (p_dev == NULL) {
       return -AM_EINVAL;
    }
    /* ����ACKȷ���źţ� ��Ӧ���ͷ�������һ֡���� */
    p_dev->p_rec_devinfo->frames_info[0] = AM_XMODEM_ACK;
    p_dev->state_rx_char = 1;
    /* ���������ж� */
    am_uart_tx_startup(p_dev->uart_handle);
    p_dev->state_rx_char = 2;
    p_dev->state_handle = am_xmodem_rec_frames;
    /* ���������ʱ���������ʱ��*/
    am_softimer_start(&p_dev->softimer, p_dev->p_rec_devinfo->rx_timeout);
    return AM_TRUE;
}

/**
 * \brief xmodem���ջص�����ע��
 */
am_bool_t am_xmodem_rec_cb_reg (am_xmodem_rec_dev_t        *p_dev,
                               am_xmodem_user_rx_t          pfn_rec_cb,
                               void                        *p_arg)
{
    if (pfn_rec_cb == NULL) {
       return AM_EINVAL;
    }
    p_dev->pfn_rx_callback = pfn_rec_cb;
    p_dev->p_arg        = p_arg;
    return AM_OK;
}

/**
 * \brief �����ʱ���ص�����
 *
 * \param[in] p_arg ָ��XMODEM �豸�ṹ���ָ��
 *
 * \return ��
 */
static void __xmodem_rec_softimer_callback (void *p_arg)
{
    am_xmodem_rec_dev_t *p_dev = (am_xmodem_rec_dev_t *)p_arg;
    /* �ر������ʱ�� */
    am_softimer_stop(&p_dev->softimer);
    /*������־λ����ʱδ���յ�����*/
    p_dev->rx_bytes = -AM_ETIME;
    if (p_dev->pfn_rx_callback != NULL) {
        p_dev->pfn_rx_callback(p_dev->p_arg,
                               p_dev->p_rec_devinfo->frames_info,
                               p_dev->rx_bytes);
     }
}

/**
 * \brief �ַ���ȡ����
 */
static int __xmodem_rec_getchar (am_xmodem_rec_dev_t  *p_dev, char *p_data)
{
    /* ����״̬ʱ��ֻ��������ĵ�һ���ֽ��е����� */
    if (p_dev->state_rx_char == 1) {
       *p_data = p_dev->p_rec_devinfo->frames_info[0];
        p_dev->state_rx_char = 0;
        return 1;
    }
    return AM_OK;
}
/**
 * \brief Xmodem��ȡһ���ַ����ͺ���
 */
static int __xmodem_rec_txchar_get (void *p_arg, char *p_outchar)
{
    am_xmodem_rec_dev_t *p_dev = (am_xmodem_rec_dev_t *)p_arg;
    if (__xmodem_rec_getchar(p_dev, p_outchar) != 1) {
        return -AM_EEMPTY;
    }
    return AM_OK;
}

/**
 * \brief Xmodem���ݽ��պ���
 */
am_static_inline
void __xmodem_rx_char (void *p_arg, char inchar)
{
    am_xmodem_rec_dev_t *p_dev = (am_xmodem_rec_dev_t *)p_arg;
    am_softimer_stop (&p_dev->softimer);
    am_xmodem_rec_pfn_handle pfn_handle = (am_xmodem_rec_pfn_handle)p_dev->state_handle;
    if (AM_FALSE == pfn_handle(p_dev, inchar)) {
        __xmodem_rec_data_err(p_dev, inchar);
    }
}

/**
 * \brief Xmodem��ʼ���պ���
 */
am_static_inline
void  __xmodem_rx_startup (am_xmodem_rec_dev_t  *p_dev)
{
    /* ���ݹ���ģʽ��ѡ���͸����ͷ����ַ�*/
    switch (p_dev->p_rec_devinfo->parity_mode) {
       /* ����ģʽΪ����λΪ128�ֽ�ʱ�������ַ�NAK*/
       case AM_XMODEM_SUM_MODE:
                 p_dev->p_rec_devinfo->frames_info[0] = AM_XMODEM_NAK;
                 break;
       /* ����ģʽΪ����λΪ1K�ֽ�ʱ�������ַ�C*/
       case AM_XMODEM_CRC_MODE:
                 p_dev->p_rec_devinfo->frames_info[0] = 'C';
                 break;
    }
    p_dev->state_rx_char = 1;
    /* ���������ж� */
    am_uart_tx_startup(p_dev->uart_handle);
    p_dev->state_rx_char = 0;
    am_softimer_start(&p_dev->softimer, p_dev->p_rec_devinfo->rx_timeout);
}

/**
 * \brief Xmodem�ļ����պ���
 */
am_bool_t am_xmodem_rec_start (am_xmodem_rec_dev_t *p_dev)
{
    /* ��ʼ���� */
    if (p_dev != NULL) {
    	__xmodem_rx_startup(p_dev);
        return AM_TRUE;
    }
    return AM_FALSE;
}

/**
 * \brief �����豸��ʼ������
 */
am_xmodem_rec_handle_t  am_xmodem_rec_init (
                                am_xmodem_rec_dev_t        *p_dev,
                                const am_xmodem_rec_dev_info_t *p_rec_devinfo,
                                am_uart_handle_t                uart_handle)
{
    if ((NULL == p_dev) || (NULL == p_rec_devinfo) || (NULL == uart_handle)) {
        return NULL;
    }

    p_dev->p_rec_devinfo = (am_xmodem_rec_dev_info_t *)p_rec_devinfo;
    /* ��ȡ���ھ�� */
    p_dev->uart_handle = uart_handle;
    p_dev->frames_num    = 0;         /**< \brief ���кų�ʼֵΪ0*/
    p_dev->fra_sum_parity = 0;        /**< \brief ��ʼ��У��λ */
    p_dev->fra_crc_pry = 0;           /**< \brief ��ʼ��CRCУ�� */
    p_dev->pfn_rx_callback = NULL;       /**< \brief ��ʼ���ص����� */
    p_dev->state_rx_char = 0;         /**< \brief ״̬λĬ��Ϊ0 */
    p_dev->rx_bytes = 0;              /**< \brief ��ʼ����ǰ�����ֽ��� */
    p_dev->nak_state = 1;             /**< \brief ��ʼ����ǰ�ط�״̬Ϊ1 */
    p_dev->tx_bytes = 0;              /**< \brief ��ʼ�������ļ���С*/
    /** ״̬���������*/
    p_dev->state_handle = (am_xmodem_rec_pfn_handle)am_xmodem_rec_frames;
    /* ʹ�ܴ����ж�ģʽ */
    am_uart_ioctl(p_dev->uart_handle, AM_UART_MODE_SET, (void *)AM_UART_MODE_INT);
    /* ע�ᷢ�ͻص����� */
    am_uart_callback_set(p_dev->uart_handle,
                         AM_UART_CALLBACK_TXCHAR_GET,
                         __xmodem_rec_txchar_get,
                         (void *)(p_dev));
    /* ע����ջص����� */
    am_uart_callback_set(p_dev->uart_handle,
                         AM_UART_CALLBACK_RXCHAR_PUT,
                         __xmodem_rx_char,
                         (void *)(p_dev));

    if (p_rec_devinfo->rx_timeout != 0) {
        if (am_softimer_init(&p_dev->softimer,
                              __xmodem_rec_softimer_callback,
                              p_dev) != AM_OK) {
            return NULL;
        }
    }
    return (am_xmodem_rec_handle_t)(p_dev);
}

/********************************************************************************
Xmodem��������
********************************************************************************/

/**
 * \brief ��ȡ�ļ����ͽ����ַ�
 */
am_static_inline
void __xmodem_tx_eot (am_xmodem_tx_dev_t *p_dev,
                             uint8_t            *outchar)
{
    *outchar = AM_XMODEM_EOT;
}
/**
 * \brief �ļ����ͽ�������
 */
am_err_t am_xmodem_tx_over (am_xmodem_tx_dev_t *p_dev)
{
    if (NULL == p_dev) {
       return -AM_EINVAL;
    }
    /* ������ȡ�ַ����ͱ�־λ*/
    p_dev->state_rx_char = 0;
    /* ������ʱ��*/
    am_softimer_start(&p_dev->softimer, p_dev->p_tra_devinfo->tx_timeout);
    /* �л����ļ����ͽ���״̬*/
    p_dev->state_handle = __xmodem_tx_eot;
    /* ���������ж� */
    am_uart_tx_startup(p_dev->uart_handle);
    /*�ر��ַ���ȡ*/
    p_dev->state_rx_char = 2;
    p_dev->state_flag = __AM_XMODEM_DOC_EOT;
    return AM_TRUE;
}

/**
 * \brief ��ȡȡ�����ͺ����ַ�
 */
am_static_inline
void __xmodem_tx_can_get (void *p_arg, char *outchar)
{
    *outchar = AM_XMODEM_CAN;
}
/**
 * \brief �û�ȡ�����ͺ���
 */
am_err_t am_xmodem_tx_can_set (am_xmodem_tx_dev_t *p_dev)
{
    int i = 0;
    if (NULL == p_dev) {
        return -AM_EINVAL;
    }
    p_dev->state_rx_char = 0;
    /* �л�������ȡ�������ļ�״̬���������������ν���ȡ���ź�*/
    for (i = 0; i < 3; i++) {
        p_dev->state_handle = __xmodem_tx_can_get;
        am_uart_tx_startup(p_dev->uart_handle);
    }
    /* �ر��жϻ�ȡ�����ַ�*/
    p_dev->state_rx_char = 2;
    p_dev->state_flag = AM_XMODEM_EOT_ACK;
    return AM_TRUE;
}
/**
 * \brief �ַ���ȡ����
 */
am_static_inline
int __xmodem_tx_getchar (am_xmodem_tx_dev_t  *p_dev, char *p_data)
{
    if (p_dev->state_rx_char == 0) {
        am_xmodem_tra_pfn_handle pfn_handle = (am_xmodem_tra_pfn_handle)p_dev->state_handle;
        pfn_handle(p_dev, p_data);
        return 1;
    }
    return AM_OK;
}
/**
 * \brief Xmodem����ģʽ��ȡһ���ַ����ͺ���
 */
am_static_inline
int __xmodem_tx_char_get (void *p_arg, char *p_outchar)
{
    am_xmodem_tx_dev_t *p_dev = (am_xmodem_tx_dev_t *)p_arg;
    if (__xmodem_tx_getchar(p_dev, p_outchar) != 1) {
        return -AM_EEMPTY;
    }
    return AM_OK;
}
/**
 * \brief xmodem���ͻص�����ע��
 */
am_err_t am_xmodem_tx_cb_reg (am_xmodem_tx_dev_t       *p_dev,
                              am_xmodem_user_tx_t       pfn_tx_cb,
                              void                     *p_arg)
{
    if (pfn_tx_cb == NULL) {
       return AM_EINVAL;
    }
    p_dev->pfn_tx_callback = pfn_tx_cb;
    p_dev->p_arg        = p_arg;
    return AM_TRUE;
}

/**
 * \brief ����У���뺯��
 */
am_static_inline
void __xmodem_tx_frames_parity (am_xmodem_tx_dev_t *p_dev,
                                char               *outchar)
{
    int16_t crc_high = 0;
    /* ������ģʽΪ1K���ȡCRCУ����*/
    if (p_dev->frame_tx_bytes == 1024) {
        crc_high = __xmodem_check_mode_crc((char *)p_dev->p_tx_arg, p_dev->frame_tx_bytes);
        if (p_dev->fra_crc_pry == crc_high >> 8) {
            p_dev->fra_crc_pry = crc_high;
            p_dev->state_rx_char = 2;
        } else {
            p_dev->fra_crc_pry = crc_high >> 8;
        }
       *outchar = p_dev->fra_crc_pry;
    }
    /* ������ģʽΪ128���ȡSUMУ����*/
    if (p_dev->frame_tx_bytes == 128) {
        int i;
        char cks = 0;
        /* �������ۼ����*/
        for (i = 0; i < p_dev->frame_tx_bytes; i++)
        {
            cks += p_dev->p_tx_arg[i];
        }
        p_dev->fra_sum_parity = cks;
        /* ������ʱ��*/
        am_softimer_start(&p_dev->softimer, p_dev->p_tra_devinfo->tx_timeout);
       *outchar = p_dev->fra_sum_parity;
        p_dev->state_rx_char = 2;
    }

}
/**
 * \brief ���ݲ���һ֡��亯��
 */
am_static_inline
am_bool_t __xmodem_tx_ctrlz_set (am_xmodem_tx_dev_t *p_dev,
                                 char               *outchar)
{
    if (p_dev->tx_bytes < p_dev->doc_bytes) {
       *outchar = p_dev->p_tx_arg[p_dev->tx_bytes];
        p_dev->tx_bytes++;
        return AM_TRUE;
    } else {
       *outchar = AM_XMODEM_CTRLZ;
        p_dev->p_tx_arg[p_dev->tx_bytes] = AM_XMODEM_CTRLZ;
        p_dev->tx_bytes++;
        return AM_TRUE;
    }
    //return AM_FALSE;
}
/**
 * \brief �������ݶκ���
 */
am_static_inline
am_bool_t __xmodem_tx_frames_data (am_xmodem_tx_dev_t *p_dev,
                                   char               *outchar)
{
    if (p_dev->doc_bytes == p_dev->frame_tx_bytes) {
       *outchar = p_dev->p_tx_arg[p_dev->tx_bytes];
        p_dev->tx_bytes++;
        if (p_dev->tx_bytes == p_dev->frame_tx_bytes) {
            p_dev->state_handle = __xmodem_tx_frames_parity;
            p_dev->tx_bytes = 0;
        }
        return AM_TRUE;
    }
    if (p_dev->doc_bytes < p_dev->frame_tx_bytes ) {
        __xmodem_tx_ctrlz_set(p_dev, outchar);
        if (p_dev->tx_bytes == p_dev->frame_tx_bytes) {
            p_dev->state_handle = __xmodem_tx_frames_parity;
            p_dev->tx_bytes = 0;
            p_dev->state_flag = AM_XMODEM_MOU_SUC;
         }
        return AM_TRUE;
    }
    return AM_FALSE;
}

/**
 * \brief �������кŷ��뺯��
 */
am_static_inline
am_bool_t __xmodem_tx_frames_pack_rmoc (am_xmodem_tx_dev_t *p_dev,
                                        char               *outchar)
{
    /* �жϷ��ͺ�����ȡ���кŷ���*/
   *outchar = ~p_dev->frames_num;
    /* �л������ݷ���״̬*/
    p_dev->state_handle = __xmodem_tx_frames_data;
    return AM_TRUE;
}
/**
 * \brief �������кź���
 */
am_static_inline
am_bool_t  __xomdem_tx_frames_packetno (am_xmodem_tx_dev_t *p_dev,
                                        char               *outchar)
{
    /* ���кż�1*/
    p_dev->frames_num++;
    /* �жϷ��ͺ�����ȡ���к�*/
   *outchar = p_dev->frames_num;
    /* �л�����ȡ���кŷ���״̬*/
    p_dev->state_handle = __xmodem_tx_frames_pack_rmoc;
    return AM_TRUE;
}
/**
 * \brief ����֡ͷ����
 */
am_static_inline
am_bool_t __xmodem_tx_frames_head (am_xmodem_tx_dev_t *p_dev,
                                   char               *outchar)
{
    if (p_dev->frame_tx_bytes == 1024) {
       /* 1K����ģʽ��֡ͷΪSTX*/
       *outchar = AM_XMODEM_STX;
    }
    if (p_dev->frame_tx_bytes == 128) {
        /* 128����ģʽ��֡ͷΪSOH*/
       *outchar = AM_XMODEM_SOH;
    }
    /* �л������кŷ���״̬*/
    p_dev->state_handle = __xomdem_tx_frames_packetno;
    return AM_TRUE;
}
/**
 * \brief ��ʼ���ͺ���
 */
am_err_t am_xmodem_tx_pack (am_xmodem_tx_dev_t *p_dev,
                            char               *p_doc,
                            uint32_t            pack_size)
{
    if (p_dev == NULL) {
        return -AM_EINVAL;
    }
    if (p_dev->state_flag != 1) {
        p_dev->state_flag = 1;
    }
    /* ��ȡ�����ļ���ָ��*/
    p_dev->p_tx_arg     = p_doc;
    /* һ�η��͵�ģ���С*/
    p_dev->doc_bytes    = pack_size;
    /* �л���֡ͷ����״̬*/
    p_dev->state_handle = __xmodem_tx_frames_head;
    am_uart_tx_startup(p_dev->uart_handle);

    return AM_TRUE;
}

am_static_inline
void __xmodem_tx_eot_ack_char (am_xmodem_tx_dev_t *p_dev,
                               char               *outchar)
{
     *outchar = AM_XMODEM_ACK;
}
am_static_inline
void __xmodem_tx_eot_ack (am_xmodem_tx_dev_t *p_dev)
{
    p_dev->state_rx_char = 0;
    p_dev->state_handle = __xmodem_tx_eot_ack_char;
    am_uart_tx_startup(p_dev->uart_handle);
    p_dev->state_rx_char = 2;
    p_dev->state_flag = AM_XMODEM_EOT_ACK;
}

/**
 * \brief ���շ�Ҫ���ط�����
 */
am_static_inline
void __xmodem_tx_nak (am_xmodem_tx_dev_t *p_dev)
{
    if (p_dev->nake_state < p_dev->p_tra_devinfo->nak_times_max) {
        p_dev->tx_bytes = 0;
        p_dev->state_rx_char = 0;
        p_dev->frames_num = p_dev->frames_num - 1;
        p_dev->state_handle = __xmodem_tx_frames_head;
        am_uart_tx_startup(p_dev->uart_handle);
    }
    if (p_dev->nake_state == p_dev->p_tra_devinfo->nak_times_max) {
       am_xmodem_tx_can_set(p_dev);
       p_dev->nake_state = 0;
       /* ֪ͨ�û���ǰ���ݰ��ط������ﵽ�涨������ط�����, ���Ѿ�ȡ���˷���*/
       p_dev->pfn_tx_callback(p_dev->p_arg, AM_XMODEM_NAK_TIME);
    }
    p_dev->nake_state++;
}
/**
 * \brief Ӧ���ź��жϺ���
 */
am_static_inline
int __xmodem_tx_result_mode (am_xmodem_tx_dev_t *p_dev,
                             char                inchar)
{
    if (p_dev->state_flag == 0) {
        p_dev->state_flag = 1;
        if (inchar == AM_XMODEM_1k) {
            p_dev->frame_tx_bytes = 1024;
        }
        if (inchar == AM_XMODEM_NAK) {
            p_dev->frame_tx_bytes = 128;
        }
        if (p_dev->frame_tx_bytes == 128 || p_dev->frame_tx_bytes == 1024) {
           p_dev->state_rx_char = 0;
           p_dev->pfn_tx_callback(p_dev->p_arg, inchar);
           return AM_TRUE;
        }
    }
    if (p_dev->state_flag == 1) {
        if (inchar == AM_XMODEM_ACK) {
            p_dev->pfn_tx_callback(p_dev->p_arg, inchar);
            am_uart_tx_startup(p_dev->uart_handle);
        }
        if (inchar == AM_XMODEM_NAK) {
            __xmodem_tx_nak(p_dev);
        }
        if (inchar == AM_XMODEM_CAN) {
           p_dev->state_rx_char = 0;
           p_dev->pfn_tx_callback(p_dev->p_arg, inchar);
        }
        return AM_TRUE;
    }
    if (p_dev->state_flag == AM_XMODEM_MOU_SUC) {
        p_dev->pfn_tx_callback(p_dev->p_arg, AM_XMODEM_MOU_SUC);
    }
    return AM_FALSE;
}

/**
 * \brief ģ�鷢������߽��շ�׼���÷���֪ͨ�û�
 */
am_static_inline
int __xmodem_tx_user_inform (am_xmodem_tx_dev_t *p_dev,
                             char                inchar)
{
    int result;
    /*��ǰ״̬Ϊ�ļ����ͽ���ʱ�����յ���Ӧ���źŽ��Զ��ظ�*/
    if (p_dev->state_flag == __AM_XMODEM_DOC_EOT) {
        __xmodem_tx_eot_ack(p_dev);
        return AM_TRUE;
    }
    if (p_dev->tx_bytes == 0 ) {
       p_dev->state_rx_char = 0;
       result = __xmodem_tx_result_mode(p_dev, inchar);
    }
    return result;
}
/**
 * \brief �жϽ����ַ�����
 */
am_static_inline
void __xmodem_tx_char (void *p_arg, uint8_t inchar)
{
    am_xmodem_tx_dev_t *p_dev = (am_xmodem_tx_dev_t *)p_arg;
    am_softimer_stop(&p_dev->softimer);
    if (AM_FALSE == __xmodem_tx_user_inform(p_dev, inchar)) {
        return;
    }
}

/**
 * \brief Xmodem���ͳ�ʱ����
 */
am_static_inline
void __xmodem_tx_time_callback(void *p_arg)
{
    am_xmodem_tx_dev_t *p_dev = (am_xmodem_tx_dev_t *)p_arg;
    /* �ر������ʱ�� */
    am_softimer_stop(&p_dev->softimer);
    if (p_dev->pfn_tx_callback != NULL) {
        am_xmodem_tx_can_set(p_dev);
        p_dev->pfn_tx_callback(p_dev->p_arg, -AM_ETIME);
     }
}
/**
 * \brief �����豸��ʼ������
 */
am_xmodem_tx_handle_t  am_xmodem_tx_init (
                           am_xmodem_tx_dev_t            *p_dev,
                           const am_xmodem_tx_dev_info_t *p_tx_devinfo,
                           am_uart_handle_t               uart_handle)
{
    if ((NULL == p_dev) || (NULL == p_tx_devinfo) || (NULL == uart_handle)) {
        return NULL;
    }

    p_dev->p_tra_devinfo = (am_xmodem_tx_dev_info_t *)p_tx_devinfo;
    /* ��ȡ���ھ�� */
    p_dev->uart_handle = uart_handle;

    p_dev->tx_bytes = 0;              /**< \brief ��ʼ�������ļ���С*/
    p_dev->p_tx_arg = NULL;           /**< \brief ��ʼ���ļ�ָ�� */
    p_dev->state_rx_char = 0;         /**< \brief Ĭ�Ͻ����жϲ�Ϊ�� */
    p_dev->state_flag = 0;            /**< \brief Ĭ�ϵ�һ�����յ����ַ�Ϊģʽ�ж��ַ� */
    p_dev->nake_state = 0;            /**< \brief ��ʼ���ط�״̬Ϊ0*/
    p_dev->state_handle = am_xmodem_tx_pack; /**< \brief ��ʼ��״̬��� */
    /* ʹ�ܴ����ж�ģʽ */
    am_uart_ioctl(p_dev->uart_handle, AM_UART_MODE_SET, (void *)AM_UART_MODE_INT);
    am_uart_callback_set(p_dev->uart_handle,
                         AM_UART_CALLBACK_RXCHAR_PUT,
                         __xmodem_tx_char,
                         (void *)(p_dev));

    /* ע�ᷢ�ͻص����� */
    am_uart_callback_set(p_dev->uart_handle,
                         AM_UART_CALLBACK_TXCHAR_GET,
                         __xmodem_tx_char_get,
                         (void *)(p_dev));

    if (p_tx_devinfo->tx_timeout != 0) {
        if (am_softimer_init(&p_dev->softimer,
                              __xmodem_tx_time_callback,
                              p_dev) != AM_OK) {
            return NULL;
        }
    }
    return (am_xmodem_tx_handle_t)(p_dev);
}

