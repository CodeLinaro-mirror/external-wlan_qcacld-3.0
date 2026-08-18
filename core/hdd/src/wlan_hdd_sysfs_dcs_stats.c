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
 * DOC: wlan_hdd_sysfs_dcs_stats.c
 *
 * Implementation to add sysfs node dcs_stats
 */

#include <wlan_hdd_includes.h>
#include "osif_vdev_sync.h"
#include "wlan_hdd_sysfs.h"
#include "wlan_hdd_sysfs_dcs_stats.h"
#include "wlan_hdd_object_manager.h"
#include "wlan_dcs_ucfg_api.h"
#include "wlan_dcs_public_structs.h"
#include "wlan_objmgr_pdev_obj.h"
#include "wlan_objmgr_vdev_obj.h"
#include "wlan_utility.h"
#include "wlan_reg_services_api.h"
#include "qdf_util.h"

static ssize_t
__hdd_sysfs_dcs_stats_show(struct net_device *net_dev, char *buf)
{
	struct hdd_adapter *adapter = netdev_priv(net_dev);
	struct hdd_context *hdd_ctx;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_channel *ch;
	struct wlan_host_dcs_im_tgt_stats snap = {};
	struct wlan_host_dcs_ch_util_stats util_stats = {};
	QDF_STATUS status;
	uint8_t pdev_id;
	qdf_freq_t freq = 0;
	ssize_t len = 0;
	int32_t rssi, nf;
	int32_t snr = 0;
	int ret;

	ret = hdd_validate_adapter(adapter);
	if (ret)
		return ret;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	if (!wlan_hdd_validate_modules_state(hdd_ctx))
		return -EINVAL;

	vdev = hdd_objmgr_get_vdev_by_user(adapter->deflink, WLAN_OSIF_ID);
	if (!vdev)
		return -ENODEV;

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		len = -ENODEV;
		goto out;
	}

	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);

	ch = wlan_vdev_get_active_channel(vdev);
	if (ch)
		freq = ch->ch_freq;

	status = ucfg_dcs_get_im_stats(hdd_ctx->psoc, pdev_id, &snap);
	if (QDF_IS_STATUS_ERROR(status))
		goto out;

	/*
	 * duty_cycle comes from DCS's own per-window (delta-based) calculation
	 * cached in dcs_ch_util_im_stats.total_cu.
	 */
	ucfg_dcs_get_ch_util(hdd_ctx->psoc, pdev_id, &util_stats);

	/*
	 * last_ack_rssi and chan_nf are transported as A_UINT32 / A_INT32 but
	 * this target sends both as signed int32 dBm values. Cast to int32_t
	 * before arithmetic so the comparison and subtraction stay well-defined
	 * regardless of sign combinations.
	 */
	rssi = (int32_t)snap.last_ack_rssi;
	nf = (int32_t)snap.chan_nf;

	if (rssi != 0 && rssi > nf)
		snr = rssi - nf;

	len += scnprintf(buf + len, PAGE_SIZE - len, "%-24s %u MHz\n",
			 "freq:", freq);
	len += scnprintf(buf + len, PAGE_SIZE - len, "%-24s %d dB\n",
			 "rssi:", rssi);
	len += scnprintf(buf + len, PAGE_SIZE - len, "%-24s %d dB\n",
			 "chan_nf:", nf);
	len += scnprintf(buf + len, PAGE_SIZE - len, "%-24s %d dB\n",
			 "snr:", snr);
	len += scnprintf(buf + len, PAGE_SIZE - len, "%-24s %u\n",
			 "duty_cycle:", util_stats.total_cu);
	len += scnprintf(buf + len, PAGE_SIZE - len, "%-24s %u\n",
			 "tx_retransmission_rate:",
			 snap.tx_retransmission_rate);
	len += scnprintf(buf + len, PAGE_SIZE - len, "%-24s %u\n",
			 "tx_packet_loss_rate:",
			 snap.tx_packet_loss_rate);
	len += scnprintf(buf + len, PAGE_SIZE - len, "%-24s %u\n",
			 "rx_error_rate:",
			 snap.rx_error_rate);
	len += scnprintf(buf + len, PAGE_SIZE - len, "%-24s %u\n",
			 "rx_duplicate_rate:",
			 snap.rx_duplicate_reception_rate);

out:
	hdd_objmgr_put_vdev_by_user(vdev, WLAN_OSIF_ID);
	return len;
}

static ssize_t
hdd_sysfs_dcs_stats_show(struct device *dev,
			 struct device_attribute *attr, char *buf)
{
	struct net_device *net_dev = container_of(dev, struct net_device, dev);
	struct osif_vdev_sync *vdev_sync;
	ssize_t err_size;

	err_size = osif_vdev_sync_op_start(net_dev, &vdev_sync);
	if (err_size)
		return err_size;

	err_size = __hdd_sysfs_dcs_stats_show(net_dev, buf);

	osif_vdev_sync_op_stop(vdev_sync);

	return err_size;
}

static DEVICE_ATTR(dcs_stats, 0444, hdd_sysfs_dcs_stats_show, NULL);

void hdd_sysfs_dcs_stats_create(struct hdd_adapter *adapter)
{
	int error;

	error = device_create_file(&adapter->dev->dev, &dev_attr_dcs_stats);
	if (error)
		hdd_err("could not create dcs_stats sysfs file");
}

void hdd_sysfs_dcs_stats_destroy(struct hdd_adapter *adapter)
{
	device_remove_file(&adapter->dev->dev, &dev_attr_dcs_stats);
}
