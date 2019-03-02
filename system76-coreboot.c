/*
 * System76 ACPI Driver
 *
 * Copyright (C) 2019 System76
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/acpi.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/pci_ids.h>
#include <linux/types.h>

struct system76_data {
	struct acpi_device * acpi_dev;
	struct input_dev * input_dev;
	struct led_classdev ap_led;
	struct led_classdev kb_led;
};

static const struct acpi_device_id device_ids[] = {
	{"17761776", 0},
	{"", 0},
};
MODULE_DEVICE_TABLE(acpi, device_ids);

static int system76_get(struct system76_data * data, char * method) {
	acpi_handle handle = acpi_device_handle(data->acpi_dev);

	unsigned long long ret = 0;
	acpi_status status = acpi_evaluate_integer(handle, method, NULL, &ret);
	if (ACPI_SUCCESS(status)) {
		return (int)ret;
	} else {
		return -1;
	}
}

static int system76_set(struct system76_data * data, char * method, int value) {
	acpi_handle handle = acpi_device_handle(data->acpi_dev);

	union acpi_object obj;
	obj.type = ACPI_TYPE_INTEGER;
	obj.integer.value = value;

	struct acpi_object_list obj_list;
	obj_list.count = 1;
	obj_list.pointer = &obj;

	acpi_status status = acpi_evaluate_object(handle, method, &obj_list, NULL);
	if (ACPI_SUCCESS(status)) {
		return 0;
	} else {
		return -1;
	}
}

static enum led_brightness ap_led_get(struct led_classdev * led) {
	struct system76_data * data = container_of(led, struct system76_data, ap_led);

	int value = system76_get(data, "GAPL");
	if (value > 0) {
		return (enum led_brightness)value;
	} else {
		return LED_OFF;
	}
}

static void ap_led_set(struct led_classdev * led, enum led_brightness value) {
	struct system76_data * data = container_of(led, struct system76_data, ap_led);

	system76_set(data, "SAPL", value == LED_OFF ? 0 : 1);
}

static enum led_brightness kb_led_get(struct led_classdev * led) {
	struct system76_data * data = container_of(led, struct system76_data, kb_led);

	int value = system76_get(data, "GKBL");
	if (value > 0) {
		return (enum led_brightness)value;
	} else {
		return LED_OFF;
	}
}

static void kb_led_set(struct led_classdev * led, enum led_brightness value) {
	struct system76_data * data = container_of(led, struct system76_data, kb_led);

	system76_set(data, "SKBL", (int)value);
}

static void system76_key(struct system76_data * data, unsigned int code) {
	input_report_key(data->input_dev, code, 1);
	input_sync(data->input_dev);
	input_report_key(data->input_dev, code, 0);
	input_sync(data->input_dev);
}

static void system76_notify(struct acpi_device *acpi_dev, u32 event) {
	struct system76_data * data = acpi_driver_data(acpi_dev);

	if (event == 0x80) {
		// Keyboard LED change
		enum led_brightness value = kb_led_get(&data->kb_led);
		led_classdev_notify_brightness_hw_changed(&data->kb_led, value);
	}
}

static int system76_add(struct acpi_device *acpi_dev) {
	int err;

	struct system76_data * data = devm_kzalloc(&acpi_dev->dev, sizeof(*data), GFP_KERNEL);
	if (!data) {
		return -ENOMEM;
	}
	acpi_dev->driver_data = data;
	data->acpi_dev = acpi_dev;

	data->input_dev = devm_input_allocate_device(&acpi_dev->dev);
	if (!data->input_dev) {
		return -ENOMEM;
	}
	data->input_dev->name = "System76 Coreboot ACPI Input";
	data->input_dev->phys = "system76/input0";
	data->input_dev->id.bustype = BUS_HOST;
	set_bit(EV_KEY, data->input_dev->evbit);
	set_bit(KEY_TOUCHPAD_TOGGLE, data->input_dev->keybit);
	err = input_register_device(data->input_dev);
	if (err) {
		return err;
	}

	data->ap_led.name = "system76::airplane";
	data->ap_led.flags = LED_CORE_SUSPENDRESUME;
	data->ap_led.brightness_get = ap_led_get;
	data->ap_led.brightness_set = ap_led_set;
	data->ap_led.max_brightness = 1;
	data->ap_led.default_trigger = "rfkill-none";
	err = devm_led_classdev_register(&acpi_dev->dev, &data->ap_led);
	if (err) {
		return err;
	}

	data->kb_led.name = "system76::kbd_backlight";
	data->kb_led.flags = LED_BRIGHT_HW_CHANGED | LED_CORE_SUSPENDRESUME;
	data->kb_led.brightness_get = kb_led_get;
	data->kb_led.brightness_set = kb_led_set;
	data->kb_led.max_brightness = 5;
	err = devm_led_classdev_register(&acpi_dev->dev, &data->kb_led);
	if (err) {
		return err;
	}

	return 0;
}

static int system76_remove(struct acpi_device *acpi_dev) {
	struct system76_data *data = acpi_driver_data(acpi_dev);

	input_unregister_device(data->input_dev);
	data->input_dev = NULL;

	devm_led_classdev_unregister(&acpi_dev->dev, &data->ap_led);

	devm_led_classdev_unregister(&acpi_dev->dev, &data->kb_led);

	return 0;
}

static struct acpi_driver system76_driver = {
	.name = "System76 ACPI Driver",
	.class = "hotkey",
	.ids = device_ids,
	.ops = {
		.add = system76_add,
		.remove = system76_remove,
		.notify = system76_notify,
	},
};
module_acpi_driver(system76_driver);

MODULE_DESCRIPTION("System76 ACPI Driver");
MODULE_AUTHOR("Jeremy Soller <jeremy@system76.com>");
MODULE_LICENSE("GPL");
