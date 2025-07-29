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
 * DOC: wlan_hdd_sysfs_peer_tid_rate.c
 *
 * WLAN Host Device Driver implementation to create sysfs peer_tid_rate
 */

#include <wlan_hdd_includes.h>
#include <wlan_hdd_sysfs_peer_tid_rate.h>
#include "osif_vdev_sync.h"
#include <wlan_hdd_sysfs.h>
#include "wlan_objmgr_vdev_obj_i.h"

#define MAX_TID_VALUE			7
#define MAX_TID_SIZE_FOR_CUSTOM		2
#define MAX_PEER_SIZE_FOR_CUSTOM	1
#define MAX_PEER_TID_HASH_SIZE		1
#define MAX_SYSFS_PEER_TID_RATE_SIZE	256

struct peer_tid_entry {
	bool valid;
	struct wmi_host_peer_tid_rate tid_rate;
};

struct peer_node {
	qdf_list_node_t node;
	uint8_t macaddr[QDF_MAC_ADDR_SIZE];
	uint8_t entry_count;
	struct peer_tid_entry entry[MAX_TID_SIZE_FOR_CUSTOM];
};

struct peer_tid_hash_table {
	qdf_list_t peer_lists[MAX_PEER_TID_HASH_SIZE];
};

struct peer_tid_hash_table peer_tid_table;

static uint8_t hdd_get_addr_hash_key(const uint8_t *macaddr, uint8_t addr_len)
{
	int i;
	uint16_t sum = 0;

	for (i = 0; i < addr_len; i++)
		sum += macaddr[i];

	return sum % MAX_PEER_TID_HASH_SIZE;
}

static struct peer_node *
hdd_find_peer_node(const uint8_t *macaddr)
{
	QDF_STATUS status;
	uint8_t key;
	qdf_list_t *head;
	qdf_list_node_t *p_node;
	struct peer_node *peer = NULL;

	key = hdd_get_addr_hash_key(macaddr, QDF_MAC_ADDR_SIZE);
	head = &peer_tid_table.peer_lists[key];

	status = qdf_list_peek_front(head, &p_node);
	while (QDF_IS_STATUS_SUCCESS(status)) {
		peer = qdf_container_of(p_node, struct peer_node, node);
		if (WLAN_ADDR_EQ(&peer->macaddr, macaddr)
		    == QDF_STATUS_SUCCESS)
			return peer;
		status = qdf_list_peek_next(head, p_node, &p_node);
	}

	hdd_debug_rl("not found peer_node of macaddr: " QDF_MAC_ADDR_FMT,
		     QDF_MAC_ADDR_REF(macaddr));
	return NULL;
}

static struct peer_node *
hdd_add_tid_rate_peer(struct wmi_host_peer_tid_rate *tid_rate)
{
	int i;
	uint8_t key = 0;
	qdf_list_t *head;
	struct peer_node *peer = NULL;

	peer = hdd_find_peer_node(tid_rate->peer_mac);
	if (!peer)
		goto add_peer;

	/* found same mac addr peer, then go find the same tid entry */
	for (i = 0; i < MAX_TID_SIZE_FOR_CUSTOM; i++) {
		if (peer->entry[i].valid &&
		    peer->entry[i].tid_rate.tid == tid_rate->tid) {
			/* entry already exist, just update it */
			qdf_mem_copy(&peer->entry[i].tid_rate,
				     tid_rate, sizeof(*tid_rate));
			return peer;
		}
	}

	/* not found entry in peer node, fill in a new entry */
	for (i = 0; i < MAX_TID_SIZE_FOR_CUSTOM; i++) {
		if (!peer->entry[i].valid) {
			qdf_mem_copy(&peer->entry[i].tid_rate,
				     tid_rate, sizeof(*tid_rate));
			peer->entry[i].valid = true;
			peer->entry_count++;
			hdd_debug_rl("fill new tid: %d mac: " QDF_MAC_ADDR_FMT,
				     tid_rate->tid,
				     QDF_MAC_ADDR_REF(tid_rate->peer_mac));
			return peer;
		}
	}

	/* entry buffer is already full, can't add new tid */
	hdd_debug_rl("tid size exceed max");
	return NULL;

add_peer:
	key = hdd_get_addr_hash_key(tid_rate->peer_mac, QDF_MAC_ADDR_SIZE);
	head = &peer_tid_table.peer_lists[key];
	if (qdf_list_size(head) >= qdf_list_max_size(head)) {
		hdd_err("peer size exceed max, key=%d", key);
		return NULL;
	}

	peer = qdf_mem_malloc(sizeof(*peer));
	if (!peer) {
		hdd_err("malloc menory for peer_node fail");
		return NULL;
	}

	qdf_mem_zero(peer, sizeof(*peer));
	qdf_mem_copy(peer->macaddr, tid_rate->peer_mac, sizeof(peer->macaddr));
	qdf_mem_copy(&peer->entry[0].tid_rate, tid_rate, sizeof(*tid_rate));
	peer->entry[0].valid = true;
	peer->entry_count = 1;
	qdf_list_insert_back(head, &peer->node);
	hdd_debug_rl("add new tid: %d peer: " QDF_MAC_ADDR_FMT,
		     tid_rate->tid, QDF_MAC_ADDR_REF(tid_rate->peer_mac));

	return peer;
}

static QDF_STATUS
hdd_free_empty_peer_node(const uint8_t *macaddr)
{
	QDF_STATUS status;
	uint8_t key;
	qdf_list_t *head;
	qdf_list_node_t *p_node;
	struct peer_node *peer = NULL;

	key = hdd_get_addr_hash_key(macaddr, QDF_MAC_ADDR_SIZE);
	head = &peer_tid_table.peer_lists[key];

	status = qdf_list_peek_front(head, &p_node);
	while (QDF_IS_STATUS_SUCCESS(status)) {
		peer = qdf_container_of(p_node, struct peer_node, node);
		if (peer->entry_count == 0 &&
		    WLAN_ADDR_EQ(&peer->macaddr, macaddr) ==
		    QDF_STATUS_SUCCESS) {
			hdd_debug_rl("remove peer: " QDF_MAC_ADDR_FMT,
				     QDF_MAC_ADDR_REF(peer->macaddr));
			qdf_list_remove_node(head, p_node);
			qdf_mem_free(peer);
			return status;
		}
		status = qdf_list_peek_next(head, p_node, &p_node);
	}

	return QDF_STATUS_E_INVAL;
}

static QDF_STATUS
hdd_remove_tid_rate_peer(uint32_t tid, const uint8_t *macaddr)
{
	int i;
	struct peer_node *peer = NULL;

	peer = hdd_find_peer_node(macaddr);
	if (!peer)
		return QDF_STATUS_E_INVAL;

	for (i = 0; i < MAX_TID_SIZE_FOR_CUSTOM; i++) {
		if (peer->entry[i].valid &&
		    peer->entry[i].tid_rate.tid == tid) {
			peer->entry[i].valid = false;
			qdf_mem_zero(&peer->entry[i],
				     sizeof(peer->entry[i]));
			peer->entry_count--;
			hdd_debug_rl("remove tid: %d, peer: " QDF_MAC_ADDR_FMT,
					tid, QDF_MAC_ADDR_REF(macaddr));
		}
	}

	if (peer->entry_count == 0)
		return hdd_free_empty_peer_node(macaddr);

	return QDF_STATUS_SUCCESS;
}

static void
hdd_update_tid_rate_peer_table(struct wmi_host_peer_tid_rate *tid_rate)
{
	if (!tid_rate->on_off)
		hdd_remove_tid_rate_peer(tid_rate->tid, tid_rate->peer_mac);
	else
		hdd_add_tid_rate_peer(tid_rate);
}

static uint32_t
hdd_dump_tid_rate_peer_table(char *buf, uint32_t size)
{
	uint16_t i, n, m;
	int ret = 0;
	qdf_list_t *head;
	qdf_list_node_t *p_node;
	QDF_STATUS status;
	struct peer_node *peer = NULL;
	struct wmi_host_peer_tid_rate *entry = NULL;

	ret = scnprintf(buf, size, "tid peer_mac_addr     on_off bw retry"
			" rate_num rate_code_x\n");

	for (i = 0; i < MAX_PEER_TID_HASH_SIZE; i++) {
		head = &peer_tid_table.peer_lists[i];

		status = qdf_list_peek_front(head, &p_node);
		while (QDF_IS_STATUS_SUCCESS(status)) {
			peer = qdf_container_of(p_node,
					struct peer_node, node);

			for (m = 0; m < MAX_TID_SIZE_FOR_CUSTOM; m++) {
				if (!peer->entry[m].valid)
					continue;

				entry = &peer->entry[m].tid_rate;
				ret += scnprintf(buf + ret, size - ret,
						 "%-3u " QDF_MAC_ADDR_FMT
						 " %-6u %-2u %-5u %-8u",
						 entry->tid,
						 QDF_MAC_ADDR_REF(
						 entry->peer_mac),
						 entry->on_off,
						 entry->bw,
						 entry->retry_count,
						 entry->num_rate_code);
				for(n = 0; n < entry->num_rate_code; n++)
					ret += scnprintf(
						buf + ret, size - ret,
						" 0x%-X",
						entry->rate_codes[n]);
				ret += scnprintf(buf + ret, size - ret, "\n");
			}
			status = qdf_list_peek_next(head, p_node, &p_node);
		}
	}

	return ret;
}

void hdd_init_tid_rate_peer_table(void)
{
	uint16_t i;

	for (i = 0; i < MAX_PEER_TID_HASH_SIZE; i++)
		qdf_list_create(&peer_tid_table.peer_lists[i],
				MAX_PEER_SIZE_FOR_CUSTOM);
}

void hdd_deinit_and_free_tid_rate_peer_table(void)
{
	uint16_t i;
	struct peer_node *peer;
	qdf_list_t *head;
	qdf_list_node_t *p_node;

	for (i = 0; i < MAX_PEER_TID_HASH_SIZE; i++) {
		head = &peer_tid_table.peer_lists[i];

		while (QDF_IS_STATUS_SUCCESS(
				qdf_list_remove_front(head, &p_node))) {
			peer = qdf_container_of(p_node,
					struct peer_node, node);
			qdf_mem_free(peer);
		}
		qdf_list_destroy(head);
	}
}

static ssize_t
__hdd_sysfs_peer_tid_rate_show(struct net_device *net_dev, char *buf)
{
	return hdd_dump_tid_rate_peer_table(buf, PAGE_SIZE);
}

static ssize_t
hdd_sysfs_peer_tid_rate_show(struct device *dev,
			     struct device_attribute *attr,
			     char *buf)
{
	struct net_device *net_dev = container_of(dev, struct net_device, dev);
	struct osif_vdev_sync *vdev_sync;
	ssize_t err_size;

	err_size = osif_vdev_sync_op_start(net_dev, &vdev_sync);
	if (err_size)
		return err_size;

	err_size = __hdd_sysfs_peer_tid_rate_show(net_dev, buf);

	osif_vdev_sync_op_stop(vdev_sync);

	return err_size;
}

static ssize_t
__hdd_sysfs_peer_tid_rate_store(struct net_device *net_dev,
				char const *buf, size_t count)
{
	struct hdd_adapter *adapter = netdev_priv(net_dev);
	char buf_local[300];
	struct hdd_context *hdd_ctx;
	char *sptr, *token;
	struct wmi_host_peer_tid_rate tid_rate;
	int ret, i;
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

	memset(&tid_rate, 0, sizeof(tid_rate));
	tid_rate.vdev_id = adapter->vdev_id;

	hdd_debug("peer_tid_rate: count %zu buf_local:(%s) net_devname %s",
		  count, buf_local, net_dev->name);

	sptr = buf_local;
	/* Get peer mac address */
	for (i = 0; i < QDF_MAC_ADDR_SIZE; i++) {
		token = strsep(&sptr, " ");
		if (!token) {
			hdd_err_rl("peer mac address error");
			return -EINVAL;
		}
		if (kstrtou8(token, 0, &tid_rate.peer_mac[i])) {
			hdd_err_rl("peer mac address kstrtou8 error");
			return -EINVAL;
		}
	}

	/* Get tid */
	token = strsep(&sptr, " ");
	if (!token) {
		hdd_err_rl("Get tid token fail");
		return -EINVAL;
	}
	if (kstrtou32(token, 0, &tid_rate.tid)) {
		hdd_err_rl("tid kstrtou32 fail");
		return -EINVAL;
	}

	if (tid_rate.tid > MAX_TID_VALUE) {
		hdd_err_rl("Invalid TID value %d, max TID is %d",
			   tid_rate.tid, MAX_TID_VALUE);
		return -EINVAL;
	}

	/* Get on_off */
	token = strsep(&sptr, " ");
	if (!token) {
		hdd_err_rl("Get on_off token fail");
		return -EINVAL;
	}
	if (kstrtou32(token, 0, &tid_rate.on_off)) {
		hdd_err_rl("on_off kstrtou32 fail");
		return -EINVAL;
	}
	if (!tid_rate.on_off)
		goto send;

	/* Get band width */
	token = strsep(&sptr, " ");
	if (!token) {
		hdd_err_rl("Get bw token fail");
		return -EINVAL;
	}
	if (kstrtou32(token, 0, &tid_rate.bw)) {
		hdd_err_rl("bw kstrtou32 fail");
		return -EINVAL;
	}

	if (tid_rate.bw >= CH_WIDTH_INVALID) {
		hdd_err_rl("Invalid band width %d, max band width index is %d",
			   tid_rate.bw, CH_WIDTH_INVALID - 1);
		return -EINVAL;
	}

	/* Get retry count */
	token = strsep(&sptr, " ");
	if (!token) {
		hdd_err_rl("Get retry token fail");
		return -EINVAL;
	}
	if (kstrtou32(token, 0, &tid_rate.retry_count)) {
		hdd_err_rl("retry kstrtou32 fail");
		return -EINVAL;
	}

	/* Get rate_num */
	token = strsep(&sptr, " ");
	if (!token) {
		hdd_err_rl("Get rate_num token fail");
		return -EINVAL;
	}
	if (kstrtou32(token, 0, &tid_rate.num_rate_code)) {
		hdd_err_rl("rate_num kstrtou32 fail");
		return -EINVAL;
	}

	if (tid_rate.num_rate_code > MAX_RATE_CODE_NUM) {
		hdd_err_rl("Invalid rate list len %d, the max len is %d",
			   tid_rate.num_rate_code, MAX_RATE_CODE_NUM);
		return -EINVAL;
	}

	for (i = 0; i < tid_rate.num_rate_code; i++) {
		token = strsep(&sptr, " ");
		if (!token) {
			hdd_err_rl("not enough rate args (%d), expected :%d",
				   i, tid_rate.num_rate_code);
			return -EINVAL;
		}
		if (kstrtou32(token, 0, &tid_rate.rate_codes[i])) {
			hdd_err_rl("rate_codes kstrtou32 fail");
			return -EINVAL;
		}
	}

send:
	hdd_debug_rl("vdev %d tid %u peer " QDF_MAC_ADDR_FMT
		     " on_off %u bw %u retry %u rate_num %u",
		     tid_rate.vdev_id,
		     tid_rate.tid,
		     QDF_MAC_ADDR_REF(tid_rate.peer_mac),
		     tid_rate.on_off,
		     tid_rate.bw,
		     tid_rate.retry_count,
		     tid_rate.num_rate_code);
	status = sme_send_peer_tid_rate_custom_cmd(&tid_rate);
	if (QDF_IS_STATUS_SUCCESS(status))
		hdd_update_tid_rate_peer_table(&tid_rate);
	return count;
}

static ssize_t
hdd_sysfs_peer_tid_rate_store(struct device *dev,
			      struct device_attribute *attr,
			      char const *buf, size_t count)
{
	struct net_device *net_dev = container_of(dev, struct net_device, dev);
	struct osif_vdev_sync *vdev_sync;
	ssize_t errno_size;

	errno_size = osif_vdev_sync_op_start(net_dev, &vdev_sync);
	if (errno_size)
		return errno_size;

	errno_size = __hdd_sysfs_peer_tid_rate_store(net_dev, buf, count);

	osif_vdev_sync_op_stop(vdev_sync);

	return errno_size;
}

static DEVICE_ATTR(peer_tid_rate, 0660,
		       hdd_sysfs_peer_tid_rate_show,
		       hdd_sysfs_peer_tid_rate_store);

int hdd_sysfs_peer_tid_rate_create(struct hdd_adapter *adapter)
{
	int error;

	error = device_create_file(&adapter->dev->dev,
				   &dev_attr_peer_tid_rate);
	if (error)
		hdd_err("could not create tid_rate sysfs file");

	return error;
}

void hdd_sysfs_peer_tid_rate_destroy(struct hdd_adapter *adapter)
{
	device_remove_file(&adapter->dev->dev, &dev_attr_peer_tid_rate);
}
