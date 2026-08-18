/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: wlan_hdd_sysfs_dcs_stats.h
 *
 * Implementation to add sysfs node dcs_stats
 */

#ifndef _WLAN_HDD_SYSFS_DCS_STATS_H
#define _WLAN_HDD_SYSFS_DCS_STATS_H

#if defined(WLAN_SYSFS) && defined(WLAN_DCS_STATS_SYSFS)
/**
 * hdd_sysfs_dcs_stats_create() - create the dcs_stats sysfs node
 * @adapter: hdd adapter
 *
 * usage:
 *      cat /sys/class/net/wlanX/dcs_stats
 *
 * Return: none
 */
void hdd_sysfs_dcs_stats_create(struct hdd_adapter *adapter);

/**
 * hdd_sysfs_dcs_stats_destroy() - remove the dcs_stats sysfs node
 * @adapter: hdd adapter
 *
 * Return: none
 */
void hdd_sysfs_dcs_stats_destroy(struct hdd_adapter *adapter);
#else
static inline void
hdd_sysfs_dcs_stats_create(struct hdd_adapter *adapter)
{
}

static inline void
hdd_sysfs_dcs_stats_destroy(struct hdd_adapter *adapter)
{
}
#endif
#endif /* _WLAN_HDD_SYSFS_DCS_STATS_H */
