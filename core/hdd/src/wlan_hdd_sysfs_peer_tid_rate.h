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
 * DOC: wlan_hdd_sysfs_peer_tid_rate.h
 *
 * implementation for creating sysfs file peer_tid_rate
 */

#ifndef _WLAN_HDD_SYSFS_PEER_TID_RATE_H
#define _WLAN_HDD_SYSFS_PEER_TID_RATE_H

#if defined(WLAN_SYSFS) && defined(WLAN_PEER_TID_RATE_CTRL)
/**
 * hdd_init_tid_rate_peer_table() - API to init peer hash table
 *
 * Return: none
 */
void hdd_init_tid_rate_peer_table(void);

/**
 * hdd_deinit_and_free_tid_rate_peer_table() - API to deinit peer hash table and free menory
 *
 * Return: none
 */
void hdd_deinit_and_free_tid_rate_peer_table(void);

/**
 * hdd_sysfs_peer_tid_rate_create() - API to create tid_rate file
 * @adapter: hdd adapter
 *
 * this file is created per adapter.
 * file path: /sys/class/net/wlan_xx/peer_tid_rate
 *            (wlan_xx is adapter name)
 * usage:
 *      echo <peer_mac> <tid> <on_off> <bw> <retry> <rate_code_num> <rate_code_1>...<rate_code_n> > peer_tid_rate
 * example:
 *      echo 0x11 0x22 0x33 0x44 0x55 0x66 7 1 0 3 2 0x101 0x102 > peer_tid_rate
 *
 * Return: 0 on success and errno on failure
 */
int hdd_sysfs_peer_tid_rate_create(struct hdd_adapter *adapter);

/**
 * hdd_sysfs_peer_tid_rate_destroy() - API to destroy peer_tid_rate sys file
 * @adapter: pointer to adapter
 *
 * Return: none
 */
void hdd_sysfs_peer_tid_rate_destroy(struct hdd_adapter *adapter);
#else
static inline int
hdd_sysfs_peer_tid_rate_create(struct hdd_adapter *adapter)
{
	return 0;
}

static inline void
hdd_sysfs_peer_tid_rate_destroy(struct hdd_adapter *adapter)
{
}

static inline void hdd_init_tid_rate_peer_table(void)
{
}

static inline void hdd_deinit_and_free_tid_rate_peer_table(void)
{
}
#endif
#endif /* #ifndef _WLAN_HDD_SYSFS_PEER_TID_RATE_H */
