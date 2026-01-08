/*
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

/**
 * DOC: wlan_hdd_sysfs_ack_rate.h
 *
 * implementation for creating sysfs file ack_rate
 */

#ifndef _WLAN_HDD_SYSFS_ACK_RATE_H
#define _WLAN_HDD_SYSFS_ACK_RATE_H

#if defined(WLAN_SYSFS) && defined(WLAN_ACK_RATE_CTRL)
/**
 * hdd_reset_ack_rate_config() - API to reset ack rate config cache to default
 *
 */
void hdd_reset_ack_rate_config(void);

/**
 * hdd_sysfs_ack_rate_create() - API to create ack_rate file
 * @adapter: hdd adapter
 *
 * this file is created per adapter.
 * file path: /sys/class/net/wlan_xx/ack_rate
 *            (wlan_xx is adapter name)
 * usage:
 *      echo <ack_rate_value> > ack_rate
 * example:
 *      echo 0x2 > ack_rate
 *
 * Return: 0 on success and errno on failure
 */
int hdd_sysfs_ack_rate_create(struct hdd_adapter *adapter);

/**
 * hdd_sysfs_ack_rate_destroy() - API to destroy ack_rate sys file
 * @adapter: pointer to adapter
 *
 * Return: none
 */
void hdd_sysfs_ack_rate_destroy(struct hdd_adapter *adapter);
#else
static inline void hdd_reset_ack_rate_config(void)
{
}

static inline int
hdd_sysfs_ack_rate_create(struct hdd_adapter *adapter)
{
	return 0;
}

static inline void
hdd_sysfs_ack_rate_destroy(struct hdd_adapter *adapter)
{
}
#endif
#endif /* #ifndef _WLAN_HDD_SYSFS_ACK_RATE_H */
