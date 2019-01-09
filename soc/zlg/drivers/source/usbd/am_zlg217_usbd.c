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
 * \brief USBD ������ʵ�ֺ���
 *
 * \internal
 * \par Modification history
 * - 1.00 17-09-29  sdq, first implementation.
 * \endinternal
 */

#include "am_zlg217_usbd.h"
#include "amhw_zlg217_usbd.h"
#include "am_usb.h"
#include "am_int.h"
#include "am_usb_spec.h"
#include "am_usbd_ch9.h"

/* ��Ҫ����USB��ʱ��ȥ��ע�ͣ������ڴ����д�ӡ������USB�жϺ��������� */
//#define USB_DEBUG

/**
 * \brief ͨ��wValue��ֵѰ��ָ����������
 *
 * \retval �ɹ�����������ָ�룬ʧ�ܷ���NULL
 *
 * \note �������������¼�����������ͨ���˷����
 */
static uint8_t * __find_desc_by_wValue1 (const am_zlg227_usbd_dev_t *p_dev,
                                         uint16_t                 w_value)
{
    int i;

    for (i = 0; i < p_dev->p_info->p_devinfo->descriptor_num; i++) {
        if (w_value == p_dev->p_info->p_devinfo->p_descriptor[i].desc_wValue) {
            return p_dev->p_info->p_devinfo->p_descriptor[i].p_desc;
        }
    }
    return NULL;
}

/**
 * \brief ��ʼ��ָ���Ķ˵�
 *
 * \param[in] p_dev  : USB�豸
 * \param[in] epInit : �˵��ʼ����Ϣ
 *
 * \note �ú�������������ָ��һ��Ӳ���еĶ˵㣬��ʼ���Ķ˵����������������ָ�����ģ����ҵ�ַ�ʹ��䷽��
 *       ����һ�£�Ҳ����˵��p_dev->device.endpoint_info[i].endpoint_address��
 *       epInit->endpoint_address��ֵ������ȣ����ܽ��г�ʼ����
 */
static am_usb_status_t __usb_device_endpoint_init (am_zlg227_usbd_dev_t       *p_dev,
                                                  am_usbd_endpoint_init_t *epinit)
{
    uint16_t max_packet_size = epinit->max_packet_size;
    uint8_t  endpoint = (epinit->endpoint_address & AM_USBD_ENDPOINT_NUMBER_MASK);
    int i;

    for (i = 0; i < AM_USBD_MAX_EP_CNT; i++) {
        /* �˵��������б���ָ��������˵� */
        if (p_dev->device.endpoint_info[i].inuse == AM_TRUE &&
            p_dev->device.endpoint_info[i].ep_address == epinit->endpoint_address) {
            break;
        }
    }
    if (i == AM_USBD_MAX_EP_CNT)
        return AM_USB_STATUS_INVALID_REQUEST;

    if (endpoint >= AM_USBD_MAX_EP_CNT)
        return AM_USB_STATUS_INVALID_PARAMETER;
    if (epinit->transfer_type > AM_USB_ENDPOINT_INTERRUPT)
        return AM_USB_STATUS_INVALID_PARAMETER;

    /* ���ö˵��ܴ��������ֽڸ��� */
    if (max_packet_size > AM_USBD_MAX_EP_DATA_CNT)
        max_packet_size = AM_USBD_MAX_EP_DATA_CNT;

    amhw_zlg217_usbd_ep_halt_reset(1 << endpoint);     /* ����˵���ͣ */
    amhw_zlg217_usbd_ep_enable(1 << endpoint);         /* ʹ�ܶ˵� */

    p_dev->device.endpoint_info[endpoint].stalled = 0;
    p_dev->device.endpoint_info[endpoint].max_packet_size = max_packet_size;
    p_dev->device.endpoint_info[endpoint].transfer_type = epinit->transfer_type;

    return AM_USB_STATUS_SUCCESS;
}

/**
 * \brief ��ĳ���˵�ȥ��ʼ��
 *
 *  \param[in] p_dev       : USB�豸
 *  \param[in] endpoint    : ָ���Ķ˵�
 */
static am_usb_status_t __usb_device_endpoint_deinit (am_zlg227_usbd_dev_t *p_dev,
                                                    uint8_t            endpoint)
{
    endpoint &= AM_USBD_ENDPOINT_NUMBER_MASK;

    amhw_zlg217_usbd_ep_disable(1 << endpoint);  /* ���ܶ˵� */

    return AM_USB_STATUS_SUCCESS;
}

/**
 * \brief ���ö˵�����
 *
 * \param[in] p_dev         : USB�豸
 * \param[in] endpoint_addr : �˵��ַ
 */
static am_usb_status_t __usb_device_endpoint_stall (am_zlg227_usbd_dev_t *p_dev,
                                                   uint8_t            endpoint_addr)
{
    uint8_t endpoint = endpoint_addr & AM_USBD_ENDPOINT_NUMBER_MASK;

    p_dev->device.endpoint_info[endpoint].stalled = 1;    /* ����������־ */

    amhw_zlg217_usbd_ep_halt_set(1 << endpoint);

    return AM_USB_STATUS_SUCCESS;
}

/**
 * \brief ���ö˵㲻����
 *
 * \param[in] p_dev         : USB�豸
 * \param[in] endpoint_addr : �˵��ַ  D7�� 1��USB_IN  0��USB_OUT
 */
static am_usb_status_t __usb_device_endpoint_unstall (am_zlg227_usbd_dev_t *p_dev,
                                                      uint8_t            endpoint_addr)
{
    uint8_t endpoint = endpoint_addr & AM_USBD_ENDPOINT_NUMBER_MASK;

    p_dev->device.endpoint_info[endpoint].stalled = 0;

    amhw_zlg217_usbd_dtog_data0(endpoint);

    amhw_zlg217_usbd_ep_halt_reset(1 << endpoint);

    return AM_USB_STATUS_SUCCESS;
}

/**
 * \brief ��ʼ��USB�豸
 */
static am_usb_status_t __usb_device_init (am_usbd_ctrl_handle_t handle)
{
    int i = 0;
    am_usbd_endpoint_init_t endpoint;
    am_zlg227_usbd_dev_t *p_dev = (am_zlg227_usbd_dev_t *)handle;

    /* ��λUSB�������Ķ˵��FIFO */
    amhw_zlg217_usbd_connect_set(ZLG217_USB_DISCONNECT);
    amhw_zlg217_usbd_reset_set(ZLG217_USB_RESET);
    amhw_zlg217_usbd_reset_set(ZLG217_USB_NORESET);
    amhw_zlg217_usbd_connect_set(ZLG217_USB_DISCONNECT);

    /* ���ж�״̬ */
    amhw_zlg217_usbd_int_state_clear(AMHW_ZLG217_USB_INT_STATE_ALL);
    p_dev->int_stat = 0;

    /* ��˵��ж�״̬ */
    amhw_zlg217_usbd_ep_int_state_clear(AMHW_ZLG217_EP_INT_STATE_EP_ALL);
    p_dev->int_ep_union.int_ep_flag = 0;

    amhw_zlg217_usbd_ep0_int_state_clear(AMHW_ZLG217_EP0_INT_STATE_ALL);
    p_dev->ep_int_type_union.ep_int_type[0] = 0;

    for (i = 0; i < AM_USBD_MAX_EP_CNT - 1; i++) {
        amhw_zlg217_usbd_epx_int_state_clear((ZLG217_USB_epx_t)i, AMHW_ZLG217_EPX_INT_STATE_ALL);
        p_dev->ep_int_type_union.ep_int_type[i + 1] = 0;
    }

    /* ʹ��USB�жϣ�δʹ��SOF����жϣ� */
    /* ���ʹ����SOF�жϵĻ���ÿ��1ms�ͻ����һ���жϣ������ʹ��SOF�жϣ��жϱ�־Ҳ����λ�����ǲ���
                 �����ж�     */
    amhw_zlg217_usbd_int_enable(AMHW_ZLG217_USB_INT_EN_RSTIE |
                                AMHW_ZLG217_USB_INT_EN_SUSPENDIE |
                                AMHW_ZLG217_USB_INT_EN_RESUMIE |
                                AMHW_ZLG217_USB_INT_EN_EPIE);

    /* ʹ�ܶ˵��ж� */
    for (i = 0; i < AM_USBD_MAX_EP_CNT; i++) {
        if (p_dev->device.endpoint_info[i].inuse == AM_TRUE)
            amhw_zlg217_usbd_ep_int_enable(1 << i);
    }

    /* ʹ�ܶ˵�0�������ж� */
    amhw_zlg217_usbd_ep0_int_enable(AMHW_ZLG217_EP0_INT_EN_ALL);
    /* ʹ�������˵�������ж� */
    for (i = 0; i < AM_USBD_MAX_EP_CNT - 1; i++) {
        if (p_dev->device.endpoint_info[i + 1].inuse == AM_TRUE)
            amhw_zlg217_usbd_epx_int_enable((ZLG217_USB_epx_t)i, AMHW_ZLG217_EPX_INT_EN_ALL);
    }

    /* ���SETUP���� */
    p_dev->device.setup_data.bm_request_type = 0;
    p_dev->device.setup_data.b_request = 0;
    p_dev->device.setup_data.w_value = 0;
    p_dev->device.setup_data.w_index = 0;
    p_dev->device.setup_data.w_length = 0;


    /**< \brief ��ʼ���˵� */
    for (i = 0; i < AM_USBD_MAX_EP_CNT; i++) {
        if (p_dev->device.endpoint_info[i].inuse == AM_TRUE) {
            endpoint.endpoint_address = p_dev->device.endpoint_info[i].ep_address;
            endpoint.max_packet_size = p_dev->device.endpoint_info[i].max_packet_size;
            endpoint.transfer_type = p_dev->device.endpoint_info[i].transfer_type;
            __usb_device_endpoint_init(p_dev, &endpoint);
        }
    }


    /* ���õ�ַΪ0 */
    amhw_zlg217_usbd_addr_set(0);
    p_dev->device.device_address = 0;

    amhw_zlg217_usbd_active_set(ZLG217_USB_ACTIVE);      /* ��Ծ */
    amhw_zlg217_usbd_speed_set(ZLG217_USB_SPEED_FULL);   /* ȫ�� */
    amhw_zlg217_usbd_connect_set(ZLG217_USB_CONNECT);    /* ���� */

    return AM_USB_STATUS_SUCCESS;
}

/**
 * \bruef USBȥ��ʼ��
 *
 * \param[in] USB�������
 *
 * \retval USB������
 */
static am_usb_status_t __usb_device_deinit (am_usbd_ctrl_handle_t handle)
{
    am_zlg227_usbd_dev_t *p_dev = (am_zlg227_usbd_dev_t *)handle;

    if (!handle)
        return AM_USB_STATUS_INVALID_HANDLE;

    amhw_zlg217_usbd_int_disable(AMHW_ZLG217_USB_INT_EN_ALL); /* ����USB�ж� */
    amhw_zlg217_usbd_ep_int_disable(AMHW_ZLG217_EP_INT_EN_ALL);   /* ���ܶ˵��ж� */
    amhw_zlg217_usbd_ep0_int_disable(AMHW_ZLG217_EP0_INT_EN_ALL); /* ���ܶ˵�0���ж� */
    amhw_zlg217_usbd_ep_disable(AMHW_ZLG217_USB_EP_ALL);      /* �������ж˵� */

    /* ���ܶ˵���ж� */
    amhw_zlg217_usbd_epx_int_disable(ZLG217_USB_EP1,
                                  AMHW_ZLG217_EPX_INT_EN_ALL);
    amhw_zlg217_usbd_epx_int_disable(ZLG217_USB_EP2,
                                  AMHW_ZLG217_EPX_INT_EN_ALL);
    amhw_zlg217_usbd_epx_int_disable(ZLG217_USB_EP3,
                                  AMHW_ZLG217_EPX_INT_EN_ALL);
    amhw_zlg217_usbd_epx_int_disable(ZLG217_USB_EP4,
                                  AMHW_ZLG217_EPX_INT_EN_ALL);

    if (p_dev->p_info->pfn_plfm_deinit)
        p_dev->p_info->pfn_plfm_deinit();

    return AM_USB_STATUS_SUCCESS;
}

/**
 * \brief USB�豸��������
 *
 * \param[in] handle           : USB���ƾ��
 * \param[in] endpoint_address : ָ���Ķ˵��ַ
 * \param[in] buffer           : ָ��Ҫ���͵�����
 * \param[in] length           : Ҫ���͵����ݳ���
 *
 * \note ��������lengthΪ0�����Ϳհ�����������bufferΪNULL����length��Ϊ0������length
 *       ��0
 *
 * \retval USB������
 */
static am_usb_status_t __usb_device_send (am_usbd_ctrl_handle_t handle,
                                          uint8_t                   endpoint_address,
                                          uint8_t                  *buffer,
                                          uint32_t                  length)
{
    //am_zlg_usbd_dev_t *p_dev = (am_zlg_usbd_dev_t *)handle;
    uint32_t send_once_size = 0;
    uint8_t endpoint = endpoint_address & AM_USBD_ENDPOINT_NUMBER_MASK;
    int i = 0;

    if (endpoint >= AM_USBD_MAX_EP_CNT)
        return AM_USB_STATUS_INVALID_PARAMETER;

    if (length == 0) {  /* ���Ϳհ� */
        while (!amhw_zlg217_usbd_transfer_end((ZLG217_USB_epx2_t)endpoint));
        amhw_zlg217_usbd_epx_transfer((ZLG217_USB_epx2_t)endpoint, 0);
    } else {
        while (length) {        /* ���һ�β�����ȫ������ֶ�δ��� */
            if (length > AM_USBD_MAX_EP_DATA_CNT) {
                send_once_size = AM_USBD_MAX_EP_DATA_CNT;
            } else {
                send_once_size = length;
            }
            length -= send_once_size;
            while (!amhw_zlg217_usbd_transfer_end((ZLG217_USB_epx2_t)endpoint));
            if (buffer) {
                for (i = 0; i < send_once_size; i++) {
                    amhw_zlg217_usbd_epx_fifo_write((ZLG217_USB_epx2_t)endpoint, *buffer++);
                }
            } else {
                for (i = 0; i < send_once_size; i++) {
                    amhw_zlg217_usbd_epx_fifo_write((ZLG217_USB_epx2_t)endpoint, 0);
                }
            }
            amhw_zlg217_usbd_epx_transfer((ZLG217_USB_epx2_t)endpoint, send_once_size);
        }
    }

    return AM_USB_STATUS_SUCCESS;
}

/**
 * \brief �˵�0���Ϳհ�
 */
static void __ep0_send_empty_packet (am_usbd_ctrl_handle_t handle)
{
    am_zlg227_usbd_dev_t *p_dev = (am_zlg227_usbd_dev_t *)handle;
    switch (p_dev->device.running_ctrl_state) {
    case AM_USBD_CTRL_SETUP:
        p_dev->device.running_ctrl_state = AM_USBD_CTRL_IN;
        break;

    case AM_USBD_CTRL_IN:
        while (!amhw_zlg217_usbd_transfer_end(ZLG217_USB_EPX0));
        amhw_zlg217_usbd_epx_transfer(ZLG217_USB_EPX0, 0);
        p_dev->device.running_ctrl_state = AM_USBD_CTRL_IDLE;
        break;

    default:
        p_dev->device.running_ctrl_state = AM_USBD_CTRL_IDLE;
    }
}

/**
 * \brief USB�豸��������
 *
 * \param[in] handle           : USB���ƾ��
 * \param[in] endpoint_address : ָ���Ķ˵��ַ
 * \param[in] buffer           : ָ��Ҫ���͵�����
 * \param[in] length           : Ҫ���յ����ݳ���
 *
 * \retval USB������
 *
 * \note �ú�����fifo��ȡ������.
 *       ���fifo��û�����ݣ��򷵻�AM_USB_STATUS_ERROR.
 *       ���fifo�������ݵ������ݸ�������ָ�������ݳ��ȣ��򷵻�AM_USB_STATUS_ALLOC_FAIL����仺������
 *       ���fifo�������ݲ������ݸ�������ָ�������ݳ��ȣ��򷵻�AM_USB_STATUS_SUCCESS����仺������
 *       ���fifo�������ݲ������ݸ�������ָ�������ݳ��ȣ��򷵻�AM_USB_STATUS_SUCCESS�����ָ�������ݳ��ȡ�
 */
static am_usb_status_t __usb_device_recv (am_usbd_ctrl_handle_t handle,
                                          uint8_t                   endpoint_address,
                                          uint8_t                  *buffer,
                                          uint32_t                  length)
{
	am_zlg227_usbd_dev_t *p_dev = (am_zlg227_usbd_dev_t *)handle;

    am_usb_status_t error = AM_USB_STATUS_ERROR;
    uint8_t endpoint = endpoint_address & AM_USBD_ENDPOINT_NUMBER_MASK;
    uint8_t avali_data_cnt = 0;     /* fifo����Ч���ݸ��� */

    /* �������ݵĶ˵㷽�������OUT_OUT */
    if ((endpoint_address & AM_USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) !=
            AM_USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_OUT)
        return AM_USB_STATUS_INVALID_REQUEST;

    if (endpoint >= AM_USBD_MAX_EP_CNT)
        return AM_USB_STATUS_INVALID_PARAMETER;

    avali_data_cnt = amhw_zlg217_usbd_epx_avail_size_get((ZLG217_USB_epx2_t)endpoint);
    p_dev->device.endpoint_info[endpoint].val_length = avali_data_cnt;

    if (avali_data_cnt == 0)
        return AM_USB_STATUS_ERROR;
    else if (avali_data_cnt < length)
        error = AM_USB_STATUS_ALLOC_FAIL;
    else if (avali_data_cnt == length)
        error = AM_USB_STATUS_SUCCESS;
    else if (avali_data_cnt > length) {
        error = AM_USB_STATUS_SUCCESS;
        avali_data_cnt = length;
    }

    while (avali_data_cnt--) {
        *buffer++ = amhw_zlg217_usbd_epx_fifo_read((ZLG217_USB_epx2_t)endpoint);
    }

    return error;
}

/**
 * \brief ��ֹĳ���˵����ڽ��еĴ���
 *
 * \param[in] handle        : �豸���
 * \param[in] endpoint_addr : �˵��ַ
 *
 * \retval USB������
 */
static am_usb_status_t __usb_device_cancel (am_usbd_ctrl_handle_t handle,
                                            uint8_t                   endpoint_addr)
{

    return AM_USB_STATUS_ERROR;
}

/**
 * \brief �����豸ΪĬ��״̬
 *
 * \param[in] p_dev:�豸ʵ�������������
 *
 * \note Ĭ��״̬Ϊʹ�������жϲ�ʹ�����ж˵㣬���豸��ַ��0
 */
static void __usb_device_setdefault_state(am_zlg227_usbd_dev_t *p_dev)
{
    /* �����ַ */
    amhw_zlg217_usbd_addr_set(0);

    amhw_zlg217_usbd_ep_enable(AMHW_ZLG217_USB_EP_ALL);  /* ʹ�ܶ˵� */

    /* ���ж�״̬ */
    amhw_zlg217_usbd_int_state_clear(AMHW_ZLG217_USB_INT_STATE_ALL);
    amhw_zlg217_usbd_ep0_int_state_clear(AMHW_ZLG217_EP0_INT_STATE_ALL);
    amhw_zlg217_usbd_epx_int_state_clear(ZLG217_USB_EP1,
                                      AMHW_ZLG217_EPX_INT_STATE_ALL);
    amhw_zlg217_usbd_epx_int_state_clear(ZLG217_USB_EP2,
                                      AMHW_ZLG217_EPX_INT_STATE_ALL);
    amhw_zlg217_usbd_epx_int_state_clear(ZLG217_USB_EP3,
                                      AMHW_ZLG217_EPX_INT_STATE_ALL);
    amhw_zlg217_usbd_epx_int_state_clear(ZLG217_USB_EP4,
                                      AMHW_ZLG217_EPX_INT_STATE_ALL);

    /* ʹ��USB�ж� */
    amhw_zlg217_usbd_int_enable(AMHW_ZLG217_USB_INT_EN_ALL);

    /* ʹ�����ж˵��ж� */
    amhw_zlg217_usbd_ep_int_enable(AMHW_ZLG217_EP_INT_EN_ALL);

    /* ʹ�ܶ˵�0�������ж� */
    amhw_zlg217_usbd_ep0_int_enable(AMHW_ZLG217_EP0_INT_EN_ALL);

    /* ʹ�������˵�������ж� */
    amhw_zlg217_usbd_epx_int_enable(ZLG217_USB_EP1,
                                 AMHW_ZLG217_EPX_INT_EN_ALL);
    amhw_zlg217_usbd_epx_int_enable(ZLG217_USB_EP2,
                                 AMHW_ZLG217_EPX_INT_EN_ALL);
    amhw_zlg217_usbd_epx_int_enable(ZLG217_USB_EP3,
                                 AMHW_ZLG217_EPX_INT_EN_ALL);
    amhw_zlg217_usbd_epx_int_enable(ZLG217_USB_EP4,
                                 AMHW_ZLG217_EPX_INT_EN_ALL);
}


/**
 * \brief USB���ƺ�������������USBΪָ����״̬
 *
 * \param[in] handle : USB�������
 * \param[in] type   : ��������
 * \param[in] param  : ���Ʋ���
 *
 * \retval USB������
 */
static am_usb_status_t __usb_device_control(am_usbd_ctrl_handle_t handle,
                                            am_usbd_control_type_t    type,
                                            void                     *param)
{
    am_zlg227_usbd_dev_t *p_dev = (am_zlg227_usbd_dev_t *)handle;
    am_usb_status_t error = AM_USB_STATUS_ERROR;
    uint8_t *temp8;
    uint8_t temp;
    am_usbd_ep_status_t *endpointStatus = NULL;
    uint8_t endpoint = 0;

    if (!handle)
        return AM_USB_STATUS_INVALID_HANDLE;

    switch ((int)type) {
        case AM_USBD_CONTROL_RUN:
            amhw_zlg217_usbd_connect_set(ZLG217_USB_CONNECT);   /* ���� */
            error = AM_USB_STATUS_SUCCESS;
            break;

        case AM_USBD_CONTROL_STOP:
            amhw_zlg217_usbd_connect_set(ZLG217_USB_DISCONNECT); /* �Ͽ����� */
            error = AM_USB_STATUS_SUCCESS;
            break;

        case AM_USBD_CONTROL_ENDPOINT_INIT:     /* ��ĳ���˵��ʼ�� */
            if (param) {
                error = __usb_device_endpoint_init(p_dev, (am_usbd_endpoint_init_t *)param);
            }
            break;

        case AM_USBD_CONTROL_ENDPOINT_DEINIT:   /* ��ĳ���˵�ȥ��ʼ�� */
            if (param) {
                temp8 = (uint8_t *)param;
                error = __usb_device_endpoint_deinit(p_dev, *temp8);
            }
            break;

        case AM_USBD_CONTROL_ENDPOINT_STALL:    /* ���ƶ˵����� */
            if (param) {
                temp8 = (uint8_t *)param;
                error = __usb_device_endpoint_stall(p_dev, *temp8);
            }
            break;

        case AM_USBD_CONTROL_ENDPOINT_UNSTALL:  /* ���ƶ˵㲻���� */
            if (param) {
                temp8 = (uint8_t *)param;
                error = __usb_device_endpoint_unstall(p_dev, *temp8);
            }
            break;

        case AM_USBD_CONTROL_GET_DEVICE_STATUS: /* ��ȡ�豸״̬ */

            break;

        case AM_USBD_CONTROL_GET_ENDPOINT_STATUS:   /* ͨ���˵��ַ��ȡ�˵�״̬ */
            if (param) {
                endpointStatus = (am_usbd_ep_status_t *)param;

                endpoint = (endpointStatus->endpoint_address) & AM_USBD_ENDPOINT_NUMBER_MASK;

                if (endpoint < AM_USBD_MAX_EP_CNT) {
                    endpointStatus->endpoint_status = p_dev->device.endpoint_info[endpoint].stalled;
                    error = AM_USB_STATUS_SUCCESS;
                } else {
                    error = AM_USB_STATUS_INVALID_PARAMETER;
                }
            }
            break;

        case AM_USBD_CONTROL_SET_DEVICE_ADDRESS:    /* ����USB�豸��ַ */
            if (param) {
                temp = (uint32_t)param;
                amhw_zlg217_usbd_addr_set(temp);
                error = AM_USB_STATUS_SUCCESS;
            }
            break;

        case AM_USBD_CONTROL_GET_SYNCH_FRAME:
            break;

        case AM_USBD_CONTROL_RESUME:
            amhw_zlg217_usbd_wakeup();
            break;

            /* ����Ĭ��״̬ */
        case AM_USBD_CONTROL_SET_DEFAULT_STATUS:
            __usb_device_setdefault_state(p_dev);
            error = AM_USB_STATUS_SUCCESS;
            break;

        case AM_USBD_CONTROL_GET_SPEED:
            if (param) {
                temp8 = (uint8_t *)param;
                *temp8 = AM_USB_SPEED_FULL;
                error = AM_USB_STATUS_SUCCESS;
            }
            break;

        case AM_USBD_CONTROL_GET_OTG_STATUS:
            break;
        case AM_USBD_CONTROL_SET_OTG_STATUS:
            break;
        case AM_USBD_CONTROL_SET_TEST_MODE:
            break;
        default:
            break;
    }

    return error;
}

static const am_usbd_interface_t __g_usb_device_interface = {
    __usb_device_init,
    __usb_device_deinit,
    __usb_device_send,
    __usb_device_recv,
    __usb_device_cancel,
    __usb_device_control
};

/**
 * \brief ������
 */
static void __ctrl_deal_handle (am_zlg227_usbd_dev_t *p_dev)
{
    if((p_dev->device.setup_data.bm_request_type & AM_USB_REQUEST_TYPE_TYPE_MASK) ==    // ��׼�豸����
            AM_USB_REQUEST_TYPE_TYPE_STANDARD) {
        if(p_dev->device.setup_data.b_request <= AM_USB_REQUEST_STANDARD_SYNCH_FRAME) {
#ifdef USB_DEBUG
            am_kprintf("������ std %d ", p_dev->device.setup_data.b_request);
            am_kprintf("w_value : %x \r\n", p_dev->device.setup_data.w_value);
            am_kprintf("leng : %d \r\n", p_dev->device.setup_data.w_length);
#endif
            //(*__StandardDeviceRequest[p_dev->device.setup_data.b_request])(p_dev);
            (p_dev->device.pfn_std_request[p_dev->device.setup_data.b_request])(&(p_dev->device));
        }

    // �豸������ ���� ���Ϳհ�
    } else if ((p_dev->device.setup_data.bm_request_type & AM_USB_REQUEST_TYPE_DIR_MASK)
             == AM_USB_REQUEST_TYPE_DIR_IN) {
    	__ep0_send_empty_packet(p_dev);

        // ���豸����
    } else if ((p_dev->device.setup_data.bm_request_type & AM_USB_REQUEST_TYPE_TYPE_MASK)
             == AM_USB_REQUEST_TYPE_TYPE_CLASS) {

        if(p_dev->device.class_req.pfn_class != NULL) {
            (p_dev->device.class_req.pfn_class)(p_dev->device.class_req.p_arg, p_dev->device.setup_data.b_request);
        }
            //am_usbd_class_request(&(p_dev->device), p_dev->device.usbd_type, p_dev->device.setup_data.b_request);

    } else if ((p_dev->device.setup_data.bm_request_type & AM_USB_REQUEST_TYPE_TYPE_MASK)
            == AM_USB_REQUEST_TYPE_TYPE_VENDOR) {
        if(p_dev->device.vendor_req.pfn_vendor != NULL) {
        	(p_dev->device.vendor_req.pfn_vendor)(p_dev->device.vendor_req.p_arg);
        }
    }
}

/**
 * \brief ����IN����
 */
static void __usb_in_handle(am_zlg227_usbd_dev_t *p_dev)
{
    if ((p_dev->device.setup_data.bm_request_type & AM_USB_REQUEST_TYPE_DIR_MASK) ==
                                                 AM_USB_REQUEST_TYPE_DIR_IN) {
        __ctrl_deal_handle(p_dev);
    } else {
        __ep0_send_empty_packet(p_dev);
    }
}


/**
 * \brief setup������
 */
static void __usb_setup_handle (am_zlg227_usbd_dev_t *p_dev)
{
    if(p_dev->device.running_ctrl_state == AM_USBD_CTRL_IDLE) {  // ���״̬Ϊ����̬
        amhw_zlg217_usbd_setupdata_get((uint8_t *)&p_dev->device.setup_data); // �������ݰ�
        p_dev->device.running_ctrl_state = AM_USBD_CTRL_SETUP;   // ����״̬
    }

    /* �ж��������͵����� */
    switch ((p_dev->device.setup_data.bm_request_type >> 5) & 0x03) {

        /* ��׼�������� */
        case 0:
#ifdef USB_DEBUG
        am_kprintf("��׼����/������� %02x\r\n", p_dev->device.setup_data.b_request);
#endif
            // �Ϸ������ж�
            if (p_dev->device.setup_data.b_request <= AM_USB_REQUEST_STANDARD_SYNCH_FRAME) {
            	(p_dev->device.pfn_std_request[p_dev->device.setup_data.b_request])(&(p_dev->device));
            }
            break;

        /* ������ */
        case 1:
        	if(p_dev->device.class_req.pfn_class != NULL) {
        	    (p_dev->device.class_req.pfn_class)(p_dev->device.class_req.p_arg, p_dev->device.setup_data.b_request);
        	}
        	//am_usbd_class_request(&(p_dev->device), p_dev->device.usbd_type, p_dev->device.setup_data.b_request);
        	break;

        /* �������� */
        case 2:
            if(p_dev->device.vendor_req.pfn_vendor != NULL) {
            	(p_dev->device.vendor_req.pfn_vendor)(p_dev->device.vendor_req.p_arg);
            }
            break;

        default:
#ifdef USB_DEBUG
        am_kprintf("δ�������������\r\n");
#endif

            break;
    }
}

/**
 * \brief �˵��ж�
 */
static void __usb_device_interrupt_endpoint (am_zlg227_usbd_dev_t *p_dev)
{
    int int_ep = 0;         /* �����жϵĶ˵� */
    int ep_int_type = 0;    /* ����Ķ˵��ж����� */
    int i = 0;

    int_ep = amhw_zlg217_usbd_ep_int_state_get();   /* ��÷����жϵĶ˵� */
    amhw_zlg217_usbd_ep_int_state_clear(int_ep);    /* ��˵��ж� */
    p_dev->int_ep_union.int_ep_flag = int_ep;

    /* �˵�0�ж� */
    if (p_dev->int_ep_union.int_ep_flag_field.ep0) {
#ifdef USB_DEBUG
        am_kprintf("0 ");
#endif

        /* ��ö˵�0�����ľ����ж����� */
        ep_int_type = amhw_zlg217_usbd_ep0_int_state_get();
        amhw_zlg217_usbd_ep0_int_state_clear(ep_int_type);
        p_dev->ep_int_type_union.ep_int_type[0] = ep_int_type;

        /* ����˵�0�ж� */
        if (p_dev->ep_int_type_union.ep_int_type_field[0].setup) {
#ifdef USB_DEBUG
            am_kprintf("setup\n");
#endif
            __usb_setup_handle(p_dev);
        }
        /* ������Ӧ����ô��֮ǰ���͹��������ٷ���һ�� */
        if (p_dev->ep_int_type_union.ep_int_type_field[0].in_nack) {
#ifdef USB_DEBUG
            am_kprintf("in_nack �� ��Ӧ������\r\n");
#endif
            __usb_in_handle(p_dev);
        }
        if (p_dev->ep_int_type_union.ep_int_type_field[0].out_ack ||
            p_dev->ep_int_type_union.ep_int_type_field[0].out_nack) {
        }

        if (p_dev->device.endpoint_info[0].pfn_callback != NULL) {
        	(p_dev->device.endpoint_info[0].pfn_callback)(p_dev->device.endpoint_info[0].p_arg);
        }

        p_dev->ep_int_type_union.ep_int_type[0] = 0;
    }

    /* �˵�x */
    for (i = 1; i < AM_USBD_MAX_EP_CNT; i++) {
        if (p_dev->device.endpoint_info[i].inuse == AM_TRUE) {
            if ((p_dev->int_ep_union.int_ep_flag >> i) & 1) {
#if 0
                am_kprintf("�˵� %d �ж�", i);
#endif

                ep_int_type = amhw_zlg217_usbd_epx_int_state_get((ZLG217_USB_epx_t)(i - 1));
                amhw_zlg217_usbd_epx_int_state_clear((ZLG217_USB_epx_t)(i - 1), ep_int_type);
                p_dev->ep_int_type_union.ep_int_type[i] = ep_int_type;

                if (p_dev->device.endpoint_info[i].pfn_callback != NULL) {
                	(p_dev->device.endpoint_info[i].pfn_callback)(p_dev->device.endpoint_info[i].p_arg);
                }

//                /* ����˵�x�ж� */
//                if (p_dev->ep_int_type_union.ep_int_type_field[i].in_nack) {
//#if 0
//                    am_kprintf("in_nack\n");
//#endif
//                }
//                if (p_dev->ep_int_type_union.ep_int_type_field[i].out_ack) {
//#if 0
//                    am_kprintf("out_ack\n");  //��ӡ����ҪӦ��
//#endif
//                }

                p_dev->ep_int_type_union.ep_int_type[i] = 0;
            }
        }
    }

    p_dev->ep_int_type_union.ep_int_type[0] = 0;

    p_dev->int_ep_union.int_ep_flag = 0;
}

/**
 * \brief USB�жϷ�����
 */
static void __usbd_isr_function(void *p_device)
{
    am_zlg227_usbd_dev_t *p_dev =(am_zlg227_usbd_dev_t *) p_device;
    uint8_t int_status;

    if (NULL == p_dev)
        return;

    int_status = amhw_zlg217_usbd_int_state_get();   /* ����ж�״̬ */
    amhw_zlg217_usbd_int_state_clear(int_status);    /* ����ж�״̬ */
    p_dev->int_stat = int_status;

#ifdef USB_DEBUG
    am_kprintf("int ");
#endif

    /* �˵��ж� */
    if (int_status & AMHW_ZLG217_USB_INT_STATE_EPINTF) {
#ifdef USB_DEBUG
        am_kprintf("ep ");
#endif
        __usb_device_interrupt_endpoint(p_dev);
    }

    /* ���߸�λ0�ж� */
    if (int_status & AMHW_ZLG217_USB_INT_STATE_RSTF) {
#ifdef USB_DEBUG
        am_kprintf("rst\n");
#endif
        am_usbd_init(&(p_dev->device));
    }

    /* ���߹��� */
    if (int_status & AMHW_ZLG217_USB_INT_STATE_SUSPENDF) {
#ifdef USB_DEBUG
        am_kprintf("susp\n");
#endif
    }

    /* ���߻��� */
    if (int_status & AMHW_ZLG217_USB_INT_STATE_RESUMF) {
#ifdef USB_DEBUG
        am_kprintf("wkup\n");
#endif
        amhw_zlg217_usbd_wakeup();
    }

    /* ��⵽SOF */
    if (int_status & AMHW_ZLG217_USB_INT_STATE_SOFF) {

    }

    p_dev->int_stat = 0;
}

/**
 * \brief ��ͨ����������������ʼ���˵���Ϣ���˵����������������ԣ�֧�ֵ�������С��
 *
 * \retval �ɹ����ظ�ʹ�õĶ˵������ʧ�ܷ���-1
 */
static am_err_t __init_ep_info (am_zlg227_usbd_dev_t *p_dev)
{
    const uint8_t *p_tmp = NULL;    /* ���ݻ���ָ�� */
    am_usb_descriptor_config_t *p_desc_conf = NULL;
    am_usb_descriptor_interface_t     *p_desc_if   = NULL;
    am_usb_descriptor_endpoint_t      *p_desc_ep   = NULL;
    uint8_t if_cnt = 0;             /* �ӿ��������ĸ��� */
    uint8_t ep_cnt = 0;             /* ĳ���ӿ��ж˵��������ĸ��� */
    uint8_t ep_num = 0;             /* �˵�� */
    uint16_t desc_size = 0;         /* ���������� */
    uint8_t ret = 0;

    /* ��ȡ���������� */
    p_tmp = __find_desc_by_wValue1(p_dev, (AM_USB_DESCRIPTOR_TYPE_CONFIGURE << 8) | 0);
    if (p_tmp == NULL)
        return AM_ERROR;

    /* �������������� */
    p_desc_conf = (am_usb_descriptor_config_t *)p_tmp;       // ����������
    p_tmp += sizeof(am_usb_descriptor_config_t);             // ����������+ ƫ���� = �ӿ�������
    /* ��һ������������������������ */
    if (p_desc_conf->b_descriptor_type != AM_USB_DESCRIPTOR_TYPE_CONFIGURE)
        return -1;

    desc_size = (p_desc_conf->w_total_length[1] << 8) |
                 p_desc_conf->w_total_length[0];

    if_cnt = p_desc_conf->b_num_interfaces; /* �ӿڸ��� */
    if (if_cnt <= 0)
        return AM_ERROR;

    while (if_cnt--) {
        p_desc_if = (am_usb_descriptor_interface_t *)p_tmp;  // �ӿ�������
        p_tmp += sizeof(am_usb_descriptor_interface_t);      // �ӿ������ķ�+ƫ���� = �˵�������

        //  ��֤����������
        if (p_tmp - (uint8_t *)p_desc_conf > desc_size)
            return AM_ERROR;

        // ��� ���ǽӿ���������
        if (p_desc_if->b_descriptor_type != AM_USB_DESCRIPTOR_TYPE_INTERFACE)
            return AM_ERROR;

        ep_cnt = p_desc_if->b_num_endpoints;    /* �˵���� */

        // �˵����������֧�ֵĶ˵���� ����ʧ��
        if (ep_cnt == 0 || ep_cnt >= AM_USBD_MAX_EP_CNT)
            return AM_ERROR;

        while (ep_cnt--) {
            p_desc_ep = (am_usb_descriptor_endpoint_t *)p_tmp;  // �˵�������
            p_tmp += sizeof(am_usb_descriptor_endpoint_t);      // ��һ��ѭ��������˵����������ڶ���������˵�������
            if (p_tmp - (uint8_t *)p_desc_conf > desc_size)
                return AM_ERROR;

            if (p_desc_ep->b_descriptor_type != AM_USB_DESCRIPTOR_TYPE_ENDPOINT)
                return AM_ERROR;

            ep_num = p_desc_ep->b_endpoint_address &
                                AM_USB_DESCRIPTOR_ENDPOINT_ADDRESS_NUMBER_MASK;

            p_dev->device.endpoint_info[ep_num].inuse = 1;  // ��ʾ�˵㱻ʹ��
            p_dev->device.endpoint_info[ep_num].ep_address = p_desc_ep->b_endpoint_address;
            p_dev->device.endpoint_info[ep_num].max_packet_size = (p_desc_ep->wmax_packet_size[1] << 8) |
                                                             p_desc_ep->wmax_packet_size[0];
            p_dev->device.endpoint_info[ep_num].transfer_type = p_desc_ep->bm_attributes;
            p_dev->device.endpoint_info[ep_num].stalled = 0;

            ret++;
        }
    }

    return ret;
}

/**
 * \brief ��ʼ��USB
 *
 * \param[in] p_dev     : ָ��USB�豸
 * \param[in] p_info    : ָ��USB�豸��Ϣ
 *
 * \return USB��׼���������������Ϊ NULL��������ʼ��ʧ�ܡ�
 */
am_usbd_dev_t *am_zlg227_usbd_init (am_zlg227_usbd_dev_t           *p_dev,
                                    const am_zlg227_usbd_devinfo_t *p_info)
{
    int i = 0;

    if (NULL == p_dev  || NULL == p_info) {
        return NULL;
    }

    p_dev->p_info = p_info;

    p_dev->device.p_info = p_info->p_devinfo;

    p_dev->device.ctrl_handle = p_dev;
    p_dev->device.p_interface = &__g_usb_device_interface;

    for (i = 0; i < AM_USBD_MAX_EP_CNT; i++) {
        p_dev->device.endpoint_info[i].pfn_callback = NULL;
        p_dev->device.endpoint_info[i].p_arg = NULL;
    }

    /* ��ʼ����������*/
    p_dev->device.vendor_req.pfn_vendor = NULL;
    p_dev->device.vendor_req.p_arg      = NULL;

    p_dev->device.class_req.pfn_class   = NULL;
    p_dev->device.class_req.p_arg       = NULL;

    p_dev->device.device_address = 0;
    p_dev->device.state          = AM_USBD_STATE_DEFAULT;

    p_dev->int_stat = 0;
    p_dev->int_ep_union.int_ep_flag = 0;

    for (i = 0; i < AM_USBD_MAX_EP_CNT; i++)
        p_dev->ep_int_type_union.ep_int_type[i] = 0;

    p_dev->device.setup_data.bm_request_type = 0;
    p_dev->device.setup_data.b_request = 0;
    p_dev->device.setup_data.w_value = 0;
    p_dev->device.setup_data.w_index = 0;
    p_dev->device.setup_data.w_length = 0;


    /**< \brief ���ö˵�0��Ĭ������ */
    p_dev->device.endpoint_info[0].stalled          = 0;
    p_dev->device.endpoint_info[0].ep_address       = 0;
    p_dev->device.endpoint_info[0].max_packet_size  = AM_USBD_MAX_EP_DATA_CNT;
    p_dev->device.endpoint_info[0].transfer_type    = AM_USB_ENDPOINT_CONTROL;
    p_dev->device.endpoint_info[0].inuse = 1;
    p_dev->device.endpoint_info[0].val_length       = 0;

    for (i = 1 ;i < AM_USBD_MAX_EP_CNT; i++) {
        p_dev->device.endpoint_info[i].stalled          = 0;
        p_dev->device.endpoint_info[i].ep_address = 0;
        p_dev->device.endpoint_info[i].max_packet_size  = 0;
        p_dev->device.endpoint_info[i].transfer_type    = 0;
        p_dev->device.endpoint_info[i].inuse            = 0;
        p_dev->device.endpoint_info[i].val_length       = 0;
    }
    if (__init_ep_info(p_dev) == -1) {
#ifdef USB_DEBUG
        am_kprintf("fail to init endpoint\n");
#endif
    }

    p_dev->device.running_ctrl_state = AM_USBD_CTRL_IDLE;

    am_usbd_ch9_std_request(&(p_dev->device));


    if (p_info->pfn_plfm_init) {
        p_info->pfn_plfm_init();
    }

    am_usbd_init(&(p_dev->device));

    am_int_connect(p_info->inum, __usbd_isr_function, (void *)p_dev);
    am_int_enable(p_info->inum);

    return &(p_dev->device);
}

/**
 * \brief USB Device ȥ��ʼ��
 *
 * \param[in] p_info : ָ��USB�豸��Ϣ
 */
void am_zlg227_usbd_deinit (const am_zlg227_usbd_devinfo_t *p_info)
{
    if (p_info != NULL && p_info->pfn_plfm_deinit != NULL) {
        p_info->pfn_plfm_deinit();
    }
}
