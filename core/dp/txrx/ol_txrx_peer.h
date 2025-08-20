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
 * DOC: ol_txrx_peer.h
 * Define the functions for ol txrx peer and mld peer.
 */

#ifndef _OL_TXRX_PEER__H_
#define _OL_TXRX_PEER__H_

#include "ol_txrx_types.h"

#ifdef WLAN_FEATURE_11BE_MLO

/* is MLO connection mld peer */
#define IS_MLO_OL_TXRX_MLD_TXRX_PEER(_peer) ((_peer)->mld_peer)

/* set peer type */
#define OL_TXRX_PEER_SET_TYPE(_peer, _type_val) \
	((_peer)->peer_type = (_type_val))

/* is legacy peer */
#define IS_OL_TXRX_LEGACY_PEER(_peer) \
	((_peer)->peer_type == CDP_LINK_PEER_TYPE && !((_peer)->mld_peer))
/* is MLO connection link peer */
#define IS_MLO_OL_TXRX_LINK_PEER(_peer) \
	((_peer)->peer_type == CDP_LINK_PEER_TYPE && (_peer)->mld_peer)
/* is MLO connection mld peer */
#define IS_MLO_OL_TXRX_MLD_PEER(_peer) \
	((_peer)->peer_type == CDP_MLD_PEER_TYPE)
/* Get Mld peer from link peer */
#define OL_TXRX_GET_MLD_PEER_FROM_PEER(link_peer) \
	((link_peer)->mld_peer)

/**
 * ol_txrx_peer_mlo_setup() - create MLD peer and MLO related initialization
 * @soc_hdl: cdp soc handle
 * @pdev: ol txrx pdev handle
 * @peer: ol txrx peer handle
 * @vdev_id: Vdev ID
 * @setup_info: peer setup information for MLO
 */
QDF_STATUS ol_txrx_peer_mlo_setup(struct cdp_soc_t *soc_hdl,
				  struct ol_txrx_pdev_t *pdev,
				  struct ol_txrx_peer_t *peer,
				  uint8_t vdev_id,
				  struct cdp_peer_setup_info *setup_info);

/**
 * ol_txrx_peer_mlo_delete() - peer MLO related delete operation
 * @peer: ol txrx peer handle
 * Return: None
 */
void ol_txrx_peer_mlo_delete(struct ol_txrx_peer_t *peer);

/**
 * ol_txrx_link_peer_add_mld_peer() - add mld peer pointer to link peer,
 *				      increase mld peer ref_cnt
 * @link_peer: link peer pointer
 * @mld_peer: mld peer pointer
 *
 * Return: none
 */
void ol_txrx_link_peer_add_mld_peer(struct ol_txrx_peer_t *link_peer,
				    struct ol_txrx_peer_t *mld_peer);

/**
 * ol_txrx_link_peer_del_mld_peer() - delete mld peer pointer from link peer,
 *				      decrease mld peer ref_cnt
 * @link_peer: link peer pointer
 *
 * Return: None
 */
void ol_txrx_link_peer_del_mld_peer(struct ol_txrx_peer_t *link_peer);

/**
 * ol_txrx_mld_peer_init_link_peers_info() - init link peers info in mld peer
 * @mld_peer: mld peer pointer
 *
 * Return: None
 */
void ol_txrx_mld_peer_init_link_peers_info(struct ol_txrx_peer_t *mld_peer);

/**
 * ol_txrx_mld_peer_deinit_link_peers_info()
 * - Deinit link peers info in mld peer
 * @mld_peer: mld peer pointer
 *
 * Return: None
 */
void ol_txrx_mld_peer_deinit_link_peers_info(struct ol_txrx_peer_t *mld_peer);

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
				   uint8_t is_bridge_peer);

/**
 * ol_txrx_mld_peer_del_link_peer() - Delete link peer info from MLD peer
 * @mld_peer: MLD dp peer pointer
 * @link_peer: link dp peer pointer
 *
 * Return: number of links left after deletion
 */
uint8_t ol_txrx_mld_peer_del_link_peer(struct ol_txrx_peer_t *mld_peer,
				       struct ol_txrx_peer_t *link_peer);

/**
 * ol_txrx_get_tgt_peer_from_peer() - Get target peer from the given peer
 * @peer: datapath peer
 *
 * Return: MLD peer in case of MLO Link peer
 *	   Peer itself in other cases
 */
static inline
struct ol_txrx_peer_t *ol_txrx_get_tgt_peer_from_peer(
	struct ol_txrx_peer_t *peer)
{
	return IS_MLO_OL_TXRX_LINK_PEER(peer) ? peer->mld_peer : peer;
}

#else

#define IS_MLO_OL_TXRX_MLD_TXRX_PEER(_peer) false

#define OL_TXRX_PEER_SET_TYPE(_peer, _type_val) /* no op */
#define IS_OL_TXRX_LEGACY_PEER(_peer) true
#define IS_MLO_OL_TXRX_LINK_PEER(_peer) false
#define IS_MLO_OL_TXRX_MLD_PEER(_peer) false
#define OL_TXRX_GET_MLD_PEER_FROM_PEER(link_peer) NULL

static inline
QDF_STATUS ol_txrx_peer_mlo_setup(struct cdp_soc_t *soc_hdl,
				  struct ol_txrx_pdev_t *pdev,
				  struct ol_txrx_peer_t *peer,
				  uint8_t vdev_id,
				  struct cdp_peer_setup_info *setup_info)
{
	return QDF_STATUS_SUCCESS;
}

static inline
void ol_txrx_peer_mlo_delete(struct ol_txrx_peer_t *peer)
{
}

static inline
void ol_txrx_link_peer_add_mld_peer(struct ol_txrx_peer_t *link_peer,
                                    struct ol_txrx_peer_t *mld_peer)
{
}

static inline
void ol_txrx_link_peer_del_mld_peer(struct ol_txrx_peer_t *link_peer)
{
}

static inline
void ol_txrx_mld_peer_init_link_peers_info(struct ol_txrx_peer_t *mld_peer)
{
}

static inline
void ol_txrx_mld_peer_deinit_link_peers_info(struct ol_txrx_peer_t *mld_peer)
{
}

static inline
void ol_txrx_mld_peer_add_link_peer(struct ol_txrx_peer_t *mld_peer,
				    struct ol_txrx_peer_t *link_peer,
				    uint8_t is_bridge_peer)
{
}

static inline
uint8_t ol_txrx_mld_peer_del_link_peer(struct ol_txrx_peer_t *mld_peer,
				       struct ol_txrx_peer_t *link_peer)
{
	return 0;
}

static inline
struct ol_txrx_peer_t *ol_txrx_get_tgt_peer_from_peer(
	struct ol_txrx_peer_t *peer)
{
	return peer;
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
				struct ol_txrx_peer_t *peer);

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
				   struct ol_txrx_peer_t *peer);

#if 0
/**
 * dp_peer_cleanup() - Cleanup peer information
 * @vdev: Datapath vdev
 * @peer: Datapath peer
 *
 */
void ol_txrx_peer_cleanup(struct dp_vdev *vdev, struct dp_peer *peer)
{
	if (IS_MLO_DP_LINK_PEER(peer))
		dp_link_peer_del_mld_peer(peer);
	if (IS_MLO_DP_MLD_PEER(peer))
		dp_mld_peer_deinit_link_peers_info(peer);
}
#endif
#endif /* _OL_TXRX_PEER__H_ */
