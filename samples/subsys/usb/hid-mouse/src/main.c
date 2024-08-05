/*
 * Copyright (c) 2018 qianfan Zhao
 * Copyright (c) 2018, 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sample_usbd.h>

#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/input/input.h>
#include <zephyr/sys/util.h>

#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/usb/class/usb_hid.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define HID_MOUSE_REPORT_DESC1(bcnt)                                                                             \
    {                                                                                                                 \
        HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP), HID_USAGE(HID_USAGE_GEN_DESKTOP_MOUSE),                                \
            HID_COLLECTION(HID_COLLECTION_APPLICATION), HID_USAGE(HID_USAGE_GEN_DESKTOP_POINTER),                     \
            HID_COLLECTION(HID_COLLECTION_PHYSICAL), HID_REPORT_COUNT(bcnt), HID_REPORT_SIZE(1), HID_LOGICAL_MIN8(0), \
            HID_LOGICAL_MAX8(1), HID_USAGE_PAGE(HID_USAGE_GEN_BUTTON), HID_USAGE_MIN8(1), HID_USAGE_MAX8(bcnt),       \
            HID_INPUT(0x02), HID_REPORT_COUNT(2), HID_REPORT_SIZE(16), 0x16, 0x01, 0x80, 0x26, 0xff, 0x7F,            \
            HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP), HID_USAGE(HID_USAGE_GEN_DESKTOP_X),                                \
            HID_USAGE(HID_USAGE_GEN_DESKTOP_Y), HID_INPUT(6), HID_REPORT_COUNT(1), HID_REPORT_SIZE(8), 0x15, 0x81,    \
            HID_LOGICAL_MAX8(127), HID_USAGE(HID_USAGE_GEN_DESKTOP_WHEEL), 0x81, 0x06, 0x95, 0x01, 0x05, 0x0c, 0x0a,  \
            0x38, 0x02, 0x81, 0x06, HID_END_COLLECTION, 0x06, 0x00, 0xff, 0x09, 0xf1, HID_REPORT_SIZE(8),             \
            HID_REPORT_COUNT(5), HID_LOGICAL_MIN8(0), HID_LOGICAL_MAX8(255), 0x81, 0x00, 0x06, 0x01,                  \
            0xff,             /*   USAGE_PAGE (Vendor Defined Page 2) */                                              \
            0x09, 0x02,       /*   USAGE (Vendor Usage 2) */                                                          \
            0x95, 0xFF,       /*   REPORT_COUNT (7) */                                                                \
            0x81, 0x02,       /*   OUTPUT (Data,Var,Abs) */                                                           \
            0x06, 0x01, 0xff, /*   USAGE_PAGE (Vendor Defined Page 2) */                                              \
            0x09, 0x03,       /*   USAGE (Vendor Usage 2) */                                                          \
            0x95, 0xF4,       /*   REPORT_COUNT (7) */                                                                \
            0x81, 0x02,       /*   OUTPUT (Data,Var,Abs) */                                                           \
                                                                                                                      \
            0x06, 0x01, 0xff, /*   USAGE_PAGE (Vendor Defined Page 2) */                                              \
            0x09, 0x04,       /*   USAGE (Vendor Usage 2) */                                                          \
            0x95, 0xFF,       /*   REPORT_COUNT (7) */                                                                \
            0x81, 0x02,       /*   OUTPUT (Data,Var,Abs) */                                                           \
            0x06, 0x01, 0xff, /*   USAGE_PAGE (Vendor Defined Page 2) */                                              \
            0x09, 0x04,       /*   USAGE (Vendor Usage 2) */                                                          \
            0x95, 0xFF,       /*   REPORT_COUNT (7) */                                                                \
            0x81, 0x02,       /*   OUTPUT (Data,Var,Abs) */                                                           \
            0x06, 0x01, 0xff, /*   USAGE_PAGE (Vendor Defined Page 2) */                                              \
            0x09, 0x04,       /*   USAGE (Vendor Usage 2) */                                                          \
            0x95, 0x02,       /*   REPORT_COUNT (7) */                                                                \
            0x81, 0x02,       /*   OUTPUT (Data,Var,Abs) */                                                           \
            0x06, 0x01, 0xff, /*   USAGE_PAGE (Vendor Defined Page 2) */                                              \
            0x09, 0x05,       /*   USAGE (Vendor Usage 2) */                                                          \
            0x95, 0xFF,       /*   REPORT_COUNT (7) */                                                                \
            0x81, 0x02,       /*   OUTPUT (Data,Var,Abs) */                                                           \
            0x06, 0x01, 0xff, /*   USAGE_PAGE (Vendor Defined Page 2) */                                              \
            0x09, 0x06,       /*   USAGE (Vendor Usage 2) */                                                          \
            0x95, 0xFF,       /*   REPORT_COUNT (7) */                                                                \
            0x81, 0x02,       /*   OUTPUT (Data,Var,Abs) */                                                           \
            0x06, 0x01, 0xff, /*   USAGE_PAGE (Vendor Defined Page 2) */                                              \
            0x09, 0x07,       /*   USAGE (Vendor Usage 2) */                                                          \
            0x95, 0xFF,       /*   REPORT_COUNT (7) */                                                                \
            0x81, 0x02,       /*   OUTPUT (Data,Var,Abs) */                                                           \
            0x06, 0x01, 0xff, /*   USAGE_PAGE (Vendor Defined Page 2) */                                              \
            0x09, 0x08,       /*   USAGE (Vendor Usage 2) */                                                          \
            0x95, 0xFF,       /*   REPORT_COUNT (7) */                                                                \
            0x81, 0x02,       /*   OUTPUT (Data,Var,Abs) */                                                           \
            0x06, 0x01, 0xff, /*   USAGE_PAGE (Vendor Defined Page 2) */                                              \
            0x09, 0x05,       /*   USAGE (Vendor Usage 2) */                                                          \
            0x95, 0x04,       /*   REPORT_COUNT (7) */                                                                \
            0x81, 0x02,       /*   OUTPUT (Data,Var,Abs) */                                                           \
\
            0x06, 0x01, 0xff, /*   USAGE_PAGE (Vendor Defined Page 2) */                                              \
            0x09, 0x05,       /*   USAGE (Vendor Usage 2) */                                                          \
            0x95, 0xFF,       /*   REPORT_COUNT (7) */                                                                \
            0x81, 0x02,       /*   OUTPUT (Data,Var,Abs) */                                                           \
            0x06, 0x01, 0xff, /*   USAGE_PAGE (Vendor Defined Page 2) */                                              \
            0x09, 0x06,       /*   USAGE (Vendor Usage 2) */                                                          \
            0x95, 0xFF,       /*   REPORT_COUNT (7) */                                                                \
            0x81, 0x02,       /*   OUTPUT (Data,Var,Abs) */                                                           \
            0x06, 0x01, 0xff, /*   USAGE_PAGE (Vendor Defined Page 2) */                                              \
            0x09, 0x07,       /*   USAGE (Vendor Usage 2) */                                                          \
            0x95, 0xFF,       /*   REPORT_COUNT (7) */                                                                \
            0x81, 0x02,       /*   OUTPUT (Data,Var,Abs) */                                                           \
            0x06, 0x01, 0xff, /*   USAGE_PAGE (Vendor Defined Page 2) */                                              \
            0x09, 0x08,       /*   USAGE (Vendor Usage 2) */                                                          \
            0x95, 0xFF,       /*   REPORT_COUNT (7) */                                                                \
            0x81, 0x02,       /*   OUTPUT (Data,Var,Abs) */                                                           \
            0x06, 0x01, 0xff, /*   USAGE_PAGE (Vendor Defined Page 2) */                                              \
            0x09, 0x05,       /*   USAGE (Vendor Usage 2) */                                                          \
            0x95, 0x04,       /*   REPORT_COUNT (7) */                                                                \
            0x81, 0x02,       /*   OUTPUT (Data,Var,Abs) */                                                           \
            HID_END_COLLECTION,                                                                                       \
    }


static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const uint8_t hid_report_desc[] =  HID_MOUSE_REPORT_DESC1(16);
static enum usb_dc_status_code usb_status;

#define MOUSE_BTN_LEFT		0
#define MOUSE_BTN_RIGHT		1

enum mouse_report_idx {
	MOUSE_BTN_REPORT_IDX = 0,
	MOUSE_X_REPORT_IDX = 1,
	MOUSE_Y_REPORT_IDX = 2,
	MOUSE_WHEEL_REPORT_IDX = 3,
	MOUSE_REPORT_COUNT = 3072,
};

static uint8_t __aligned(sizeof(void *)) report[MOUSE_REPORT_COUNT];
static K_SEM_DEFINE(report_sem, 0, 1);

static inline void status_cb(enum usb_dc_status_code status, const uint8_t *param)
{
	usb_status = status;
}

static ALWAYS_INLINE void rwup_if_suspended(void)
{
	if (IS_ENABLED(CONFIG_USB_DEVICE_REMOTE_WAKEUP)) {
		if (usb_status == USB_DC_SUSPEND) {
			usb_wakeup_request();
			return;
		}
	}
}

static void input_cb(struct input_event *evt)
{
	static uint8_t tmp[MOUSE_REPORT_COUNT];

	switch (evt->code) {
	case INPUT_KEY_0:
		rwup_if_suspended();
		WRITE_BIT(tmp[MOUSE_BTN_REPORT_IDX], MOUSE_BTN_LEFT, evt->value);
		break;
	case INPUT_KEY_1:
		rwup_if_suspended();
		WRITE_BIT(tmp[MOUSE_BTN_REPORT_IDX], MOUSE_BTN_RIGHT, evt->value);
		break;
	case INPUT_KEY_2:
		if (evt->value) {
			tmp[MOUSE_X_REPORT_IDX] += 10U;
		}

		break;
	case INPUT_KEY_3:
		if (evt->value) {
			tmp[MOUSE_Y_REPORT_IDX] += 10U;
		}

		break;
	default:
		LOG_INF("Unrecognized input code %u value %d",
			evt->code, evt->value);
		return;
	}

	if (memcmp(tmp, report, sizeof(tmp))) {
		memcpy(report, tmp, sizeof(report));
		k_sem_give(&report_sem);
	}
}

INPUT_CALLBACK_DEFINE(NULL, input_cb);

#if defined(CONFIG_USB_DEVICE_STACK_NEXT)
static int enable_usb_device_next(void)
{
	struct usbd_context *sample_usbd;
	int err;

	sample_usbd = sample_usbd_init_device(NULL);
	if (sample_usbd == NULL) {
		LOG_ERR("Failed to initialize USB device");
		return -ENODEV;
	}

	err = usbd_enable(sample_usbd);
	if (err) {
		LOG_ERR("Failed to enable device support");
		return err;
	}

	LOG_DBG("USB device support enabled");

	return 0;
}
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK_NEXT) */

int main(void)
{
	const struct device *hid_dev,*hid_dev1,*hid_dev2;
	int ret;

	if (!gpio_is_ready_dt(&led0)) {
		LOG_ERR("LED device %s is not ready", led0.port->name);
		return 0;
	}

#if defined(CONFIG_USB_DEVICE_STACK_NEXT)
	// hid_dev = DEVICE_DT_GET_ONE(zephyr_hid_device);
	hid_dev = device_get_binding("hid_dev_0");
	hid_dev1 = device_get_binding("hid_dev_1");
	hid_dev2 = device_get_binding("hid_dev_2");
#else
	hid_dev = device_get_binding("HID_0");
#endif
	if (hid_dev == NULL) {
		LOG_ERR("Cannot get USB HID Device");
		return 0;
	}

	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT);
	if (ret < 0) {
		LOG_ERR("Failed to configure the LED pin, error: %d", ret);
		return 0;
	}

	usb_hid_register_device(hid_dev,
				hid_report_desc, sizeof(hid_report_desc),
				NULL);
	usb_hid_register_device(hid_dev1,
				hid_report_desc, sizeof(hid_report_desc),
				NULL);
	usb_hid_register_device(hid_dev2, hid_report_desc, sizeof(hid_report_desc), NULL);				
	usb_hid_init(hid_dev);
	usb_hid_init(hid_dev1);
	usb_hid_init(hid_dev2);
#if defined(CONFIG_USB_DEVICE_STACK_NEXT)
	ret = enable_usb_device_next();
#else
	ret = usb_enable(status_cb);
#endif
	if (ret != 0) {
		LOG_ERR("Failed to enable USB");
		return 0;
	}

	while (true) {
		// k_sem_take(&report_sem, K_FOREVER);

		ret = hid_int_ep_write(hid_dev, report, 64, NULL);
		ret = hid_int_ep_write(hid_dev1, report, sizeof(report), NULL);
		ret = hid_int_ep_write(hid_dev2, report, 64, NULL);
		if (ret) {
			LOG_ERR("HID write error, %d", ret);
		}

		/* Toggle LED on sent report */
		ret = gpio_pin_toggle(led0.port, led0.pin);
		if (ret < 0) {
			LOG_ERR("Failed to toggle the LED pin, error: %d", ret);
		}
	}
	return 0;
}