/*
 * Copyright (c) 2011-2012, 2014-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

/**=========================================================================

   \file  rrm_api.h

   \brief RRM APIs

   ========================================================================*/

/* $Header$ */

#ifndef __RRM_API_H__
#define __RRM_API_H__

#define RRM_BCN_RPT_NO_BSS_INFO    0
#define RRM_BCN_RPT_MIN_RPT        1
#define RRM_CH_BUF_LEN             45

QDF_STATUS rrm_initialize(struct mac_context *mac);

/**
 * rrm_cleanup  - cleanup RRM measurement related data for the measurement
 * index
 * @mac: Pointer to mac context
 * @idx: Measurement index
 *
 * Return: None
 */
void rrm_cleanup(struct mac_context *mac, uint8_t idx);

QDF_STATUS rrm_process_link_measurement_request(struct mac_context *mac,
						uint8_t *pRxPacketInfo,
						tDot11fLinkMeasurementRequest
							  *pLinkReq,
						struct pe_session *
							  pe_session);

QDF_STATUS
rrm_process_radio_measurement_request(struct mac_context *mac_ctx,
				      tSirMacAddr peer,
				      tDot11fRadioMeasurementRequest *rrm_req,
				      struct pe_session *session_entry,
				      tpSirMacMgmtHdr pHdr, uint32_t frame_len,
				      uint8_t *pRxPacketInfo);

QDF_STATUS rrm_process_neighbor_report_response(struct mac_context *mac,
						tDot11fNeighborReportResponse
							  *pNeighborRep,
						struct pe_session *
							  pe_session);

QDF_STATUS rrm_send_set_max_tx_power_req(struct mac_context *mac,
					 int8_t txPower,
					 struct pe_session *pe_session);

int8_t rrm_get_mgmt_tx_power(struct mac_context *mac,
			     struct pe_session *pe_session);

void rrm_cache_mgmt_tx_power(struct mac_context *mac,
			     int8_t txPower, struct pe_session *pe_session);

tpRRMCaps rrm_get_capabilities(struct mac_context *mac,
			       struct pe_session *pe_session);

void rrm_get_start_tsf(struct mac_context *mac, uint32_t *pStartTSF);

QDF_STATUS rrm_set_max_tx_power_rsp(struct mac_context *mac,
				    struct scheduler_msg *limMsgQ);

QDF_STATUS
rrm_process_neighbor_report_req(struct mac_context *mac,
				tpSirNeighborReportReqInd pNeighborReq);

QDF_STATUS
rrm_process_beacon_report_xmit(struct mac_context *mac_ctx,
			       tpSirBeaconReportXmitInd beacon_xmit_ind);

/**
 * rrm_process_chan_load_report_xmit() - process channel load report xmit
 * @mac_ctx: Mac context
 * @chan_load_ind: channel load xmit structure
 *
 * Return: None
 */
void
rrm_process_chan_load_report_xmit(struct mac_context *mac_ctx,
				  struct chan_load_xmit_ind *chan_load_ind);

/**
 * rrm_get_country_code_from_connected_profile() - get country code
 * from connected profile
 * @mac: Mac context
 * @vdev_id: vdev_id or csr session id
 * @country_code: country code
 *
 * Return: None
 */
void rrm_get_country_code_from_connected_profile(struct mac_context *mac,
						 uint8_t vdev_id,
						 uint8_t *country_code);
/**
 * rrm_reject_req - Reject rrm request
 * @radiomes_report: radio measurement report
 * @rrm_req: Array of Measurement request IEs
 * @num_report: Num of report
 * @index: Measurement index
 * @measurement_type: Measurement Type
 *
 * Reject the Radio Resource Measurement request, if one is
 * already in progress
 *
 * Return: QDF_STATUS
 */
QDF_STATUS rrm_reject_req(tpSirMacRadioMeasureReport *radiomes_report,
			  tDot11fRadioMeasurementRequest *rrm_req,
			  uint8_t *num_report, uint8_t index,
			  uint8_t measurement_type);

void lim_update_rrm_capability(struct mac_context *mac_ctx);

#ifdef WLAN_SUPPORT_INFRA_CTRL_PATH_STATS
/**
 * rrm_send_sta_stats_req - Send RRM STA STATS request
 * @mac: mac context
 * @session: pe session
 * @peer_mac: peer mac
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
rrm_send_sta_stats_req(struct mac_context *mac,
		       struct pe_session *session,
		       tSirMacAddr peer_mac);
#else
static inline QDF_STATUS
rrm_send_sta_stats_req(struct mac_context *mac,
		       struct pe_session *session,
		       tSirMacAddr peer_mac)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif

/**
 * rrm_process_rrm_sta_stats_request_failure: send RRM STA Stats report with
 * failure
 * @mac: mac context
 * @pe_session: pe session
 * @peer: peer mac
 * @status: failure status
 * @index: index of report
 *
 * Return: void
 */
void
rrm_process_rrm_sta_stats_request_failure(struct mac_context *mac,
					  struct pe_session *pe_session,
					  tSirMacAddr peer,
					  tRrmRetStatus status, uint8_t index);

/**
 * lim_prepare_n_send_dar_req_frame - Prepare and send a Directed
 *                                    Address Request (DAR) frame.
 * @mac_ctx: Pointer to mac context.
 * @session: Pointer to PE session.
 * @dar_req: Pointer to the DAR request structure.
 *
 * This function prepares the necessary frame elements and sends a
 * Directed Address Request (DAR) frame over the air.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
lim_prepare_n_send_dar_req_frame(struct mac_context *mac_ctx,
				 struct pe_session *session,
				 struct sir_sme_dar_frame_req *dar_req);

/**
 * lim_prepare_n_send_dar_rsp_frame - Prepare and send a Directed
 *                                    Address Response (DAR) frame.
 * @mac_ctx: Pointer to mac context.
 * @session: Pointer to PE session.
 * @dar_status: Status of the DAR operation to be included in the response.
 *
 * This function prepares the necessary frame elements and sends a
 * Directed Address Response (DAR) frame over the air, indicating the
 * status of the previously received DAR request.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
lim_prepare_n_send_dar_rsp_frame(struct mac_context *mac_ctx,
				 struct pe_session *session,
				 uint8_t dar_status);

/**
 * lim_prepare_n_send_dar_report_frame - Prepare and send a Directed
 *                                       Address Report (DAR) frame.
 * @mac_ctx: Pointer to mac context.
 * @session: Pointer to PE session.
 * @req: Pointer to the DAR statistics message structure containing
 *       the report data.
 *
 * This function prepares the necessary frame elements and sends a
 * Directed Address Report (DAR) frame over the air, providing
 * statistics related to directed address activity.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
lim_prepare_n_send_dar_report_frame(struct mac_context *mac_ctx,
				    struct pe_session *session,
				    struct sir_sme_dar_stats_msg *req);
#endif
