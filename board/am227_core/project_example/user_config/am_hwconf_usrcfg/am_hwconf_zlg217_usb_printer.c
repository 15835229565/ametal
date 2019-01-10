/*******************************************************************************
*                                 AMetal
*                       ---------------------------
*                       innovating embedded systems
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
 * \brief zlg227 USB_printer �û������ļ�
 * \sa am_zlg227_hwconfig_usb_printer.c
 *
 * \internal
 * \par Modification history
 * - 1.00 18-12-12  adw, first implementation.
 * \endinternal
 */

#include "ametal.h"
#include "am_clk.h"
#include "am_usbd.h"
#include "am_zlg217.h"
#include "am_gpio.h"
#include "am_zlg217_usbd.h"
#include "am_usbd_printer.h"
#include "am_zlg217_inst_init.h"

/** \brief ��ӡ������ֵ */
#define AM_USBD_PRINTER_CONFIGURE_INDEX               (1U)

/** \brief ��ӡ���ӿ����� */
#define AM_USBD_PRINTER_INTERFACE_INDEX               (0U)

/** \brief ��ӡ���ӿ���*/
#define AM_USBD_PRINTER_INTERFACE_COUNT               (1U)

/** ��ӡ���˵�����������С����*/
#define AM_USBD_PRINTER_BULK_IN_PACKET_SIZE           AM_USBD_MAX_EP_DATA_CNT
#define AM_USBD_PRINTER_BULK_OUT_PACKET_SIZE          AM_USBD_MAX_EP_DATA_CNT

#define AM_USBD_PRINTER_BULK_IN_INTERVAL              (0x06U)
#define AM_USBD_PRINTER_BULK_OUT_INTERVAL             (0x06U)


/** \brief �˵�buff���ֻ֧��64 */
#define AM_USBD_PRINTER_BUFFER_SIZE                   (64U)

#define __USBD_CONFIGURATION_COUNT                    (1U)



/**
 * \brief ��ӡ���豸id �ַ�������
 * ǰ�����ֽ�Ϊ�ַ������ݳ��ȣ���˶���.���Ա���ǰ�����ַ����ַ����ݣ���id������Ϊ������������ʹ�õı�ʶ��
 */
static const uint8_t __g_printer_id[] = "xxMFG:ZLG;MDL: usb printer;CMD:POSTSCRIPT";

/**
 * \brief ��ӡ�����ݴ���buff����
 * \note ��buff��С������ڶ˵�������С������buff��СӦ�ô��ڴ�ӡ��id(__g_printer_id)�ַ�������,�����������
 */
static uint8_t __g_printer_buff[AM_USBD_PRINTER_BUFFER_SIZE + 1] = {0};
/**
 * \addtogroup am_zlg217_if_hwconfig_src_usb_printer
 * \copydoc am_zlg217_hwconfig_usb_printer.c
 * @{
 */

/* USB �豸������ */
static uint8_t __g_am_usbd_printer_desc_dev[AM_USB_DESCRIPTOR_LENGTH_DEVICE]  = {
    AM_USB_DESCRIPTOR_LENGTH_DEVICE,        /* �豸���������ֽ��� */
    AM_USB_DESCRIPTOR_TYPE_DEVICE,          /* �豸���������ͱ�ţ��̶�Ϊ0x01 */
    0x00,0x02,
	AM_USBD_CLASS,                          /* ͨ���� */
	AM_USBD_SUBCLASS,                       /* �豸���� */
	AM_USBD_PROTOCOL,                       /* Э���� */
	AM_USBD_MAX_EP_DATA_CNT,                /* �˵�0��������С */

    /**
     * ���̱�š���Ҫ��USBЭ�����룬�����Ϊѧϰʹ�ÿ������ѡһ���Ѿ�ע����ģ�
     * ������Ϊ��Ʒ�����Ļ��ͱ���д�Լ���˾�ĳ��̱�ţ�������Ȩ���˴�����һ��û����USBЭ��ע��ı��
     */
    0x22, 0x1F,
    0x9B, 0x02,                             /* ��Ʒ��� */
    AM_USB_SHORT_GET_LOW(AM_USBD_DEMO_BCD_VERSION),
    AM_USB_SHORT_GET_HIGH(AM_USBD_DEMO_BCD_VERSION), /* �豸������� */
    0x01,                                   /* �������̵��ַ������� */
    0x02,                                   /* ������Ʒ���ַ������� */
    0x00,                                   /* �����豸���кŵ��ַ������� */
    __USBD_CONFIGURATION_COUNT,            /* ���õ�������ֻ����һ���� */
};

/* ���������������¼�������������Խ���ϼ�������ֱ�ӵõ��¼��������� */
static uint8_t __g_am_usbd_printer_desc_conf[AM_USB_DESCRIPTOR_LENGTH_CONFIGURE +
                                             AM_USB_DESCRIPTOR_LENGTH_INTERFACE +
                                             AM_USB_DESCRIPTOR_LENGTH_ENDPOINT +
                                             AM_USB_DESCRIPTOR_LENGTH_ENDPOINT ] = {
    /* ���������� */
    AM_USB_DESCRIPTOR_LENGTH_CONFIGURE,     /* �����������ֽ��� */
    AM_USB_DESCRIPTOR_TYPE_CONFIGURE,       /* �������������ͱ�ţ��̶�Ϊ0x02 */

    /* �������������������������ܳ���(�������������ӿ��������ţ��������˵�������) */
    AM_USB_SHORT_GET_LOW(sizeof(__g_am_usbd_printer_desc_conf)),
    AM_USB_SHORT_GET_HIGH(sizeof(__g_am_usbd_printer_desc_conf)),
    AM_USBD_PRINTER_INTERFACE_COUNT,        /* �ӿ����������� */
    AM_USBD_PRINTER_CONFIGURE_INDEX,        /* ����ֵ */
    0x00,                                   /* ���������õ��ַ������� */

    /* �豸���ԣ����߹��磬��֧��Զ�̻��� */
    (AM_USBD_CONFIG_SELF_POWER  | AM_USBD_CONFIG_NOT_REMOTE_WAKEUP),
    AM_USBD_MAX_POWER,                      /* �����߻�ȡ����������100mA�� 2mAһ����λ */

    /* �ӿ������� */
    AM_USB_DESCRIPTOR_LENGTH_INTERFACE,     /* �ӿ��������ֽ��� */
    AM_USB_DESCRIPTOR_TYPE_INTERFACE,       /* �ӿ����������ͱ�ţ��̶�Ϊ0x04 */
    AM_USBD_PRINTER_INTERFACE_INDEX,        /* �ýӿڱ�� */
    0x00,                                   /* ��ѡ���õ�����ֵ���ýӿڵı��ñ�ţ� */
    AM_USBD_PRINTER_ENDPOINT_COUNT,         /* �ýӿ�ʹ�õĶ˵������������˵�0�� */
    AM_USBD_CONFIG_PRINTER_CLASS_CODE,      /* printer_CLASS�� */
    AM_USBD_PRINTER_SUBCLASS,               /* printer������ */
    AM_USBD_PRINTER_PROTOCOL,               /* printerЭ������ */
    0x00,                                   /* �����ýӿڵ��ַ������� */

    /* ����˵������� */
    AM_USB_DESCRIPTOR_LENGTH_ENDPOINT,      /* �˵��������ֽ��� */
    AM_USB_DESCRIPTOR_TYPE_ENDPOINT,        /* �˵����������ͱ�ţ��̶�Ϊ0x05 */

    /* D7 1:USB_IN  0:USB_OUT D3:D0 �˵�� */
    (AM_USBD_PRINTER_BULK_EP_IN | (AM_USB_IN << AM_USB_REQUEST_TYPE_DIR_SHIFT)),
    AM_USB_ENDPOINT_BULK,                   /* �˵����� 02��ʾ����  */

    AM_USB_SHORT_GET_LOW(AM_USBD_PRINTER_BULK_IN_PACKET_SIZE),
    AM_USB_SHORT_GET_HIGH(AM_USBD_PRINTER_BULK_IN_PACKET_SIZE), /* �˵�һ�����շ���������С */

	AM_USBD_PRINTER_BULK_IN_INTERVAL,       /* ������ѯ�˵�ʱ��ʱ������10ms  */

    /* ����˵������� */
    AM_USB_DESCRIPTOR_LENGTH_ENDPOINT,      /* �˵��������ֽ��� */
    AM_USB_DESCRIPTOR_TYPE_ENDPOINT,        /* �˵����������ͱ�ţ��̶�Ϊ0x05 */

    /* �˵��ַ��������� */
    (AM_USBD_PRINTER_BULK_EP_OUT | (AM_USB_OUT << AM_USB_REQUEST_TYPE_DIR_SHIFT)),

    AM_USB_ENDPOINT_BULK,                   /* �˵����� */

    AM_USB_SHORT_GET_LOW(AM_USBD_PRINTER_BULK_OUT_PACKET_SIZE),
    AM_USB_SHORT_GET_HIGH(AM_USBD_PRINTER_BULK_OUT_PACKET_SIZE), /* �˵�һ�����շ���������С */

	AM_USBD_PRINTER_BULK_OUT_INTERVAL       /* ������ѯ�˵�ʱ��ʱ���� 10ms */
};

/**< \brief ������Ʒ���ַ��������� */
static uint8_t __g_am_usbd_printer_desc_str_iproduct[18] = {
    sizeof(__g_am_usbd_printer_desc_str_iproduct),       /* �ַ����������ֽ��� */
    AM_USB_DESCRIPTOR_TYPE_STRING,          /* �ַ������������ͱ�ţ��̶�Ϊ0x03 */

     0x55, 0x00, /* U */
     0x42, 0x00, /* S */
     0x53, 0x00, /* B */
     0x21, 0x6a, /* ģ */
     0xdf, 0x62, /* �� */
     0x53, 0x62, /* �� */
     0x70, 0x53, /* ӡ */
     0x73, 0x67, /* �� */
};

/**< \brief ����ID�ַ��������� */
/**< \brief ����ʹ����ʽӢ���ʹ�ü������ĵ�ԭ�������ʹ�ü������ģ�������������ӻ�Ҫ�ַ��������� */
/**< \brief ��ʽӢ�������IDΪ0x0409���������ĵ�����IDΪ0x0804��ע���С�ˡ� */
static uint8_t __g_am_usbd_printer_desc_str_language_id[4] = {
    sizeof(__g_am_usbd_printer_desc_str_language_id),       /* �ַ����������ֽ��� */
    AM_USB_DESCRIPTOR_TYPE_STRING,          /* �ַ������������ͱ�ţ��̶�Ϊ0x03 */
    0x04,
    0x08,       /* �������� */
};

/**< \brief �������̵��ַ��������� */
static uint8_t __g_am_usbd_printer_desc_str_imanufacturer[22] = {
    sizeof(__g_am_usbd_printer_desc_str_imanufacturer),       /* �ַ����������ֽ��� */
    AM_USB_DESCRIPTOR_TYPE_STRING,          /* �ַ������������ͱ�ţ��̶�Ϊ0x03 */
    0x7f, 0x5e, /* �� */
    0xde, 0x5d, /* �� */
    0xf4, 0x81, /* �� */
    0xdc, 0x8f, /* Զ */
    0x35, 0x75, /* �� */
    0x50, 0x5b, /* �� */
    0x09, 0x67, /* �� */
    0x50, 0x96, /* �� */
    0x6c, 0x51, /* �� */
    0xf8, 0x53, /* ˾ */
};

/**< \brief �����豸���кŵ��ַ��������� */
static uint8_t __g_am_usbd_printer_desc_str_iserialnumber[18] = {
    sizeof(__g_am_usbd_printer_desc_str_iserialnumber), /* �ַ����������ֽ��� */
    AM_USB_DESCRIPTOR_TYPE_STRING,          /* �ַ������������ͱ�ţ��̶�Ϊ0x03 */
    'Z', 0x00,
    'L', 0x00,
    'G', 0x00,
    ' ', 0x00,
    'D', 0x00,
    'E', 0x00,
    'M', 0x00,
    'O', 0x00,
};


// ��8λΪ���������� �ڰ�λΪ���������
static const am_usbd_descriptor_t __g_am_usbd_printer_descriptor[] = {
    {0x0100, sizeof(__g_am_usbd_printer_desc_dev), __g_am_usbd_printer_desc_dev},     /* �豸������ */
    {0x0200, sizeof(__g_am_usbd_printer_desc_conf), __g_am_usbd_printer_desc_conf},   /* ���������������¼������� */
    {0x0300, sizeof(__g_am_usbd_printer_desc_str_language_id), __g_am_usbd_printer_desc_str_language_id},       /* �ַ���������0����������id */
    {0x0301, sizeof(__g_am_usbd_printer_desc_str_imanufacturer), __g_am_usbd_printer_desc_str_imanufacturer},   /* �ַ���������1���������� */
    {0x0302, sizeof(__g_am_usbd_printer_desc_str_iproduct), __g_am_usbd_printer_desc_str_iproduct},             /* �ַ���������2��������Ʒ */
    {0x0303, sizeof(__g_am_usbd_printer_desc_str_iserialnumber), __g_am_usbd_printer_desc_str_iserialnumber},   /* �ַ���������3�������豸���к� */
};

/**
 * \brief ƽ̨��ʼ��
 */
static void __am_usbd_printer_init (void) {
    /* ʹ��ʱ�� */
    am_clk_enable(CLK_USB);
    am_clk_enable(CLK_IOPA);
    am_clk_enable(CLK_AFIO);

    /* ����PIOA_11 PIOA_12ΪUSB����   */
    am_gpio_pin_cfg(PIOA_11, PIOA_11_USBDM);
    am_gpio_pin_cfg(PIOA_12, PIOA_12_USBDP);
}

/**
 * \brief ƽ̨ȥ��ʼ��
 */
static void __am_usbd_printer_deinit (void) {
    amhw_zlg217_usbd_connect_set(ZLG217_USB_DISCONNECT);   /* �Ͽ����� */
    am_clk_disable(CLK_USB);                               /* ����USBʱ�� */
}

static const am_usbd_devinfo_t __g_usbd_info = {
        __g_am_usbd_printer_descriptor,                                                         /* ��������ַ */
        sizeof(__g_am_usbd_printer_descriptor) / sizeof(__g_am_usbd_printer_descriptor[0]),     /* ���������� */
};

/**< \brief ����USB�豸��Ϣ */
static const am_zlg227_usbd_devinfo_t  __g_am_usbd_printer_info = {
    ZLG217_USB_BASE,                  /**< \brief �Ĵ�������ַ */
    INUM_USB,                         /**< \brief �жϺ� */
    __am_usbd_printer_init,           /**< \brief ƽ̨��ʼ�� */
    __am_usbd_printer_deinit,         /**< \brief ƽ̨ȥ��ʼ�� */
    &__g_usbd_info,
};

/** \brief USB��ӡ����Ϣ�ṹ��*/
static am_usbd_printer_info_t __g_usbd_printer_info = {
        __g_printer_id,               /**< \brief ��ӡ��id */
        sizeof(__g_printer_id) - 1,   /**< \brief ��ӡ��id����,(��һΪ��������\0��)*/
        __g_printer_buff,             /**< \brief ��ӡ��ʹ��buff */
};

/** \brief ��ӡ��ʹ��handle(USB�豸��) */
static am_usbd_printer_t     __g_usb_printer_dev;

/** \brief ����zlg227 ��USB handle*/
static am_zlg227_usbd_dev_t  __g_zlg227_usb_printer_dev;

/** \brief usb_printerʵ����ʼ�������usb_printer��׼������ */
am_usbd_printer_handle am_zlg227_usbd_printer_inst_init (void)
{
    return am_usbd_printer_init(&__g_usb_printer_dev,
                                &__g_usbd_printer_info,
                                am_zlg227_usbd_init(&__g_zlg227_usb_printer_dev, &__g_am_usbd_printer_info));
}

/** \brief usb_printer���ʼ�������usb_printer��׼������ */
void am_zlg227_usbd_printer_inst_deinit (void)
{
    am_usbd_printer_deinit(&__g_usb_printer_dev);
}

/**
 * @}
 */

/* end of file */
