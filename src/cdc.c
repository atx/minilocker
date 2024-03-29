/* 
 * Copyright (C) 2010 Gareth McMullin <gareth@blacksphere.co.nz>
 * Copyright (C) 2016 Institute of Applied and Experimental Physics (http://www.utef.cvut.cz/)
 *
 * Derived from libopencm3-examples/stm32/f1/stm32-h103/usb_cdcacm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.*
 */

#include "cdc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/desig.h>
#include <libopencm3/stm32/f0/syscfg.h>
#include <libopencm3/stm32/st_usbfs.h>

#include "comm.h"
#include "prios.h"
#include "utils.h"

#define EP_RX	0x01
#define EP_TX	0x82
#define EP_ITR	0x83

#define TX_MAX_PACKET  64
#define RX_MAX_PACKET  64

static const struct usb_device_descriptor dev = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = USB_CLASS_CDC,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = 64,
	.idVendor = 0x0483,
	.idProduct = 0x5740,
	.bcdDevice = 0x0200,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,
};

/*
 * This notification endpoint isn't implemented. According to CDC spec its
 * optional, but its absence causes a NULL pointer dereference in Linux
 * cdc_acm driver.
 */
static const struct usb_endpoint_descriptor comm_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = EP_ITR,
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = 16,
	.bInterval = 255,
}};

static const struct usb_endpoint_descriptor data_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = EP_RX,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = RX_MAX_PACKET,
	.bInterval = 1,
}, {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = EP_TX,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = TX_MAX_PACKET,
	.bInterval = 1,
}};

static const struct {
	struct usb_cdc_header_descriptor header;
	struct usb_cdc_call_management_descriptor call_mgmt;
	struct usb_cdc_acm_descriptor acm;
	struct usb_cdc_union_descriptor cdc_union;
} __attribute__((packed)) cdcacm_functional_descriptors = {
	.header = {
		.bFunctionLength = sizeof(struct usb_cdc_header_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_HEADER,
		.bcdCDC = 0x0110,
	},
	.call_mgmt = {
		.bFunctionLength =
			sizeof(struct usb_cdc_call_management_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_CALL_MANAGEMENT,
		.bmCapabilities = 0,
		.bDataInterface = 1,
	},
	.acm = {
		.bFunctionLength = sizeof(struct usb_cdc_acm_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_ACM,
		.bmCapabilities = 0,
	},
	.cdc_union = {
		.bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_UNION,
		.bControlInterface = 0,
		.bSubordinateInterface0 = 1,
	 },
};

static const struct usb_interface_descriptor comm_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 1,
	.bInterfaceClass = USB_CLASS_CDC,
	.bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
	.bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
	.iInterface = 0,

	.endpoint = comm_endp,

	.extra = &cdcacm_functional_descriptors,
	.extralen = sizeof(cdcacm_functional_descriptors),
}};

static const struct usb_interface_descriptor data_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 1,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_DATA,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,

	.endpoint = data_endp,
}};

static const struct usb_interface ifaces[] = {{
	.num_altsetting = 1,
	.altsetting = comm_iface,
}, {
	.num_altsetting = 1,
	.altsetting = data_iface,
}};

static const struct usb_config_descriptor config = {
	.bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = 0,
	.bNumInterfaces = 2,
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	.bmAttributes = 0x80,
	.bMaxPower = 0x32,

	.interface = ifaces,
};
static uint8_t usb_control_buffer[128];

static int cdcacm_control_request(usbd_device *usbd_dev, struct usb_setup_data *req, uint8_t **buf,
		uint16_t *len, void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
	(void)complete;
	(void)buf;
	(void)usbd_dev;
	(void)req;
	(void)len;
	return 0;
}

static void cdcacm_data_rx_cb(usbd_device *usbd_dev, uint8_t ep)
{
	(void)ep;
	(void)usbd_dev;

	char buf[64];
	int len = usbd_ep_read_packet(usbd_dev, EP_RX, buf, 64);

	comm_push_rx(buf, len);

	(void)len;
}

static void cdcacm_set_config(usbd_device *usbd_dev, uint16_t wValue)
{
	(void)wValue;
	(void)usbd_dev;

	usbd_ep_setup(usbd_dev, EP_RX, USB_ENDPOINT_ATTR_BULK, 64, cdcacm_data_rx_cb);
	usbd_ep_setup(usbd_dev, EP_TX, USB_ENDPOINT_ATTR_BULK, 64, NULL);
	usbd_ep_setup(usbd_dev, EP_ITR, USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL);

	usbd_register_control_callback(
				usbd_dev,
				USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE,
				USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
				cdcacm_control_request);
}

static usbd_device *usbd_dev;

inline static void cdc_poll()
{
	usbd_poll(usbd_dev);
}

void usb_isr()
{
	cdc_poll();
}

static const char usb_string_manufacturer[] = "Josef Gajdusek <atx@atx.name>";
static const char usb_string_product[] = "MiniLocker";
static const char usb_string_serno[] = SERSTR;

static const char *usb_strings[] = {
	usb_string_manufacturer,
	usb_string_product,
	usb_string_serno,
};

static char tx_buffer[TX_MAX_PACKET];
static int tx_buffer_cnt;

int cdc_flush()
{
	int ret = usbd_ep_write_packet(usbd_dev, EP_TX, tx_buffer, tx_buffer_cnt);
	tx_buffer_cnt = 0;
	return ret;
}

int cdc_send(char *buf, int len)
{
	// Note that this will only send TX_MAX_PACKETLEN * 2
	int tc = min(TX_MAX_PACKET - tx_buffer_cnt, len);
	int ntc = 0;
	memcpy(&tx_buffer[tx_buffer_cnt], buf, tc);
	tx_buffer_cnt += tc;
	if (tx_buffer_cnt == TX_MAX_PACKET) {
		ntc = min(len - tc, TX_MAX_PACKET);
		if (cdc_flush() && ntc > 0) {
			memcpy(&tx_buffer[tx_buffer_cnt], &buf[tc], ntc);
			tx_buffer_cnt += ntc;
		}
	}
	return tc + ntc;
}

void cdc_init()
{
	SYSCFG_CFGR1 |= SYSCFG_CFGR1_PA11_PA12_RMP;

	rcc_set_usbclk_source(RCC_HSI48);

	usbd_dev = usbd_init(&st_usbfs_v2_usb_driver, &dev, &config, usb_strings, 3,
						 usb_control_buffer, sizeof(usb_control_buffer));

	usbd_register_set_config_callback(usbd_dev, cdcacm_set_config);

	nvic_set_priority(NVIC_USB_IRQ, PRIO_USB);
	nvic_enable_irq(NVIC_USB_IRQ);
}
