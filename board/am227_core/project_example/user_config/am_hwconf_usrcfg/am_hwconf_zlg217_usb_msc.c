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
 *
 * - 1.10 19-01-11  adw, add configuration macro
 * - 1.00 18-10-29  enf, first implementation.
 * \endinternal
 */

#include "ametal.h"
#include "am_types.h"
#include "am_clk.h"
#include "am_usb.h"
#include "am_gpio.h"
#include "am_usbd.h"
#include "am_usbd_msc.h"
#include "am_zlg217.h"
#include "am_zlg217_usbd.h"
#include "amhw_zlg217_usbd.h"

/**
 * \addtogroup am_zlg217_if_hwconfig_src_usb_msc
 * \copydoc am_zlg217_hwconfig_usb_msc.c
 * @{
 */

/*******************************************************************************
 * �û�USB���������ú�,�û������������꼴��,�������USB��������
 ******************************************************************************/

/** \brief USB�豸�������ú�*/
#define __USBD_MSC_VENDOR_ID                 (0x4195) /**< \brief ���̱��,��Ϊ��Ʒ��������д�Լ���˾�ĳ��̱�ţ�������Ȩ���˴�����һ��û����USBЭ��ע��ı��*/
#define __USBD_MSC_PRODUCT_ID                (0x6515) /**< \brief ��Ʒ���*/
#define __USBD_MSC_DEVICE_ID                 (0x0101) /**< \brief �豸���*/
#define __USBD_MSC_VENDOR_STRING_INDEX       (0x01)   /**< \brief �������̵��ַ�������*/
#define __USBD_MSC_PRODUCT_STRING_INDEX      (0x02)   /**< \brief ������Ʒ���ַ������� */
#define __USBD_MSC_DEVICE_STRING_INDEX       (0x03)   /**< \brief �����豸���кŵ��ַ�������*/
#define __USBD_CONFIGURATION_COUNT           (1U)     /**< \brief ��������һ������ֻ��һ������*/

/** \brief USB�������������ú�*/
#define __USBD_MSC_INTERFACE_COUNT           (1U)     /**< \brief �ӿ���*/
#define __USBD_MSC_CONFIGURE_INDEX           (1U)     /**< \brief ��������������*/
#define __USBD_MSC_DEVICE_POWER               AM_USBD_MAX_POWER /**< \brief �豸����������ĵ�������λ2mA,���100mA*/
/** \brief �豸���ԣ��Թ��磬��֧��Զ�̻���*/
#define __USBD_MSC_DEVICE_ATTRIBUTE          \
              (AM_USBD_CONFIG_SELF_POWER | AM_USBD_CONFIG_NOT_REMOTE_WAKEUP)

/**\brief USB�˵����������ú�*/
#define __USBD_MSC_ENDPOINT_IN                AM_USBD_MSC_BULK_IN_ENDPOINT  /**< \brief ����˵��*/
#define __USBD_MSC_ENDPOINT_IN_PACKSIZE       AM_USBD_MAX_EP_DATA_CNT       /**< \brief �˵����С,����Ϊ64*/
#define __USBD_MSC_ENDPOINT_IN_ATTRIBUTE      AM_USB_ENDPOINT_BULK          /**< \brief ���ö˵�����Ϊ��������*/
#define __USBD_MSC_ENDPOINT_IN_QUERY_TIME    (0x01)                         /**< \brief ���ö˵��ѯʱ��Ϊ10ms,��λΪ1ms*/

#define __USBD_MSC_ENDPOINT_OUT               AM_USBD_MSC_BULK_OUT_ENDPOINT
#define __USBD_MSC_ENDPOINT_OUT_PACKSIZE      AM_USBD_MAX_EP_DATA_CNT
#define __USBD_MSC_ENDPOINT_OUT_ATTRIBUTE     AM_USB_ENDPOINT_BULK
#define __USBD_MSC_ENDPOINT_OUT_QUERY_TIME   (0x01)

/**\brief USB�ӿ����������ú�*/
#define __USBD_MSC_ENDPOINT_COUNT            (2U)     /**< \brief �˵����,����˵�������˵�(���������ƶ˵�)*/

/*****************************************************************************
 * USB������(�豸������,�������������ӿ�������,�˵�������),�ò����û�ֻ�����������꼴��
 *****************************************************************************/
/* �豸������ */
static const uint8_t __g_usb_msc_desc_dev[AM_USB_DESC_LENGTH_DEVICE]  = {
    AM_USB_DESC_LENGTH_DEVICE,       /* �豸���������ֽ��� */
    AM_USB_DESCRIPTOR_TYPE_DEVICE,         /* �豸���������ͱ�ţ��̶�Ϊ0x01 */

    /* USB�汾 USB2.0 */
    AM_USB_SHORT_GET_LOW(AM_USB_VERSION), AM_USB_SHORT_GET_HIGH(AM_USB_VERSION),

    AM_USBD_CLASS,                         /* ͨ���� */
    AM_USBD_SUBCLASS,                      /* �豸���� */
    AM_USBD_PROTOCOL,                      /* Э���� */
    AM_USBD_MAX_EP_DATA_CNT,               /* �˵�0��������С */

    /**
     * ���̱�š���Ҫ��USBЭ�����룬�����Ϊѧϰʹ�ÿ������ѡһ���Ѿ�ע����ģ�
     * ������Ϊ��Ʒ�����Ļ��ͱ���д�Լ���˾�ĳ��̱�ţ�������Ȩ���˴�����һ��û����USBЭ��ע��ı��
     */
    AM_USB_SHORT_GET_LOW(__USBD_MSC_VENDOR_ID), AM_USB_SHORT_GET_HIGH(__USBD_MSC_VENDOR_ID),

    /* ��Ʒ��� */
    AM_USB_SHORT_GET_LOW(__USBD_MSC_PRODUCT_ID), AM_USB_SHORT_GET_HIGH(__USBD_MSC_PRODUCT_ID),

    /* �豸������� */
    AM_USB_SHORT_GET_LOW(__USBD_MSC_DEVICE_ID), AM_USB_SHORT_GET_HIGH(__USBD_MSC_DEVICE_ID),

    __USBD_MSC_VENDOR_STRING_INDEX,         /* �������̵��ַ������� */
    __USBD_MSC_PRODUCT_STRING_INDEX,        /* ������Ʒ���ַ������� */
    __USBD_MSC_DEVICE_STRING_INDEX,         /* �����豸���кŵ��ַ������� */
    __USBD_CONFIGURATION_COUNT,             /* ���õ�������ֻ����һ���� */
};

/* ���������������¼�������������Խ���ϼ�������ֱ�ӵõ��¼��������� */
static uint8_t __g_usb_msc_desc_conf[AM_USB_DESC_LENGTH_ALL(__USBD_MSC_ENDPOINT_COUNT)] = {
    /* ���������� */
    AM_USB_DESC_LENGTH_CONFIGURE,     /* �����������ֽ��� */
    AM_USB_DESCRIPTOR_TYPE_CONFIGURE,       /* �������������ͱ�ţ��̶�Ϊ0x02 */

    /* �������������������������ܳ��� */
    AM_USB_SHORT_GET_LOW(AM_USB_DESC_LENGTH_ALL(__USBD_MSC_ENDPOINT_COUNT)),
    AM_USB_SHORT_GET_HIGH(AM_USB_DESC_LENGTH_ALL(__USBD_MSC_ENDPOINT_COUNT)),
    __USBD_MSC_INTERFACE_COUNT,             /* �ӿ����������� */
    __USBD_MSC_CONFIGURE_INDEX,             /* ����ֵ */
    0x00,                                   /* ���������õ��ַ�������,0x00��ʾû�� */

    /* �豸���ԣ����߹��磬��֧��Զ�̻��� */
	__USBD_MSC_DEVICE_ATTRIBUTE,
	__USBD_MSC_DEVICE_POWER,                /* �����߻�ȡ����������100mA�� 2mAһ����λ */

    /* �ӿ������� */
    AM_USB_DESC_LENGTH_INTERFACE,     /* �ӿ��������ֽ��� */
    AM_USB_DESCRIPTOR_TYPE_INTERFACE,       /* �ӿ����������ͱ�ţ��̶�Ϊ0x04 */
    0x00,                                   /* �ýӿڱ��,�ӿ������������0��ʼ,�Դ����� */
    0x00,                                   /* ��ѡ���õ�����ֵ���ýӿڵı��ñ�ţ� */
    __USBD_MSC_ENDPOINT_COUNT,              /* �ýӿ�ʹ�õĶ˵������������˵�0�� */
    AM_USBD_CONFIG_MSC_CLASS_CODE,          /* MSC_CLASS�� */
    AM_USBD_MSC_SUBCLASS,                   /* msc������ */
    AM_USBD_MSC_PROTOCOL,                   /* mscЭ������ */
    0x00,                                   /* �����ýӿڵ��ַ�������, 0x00��ʾû�� */

    /* ����˵������� */
    AM_USB_DESC_LENGTH_ENDPOINT,            /* �˵��������ֽ��� */
    AM_USB_DESCRIPTOR_TYPE_ENDPOINT,        /* �˵����������ͱ�ţ��̶�Ϊ0x05 */
    (__USBD_MSC_ENDPOINT_IN | (AM_USB_IN << AM_USB_REQUEST_TYPE_DIR_SHIFT)),
	__USBD_MSC_ENDPOINT_IN_ATTRIBUTE,       /* �˵����� 02��ʾ����  */

    AM_USB_SHORT_GET_LOW(__USBD_MSC_ENDPOINT_IN_PACKSIZE),
    AM_USB_SHORT_GET_HIGH(__USBD_MSC_ENDPOINT_IN_PACKSIZE), /* �˵�һ�����շ���������С */

	__USBD_MSC_ENDPOINT_IN_QUERY_TIME,      /* ������ѯ�˵�ʱ��ʱ������10ms  */

    /* ����˵������� */
    AM_USB_DESC_LENGTH_ENDPOINT,            /* �˵��������ֽ��� */
    AM_USB_DESCRIPTOR_TYPE_ENDPOINT,        /* �˵����������ͱ�ţ��̶�Ϊ0x05 */

    /* �˵��ַ��������� */
    (__USBD_MSC_ENDPOINT_OUT | (AM_USB_OUT << AM_USB_REQUEST_TYPE_DIR_SHIFT)),

	__USBD_MSC_ENDPOINT_OUT_ATTRIBUTE,      /* �˵����� */

    AM_USB_SHORT_GET_LOW(__USBD_MSC_ENDPOINT_OUT_PACKSIZE),
    AM_USB_SHORT_GET_HIGH(__USBD_MSC_ENDPOINT_OUT_PACKSIZE), /* �˵�һ�����շ���������С */

	__USBD_MSC_ENDPOINT_OUT_QUERY_TIME,     /* ������ѯ�˵�ʱ��ʱ���� 10ms */
};

/*******************************************************************************
 * �ַ���������,����û���Ҫ�޸���������Ϣ��ע��ʹ�õ���UINCODE��(ע���С��)
 ******************************************************************************/
/**< \brief ������Ʒ���ַ��������� */
static const uint8_t __g_usb_msc_desc_str_iproduct[16] = {
    sizeof(__g_usb_msc_desc_str_iproduct), /* �ַ����������ֽ��� */
    AM_USB_DESCRIPTOR_TYPE_STRING,         /* �ַ������������ͱ�ţ��̶�Ϊ0x03 */

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
static const uint8_t __g_usb_msc_desc_str_language_id[4] = {
    sizeof(__g_usb_msc_desc_str_language_id),       /* �ַ����������ֽ��� */
    AM_USB_DESCRIPTOR_TYPE_STRING,       /* �ַ������������ͱ�ţ��̶�Ϊ0x03 */
    0x04,
    0x08,       /* �������� */
};

/**< \brief �������̵��ַ��������� */
static const uint8_t __g_usb_msc_desc_str_imanufacturer[22] = {
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
static const uint8_t __g_usb_msc_desc_str_iserialnumber[22] = {
    sizeof(__g_usb_msc_desc_str_iserialnumber),    /* �ַ����������ֽ��� */
    AM_USB_DESCRIPTOR_TYPE_STRING,                 /* �ַ������������ͱ�ţ��̶�Ϊ0x03 */
    0x32, 0x00, /* 2 */
    0x30, 0x00, /* 0 */
    0x31, 0x00, /* 1 */
    0x38, 0x00, /* 8 */
    0x2d, 0x00, /* - */
    0x31, 0x00, /* 1 */
    0x30, 0x00, /* 0 */
    0x2d, 0x00, /* - */
    0x32, 0x00, /* 2 */
    0x39, 0x00, /* 9 */
};


/******************************************************************************
 * ����������Ϣ
 *****************************************************************************/
static const am_usbd_descriptor_t __g_usb_msc_descriptor[] = {
    /* �豸������ */
    {
        (AM_USB_DESCRIPTOR_TYPE_DEVICE << 8) | (0x00),
        sizeof(__g_usb_msc_desc_dev),
        __g_usb_msc_desc_dev
    },

    /* ���������������¼������� */
    {
        (AM_USB_DESCRIPTOR_TYPE_CONFIGURE << 8) | (0x00),
        sizeof(__g_usb_msc_desc_conf),
        __g_usb_msc_desc_conf
    },

    /* �ַ�������������������id */
    {
        (AM_USB_DESCRIPTOR_TYPE_STRING << 8) | (0x00),
        sizeof(__g_usb_msc_desc_str_language_id),
        __g_usb_msc_desc_str_language_id
    },

    /* �ַ������������������� */
    {
        (AM_USB_DESCRIPTOR_TYPE_STRING << 8) | __USBD_MSC_VENDOR_STRING_INDEX,
        sizeof(__g_usb_msc_desc_str_imanufacturer),
        __g_usb_msc_desc_str_imanufacturer
    },

    /* �ַ�����������������Ʒ */
    {
        (AM_USB_DESCRIPTOR_TYPE_STRING << 8) | __USBD_MSC_PRODUCT_STRING_INDEX,
        sizeof(__g_usb_msc_desc_str_iproduct),
        __g_usb_msc_desc_str_iproduct
    },

    /* �ַ����������������豸���к� */
    {
        (AM_USB_DESCRIPTOR_TYPE_STRING << 8)| __USBD_MSC_DEVICE_STRING_INDEX,
        sizeof(__g_usb_msc_desc_str_iserialnumber),
        __g_usb_msc_desc_str_iserialnumber
    },
};


/******************************************************************************
 * ƽ̨��ʼ�����������ʼ�������Ѿ��豸��Ϣ
 ******************************************************************************/
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
    am_clk_disable(CLK_USB);                               /* ����USBʱ�� */
}

/**< \brief �豸��Ϣ*/
static const am_usbd_devinfo_t __g_usbd_msc_info = {
        __g_usb_msc_descriptor,                                                 /* ��������ַ */
        sizeof(__g_usb_msc_descriptor) / sizeof(__g_usb_msc_descriptor[0]),     /* ���������� */
};

/**< \brief ����USB�豸��Ϣ */
static const am_zlg227_usbd_devinfo_t  __g_zlg227_usbd_msc_info = {
    ZLG217_USB_BASE,                           /* �Ĵ�������ַ */
    INUM_USB,                                  /* �жϺ� */
    __usb_msc_init,                            /* ƽ̨��ʼ�� */
    __usb_msc_deinit,                          /* ƽ̨ȥ��ʼ�� */
    &__g_usbd_msc_info,
};

/** \brief USB MSC�豸ʵ�� */
static am_usbd_msc_t      __g_usb_msc_dev;

/** \brief AM227 USB�豸ʵ�� */
static am_zlg227_usbd_dev_t  __g_zlg_usbd_msc;

/** \brief MSC ʹ��buff */
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
