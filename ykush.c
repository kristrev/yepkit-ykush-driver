/*
 * Yepkit YKUSH driver
 *
 * Copyright (C) 2015 Kristian Evensen (kristian.evensen@gmail.com)
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation, version 2.
 *
 */

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/usb.h>


#define DRIVER_AUTHOR "Kristian Evensen, kristian.evensen@gmail.com"
#define DRIVER_DESC "Yepkit YKUSH driver"

#define CMD_PORT_1	0x01
#define CMD_PORT_2	0x02
#define CMD_PORT_3	0x03
#define CMD_PORT_ALL	0x0A

static const struct usb_device_id id_table[] = {
	{ USB_DEVICE(0x04d8, 0x0042) },
	{ },
};
MODULE_DEVICE_TABLE(usb, id_table);

struct ykush_hub {
	struct usb_device *udev;
	u8 port1;
	u8 port2;
	u8 port3;
	u8 all;
};

static void send_port_cmd(struct ykush_hub *hub, u8 cmd)
{
	int retval = 0, actlength;
	u8 *buffer = kmalloc(6, GFP_KERNEL);

	if (!buffer) {
		dev_err(&hub->udev->dev, "out of memory\n");
		return;
	}

	buffer[0] = cmd;
	buffer[1] = cmd;

	retval = usb_interrupt_msg(hub->udev,
				   usb_sndctrlpipe(hub->udev, 1),
				   buffer,
				   6,
				   &actlength,
				   5000);

	if (retval)
		pr_info("retval = %d\n", retval);

	kfree(buffer);
}

#define show_set(value, cmd) \
static ssize_t show_##value(struct device *dev, struct device_attribute *attr,\
			    char *buf)					\
{									\
	struct usb_interface *intf = to_usb_interface(dev);		\
	struct ykush_hub *hub = usb_get_intfdata(intf);			\
									\
	return sprintf(buf, "%u\n", hub->value);			\
}									\
static ssize_t set_##value(struct device *dev, struct device_attribute *attr,\
			   const char *buf, size_t count)		\
{									\
	struct usb_interface *intf = to_usb_interface(dev);		\
	struct ykush_hub *hub = usb_get_intfdata(intf);			\
	u8 enable = 0, cmd = CMD_##cmd;					\
									\
	if (kstrtou8(buf, 0, &enable) || (enable > 1))			\
		return -EINVAL;						\
									\
	hub->value = enable;						\
									\
	if (enable)							\
		cmd |= 0x10;						\
									\
	send_port_cmd(hub, cmd);					\
	return count;							\
}									\
static DEVICE_ATTR(value, S_IRUGO | S_IWUSR, show_##value, set_##value)
show_set(port1, PORT_1);
show_set(port2, PORT_2);
show_set(port3, PORT_3);
show_set(all, PORT_ALL);

static int ykush_probe(struct usb_interface *interface,
		       const struct usb_device_id *id)
{
	struct usb_device *udev = interface_to_usbdev(interface);
	struct ykush_hub *dev = NULL;
	int retval = -ENOMEM;

	dev = kzalloc(sizeof(struct ykush_hub), GFP_KERNEL);

	if (dev == NULL) {
		dev_err(&interface->dev, "out of memory\n");
		goto error_mem;
	}

	dev->udev = usb_get_dev(udev);
	usb_set_intfdata(interface, dev);

	retval = device_create_file(&interface->dev, &dev_attr_port1);
	if (retval)
		goto error;
	retval = device_create_file(&interface->dev, &dev_attr_port2);
	if (retval)
		goto error;
	retval = device_create_file(&interface->dev, &dev_attr_port3);
	if (retval)
		goto error;
	retval = device_create_file(&interface->dev, &dev_attr_all);
	if (retval)
		goto error;

	dev_info(&interface->dev, "Yepkit YKUSH hub now attached\n");
	return 0;

error:
	device_remove_file(&interface->dev, &dev_attr_port1);
	device_remove_file(&interface->dev, &dev_attr_port2);
	device_remove_file(&interface->dev, &dev_attr_port3);
	device_remove_file(&interface->dev, &dev_attr_all);
	usb_set_intfdata(interface, NULL);
	usb_put_dev(dev->udev);
	kfree(dev);
error_mem:
	return retval;
}

static void ykush_disconnect(struct usb_interface *interface)
{
	struct ykush_hub *dev;

	dev = usb_get_intfdata(interface);

	device_remove_file(&interface->dev, &dev_attr_port1);
	device_remove_file(&interface->dev, &dev_attr_port2);
	device_remove_file(&interface->dev, &dev_attr_port3);
	device_remove_file(&interface->dev, &dev_attr_all);

	usb_set_intfdata(interface, NULL);

	usb_put_dev(dev->udev);

	kfree(dev);

	dev_info(&interface->dev, "Yepkit YKUSH now disconnected\n");
}

static struct usb_driver ykush_driver = {
	.name =		"ykush",
	.probe =	ykush_probe,
	.disconnect =	ykush_disconnect,
	.id_table =	id_table,
};

module_usb_driver(ykush_driver);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");
