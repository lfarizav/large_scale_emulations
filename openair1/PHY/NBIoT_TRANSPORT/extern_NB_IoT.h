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


#ifndef __PHY_LTE_TRANSPORT_EXTERN_NB_IOT__H__
#define __PHY_LTE_TRANSPORT_EXTERN_NB_IOT__H__


extern unsigned int TBStable_NB_IoT[14][8];
extern unsigned int TBStable_NB_IoT_SIB1[16];

extern unsigned char cs_ri_normal_NB_IoT[4];
extern unsigned char cs_ri_extended_NB_IoT[4];
extern unsigned char cs_ack_normal_NB_IoT[4];
extern unsigned char cs_ack_extended_NB_IoT[4];
extern int8_t wACK_RX_NB_IoT[5][4];
extern int G_tab[18];


extern short conjugate[8],conjugate2[8];
extern short *ul_ref_sigs_rx_NB_IoT[30][4]; // NB-IoT: format 1 pilots
extern short *ul_ref_sigs_f2_rx_NB_IoT[16]; // NB-IoT: format 2 pilots
//extern unsigned short dftsizes[33];

extern int16_t unscrambling_lut_NB_IoT[65536*16];

extern  int16_t e_phi_re_m6[120];
extern  int16_t e_phi_im_m6[120]; 
extern  int16_t e_phi_re_m5[120];
extern  int16_t e_phi_im_m5[120];
extern  int16_t e_phi_re_m4[120];
extern  int16_t e_phi_im_m4[120];
extern  int16_t e_phi_re_m3[120];
extern  int16_t e_phi_im_m3[120];
extern  int16_t e_phi_re_m2[120];
extern  int16_t e_phi_im_m2[120];
extern  int16_t e_phi_re_m1[120];
extern  int16_t e_phi_im_m1[120];
extern  int16_t e_phi_re_0[120];
extern  int16_t e_phi_im_0[120];
extern  int16_t e_phi_re_p1[120];
extern  int16_t e_phi_im_p1[120];
extern  int16_t e_phi_re_p2[120];
extern  int16_t e_phi_im_p2[120];
extern  int16_t e_phi_re_p3[120];
extern  int16_t e_phi_im_p3[120];
extern  int16_t e_phi_re_p4[120];
extern  int16_t e_phi_im_p4[120];
extern  int16_t e_phi_re_p5[120];  
extern  int16_t e_phi_im_p5[120]; 

#endif