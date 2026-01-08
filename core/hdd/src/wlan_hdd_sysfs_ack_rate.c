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
 * DOC: wlan_hdd_sysfs_ack_rate.c
 *
 * WLAN Host Device Driver implementation to create sysfs aack_rate
 */

#include <wlan_hdd_includes.h>
#include <wlan_hdd_sysfs_ack_rate.h>
#include "osif_vdev_sync.h"
#include <wlan_hdd_sysfs.h>
#include "wlan_objmgr_vdev_obj_i.h"

static uint32_t ack_rate_config = ACK_RATE_DEFAULT; /* defalut setting */
static DEFINE_MUTEX(ack_rate_config_mutex);

void hdd_reset_ack_rate_config(void)
{
	mutex_lock(&ack_rate_config_mutex);
	ack_rate_config = ACK_RATE_DEFAULT;
	mutex_unlock(&ack_rate_config_mutex);
}

static ssize_t
__hdd_sysfs_ack_rate_show(struct net_device *net_dev, char *buf)
{
	int ret = 0;

	/* ACK RATE CODE:
	 * - ACK_RATE_CODE_6MBPS_OFDM (0x003): 6Mbps OFDM ACK rate
	 * - ACK_RATE_CODE_12MBPS_OFDM (0x002): 12Mbps OFDM ACK rate
	 * - ACK_RATE_CODE_24MBPS_OFDM (0x001): 24Mbps OFDM ACK rate
	 * - ACK_RATE_RESTORE_ORIGINAL (0xFFFF): Restore defalut rates
	 */
	mutex_lock(&ack_rate_config_mutex);
	ret = scnprintf(buf, PAGE_SIZE, "%d\n", ack_rate_config);
	mutex_unlock(&ack_rate_config_mutex);

	return ret;
}

static ssize_t
hdd_sysfs_ack_rate_show(struct device *dev,
			struct device_attribute *attr,
			char *buf)
{
	struct net_device *net_dev = container_of(dev, struct net_device, dev);
	struct osif_vdev_sync *vdev_sync;
	ssize_t err_size;

	err_size = osif_vdev_sync_op_start(net_dev, &vdev_sync);
	if (err_size)
		return err_size;

	err_size = __hdd_sysfs_ack_rate_show(net_dev, buf);

	osif_vdev_sync_op_stop(vdev_sync);

	return err_size;
}

static ssize_t
__hdd_sysfs_ack_rate_store(struct net_device *net_dev,
			   char const *buf, size_t count)
{
	struct hdd_adapter *adapter = netdev_priv(net_dev);
	char buf_local[MAX_SYSFS_USER_COMMAND_SIZE_LENGTH + 1];
	struct hdd_context *hdd_ctx;
	uint32_t ack_rate;
	char *sptr, *token;
	int ret;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (hdd_validate_adapter(adapter))
		return -EINVAL;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret != 0)
		return ret;

	if (!wlan_hdd_validate_modules_state(hdd_ctx))
		return -EINVAL;

	ret = hdd_sysfs_validate_and_copy_buf(buf_local, sizeof(buf_local),
					      buf, count);

	if (ret) {
		hdd_err_rl("invalid input");
		return ret;
	}

	hdd_debug("ack_rate: count %zu buf_local:(%s) net_devname %s",
		  count, buf_local, net_dev->name);

	sptr = buf_local;
	/* Get ack_rate value */
	token = strsep(&sptr, " ");
	if (!token) {
		hdd_err_rl("Get ack rate fail");
		return -EINVAL;
	}
	if (kstrtou32(token, 0, &ack_rate))
		return -EINVAL;

	hdd_debug("ack rate=%d", ack_rate);

	status = sme_set_ack_rate(adapter->vdev_id, ack_rate);
	if (QDF_IS_STATUS_ERROR(status))
		return -EINVAL;

	mutex_lock(&ack_rate_config_mutex);
	ack_rate_config = ack_rate;
	mutex_unlock(&ack_rate_config_mutex);

	return count;
}

static ssize_t
hdd_sysfs_ack_rate_store(struct device *dev,
			 struct device_attribute *attr,
			 char const *buf, size_t count)
{
	struct net_device *net_dev = container_of(dev, struct net_device, dev);
	struct osif_vdev_sync *vdev_sync;
	ssize_t errno_size;

	errno_size = osif_vdev_sync_op_start(net_dev, &vdev_sync);
	if (errno_size)
		return errno_size;

	errno_size = __hdd_sysfs_ack_rate_store(net_dev, buf, count);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno_size;
}

static DEVICE_ATTR(ack_rate, 0660,
		   hdd_sysfs_ack_rate_show,
		   hdd_sysfs_ack_rate_store);

int hdd_sysfs_ack_rate_create(struct hdd_adapter *adapter)
{
	int error;

	error = device_create_file(&adapter->dev->dev,
				   &dev_attr_ack_rate);
	if (error)
		hdd_err("could not create tid_rate sysfs file");

	return error;
}

void hdd_sysfs_ack_rate_destroy(struct hdd_adapter *adapter)
{
	device_remove_file(&adapter->dev->dev, &dev_attr_ack_rate);
}
