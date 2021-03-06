

#ifndef __openair_SCHED_NB_IOT_H__
#define __openair_SCHED_NB_IOT_H__

#include "PHY/defs_eNB.h"
#include "PHY/defs_UE.h"
#include "PHY/defs_L1_NB_IoT.h"
//#include "openair2/PHY_INTERFACE/IF_Module_nb_iot.h"
#include "nfapi_interface.h"

extern uint16_t hundred_times_log10_NPRB_NB_IoT[100];


enum openair_HARQ_TYPE_NB_IoT {
  openair_harq_DL_NB_IoT = 0,
  openair_harq_UL_NB_IoT,
  openair_harq_RA_NB_IoT
};


void process_schedule_rsp_NB_IoT(Sched_Rsp_NB_IoT_t *sched_rsp,
                          		 PHY_VARS_eNB_NB_IoT *eNB,
                          		 eNB_rxtx_proc_NB_IoT_t *proc);


/* For NB-IoT, we put NPBCH in later part, since it would be scheduled by MAC scheduler,this generates NRS/NPSS/NSSS*/
void common_signal_procedures_NB_IoT(PHY_VARS_eNB_NB_IoT *eNB,eNB_rxtx_proc_NB_IoT_t *proc); 

/*Generate the ulsch params and do the mapping for the FAPI style parameters to OAI, and then do the packing*/
void generate_eNB_ulsch_params_NB_IoT(PHY_VARS_eNB_NB_IoT *eNB,eNB_rxtx_proc_NB_IoT_t *proc,nfapi_hi_dci0_request_pdu_t *hi_dci0_pdu);

/*Generate the dlsch params and do the mapping for the FAPI style parameters to OAI, and then do the packing*/
void generate_eNB_dlsch_params_NB_IoT(PHY_VARS_eNB_NB_IoT *eNB,eNB_rxtx_proc_NB_IoT_t * proc,nfapi_dl_config_request_pdu_t *dl_config_pdu);

/*Process all the scheduling result from MAC and also common signals.*/
void phy_procedures_eNB_TX_NB_IoT(PHY_VARS_eNB_NB_IoT     *eNB,eNB_rxtx_proc_NB_IoT_t  *proc,int do_meas);

int8_t find_ue_NB_IoT(uint16_t rnti, PHY_VARS_eNB_NB_IoT *eNB);

NB_IoT_DL_FRAME_PARMS *get_NB_IoT_frame_parms(module_id_t Mod_id, uint8_t CC_id);

int16_t get_hundred_times_delta_IF_eNB_NB_IoT(PHY_VARS_eNB_NB_IoT *phy_vars_eNB,uint8_t UE_id,uint8_t harq_pid, uint8_t bw_factor);

uint32_t is_SIB1_NB_IoT(const frame_t          frameP,
                        long                   schedulingInfoSIB1,   //from the mib
                        int                    physCellId,           //by configuration
                        NB_IoT_eNB_NDLSCH_t   *ndlsch_SIB1
                        );

uint32_t rx_nprach_NB_IoT(PHY_VARS_eNB_NB_IoT *eNB,int frame, uint8_t subframe, uint16_t *rnti, uint16_t *preamble_index, uint16_t *timing_advance);

void npusch_procedures(PHY_VARS_eNB_NB_IoT *eNB,eNB_rxtx_proc_NB_IoT_t *proc);

////////////////// NB-IoT testing ////////////////////
void fill_rx_indication_NB_IoT(PHY_VARS_eNB_NB_IoT *eNB,eNB_rxtx_proc_NB_IoT_t *proc,uint8_t data_or_control, uint8_t decode_flag);

void fill_crc_indication_NB_IoT(PHY_VARS_eNB_NB_IoT *eNB,int UE_id,int frame,int subframe,uint8_t decode_flag);

#endif


