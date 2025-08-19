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
