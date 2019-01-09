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
 * \brief zlg217 USB_MSC �û������ļ�
 * \sa am_zlg217_hwconfig_usb_msc.c
 *
 * \internal
 * \par Modification history
 * - 1.00 18-10-29  enf, first implementation.
 * \endinternal
 */

#include "ametal.h"
#include "am_clk.h"
#include "am_gpio.h"
#include "am_usbd_msc.h"
#include "am_zlg217.h"
#include "am_zlg217_usbd.h"
#include "amhw_zlg217_usbd.h"
#include "am_zlg217_inst_init.h"
/**
 * \addtogroup am_zlg217_if_hwconfig_src_usb_msc
 * \copydoc am_zlg217_hwconfig_usb_msc.c
 * @{
 */

#define __USBD_MSC_CONFIGURE_INDEX               (1U)
#define __USBD_MSC_INTERFACE_COUNT               (1U)
#define __USBD_MSC_INTERFACE_INDEX               (0U)
#define __USBD_CONFIGURATION_COUNT               (1U)


/* �豸������ */
static uint8_t __g_usb_msc_desc_dev[AM_USB_DESCRIPTOR_LENGTH_DEVICE]  = {
    AM_USB_DESCRIPTOR_LENGTH_DEVICE,       /* �豸���������ֽ��� */
    AM_USB_DESCRIPTOR_TYPE_DEVICE,         /* �豸���������ͱ�ţ��̶�Ϊ0x01 */
	0x10, 0x01,                            /* USB�汾 USB1.1 */

	AM_USBD_CLASS,                         /* ͨ���� */
	AM_USBD_SUBCLASS,                      /* �豸���� */
	AM_USBD_PROTOCOL,                      /* Э���� */
	AM_USBD_MAX_EP_DATA_CNT,               /* �˵�0��������С */

    /**
     * ���̱�š���Ҫ��USBЭ�����룬�����Ϊѧϰʹ�ÿ������ѡһ���Ѿ�ע����ģ�
     * ������Ϊ��Ʒ�����Ļ��ͱ���д�Լ���˾�ĳ��̱�ţ�������Ȩ���˴�����һ��û����USBЭ��ע��ı��
     */
    0x95, 0x41,
    0x15, 0x65, /* ��Ʒ��� */
    AM_USB_SHORT_GET_LOW(AM_USBD_DEMO_BCD_VERSION),
    AM_USB_SHORT_GET_HIGH(AM_USBD_DEMO_BCD_VERSION), /* �豸������� */
    0x01,       /* �������̵��ַ������� */
    0x02,       /* ������Ʒ���ַ������� */
    0x03,       /* �����豸���кŵ��ַ������� */
	__USBD_CONFIGURATION_COUNT,       /* ���õ�������ֻ����һ���� */
};

/* ���������������¼�������������Խ���ϼ�������ֱ�ӵõ��¼��������� */
static uint8_t __g_usb_msc_desc_conf[AM_USB_DESCRIPTOR_LENGTH_CONFIGURE +
                                     AM_USB_DESCRIPTOR_LENGTH_INTERFACE +
                                     AM_USB_DESCRIPTOR_LENGTH_ENDPOINT  +
                                     AM_USB_DESCRIPTOR_LENGTH_ENDPOINT ] = {
    /* ���������� */
    AM_USB_DESCRIPTOR_LENGTH_CONFIGURE,        /* �����������ֽ��� */
    AM_USB_DESCRIPTOR_TYPE_CONFIGURE,          /* �������������ͱ�ţ��̶�Ϊ0x02 */
    AM_USB_SHORT_GET_LOW(sizeof(__g_usb_msc_desc_conf)),
    AM_USB_SHORT_GET_HIGH(sizeof(__g_usb_msc_desc_conf)),/* �������������������������ܳ��� */
    __USBD_MSC_INTERFACE_COUNT,       /* �ӿ����������� */
    __USBD_MSC_CONFIGURE_INDEX,       /* ����ֵ */
    0x00,       /* ���������õ��ַ������� */

	/* �豸���ԣ����߹��磬��֧��Զ�̻��� */
	(AM_USBD_CONFIG_SELF_POWER  | AM_USBD_CONFIG_NOT_REMOTE_WAKEUP),
    AM_USBD_MAX_POWER,                  /* �����߻�ȡ����������100mA�� 2mAһ����λ */

    /* �ӿ������� */
    AM_USB_DESCRIPTOR_LENGTH_INTERFACE,        /* �ӿ��������ֽ��� */
    AM_USB_DESCRIPTOR_TYPE_INTERFACE,          /* �ӿ����������ͱ�ţ��̶�Ϊ0x04 */
	__USBD_MSC_INTERFACE_INDEX,            /* �ýӿڱ�� */
    0x00,       /* ��ѡ���õ�����ֵ���ýӿڵı��ñ�ţ� */
	AM_USBD_PRINTER_ENDPOINT_COUNT,         /* �ýӿ�ʹ�õĶ˵������������˵�0�� */
	AM_USBD_CONFIG_MSC_CLASS_CODE,          /* MSC_CLASS�� */
	AM_USBD_MSC_SUBCLASS,       /* msc������ */
	AM_USBD_MSC_PROTOCOL,       /* mscЭ������ */
    0x00,       /* �����ýӿڵ��ַ������� */

    /* ����˵������� */
    AM_USB_DESCRIPTOR_LENGTH_ENDPOINT,       /* �˵��������ֽ��� */
    AM_USB_DESCRIPTOR_TYPE_ENDPOINT,         /* �˵����������ͱ�ţ��̶�Ϊ0x05 */
    (AM_USBD_MSC_BULK_IN_ENDPOINT | (AM_USB_IN << AM_USB_REQUEST_TYPE_DIR_SHIFT)),
	AM_USB_ENDPOINT_BULK,       /* �˵����� 02��ʾ����  */

    AM_USB_SHORT_GET_LOW(AM_USBD_MAX_EP_DATA_CNT),
    AM_USB_SHORT_GET_HIGH(AM_USBD_MAX_EP_DATA_CNT), /* �˵�һ�����շ���������С */

    0x0A,       /* ������ѯ�˵�ʱ��ʱ������10ms  */

    /* ����˵������� */
    AM_USB_DESCRIPTOR_LENGTH_ENDPOINT,       /* �˵��������ֽ��� */
    AM_USB_DESCRIPTOR_TYPE_ENDPOINT,        /* �˵����������ͱ�ţ��̶�Ϊ0x05 */

    /* �˵��ַ��������� */
    (AM_USBD_MSC_BULK_OUT_ENDPOINT | (AM_USB_OUT << AM_USB_REQUEST_TYPE_DIR_SHIFT)),

	AM_USB_ENDPOINT_BULK,        /* �˵����� */

    AM_USB_SHORT_GET_LOW(AM_USBD_MAX_EP_DATA_CNT),
    AM_USB_SHORT_GET_HIGH(AM_USBD_MAX_EP_DATA_CNT), /* �˵�һ�����շ���������С */

    0x0A        /* ������ѯ�˵�ʱ��ʱ���� 10ms */
};

/**< \brief ������Ʒ���ַ��������� */
static uint8_t __g_usb_msc_desc_str_iproduct[16] = {
    sizeof(__g_usb_msc_desc_str_iproduct),       /* �ַ����������ֽ��� */
    AM_USB_DESCRIPTOR_TYPE_STRING,                  /* �ַ������������ͱ�ţ��̶�Ϊ0x03 */

	 0x55, 0x00, /* U */
	 0x42, 0x00, /* S */
	 0x53, 0x00, /* B */
     0x21, 0x6a, /* ģ */
     0xdf, 0x62, /* �� */
     0x55, 0x00, /* U */
     0xd8, 0x76  /* �� */
};

/**< \brief ����ID�ַ��������� */
/**< \brief ����ʹ����ʽӢ���ʹ�ü������ĵ�ԭ�������ʹ�ü������ģ�������������ӻ�Ҫ�ַ��������� */
/**< \brief ��ʽӢ�������IDΪ0x0409���������ĵ�����IDΪ0x0804��ע���С�ˡ� */
static uint8_t __g_usb_msc_desc_str_language_id[4] = {
    sizeof(__g_usb_msc_desc_str_language_id),       /* �ַ����������ֽ��� */
    AM_USB_DESCRIPTOR_TYPE_STRING,       /* �ַ������������ͱ�ţ��̶�Ϊ0x03 */
    0x04,
    0x08,       /* �������� */
};

/**< \brief �������̵��ַ��������� */
static uint8_t __g_usb_msc_desc_str_imanufacturer[22] = {
    sizeof(__g_usb_msc_desc_str_imanufacturer),       /* �ַ����������ֽ��� */
    AM_USB_DESCRIPTOR_TYPE_STRING,       /* �ַ������������ͱ�ţ��̶�Ϊ0x03 */
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
static uint8_t __g_usb_msc_desc_str_iserialnumber[22] = {
    sizeof(__g_usb_msc_desc_str_iserialnumber),    /* �ַ����������ֽ��� */
    AM_USB_DESCRIPTOR_TYPE_STRING,                    /* �ַ������������ͱ�ţ��̶�Ϊ0x03 */
    0x32, 0x00, /* 2 */
    0x30, 0x00, /* 0 */
    0x31, 0x00, /* 1 */
    0x38, 0x00, /* 8 */
    0x2d, 0x00, /* - */
    0x31, 0x00, /* 1 */
    0x30, 0x00, /* 0 */
    0x2d, 0x00, /* - */
    0x32, 0x00, /* 2 */
    0x29, 0x00, /* 9 */
};


// ��8λΪ���������� �ڰ�λΪ���������
static const am_usbd_descriptor_t __g_usb_msc_descriptor[] = {
    {0x0100, sizeof(__g_usb_msc_desc_dev), __g_usb_msc_desc_dev},     /* �豸������ */
    {0x0200, sizeof(__g_usb_msc_desc_conf), __g_usb_msc_desc_conf},   /* ���������������¼������� */
    {0x0300, sizeof(__g_usb_msc_desc_str_language_id), __g_usb_msc_desc_str_language_id},       /* �ַ���������0����������id */
    {0x0301, sizeof(__g_usb_msc_desc_str_imanufacturer), __g_usb_msc_desc_str_imanufacturer},   /* �ַ���������1���������� */
    {0x0302, sizeof(__g_usb_msc_desc_str_iproduct), __g_usb_msc_desc_str_iproduct},             /* �ַ���������2��������Ʒ */
    {0x0303, sizeof(__g_usb_msc_desc_str_iserialnumber), __g_usb_msc_desc_str_iserialnumber},   /* �ַ���������3�������豸���к� */
};

/**
 * \brief ƽ̨��ʼ��
 */
static void __usb_msc_init (void) {
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
static void __usb_msc_deinit (void) {
    amhw_zlg217_usbd_connect_set(ZLG217_USB_DISCONNECT);   /* �Ͽ����� */

    am_clk_disable(CLK_USB);    /* ����USBʱ�� */
}

static const am_usbd_devinfo_t __g_usbd_msc_info = {
	    __g_usb_msc_descriptor,                                                 /* ��������ַ */
	    sizeof(__g_usb_msc_descriptor) / sizeof(__g_usb_msc_descriptor[0]),     /* ���������� */
};

/**< \brief ����USB�豸��Ϣ */
static const am_zlg227_usbd_devinfo_t  __g_zlg227_usbd_msc_info = {
    ZLG217_USB_BASE,                                                      /* �Ĵ�������ַ */
    INUM_USB,                                                               /* �жϺ� */
    __usb_msc_init,                                                         /* ƽ̨��ʼ�� */
    __usb_msc_deinit,                                                       /* ƽ̨ȥ��ʼ�� */
	&__g_usbd_msc_info,
};

static am_usbd_msc_t      __g_usb_msc_dev;

static am_zlg227_usbd_dev_t  __g_zlg_usbd_msc;

static uint8_t __g_data_buf[AM_USBD_MAX_EP_DATA_CNT * 8];

static const am_usbd_msc_diskinfo_t __g_usbd_msc_disk_info = {
		AM_USBD_MSC_DISD_SIZE,
		AM_USBD_MSC_SECTOR_SIZE,
		AM_USBD_MSC_DISD_SIZE / AM_USBD_MSC_SECTOR_SIZE,
		(AM_USBD_MSC_DISD_SIZE / 256 / 1024 + 1) * 512,
		(AM_USBD_MSC_DISD_SIZE / 256 / 1024 * 2 + 1) * 512,
		(AM_USBD_MSC_DISD_SIZE / 256 / 1024 * 2 + 17) * 512,
		__g_data_buf,
};

/** \brief usb_mscʵ����ʼ�������usb_msc��׼������ */
am_usbd_msc_handle am_zlg217_usb_msc_inst_init (void)
{
    return am_usbd_msc_init(&__g_usb_msc_dev,
    						&__g_usbd_msc_disk_info,
							am_zlg227_usbd_init(&__g_zlg_usbd_msc, &__g_zlg227_usbd_msc_info));
}

/** \brief usb_msc���ʼ�������usb_msc��׼������ */
void am_mzlg217_usb_msc_inst_deinit (void)
{
    am_zlg217_usb_msc_deinit(&__g_usb_msc_dev);
}

/**
 * @}
 */

/* end of file */
