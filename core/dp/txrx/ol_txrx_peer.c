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

#include "ol_txrx.h"
#include "ol_txrx_peer.h"
#include "ol_txrx_peer_find.h"

#ifdef WLAN_FEATURE_11BE_MLO
/**
 * ol_txrx_link_peer_add_mld_peer() - add mld peer pointer to link peer,
 *                                    increase mld peer ref_cnt
 * @link_peer: link peer pointer
 * @mld_peer: mld peer pointer
 *
 * Return: none
 */
void ol_txrx_link_peer_add_mld_peer(struct ol_txrx_peer_t *link_peer,
				    struct ol_txrx_peer_t *mld_peer)
{
	/* increase mld_peer ref_cnt */
	ol_txrx_peer_get_ref(mld_peer, PEER_DEBUG_ID_OL_INTERNAL);
	link_peer->mld_peer = mld_peer;
}

/**
 * ol_txrx_link_peer_del_mld_peer() - delete mld peer pointer from link peer,
 *                                    decrease mld peer ref_cnt
 * @link_peer: link peer pointer
 *
 * Return: None
 */
void ol_txrx_link_peer_del_mld_peer(struct ol_txrx_peer_t *link_peer)
{
	/* decrease mld_peer ref_cnt added by link peer add mld peer */
	ol_txrx_peer_release_ref(link_peer->mld_peer,
				 PEER_DEBUG_ID_OL_INTERNAL);
	link_peer->mld_peer = NULL;
}

/**
 * ol_txrx_mld_peer_init_link_peers_info() - init link peers info in mld peer
 * @mld_peer: mld peer pointer
 *
 * Return: None
 */
void ol_txrx_mld_peer_init_link_peers_info(struct ol_txrx_peer_t *mld_peer)
{
	int i;

	qdf_spinlock_create(&mld_peer->link_peers_info_lock);
	mld_peer->num_links = 0;
	for (i = 0; i < OL_TXRX_MAX_MLO_LINKS; i++)
		mld_peer->link_peers[i].is_valid = false;
}

/**
 * ol_txrx_mld_peer_deinit_link_peers_info()
 * - Deinit link peers info in mld peer
 * @mld_peer: mld peer pointer
 *
 * Return: None
 */
void ol_txrx_mld_peer_deinit_link_peers_info(struct ol_txrx_peer_t *mld_peer)
{
	qdf_spinlock_destroy(&mld_peer->link_peers_info_lock);
}


/**
 * ol_txrx_mld_peer_add_link_peer() - add link peer info to mld peer
 * @mld_peer: mld ol txrx peer pointer
 * @link_peer: link ol txrx peer pointer
 * @is_bridge_peer: flag to indicate if peer is bridge peer
 *
 * Return: None
 */
void ol_txrx_mld_peer_add_link_peer(struct ol_txrx_peer_t *mld_peer,
				    struct ol_txrx_peer_t *link_peer,
				    uint8_t is_bridge_peer)
{
	int i;
	struct ol_txrx_peer_link_info *link_peer_info;

	qdf_spin_lock_bh(&mld_peer->link_peers_info_lock);
	for (i = 0; i < OL_TXRX_MAX_MLO_LINKS; i++) {
		link_peer_info = &mld_peer->link_peers[i];
		if (!link_peer_info->is_valid) {
			qdf_mem_copy(link_peer_info->mac_addr.raw,
				     link_peer->mac_addr.raw,
				     QDF_MAC_ADDR_SIZE);
			link_peer_info->is_valid = true;
			link_peer_info->vdev_id = link_peer->vdev->vdev_id;
			link_peer_info->chip_id = 0;
			link_peer_info->is_bridge_peer = is_bridge_peer;
			mld_peer->num_links++;
			break;
		}
	}
	qdf_spin_unlock_bh(&mld_peer->link_peers_info_lock);

	ol_txrx_info("%s addition of link peer %pK (" QDF_MAC_ADDR_FMT ") "
		     "to MLD peer %pK (" QDF_MAC_ADDR_FMT "), "
		     "idx %u num_links %u",
		     (i != OL_TXRX_MAX_MLO_LINKS) ? "Successful" : "Failed",
		     link_peer, QDF_MAC_ADDR_REF(link_peer->mac_addr.raw),
		     mld_peer, QDF_MAC_ADDR_REF(mld_peer->mac_addr.raw),
		     i, mld_peer->num_links);
}

/**
 * ol_txrx_mld_peer_del_link_peer() - Delete link peer info from MLD peer
 * @mld_peer: MLD dp peer pointer
 * @link_peer: link dp peer pointer
 *
 * Return: number of links left after deletion
 */
uint8_t ol_txrx_mld_peer_del_link_peer(struct ol_txrx_peer_t *mld_peer,
				       struct ol_txrx_peer_t *link_peer)
{
	int i;
	struct ol_txrx_peer_link_info *link_peer_info;
	uint8_t num_links;

	qdf_spin_lock_bh(&mld_peer->link_peers_info_lock);
	for (i = 0; i < OL_TXRX_MAX_MLO_LINKS; i++) {
		link_peer_info = &mld_peer->link_peers[i];
		if (link_peer_info->is_valid &&
		    !ol_txrx_peer_find_mac_addr_cmp(&link_peer->mac_addr,
						    &link_peer_info->mac_addr))
		{
			link_peer_info->is_valid = false;
			mld_peer->num_links--;
			break;
		}
	}
	num_links = mld_peer->num_links;
	qdf_spin_unlock_bh(&mld_peer->link_peers_info_lock);

	ol_txrx_info("%s deletion of link peer %pK (" QDF_MAC_ADDR_FMT ") "
		     "from MLD peer %pK (" QDF_MAC_ADDR_FMT "), "
		     "idx %u num_links %u",
		     (i != OL_TXRX_MAX_MLO_LINKS) ? "Successful" : "Failed",
		     link_peer, QDF_MAC_ADDR_REF(link_peer->mac_addr.raw),
		     mld_peer, QDF_MAC_ADDR_REF(mld_peer->mac_addr.raw),
		     i, mld_peer->num_links);

	return num_links;
}
#endif /* WLAN_FEATURE_11BE_MLO */

/**
 * ol_txrx_peer_vdev_list_add() - add peer into vdev's peer list
 * @pdev: ol txrx pdev handle
 * @vdev: ol txrx vdev handle
 * @peer: ol txrx peer handle
 *
 * Return: none
 */
void ol_txrx_peer_vdev_list_add(struct ol_txrx_pdev_t *pdev,
				struct ol_txrx_vdev_t *vdev,
				struct ol_txrx_peer_t *peer)
{
	/* only link peer will be added to vdev peer list */
	if (IS_MLO_OL_TXRX_MLD_PEER(peer))
		return;

	qdf_spin_lock_bh(&vdev->peer_list_lock);

	/* Inc peer ref count when it is added to vdev's peer list */
	if (ol_txrx_peer_get_ref(peer, PEER_DEBUG_ID_OL_INTERNAL) < 0) {
		ol_txrx_err("unable to get peer ref at MAP mac: "
			    QDF_MAC_ADDR_FMT,
			    QDF_MAC_ADDR_REF(peer->mac_addr.raw));
		qdf_spin_unlock_bh(&vdev->peer_list_lock);
		return;
	}

	/* add this peer into the vdev's list */
	if (wlan_op_mode_sta == vdev->opmode)
		TAILQ_INSERT_HEAD(&vdev->peer_list, peer, peer_list_elem);
	else
		TAILQ_INSERT_TAIL(&vdev->peer_list, peer, peer_list_elem);
	vdev->num_peers++;

        qdf_spin_unlock_bh(&vdev->peer_list_lock);
}

/**
 * ol_txrx_peer_vdev_list_remove() - remove peer from vdev's peer list
 * @pdev: ol txrx pdev handle
 * @vdev: ol txrx vdev handle
 * @peer: ol txrx peer handle
 *
 * Return: none
 */
void ol_txrx_peer_vdev_list_remove(struct ol_txrx_pdev_t *pdev,
				   struct ol_txrx_vdev_t *vdev,
				   struct ol_txrx_peer_t *peer)
{
	bool found = false;
	struct ol_txrx_peer_t *tmp_peer;

	/* only link peer was added to vdev peer list */
	if (IS_MLO_OL_TXRX_MLD_PEER(peer))
		return;

	qdf_spin_lock_bh(&vdev->peer_list_lock);

	TAILQ_FOREACH(tmp_peer, &vdev->peer_list, peer_list_elem) {
		if (tmp_peer == peer) {
			found = true;
			break;
		}
	}

	if (found) {
		TAILQ_REMOVE(&vdev->peer_list, peer, peer_list_elem);
		/* Release peer ref cnt inc by vdev list add */
		ol_txrx_peer_release_ref(peer, PEER_DEBUG_ID_OL_INTERNAL);
		vdev->num_peers--;
	} else {
		/*Ignoring the remove operation as peer not found*/
		ol_txrx_dbg("peer:%pK not found in vdev:%pK vdev_id %d",
			    peer, vdev, vdev->vdev_id);
	}
	qdf_spin_unlock_bh(&vdev->peer_list_lock);
}

#ifdef OL_TXRX_PEER_UNMAP_TRACK
/**
 * ol_txrx_peer_unmap_track_timer()
 * - handler when peer unmap track timer expire
 * @arg: argument parameter
 *
 * If timer expire, check 1st node in list, if it still exists
 * in peer_id_object table, trigger crash. restart timer by new
 * timeout value according to 2nd node in list.
 *
 * Return: None
 */
static void ol_txrx_peer_unmap_track_timer(void *arg)
{
	int mod_timeout;
	QDF_STATUS status;
	qdf_list_node_t *node;
	struct ol_txrx_peer_t *peer;
	uint64_t cur_ts, delta_ts;
	struct ol_txrx_peer_unmap_track_elem *elem;
	struct ol_txrx_pdev_t *pdev = (struct ol_txrx_pdev_t *)arg;

	qdf_spin_lock_bh(&pdev->peer_unmap_track_lock);
	/* check 1st element that timer expires */
	status = qdf_list_remove_front(&pdev->peer_unmap_track_list, &node);
	if (status != QDF_STATUS_SUCCESS) {
		ol_txrx_err("1st element not found");
		goto timer_reset;
	}
	qdf_spin_unlock_bh(&pdev->peer_unmap_track_lock);
	elem = (struct ol_txrx_peer_unmap_track_elem *)node;
	peer = ol_txrx_peer_find_get_ref_by_id(pdev, elem->peer_id,
					       PEER_DEBUG_ID_OL_INTERNAL);
	/*
	 * If peer NULL, peer unmapped and same peer id not reused.
	 * If peer is not NULL,
	 * peer != elem->peer, peer unmapped and new different ol_txrx_peer
	 * reuse same peer_id.
	 * peer == elem->peer, but cookie ID mismatch, peer unmaped and
	 * same address peer re-created and mapped with same peer ID.
	 */
	if (!peer || (peer != elem->peer) ||
	    (peer->unmap_track_cookie != elem->unmap_track_cookie))  {
		ol_txrx_dbg("Peer unmap already completed.");
		qdf_mem_free(elem);
		if (peer)
			ol_txrx_peer_release_ref(peer,
						 PEER_DEBUG_ID_OL_INTERNAL);
	} else {
		/* same peer still exist in peer_id_object table, no unmap */
		ol_txrx_err("peer %pK("QDF_MAC_ADDR_FMT")unmap timeout "
			    "id %d, delete timestamp 0x%llx",
			    peer, QDF_MAC_ADDR_REF(peer->mac_addr.raw),
			    peer->peer_id, elem->track_start_time);

		/* It's expected if FW is down */
		if (qdf_is_recovering() || qdf_is_fw_down()) {
			ol_txrx_err("bypass assert as FW is down");
			qdf_mem_free(elem);
			if (peer)
				ol_txrx_peer_release_ref(peer,
					PEER_DEBUG_ID_OL_INTERNAL);
		} else {
			qdf_assert_always(0);
		}
	}

	cur_ts = qdf_get_log_timestamp();
	/* check 2nd element if restart timer needed */
	qdf_spin_lock_bh(&pdev->peer_unmap_track_lock);
	status = qdf_list_peek_front(&pdev->peer_unmap_track_list, &node);
	if (status != QDF_STATUS_SUCCESS)
		goto timer_reset;
	elem = (struct ol_txrx_peer_unmap_track_elem *)node;
	delta_ts = qdf_log_timestamp_to_usecs(cur_ts) -
			qdf_log_timestamp_to_usecs(elem->track_start_time);
	qdf_spin_unlock_bh(&pdev->peer_unmap_track_lock);
	delta_ts /= 1000; /* ms */
	if (delta_ts >= OL_TXRX_PEER_UNMAP_TRACK_TIMEOUT)
		mod_timeout = 1; /* set 1 ms */
	else
		mod_timeout = OL_TXRX_PEER_UNMAP_TRACK_TIMEOUT - delta_ts;
	ol_txrx_dbg("Restart peer unmap timer for peer %pK", elem->peer);
	qdf_timer_mod(&pdev->peer_unmap_track_timer, mod_timeout);
	return;

timer_reset:
	pdev->peer_unmap_track_timer_start = false;
	qdf_spin_unlock_bh(&pdev->peer_unmap_track_lock);
}


/**
 * ol_txrx_peer_unmap_track_update() - update for peer unmap tracking
 * @pdev: ol txrx pdev handle
 * @peer: ol txrx peer handle
 *
 * If peer ID is still valid, then it means this peer has not received
 * unmap before, queue one element into list and start timer to track
 * peer unmap next.
 *
 * Return: None
 */
void ol_txrx_peer_unmap_track_update(struct ol_txrx_pdev_t *pdev,
				     struct ol_txrx_peer_t *peer)
{
	struct ol_txrx_peer_unmap_track_elem *elem;
	bool timer_start = false;

	/* only if peer still mapped */
	if (peer->peer_id == HTT_INVALID_PEER)
		return;

	if (qdf_is_recovering() || qdf_is_fw_down())
		return;

	elem = qdf_mem_malloc(sizeof(*elem));
	if (!elem) {
		ol_txrx_err("failed to allocate memory for unmap tracking");
		return;
	}

	elem->peer = peer;
	elem->peer_id = peer->peer_id;
	elem->unmap_track_cookie = peer->unmap_track_cookie;
	elem->track_start_time = qdf_get_log_timestamp();

	qdf_spin_lock_bh(&pdev->peer_unmap_track_lock);
	qdf_list_insert_back(&pdev->peer_unmap_track_list, &elem->node);
	if (!pdev->peer_unmap_track_timer_start) {
		pdev->peer_unmap_track_timer_start = true;
		timer_start = true;
	}
	qdf_spin_unlock_bh(&pdev->peer_unmap_track_lock);

	/* Start timer if not started */
	if (timer_start) {
		ol_txrx_dbg("start peer unamp timer for peer %pK", peer);
		qdf_timer_mod(&pdev->peer_unmap_track_timer,
			      OL_TXRX_PEER_UNMAP_TRACK_TIMEOUT);
	}
}

/**
 * ol_txrx_peer_unmap_track_init() - Initial ol txrx peer unmap tracking
 * @pdev: ol txrx pdev handle
 *
 * Return: None
 */
void ol_txrx_peer_unmap_track_init(struct ol_txrx_pdev_t *pdev)
{
	pdev->peer_unmap_track_timer_start = false;
	qdf_spinlock_create(&pdev->peer_unmap_track_lock);
	qdf_list_create(&pdev->peer_unmap_track_list, 128);
	qdf_timer_init(pdev->osdev, &pdev->peer_unmap_track_timer,
		       ol_txrx_peer_unmap_track_timer, (void *)pdev,
		       QDF_TIMER_TYPE_WAKE_APPS);
}

/**
 * ol_txrx_peer_unmap_track_deinit() - De-initial peer unmap tracking
 * @pdev: ol txrx pdev handle
 *
 * Return: None
 */
void ol_txrx_peer_unmap_track_deinit(struct ol_txrx_pdev_t *pdev)
{
	struct ol_txrx_peer_unmap_track_elem *elem;
	qdf_list_node_t *node;

	qdf_timer_stop(&pdev->peer_unmap_track_timer);

	qdf_spin_lock_bh(&pdev->peer_unmap_track_lock);
	while (qdf_list_remove_front(&pdev->peer_unmap_track_list, &node) ==
	       QDF_STATUS_SUCCESS) {
		elem = (struct ol_txrx_peer_unmap_track_elem *)node;
		qdf_mem_free(elem);
	}
	qdf_spin_unlock_bh(&pdev->peer_unmap_track_lock);

	qdf_timer_free(&pdev->peer_unmap_track_timer);
	qdf_spinlock_destroy(&pdev->peer_unmap_track_lock);
	qdf_list_destroy(&pdev->peer_unmap_track_list);
}
#endif
