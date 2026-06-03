/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "wlan_hdd_main.h"
#include "wlan_hdd_gpio_wakeup.h"

#if defined(WLAN_GPIO_WAKEUP)

#include <linux/interrupt.h>
#include "wlan_pmo_ucfg_api.h"
#include "oob_wake.h"

static unsigned int
hdd_gpio_wakeup_trigger_to_irqflags(enum pmo_gpio_wakeup_trigger trigger)
{
	switch (trigger) {
	case PMO_GPIO_WAKEUP_MODE_RISING:
		return IRQF_TRIGGER_RISING;
	case PMO_GPIO_WAKEUP_MODE_FALLING:
		return IRQF_TRIGGER_FALLING;
	case PMO_GPIO_WAKEUP_MODE_HIGH:
		return IRQF_TRIGGER_HIGH;
	case PMO_GPIO_WAKEUP_MODE_LOW:
		return IRQF_TRIGGER_LOW;
	default:
		return IRQF_TRIGGER_NONE;
	}
}

int wlan_hdd_gpio_wakeup_init(struct hdd_context *hdd_ctx)
{
	struct gpio_wakeup_cfg cfg = {};
	enum pmo_gpio_wakeup_trigger trigger;

	if (!ucfg_pmo_is_gpio_wakeup_enabled(hdd_ctx->psoc)) {
		hdd_debug("gpio wakeup not enabled");
		return 0;
	}

	cfg.pin = ucfg_pmo_get_gpio_wakeup_pin(hdd_ctx->psoc);
	trigger = ucfg_pmo_get_gpio_wakeup_trigger(hdd_ctx->psoc);
	cfg.irq_trigger = hdd_gpio_wakeup_trigger_to_irqflags(trigger);
	cfg.backend = ucfg_pmo_get_gpio_wakeup_backend(hdd_ctx->psoc);

	return cnss_gpio_wakeup_init(hdd_ctx->parent_dev, &cfg);
}

int wlan_hdd_gpio_wakeup_deinit(struct hdd_context *hdd_ctx)
{
	if (!ucfg_pmo_is_gpio_wakeup_enabled(hdd_ctx->psoc)) {
		hdd_debug("gpio wakeup not enabled");
		return 0;
	}

	return cnss_gpio_wakeup_deinit(hdd_ctx->parent_dev);
}

#endif /* WLAN_GPIO_WAKEUP */
