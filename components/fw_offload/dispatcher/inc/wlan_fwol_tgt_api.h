/*
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: fw offload south bound interface declaration
 */
#ifndef _WLAN_FWOL_TGT_API_H
#define _WLAN_FWOL_TGT_API_H

#include "wlan_fwol_public_structs.h"

#define FWOL_WILDCARD_PDEV_ID   0

/**
 * tgt_fwol_register_ev_handler() - register south bound event handler
 * @psoc: psoc handle
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS tgt_fwol_register_ev_handler(struct wlan_objmgr_psoc *psoc);

/**
 * tgt_fwol_unregister_ev_handler() - unregister south bound event handler
 * @psoc: psoc handle
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS tgt_fwol_unregister_ev_handler(struct wlan_objmgr_psoc *psoc);

/**
 * tgt_fwol_register_rx_ops() - register fw offload rx operations
 * @rx_ops: fps to rx operations
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS tgt_fwol_register_rx_ops(struct wlan_fwol_rx_ops *rx_ops);

/**
 * tgt_fwol_pdev_param_send() - send pdev params to firmware
 * @pdev: pdev handle
 * @pdev_params: pdev params
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS tgt_fwol_pdev_param_send(struct wlan_objmgr_pdev *pdev,
				    struct pdev_params pdev_param);

/**
 * tgt_fwol_vdev_param_send() - send vdev params to firmware
 * @psoc: psoc handle
 * @vdev_set_params: vdev params
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS tgt_fwol_vdev_param_send(struct wlan_objmgr_psoc *psoc,
				    struct vdev_set_params vdev_param);

#ifdef WLAN_FEATURE_TSF_BY_REG
/**
 * tgt_fwol_update_vdev_obj_tsf_info() - update tsf info of fwol_vdev_obj
 * @vdev: pointer to wlan_objmgr_vdev objects
 * @ptsf: pointer to tsf info
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS
tgt_fwol_update_vdev_obj_tsf_info(struct wlan_objmgr_vdev *vdev,
				  uint32_t tsf_id,
				  uint32_t tsf_id_valid,
				  uint32_t mac_id,
				  uint32_t mac_id_valid);

/**
 * tgt_fwol_get_tsf64_reg_val() - get tsf value from mac's tsf register
 * @psoc: psoc handle
 * @mac_id: mac identifier
 * @tsf_id: tsf identifier
 * @value: pointer to tsf 64bit values for return to caller
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS
tgt_fwol_get_tsf64_reg_val(struct wlan_objmgr_psoc *psoc, uint32_t mac_id,
			   uint32_t tsf_id, uint64_t *value);
#else
static inline QDF_STATUS
tgt_fwol_update_vdev_obj_tsf_info(struct wlan_objmgr_vdev *vdev,
				  uint32_t tsf_id,
				  uint32_t tsf_id_valid,
				  uint32_t mac_id,
				  uint32_t mac_id_valid)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_FEATURE_TSF_BY_REG */
#endif /* _WLAN_FWOL_TGT_API_H */
