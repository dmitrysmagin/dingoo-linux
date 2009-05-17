/*
 * JZ Keypad ( 5 x 5 ) Driver
 *
 * Copyright (c) 2005 - 2008  Ingenic Semiconductor Inc.
 *
 * Author: Jason <xwang@ingenic.cn> 20090210
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/version.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/gpio_keys.h>

#include <asm/gpio.h>
#include <asm/jzsoc.h>

#define SCAN_INTERVAL_MS	20
#define SCAN_INTERVAL		(1000000UL * SCAN_INTERVAL_MS / TICK_NSEC)

struct {
	unsigned int gpio;
	unsigned int active_low;
	unsigned int wakeup;
	unsigned int keycode;
	unsigned int state;
} jz_kbd_button [] = {

#ifdef CONFIG_JZ4740_A320
	{ .gpio = 102,	.active_low = 1,	.keycode = KEY_UP },
	{ .gpio = 123,	.active_low = 1,	.keycode = KEY_DOWN },
	{ .gpio = 101,	.active_low = 1,	.keycode = KEY_LEFT },
	{ .gpio = 114,	.active_low = 1,	.keycode = KEY_RIGHT },
	{ .gpio = 96,	.active_low = 1,	.keycode = KEY_D },
	{ .gpio = 97,	.active_low = 1,	.keycode = KEY_C },
	{ .gpio = 115,	.active_low = 1,	.keycode = KEY_S },
	{ .gpio = 98,	.active_low = 1,	.keycode = KEY_X },
	{ .gpio = 110,	.active_low = 1,	.keycode = KEY_A },
	{ .gpio = 111,	.active_low = 1,	.keycode = KEY_Z },
	{ .gpio = 81,	.active_low = 1,	.keycode = KEY_ENTER },
	{ .gpio = 113,	.active_low = 1,	.keycode = KEY_SPACE },
#endif /* CONFIG_JZ4740_A320 */

#ifdef CONFIG_JZ4740_PAVO
	{ .gpio = 96,	.active_low = 1,	.keycode = KEY_1 },
	{ .gpio = 97,	.active_low = 1,	.keycode = KEY_2 },
	{ .gpio = 98,	.active_low = 1,	.keycode = KEY_3 },
	{ .gpio = 99,	.active_low = 1,	.keycode = KEY_4 },
#endif /* CONFIG_JZ4740_PAVO */

};



struct jz_kbd {
	unsigned int keycode[ARRAY_SIZE(jz_kbd_button)];
	struct input_dev *input;

	spinlock_t lock;
	struct timer_list timer;

	unsigned int suspended;
	unsigned long suspend_jiffies;
};
static struct jz_kbd g_jz_kbd;

/*
 *  Call scan function and handle 'GPIO event'(like key down, key up),
 *  and report it to upper layer of input subsystem ... if necessary
 */
static void jz_kbd_scan(struct jz_kbd *kbd_data)
{
	unsigned int i, state;
	unsigned long flags;

	if (kbd_data->suspended)
		return;

	spin_lock_irqsave(&kbd_data->lock, flags);

	for (i = 0; i < ARRAY_SIZE(jz_kbd_button); i++) {
		state = __gpio_get_pin(jz_kbd_button[i].gpio);
		state = !state;

		if (jz_kbd_button[i].state != state) {
			jz_kbd_button[i].state = state;
			input_report_key(kbd_data->input, kbd_data->keycode[i], state);
			input_sync(kbd_data->input);
		}	
	}

	spin_unlock_irqrestore(&kbd_data->lock, flags);
}

static void jz_kbd_timer_callback(unsigned long data)
{
	jz_kbd_scan(&g_jz_kbd);
	mod_timer(&g_jz_kbd.timer, jiffies + SCAN_INTERVAL);
}

#ifdef CONFIG_PM
static int jz_kbd_suspend(struct platform_device *dev, pm_message_t state)
{
	struct jz_kbd *jz_kbd = platform_get_drvdata(dev);
	jz_kbd->suspended = 1;

	return 0;
}

static int jz_kbd_resume(struct platform_device *dev)
{
	struct jz_kbd *jz_kbd = platform_get_drvdata(dev);

	jz_kbd->suspend_jiffies = jiffies;
	jz_kbd->suspended = 0;

	return 0;
}

#else
#define jz_kbd_suspend		NULL
#define jz_kbd_resume		NULL
#endif

/** 
 *  Driver init
 */
static int __init jz_kbd_probe(struct platform_device *dev)
{
	struct input_dev *input_dev;
	int i, error;

	if (SCAN_INTERVAL < 1) {
		printk("jz-gpio-keys: scan interval too small\n");
		return -EINVAL;
	}

	printk("jz-gpio-keys: %lu jiffies scan interval\n", SCAN_INTERVAL);

	input_dev = input_allocate_device();
	if (!input_dev)
		return -ENOMEM;

	platform_set_drvdata(dev, &g_jz_kbd);

	spin_lock_init(&g_jz_kbd.lock);

	g_jz_kbd.suspend_jiffies = jiffies;
	g_jz_kbd.input = input_dev;

	input_dev->private = &g_jz_kbd;
	input_dev->name = "jz-gpio-keys";
	input_dev->phys = "jz-gpio-keys/input0";
	input_dev->cdev.dev = &dev->dev;

	input_dev->id.bustype = BUS_PARPORT;
	input_dev->id.vendor = 0x0001;
	input_dev->id.product = 0x0001;
	input_dev->id.version = 0x0100;

	input_dev->evbit[0] = BIT(EV_KEY) | BIT(EV_REP) | BIT(EV_SYN);
	input_dev->keycode = g_jz_kbd.keycode;
	input_dev->keycodesize = sizeof(unsigned int);
	input_dev->keycodemax = ARRAY_SIZE(jz_kbd_button);

	for (i = 0; i < ARRAY_SIZE(jz_kbd_button); i++) {
		g_jz_kbd.keycode[i] = jz_kbd_button[i].keycode;
		set_bit(g_jz_kbd.keycode[i], input_dev->keybit);
	}

	error = input_register_device(input_dev);
	if (error) {
		pr_err("jz-gpio-keys: unable to register input device\n");
		platform_set_drvdata(dev, NULL);
		input_free_device(input_dev);
		return error;
	}

	init_timer(&g_jz_kbd.timer);
	g_jz_kbd.timer.function = jz_kbd_timer_callback;
	g_jz_kbd.timer.data = (unsigned long)&g_jz_kbd;
	mod_timer(&g_jz_kbd.timer, jiffies + SCAN_INTERVAL);

	return 0;
}

static int jz_kbd_remove(struct platform_device *dev)
{
	struct jz_kbd *kbd = platform_get_drvdata(dev);

	del_timer_sync(&kbd->timer);

	input_unregister_device(kbd->input);

	return 0;
}

static struct platform_driver jz_kbd_driver = {
	.probe	= jz_kbd_probe,
	.remove	= jz_kbd_remove,
	.suspend= jz_kbd_suspend,
	.resume	= jz_kbd_resume,
	.driver	= {
		.name	= "jz-gpio-keys",
	},
};

static struct platform_device jz_kbd_device = {
	.name	= "jz-gpio-keys",
	.id	= -1,
};

static int __init jz_kbd_init(void)
{
	platform_device_register(&jz_kbd_device);
	return platform_driver_register(&jz_kbd_driver);
}

static void __exit jz_kbd_exit(void)
{
	platform_device_unregister(&jz_kbd_device);
	platform_driver_unregister(&jz_kbd_driver);
}

module_init(jz_kbd_init);
module_exit(jz_kbd_exit);

MODULE_AUTHOR("Ignacio Garcia Perez <iggarpe@gmail.com>");
MODULE_DESCRIPTION("JZ GPIO keys driver");
MODULE_LICENSE("GPL");
