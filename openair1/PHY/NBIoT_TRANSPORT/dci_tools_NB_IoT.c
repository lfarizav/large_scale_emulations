/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.0  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

/*! \file PHY/LTE_TRANSPORT/dci_tools.c
 * \brief PHY Support routines (eNB/UE) for filling PDSCH/PUSCH/DLSCH/ULSCH data structures based on DCI PDUs generated by eNB MAC scheduler.
 * \author R. Knopp
 * \date 2011
 * \version 0.1
 * \company Eurecom
 * \email: knopp@eurecom.fr
 * \note
 * \warning
 */
//#include "PHY/defs.h"
#include "PHY/impl_defs_lte_NB_IoT.h"
#include "PHY/extern_NB_IoT.h"
//#include "PHY/LTE_TRANSPORT/extern_NB_IoT.h"
#include "defs_NB_IoT.h"
/*
#ifdef DEBUG_DCI_TOOLS
#include "PHY/vars_NB_IoT.h"
#endif
*/
//#include "assertions.h"
//#include "dlsch_tbs_full.h"
#include "dlsch_tbs_full_NB_IoT.h"

//#define DEBUG_HARQ

//#include "LAYER2/MAC/extern_NB_IoT.h"
//#include "LAYER2/MAC/defs_NB_IoT.h"
#include "PHY/defs_L1_NB_IoT.h"

//#define DEBUG_DCI

void add_dci_NB_IoT(DCI_PDU_NB_IoT *DCI_pdu,void *pdu,rnti_t rnti,unsigned char dci_size_bytes,unsigned char aggregation,unsigned char dci_size_bits,unsigned char dci_fmt, uint8_t npdcch_start_symbol)
{
	//put the pdu
  memcpy(&DCI_pdu->dci_alloc[DCI_pdu->Num_dci].dci_pdu[0],pdu,dci_size_bytes);
  //configure the dci alloc
  DCI_pdu->dci_alloc[DCI_pdu->Num_dci].dci_length      = dci_size_bits;
  DCI_pdu->dci_alloc[DCI_pdu->Num_dci].L               = aggregation;
  DCI_pdu->dci_alloc[DCI_pdu->Num_dci].rnti            = rnti;
  DCI_pdu->dci_alloc[DCI_pdu->Num_dci].format          = dci_fmt;
  DCI_pdu->npdcch_start_symbol                         = npdcch_start_symbol;

  DCI_pdu->Num_dci++;

  LOG_D(MAC,"add ue specific dci format %d for rnti %x \n",dci_fmt,rnti);
}

//map the Isf (DCI param) to the number of subframes (Nsf)
int resource_to_subframe[8] = {1,2,3,4,5,6,8,10};

int Scheddly_less_128[8] = {0,4,8,12,16,32,64,128};
int Scheddly_bigger_128[8] = {0,16,32,64,128,256,512,1024};
int Irep_to_Nrep[16] = {1,2,4,8,16,32,64,128,192,256,384,512,768,1024,1536,2048};


int Idelay_to_K0(uint8_t Sched_delay, int Rmax)
{
  int k0=0;

  if(Rmax <128)
  {
    k0 = Scheddly_less_128[Sched_delay];
  }else if(Rmax >=128)
  {
    k0 = Scheddly_bigger_128[Sched_delay];
  } 
  return k0;
}


int DCIrep_to_real_rep(uint8_t DCI_rep, int Rmax)
{
    int R=0;
    if(Rmax == 1)
    {
      if(DCI_rep == 0)
        R = 1;
    }else if (Rmax == 2)
    {
      if(DCI_rep == 0)
        R = 1;
      else if(DCI_rep == 1)
        R = 2;
    }else if (Rmax == 4)
    {
      if(DCI_rep == 0)
        R = 1;
      else if(DCI_rep == 1)
        R = 2;
        else if(DCI_rep == 2)
        R = 4;
    }else if (Rmax >= 8)
    {
      if(DCI_rep == 0)
        R = Rmax/8;
      else if(DCI_rep == 1)
        R = Rmax/4;
      else if(DCI_rep == 2)
        R = Rmax/2;
      else if(DCI_rep == 3)
        R = Rmax;
    }

    return R;
}


int generate_eNB_ulsch_params_from_dci_NB_IoT(PHY_VARS_eNB_NB_IoT            *eNB,
                                              int                     frame,
                                              uint8_t                 subframe,
                                              DCI_CONTENT             *DCI_Content,
                                              uint16_t                rnti,
                                              NB_IoT_eNB_NPDCCH_t     *ndlcch,
                                              uint8_t                 aggregation,
                                              uint8_t                 npdcch_start_symbol,
                                              uint8_t                 ncce_index)
{

  int tmp = 0;
  int i = 0;
  uint8_t  *DCI_flip = NULL;
  ncce_index = 0;

  /// type = 0 => DCI Format N0, type = 1 => DCI Format N1, 1 bits
  uint8_t type;
  /// Subcarrier indication, 6 bits
  uint8_t scind;
  /// Resourse Assignment (RU Assignment), 3 bits
  uint8_t ResAssign;
  /// Modulation and Coding Scheme, 4 bits
  uint8_t mcs;
  /// New Data Indicator, 1 bits
  uint8_t ndi;
  /// Scheduling Delay, 2 bits
  uint8_t Scheddly;
  /// Repetition Number, 3 bits
  uint8_t RepNum;
  /// Redundancy version for HARQ (only use 0 and 2), 1 bits
  uint8_t rv;
  /// DCI subframe repetition Number, 2 bits
  uint8_t DCIRep;
  
      
  type        = DCI_Content->DCIN0.type;
  scind       = DCI_Content->DCIN0.scind;
  ResAssign   = DCI_Content->DCIN0.ResAssign;
  mcs         = DCI_Content->DCIN0.mcs;
  ndi         = DCI_Content->DCIN0.ndi;
  Scheddly    = DCI_Content->DCIN0.Scheddly;
  RepNum      = DCI_Content->DCIN0.RepNum;
  rv          = DCI_Content->DCIN0.rv;
  DCIRep      = DCI_Content->DCIN0.DCIRep;

  /*Now configure the npdcch structure*/

  // ndlcch->ncce_index          =   NCCE_index;
  // ndlcch->aggregation_level   =   aggregation;

  ndlcch->A[ncce_index]               = sizeof_DCIN0_t; // number of bits in DCI

  ndlcch->rnti[ncce_index] = rnti; //we store the RNTI (e.g. for RNTI will be used later)
  ndlcch->active[ncce_index] = 1; //will be activated by the corresponding NDSLCH pdu

  ndlcch->dci_repetitions[ncce_index]          = DCIrep_to_real_rep(DCIRep,16);        ////??????? should be repalce by the value in spec table 16.6-3, check also Rmax

  printf("dci_repetitions: %d, A = %d\n",ndlcch->dci_repetitions[ncce_index],ndlcch->A[ncce_index]);

  DCI_flip = (uint8_t*)malloc(3*sizeof(uint8_t));

    for(i=0; i<3; ++i){
      DCI_flip[i] = 0x0;
    }

    DCI_flip[0] = (type << 7) | (scind << 1) | (ResAssign>>2);
    DCI_flip[1] =  (uint8_t)(ResAssign << 6) | (Scheddly << 4) | mcs;
    DCI_flip[2] = (rv << 7) | (RepNum << 4) | (ndi << 3) |(DCIRep <<1);
    
    ndlcch->pdu[ncce_index]    = DCI_flip;

    printf("DCI N0 content:");
    for (tmp =0;tmp<3;tmp++)
      printf("%d ",DCI_flip[tmp]);
    printf("\n");
    /*
     * TS 36.213 ch 16.4.1.5
     * ITBS is always set equivalent to IMCS for data
     * ISF = ResAssign
     */

    ndlcch->counter_repetition_number[ncce_index] = DCIrep_to_real_rep(DCIRep,16);          ////??????? should be repalce by the value in spec table 16.6-3, check also Rmax


    LOG_I(PHY,"DCI packing for N0 done \n");

}


int generate_eNB_dlsch_params_from_dci_NB_IoT(PHY_VARS_eNB_NB_IoT      *eNB,
                                              int                      frame,
                                              uint8_t                  subframe,
                                              DCI_CONTENT              *DCI_Content,
                                              uint16_t                 rnti,
                                              DCI_format_NB_IoT_t      dci_format,
                                              NB_IoT_eNB_NPDCCH_t      *ndlcch,
                                              NB_IoT_DL_FRAME_PARMS    *frame_parms,
                                              uint8_t                  aggregation,         //////????? maybe add the ncce index ??????????
									                            uint8_t                  npdcch_start_symbol,
                                              uint8_t                  ncce_index)
{

 // NB_IoT_eNB_NPDCCH_t  *ndlcch     = ;
  int tmp = 0;
  int i = 0;
  uint8_t  *DCI_flip = NULL;

  //N1 parameters
  //uint8_t ncce_index = 0;

  /// type = 0 => DCI Format N0, type = 1 => DCI Format N1, 1 bits
  uint8_t type = 0;
  //NPDCCH order indicator (set to 0),1 bits
  uint8_t orderIndicator = 0;
  // Scheduling Delay, 3 bits
  uint8_t Sched_delay = 0;
  // Resourse Assignment (RU Assignment), 3 bits
  uint8_t ResAssign = 0;//Isf
  // Modulation and Coding Scheme, 4 bits
  uint8_t mcs = 0;
  // Repetition Number, 4 bits
  uint8_t RepNum = 0;
  // DCI subframe repetition Number, 2 bits
  uint8_t DCIRep = 0;
  // New Data Indicator,1 bits
  uint8_t ndi = 0;
  // HARQ-ACK resource,4 bits
  uint8_t HARQackRes = 0;

  //N2 parameters

  //Direct indication information, 8 bits
  uint8_t directIndInf= 0;
  // Reserved information bits, 6 bits
  uint8_t resInfoBits =0;

  //   printf("Generate eNB DCI, format %d, rnti %x (pdu %p)\n",dci_format,rnti,dci_pdu);

  switch (dci_format) {

  // Impossible to have a DCI N0, we have condition before
  case DCIFormatN0:
    return(-1);
    break;

  case DCIFormatN1_RAR:  // This is DLSCH allocation for control traffic (no NDI and no ACK/NACK resource for RAR DCI)


	/*Packed DCI here-------------------------------------------*/
    type               = DCI_Content->DCIN1_RAR.type;
    orderIndicator     = DCI_Content->DCIN1_RAR.orderIndicator; 
    Sched_delay        = DCI_Content->DCIN1_RAR.Scheddly;
    ResAssign          = DCI_Content->DCIN1_RAR.ResAssign; 
    mcs                = DCI_Content->DCIN1_RAR.mcs; 
    RepNum             = DCI_Content->DCIN1_RAR.RepNum; 
    ndi                = DCI_Content->DCIN1_RAR.ndi;
    HARQackRes         = DCI_Content->DCIN1_RAR.HARQackRes; 
    DCIRep             = DCI_Content->DCIN1_RAR.DCIRep;

    /*Now configure the npdcch structure*/

   // ndlcch->ncce_index          =   NCCE_index;
   // ndlcch->aggregation_level   =   aggregation;

    ndlcch->A[ncce_index]               = sizeof_DCIN1_RAR_t; // number of bits in DCI

    //ndlcch->subframe_tx[subframe] = 1; // check if it's OK
    ndlcch->rnti[ncce_index] = rnti; //we store the RNTI (e.g. for RNTI will be used later)
    ndlcch->active[ncce_index] = 1; //will be activated by the corresponding NDSLCH pdu

    // use this value to configure PHY both harq_processes and resource mapping.
    ndlcch->scheduling_delay[ncce_index]         = Idelay_to_K0(Sched_delay,4);
    ndlcch->resource_assignment[ncce_index]      = resource_to_subframe[ResAssign];  //from Isf of DCI to the number of subframe
    ndlcch->repetition_number[ncce_index]        = Irep_to_Nrep[RepNum];                             // repetition number for NPDSCH
    ndlcch->dci_repetitions[ncce_index]          = DCIrep_to_real_rep(DCIRep,4);        ////??????? should be repalce by the value in spec table 16.6-3, check also Rmax

    //printf("dci_repetitions: %d, A = %d\n",ndlcch->dci_repetitions[ncce_index],ndlcch->A[ncce_index]);

    ndlcch->modulation[ncce_index]               = 2; //QPSK
    //// ////////////////////////////////////////////////if(ndlcch->round == 0) //this should be set from initialization (init-lte)

    //ndlcch->status[ncce_index] = ACTIVE_NB_IoT;
    ndlcch->mcs[ncce_index]    = mcs;


    DCI_flip = (uint8_t*)malloc(3*sizeof(uint8_t));

    for(i=0; i<3; ++i){
      DCI_flip[i] = 0x0;
    }

    DCI_flip[0] = (type << 7) | (orderIndicator << 6) | (Sched_delay<<2) | ResAssign ;
    DCI_flip[1] = (mcs << 4) | RepNum;
    DCI_flip[2] = (ndi << 7) | (HARQackRes << 3) | (DCIRep <<1);
    
    ndlcch->pdu[ncce_index]    = DCI_flip;

    printf("DCI N1 RAR PDU content:");
    for (tmp =0;tmp<3;tmp++)
      printf("%d ",DCI_flip[tmp]);
    printf("\n");
    /*
     * TS 36.213 ch 16.4.1.5
     * ITBS is always set equivalent to IMCS for data
     * ISF = ResAssign
     */

    ndlcch->TBS[ncce_index]      = TBStable_NB_IoT[mcs][ResAssign];
    //ndlcch->subframe[ncce_index] = subframe;
    ndlcch->counter_repetition_number[ncce_index] = DCIrep_to_real_rep(DCIRep,4);          ////??????? should be repalce by the value in spec table 16.6-3, check also Rmax
    //ndlsch_harq->B; we don-t have now my is given when we receive the dlsch data
    //ndlsch->error_treshold
    //ndlsch->G??
    //ndlsc->nsoft?? //set in new_eNB_dlsch (initialization)
    //ndlsch-> sqrt_rho_a?? set in dlsch_modulation
    //ndlsch-> sqrt_rho_b??? set in dlsch_modulation

    LOG_I(PHY,"DCI packing for N1RAR done \n");

    //set in new_eNB_dlsch (initialization)
    /*
     * Mlimit
     * nsoft
     * round
     */


    break;

  case DCIFormatN1: // for user data
    

    type               = DCI_Content->DCIN1.type;
    orderIndicator     = DCI_Content->DCIN1.orderIndicator; 
    Sched_delay        = DCI_Content->DCIN1.Scheddly;
    ResAssign          = DCI_Content->DCIN1.ResAssign; 
    mcs                = DCI_Content->DCIN1.mcs; 
    RepNum             = DCI_Content->DCIN1.RepNum; 
    ndi                = DCI_Content->DCIN1.ndi;
    HARQackRes         = DCI_Content->DCIN1.HARQackRes; 
    DCIRep             = DCI_Content->DCIN1.DCIRep;

    //add_dci_NB_IoT(eNB->DCI_pdu,DLSCH_DCI_NB_IoT,rnti,sizeof(DCIN1_t),aggregation,sizeof_DCIN1_t,DCIFormatN1,npdcch_start_symbol);

    /*Now configure the npdcch structure*/
    ndlcch->A[ncce_index]               = sizeof_DCIN1_t; // number of bits in DCI

    // ndlcch->ncce_index          =   NCCE_index;
    // ndlcch->aggregation_level   =   aggregation;

    //ndlcch->subframe_tx[subframe] = 1; // check if it's OK
    ndlcch->rnti[ncce_index]                  = rnti; //we store the RNTI (e.g. for RNTI will be used later)
    ndlcch->active[ncce_index]                = 1;//will be activated by the corresponding NDSLCH pdu

    // use this value to configure PHY both harq_processes and resource mapping.
    ndlcch->scheduling_delay[ncce_index]         = Idelay_to_K0(Sched_delay,4);
    ndlcch->resource_assignment[ncce_index]      = resource_to_subframe[ResAssign];  //from Isf of DCI to the number of subframe
    ndlcch->repetition_number[ncce_index]        = Irep_to_Nrep[RepNum];                             // repetition number for NPDSCH
    ndlcch->dci_repetitions[ncce_index]          = DCIrep_to_real_rep(DCIRep,4);        ////??????? should be repalce by the value in spec table 16.6-3, check also Rmax
    ndlcch->modulation[ncce_index]               = 2; //QPSK
    //if(ndlcch->round == 0){ //this should be set from initialization (init-lte)

  	//ndlcch->status[ncce_index]  = ACTIVE_NB_IoT;
  	ndlcch->mcs[ncce_index]     = mcs;
  	ndlcch->TBS[ncce_index]     = TBStable_NB_IoT[mcs][ResAssign]; // this table should be rewritten for nb-iot
    //ndlcch->pdu[ncce_index]    = DLSCH_DCI_NB_IoT;

    DCI_flip = (uint8_t*)malloc(3*sizeof(uint8_t));

    for(i=0; i<3; ++i){
      DCI_flip[i] = 0x0;
    }

    DCI_flip[0] = (type << 7) | (orderIndicator << 6) | (Sched_delay<<2) | ResAssign ;
    DCI_flip[1] = (mcs << 4) | RepNum;
    DCI_flip[2] = (ndi << 7) | (HARQackRes << 3) | (DCIRep <<1);
    //DCI_flip[0]        = 129;

    //DCI_flip[0]        = DCI_tmp[2]*2;
    //DCI_flip[1]        = DCI_tmp[1]*2;
    //DCI_flip[2]        = DCI_tmp[0]*2;
    //DCI_flip[2]        = 4;
    ndlcch->pdu[ncce_index]    = DCI_flip;

    printf("DCI N1 PDU content:");
    for (tmp =0;tmp<3;tmp++)
      printf("%d ",DCI_flip[tmp]);
    printf("\n");

    ndlcch->counter_repetition_number[ncce_index] = DCIrep_to_real_rep(DCIRep,4);          ////??????? should be repalce by the value in spec table 16.6-3, check also Rmax
    //}
    //ndlcch->frame[ncce_index]    = frame;
    //ndlcch->subframe[ncce_index] = subframe;

    break;

  case DCIFormatN2_Ind: //MP: for the moment is not implemented

    type               = DCI_Content->DCIN2_Ind.type;
    directIndInf       = DCI_Content->DCIN2_Ind.directIndInf; 
    resInfoBits        = DCI_Content->DCIN2_Ind.resInfoBits; 



    //add_dci_NB_IoT(eNB->DCI_pdu,DLSCH_DCI_NB_IoT,rnti,sizeof(DCIN2_Ind_t),aggregation,sizeof_DCIN2_Ind_t,DCIFormatN2_Ind,npdcch_start_symbol);

    // use this value to configure PHY both harq_processes and resource mapping.
    break;

    
  case DCIFormatN2_Pag:  //MP: for the moment is not implemented

    type               = DCI_Content->DCIN2_Pag.type;
    ResAssign          = DCI_Content->DCIN2_Pag.ResAssign; 
    mcs                = DCI_Content->DCIN2_Pag.mcs; 
    RepNum             = DCI_Content->DCIN2_Pag.RepNum; 
    DCIRep             = DCI_Content->DCIN2_Pag.DCIRep;

    //add_dci_NB_IoT(eNB->DCI_pdu,DLSCH_DCI_NB_IoT,rnti,sizeof(DCIN2_Pag_t),aggregation,sizeof_DCIN2_Pag_t,DCIFormatN2_Pag,npdcch_start_symbol);

    // use this value to configure PHY both harq_processes and resource mapping.
    break;


  default:
    LOG_E(PHY,"Unknown DCI format\n");
    return(-1);
    break;
  }

  // compute DL power control parameters

  //free(DLSCH_DCI_NB_IoT);

  return(0);
}


uint8_t subframe2harq_pid_NB_IoT(NB_IoT_DL_FRAME_PARMS *frame_parms,uint32_t frame,uint8_t subframe)
{
  //MAC_xface_NB_IoT *mac_xface_NB_IoT; //test_xface
  /*
    #ifdef DEBUG_DCI
    if (frame_parms->frame_type == TDD)
    printf("dci_tools.c: subframe2_harq_pid, subframe %d for TDD configuration %d\n",subframe,frame_parms->tdd_config);
    else
    printf("dci_tools.c: subframe2_harq_pid, subframe %d for FDD \n",subframe);
    #endif
  */

  uint8_t ret = 255;

  if (frame_parms->frame_type == FDD_NB_IoT) {
    ret = (((frame<<1)+subframe)&7);
  } else {

    switch (frame_parms->tdd_config) {

    case 1:
      if ((subframe==2) ||
          (subframe==3) ||
          (subframe==7) ||
          (subframe==8))
        switch (subframe) {
        case 2:
        case 3:
          ret = (subframe-2);
          break;

        case 7:
        case 8:
          ret = (subframe-5);
          break;

        default:
          LOG_E(PHY,"subframe2_harq_pid_NB_IoT, Illegal subframe %d for TDD mode %d\n",subframe,frame_parms->tdd_config);
          ret = (255);
          break;
        }

      break;

    case 2:
      if ((subframe!=2) && (subframe!=7)) {
        LOG_E(PHY,"subframe2_harq_pid, Illegal subframe %d for TDD mode %d\n",subframe,frame_parms->tdd_config);
        //mac_xface_NB_IoT->macphy_exit("subframe2_harq_pid_NB_IoT, Illegal subframe");
        ret = (255);
      }

      ret = (subframe/7);
      break;

    case 3:
      if ((subframe<2) || (subframe>4)) {
        LOG_E(PHY,"subframe2_harq_pid_NB_IoT, Illegal subframe %d for TDD mode %d\n",subframe,frame_parms->tdd_config);
        ret = (255);
      }

      ret = (subframe-2);
      break;

    case 4:
      if ((subframe<2) || (subframe>3)) {
        LOG_E(PHY,"subframe2_harq_pid_NB_IoT, Illegal subframe %d for TDD mode %d\n",subframe,frame_parms->tdd_config);
        ret = (255);
      }

      ret = (subframe-2);
      break;

    case 5:
      if (subframe!=2) {
        LOG_E(PHY,"subframe2_harq_pid_NB_IoT, Illegal subframe %d for TDD mode %d\n",subframe,frame_parms->tdd_config);
        ret = (255);
      }

      ret = (subframe-2);
      break;

    default:
      LOG_E(PHY,"subframe2_harq_pid_NB_IoT, Unsupported TDD mode %d\n",frame_parms->tdd_config);
      ret = (255);

    }
  }

  if (ret == 255) {
    LOG_E(PHY, "invalid harq_pid(%d) at SFN/SF = %d/%d\n", ret, frame, subframe);
    //mac_xface_NB_IoT->macphy_exit("invalid harq_pid");
  }
  return ret;
}

