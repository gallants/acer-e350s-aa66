/* Copyright Statement: 
* 
* This software/firmware and related documentation ("MediaTek Software") are 
* protected under relevant copyright laws. The information contained herein 
* is confidential and proprietary to MediaTek Inc. and/or its licensors. 
* Without the prior written permission of MediaTek inc. and/or its licensors, 
* any reproduction, modification, use or disclosure of MediaTek Software, 
* and information contained herein, in whole or in part, shall be strictly prohibited. 
*/ 
/* MediaTek Inc. (C) 2010. All rights reserved. 
* 
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES 
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") 
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON 
* AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF 
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT. 
* NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE 
* SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR 
* SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH 
* THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
* THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES 
* CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK 
* SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR 
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND 
* CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, 
* AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, 
* OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO 
* MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
* 
* The following software/firmware and/or related documentation ("MediaTek Software") 
* have been modified by MediaTek Inc. All revisions are subject to any receiver's 
* applicable license agreements with MediaTek Inc. 
*/ 
 
/***************************************************************************** 
*  Copyright Statement: 
*  -------------------- 
*  This software is protected by Copyright and the information contained 
*  herein is confidential. The software may not be copied and the information 
*  contained herein may not be used or disclosed except with the written 
*  permission of MediaTek Inc. (C) 2008 
* 
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES 
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") 
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON 
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF 
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT. 
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE 
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR 
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH 
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO 
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S 
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM. 
* 
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE 
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, 
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, 
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO 
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE. 
* 
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE 
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF 
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND 
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER 
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC). 
* 
*****************************************************************************/ 
 
/******************************************************************************* 
* 
* Filename: 
* --------- 
* aud_custom_exp.h 
* 
* Project: 
* -------- 
*   DUMA 
* 
* Description: 
* ------------ 
* This file is the header of audio customization related function or definition. 
* 
* Author: 
* ------- 
* JY Huang 
* 
*============================================================================ 
*             HISTORY 
* Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!! 
*------------------------------------------------------------------------------ 
* $Revision:$ 
* $Modtime:$ 
* $Log:$ 
* 
* 05 26 2010 chipeng.chang 
* [ALPS00002287][Need Patch] [Volunteer Patch] ALPS.10X.W10.11 Volunteer patch for audio paramter 
* modify audio parameter. 
* 
* 05 26 2010 chipeng.chang 
* [ALPS00002287][Need Patch] [Volunteer Patch] ALPS.10X.W10.11 Volunteer patch for audio paramter 
* modify for Audio parameter 
* 
*    mtk80306 
* [DUMA00132370] waveform driver file re-structure. 
* waveform driver file re-structure. 
* 
* Jul 28 2009 mtk01352 
* [DUMA00009909] Check in TWO_IN_ONE_SPEAKER and rearrange 
* 
* 
* 
*------------------------------------------------------------------------------ 
* Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!! 
*============================================================================ 
****************************************************************************/ 
#ifndef AUDIO_CUSTOM_H
#define AUDIO_CUSTOM_H
 
/* define Gain For Normal */ 
/* Normal volume: TON, SPK, MIC, FMR, SPH, SID, MED */ 
/* 
#define GAIN_NOR_TON_VOL      8     // reserved 
#define GAIN_NOR_KEY_VOL      43    // TTY_CTM_Mic 
#define GAIN_NOR_MIC_VOL      26    // IN_CALL BuiltIn Mic gain 
// GAIN_NOR_FMR_VOL is used as idle mode record volume 
#define GAIN_NOR_FMR_VOL      0     // Normal BuiltIn Mic gain 
#define GAIN_NOR_SPH_VOL      20     // IN_CALL EARPIECE Volume 
#define GAIN_NOR_SID_VOL      100  // IN_CALL EARPICE sidetone 
#define GAIN_NOR_MED_VOL      25   // reserved 
*/ 
 
#define GAIN_NOR_TON_VOL    8    // reserved 
#define GAIN_NOR_KEY_VOL    43    // TTY_CTM_Mic 
#define GAIN_NOR_MIC_VOL    26    // IN_CALL BuiltIn Mic gain 
// GAIN_NOR_FMR_VOL is used as idle mode record volume 
#define GAIN_NOR_FMR_VOL    0    // Normal BuiltIn Mic gain 
#define GAIN_NOR_SPH_VOL    20    // IN_CALL EARPIECE Volume 
#define GAIN_NOR_SID_VOL    100    // IN_CALL EARPICE sidetone 
#define GAIN_NOR_MED_VOL    25    // reserved 
 
/* define Gain For Headset */ 
/* Headset volume: TON, SPK, MIC, FMR, SPH, SID, MED */ 
/* 
#define GAIN_HED_TON_VOL      8   // reserved 
#define GAIN_HED_KEY_VOL      24  // reserved 
#define GAIN_HED_MIC_VOL      20   // IN_CALL BuiltIn headset gain 
#define GAIN_HED_FMR_VOL      24  // reserved 
#define GAIN_HED_SPH_VOL      12  // IN_CALL Headset volume 
#define GAIN_HED_SID_VOL      100 // IN_CALL Headset sidetone 
#define GAIN_HED_MED_VOL      12   // Idle, headset Audio Buf Gain setting 
*/ 
 
#define GAIN_HED_TON_VOL    8    // reserved 
#define GAIN_HED_KEY_VOL    24    // reserved 
#define GAIN_HED_MIC_VOL    20    // IN_CALL BuiltIn headset gain 
#define GAIN_HED_FMR_VOL    24    // reserved 
#define GAIN_HED_SPH_VOL    12    // IN_CALL Headset volume 
#define GAIN_HED_SID_VOL    100    // IN_CALL Headset sidetone 
#define GAIN_HED_MED_VOL    12    // Idle, headset Audio Buf Gain setting 
 
/* define Gain For Handfree */ 
/* Handfree volume: TON, SPK, MIC, FMR, SPH, SID, MED */ 
/* GAIN_HND_TON_VOL is used as class-D Amp gain*/ 
/* 
#define GAIN_HND_TON_VOL      15  // 
#define GAIN_HND_KEY_VOL      24  // reserved 
#define GAIN_HND_MIC_VOL      20  // IN_CALL LoudSpeak Mic Gain = BuiltIn Gain 
#define GAIN_HND_FMR_VOL      24 // reserved 
#define GAIN_HND_SPH_VOL      6 // IN_CALL LoudSpeak 
#define GAIN_HND_SID_VOL      100// IN_CALL LoudSpeak sidetone 
#define GAIN_HND_MED_VOL      12 // Idle, loudSPK Audio Buf Gain setting 
*/ 
 
#define GAIN_HND_TON_VOL    8    // use for ringtone volume 
#define GAIN_HND_KEY_VOL    24    // reserved 
#define GAIN_HND_MIC_VOL    20    // IN_CALL LoudSpeak Mic Gain = BuiltIn Gain 
#define GAIN_HND_FMR_VOL    24    // reserved 
#define GAIN_HND_SPH_VOL    12    // IN_CALL LoudSpeak 
#define GAIN_HND_SID_VOL    100    // IN_CALL LoudSpeak sidetone 
#define GAIN_HND_MED_VOL    12    // Idle, loudSPK Audio Buf Gain setting 
 
    /* 0: Input FIR coefficients for 2G/3G Normal mode */
    /* 1: Input FIR coefficients for 2G/3G/VoIP Headset mode */
    /* 2: Input FIR coefficients for 2G/3G Handfree mode */
    /* 3: Input FIR coefficients for 2G/3G/VoIP BT mode */
    /* 4: Input FIR coefficients for VoIP Normal mode */
    /* 5: Input FIR coefficients for VoIP Handfree mode */
#define SPEECH_INPUT_FIR_COEFF \
      237,  -919,   601,  -857,   115,\
     -750,  -346,  -302,  -908,   995,\
    -1430,  1419, -2033,  3688, -4234,\
     6494, -7487,  7870,-10686, 11924,\
   -11614, 18426, 18426,-11614, 11924,\
   -10686,  7870, -7487,  6494, -4234,\
     3688, -2033,  1419, -1430,   995,\
     -908,  -302,  -346,  -750,   115,\
     -857,   601,  -919,   237,     0,\
                                      \
     -232,   600,   603,  -551,  4340,\
      387,  3267, -2835, -1524,  -526,\
     6620,-10843,  6498,-11063, 26028,\
    26028,-11063,  6498,-10843,  6620,\
     -526, -1524, -2835,  3267,   387,\
     4340,  -551,   603,   600,  -232,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
                                      \
      476,    86,  -129,   -25,  1804,\
    -1577,  -417,  -956,  1375, -3560,\
     2995,  3098,  -988,-12253, 20675,\
    20675,-12253,  -988,  3098,  2995,\
    -3560,  1375,  -956,  -417, -1577,\
     1804,   -25,  -129,    86,   476,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
                                      \
    32767,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
                                      \
    32767,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
                                      \
    32767,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0   
    /* 0: Output FIR coefficients for 2G/3G Normal mode */
    /* 1: Output FIR coefficients for 2G/3G/VoIP Headset mode */
    /* 2: Output FIR coefficients for 2G/3G Handfree mode */
    /* 3: Output FIR coefficients for 2G/3G/VoIP BT mode */
    /* 4: Output FIR coefficients for VoIP Normal mode */
    /* 5: Output FIR coefficients for VoIP Handfree mode */
#define SPEECH_OUTPUT_FIR_COEFF \
      242,  -370,   386,  -287,   184,\
      -51,    37,  -105,  -325,   733,\
     -457,   713, -1553,  1507, -1888,\
     2848, -1830,  2427, -8004, 10200,\
    -8303, 32767, 32767, -8303, 10200,\
    -8004,  2427, -1830,  2848, -1888,\
     1507, -1553,   713,  -457,   733,\
     -325,  -105,    37,   -51,   184,\
     -287,   386,  -370,   242,     0,\
                                      \
      566,  -193,  -200,   511,  1658,\
     -185, -2276, -2978,   622,  3838,\
     1121, -9163, -7951,   886, 20675,\
    20675,   886, -7951, -9163,  1121,\
     3838,   622, -2978, -2276,  -185,\
     1658,   511,  -200,  -193,   566,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
                                      \
     -871,  -442,   115,   -78,   344,\
     1176,  1892, -2988,  1267, -1046,\
    -2102,   315,  6881, -9574, 20675,\
    20675, -9574,  6881,   315, -2102,\
    -1046,  1267, -2988,  1892,  1176,\
      344,   -78,   115,  -442,  -871,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
                                      \
    32767,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
                                      \
    32767,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
                                      \
    32767,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0   
#define   DG_DL_Speech    0xE3D
#define   DG_Microphone    0x1400
#define   FM_Record_Vol    6     /* 0 is smallest. each step increase 1dB.
                            Be careful of distortion when increase too much. 
                            Generally, it's not suggested to tune this parameter */ 
/* 
* The Bluetooth DAI Hardware COnfiguration Parameter 
*/ 
#define   DEFAULT_BLUETOOTH_SYNC_TYPE    0
#define   DEFAULT_BLUETOOTH_SYNC_LENGTH    1
    /* 0: Input FIR coefficients for 2G/3G Normal mode */
    /* 1: Input FIR coefficients for 2G/3G/VoIP Headset mode */
    /* 2: Input FIR coefficients for 2G/3G Handfree mode */
    /* 3: Input FIR coefficients for 2G/3G/VoIP BT mode */
    /* 4: Input FIR coefficients for VoIP Normal mode */
    /* 5: Input FIR coefficients for VoIP Handfree mode */
#define WB_Speech_Input_FIR_Coeff \
       37,    72,   -57,   -75,   106,   175,    -1,   -36,    50,    11,\
      -21,   -80,    79,    35,    31,  -203,  -144,    -6,   109,   171,\
       13,   -64,  -264,    99,  -147,    43,   265,   543,  -134,   121,\
     -275,   499,  1375,  -541, -1022,  -280,  1625, -2220,   504, -1101,\
     3647, -1855,  -455, -5130, 17037, 17037, -5130,  -455, -1855,  3647,\
    -1101,   504, -2220,  1625,  -280, -1022,  -541,  1375,   499,  -275,\
      121,  -134,   543,   265,    43,  -147,    99,  -264,   -64,    13,\
      171,   109,    -6,  -144,  -203,    31,    35,    79,   -80,   -21,\
       11,    50,   -36,    -1,   175,   106,   -75,   -57,    72,    37,\
                                      \
    32767,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
                                      \
    -1355,  -733,  -959,  -958,  -334,  -895,  -725,  -633,  -563,  -619,\
     -452,  -699,  -230,  -164,   341,  -153,   373,  -164,    62,   -37,\
     -890,   485, -1456,    26,  -342,   218,  1608,  1436,  1706,  2731,\
     1878,  1741,  2806,  1175,  1790, -1233,  -231, -2168,  3017,   127,\
     2734,   773,  3331,   -35, 20675, 20675,   -35,  3331,   773,  2734,\
      127,  3017, -2168,  -231, -1233,  1790,  1175,  2806,  1741,  1878,\
     2731,  1706,  1436,  1608,   218,  -342,    26, -1456,   485,  -890,\
      -37,    62,  -164,   373,  -153,   341,  -164,  -230,  -699,  -452,\
     -619,  -563,  -633,  -725,  -895,  -334,  -958,  -959,  -733, -1355,\
                                      \
    32767,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
                                      \
     2352, 31760, 21481, 30602, -3776,    -3,-14068,   942, 12880,   542,\
        0,     0,    -1,    -1,-14128,   942,  9062, 30519,  5248,   148,\
        0,     0,  9076, 30519, -2039,-19217,-14068,   942, 12880,   542,\
        0,     0,    16,     0,-18504, 26760, 11616,  1182,    40,     0,\
       20,     0, 25448,  1462, 26104,  1462,    16,     0,  3383, 26712,\
    25468,  1462,     0,     0,     0,     0,-31476,   542,-15296, 30602,\
    -8538, 30602,-17404, 30602,-17437, 30602,  4134,   513,   500,     0,\
       31,     0,     4,     0,     0,     0,-31476,   542,-13876,   942,\
     7599, 26701,  4134,   513,   500,     0,    31,     0, 15706,  1461,\
                                      \
        5,     0,     4,     0,     0,     0, 29108, 26715,-13876,   942,\
        4,     0,     1,     0, 11640,  1182,   720,     0,-24116,     0,\
    30881, 26715,     5,     0,     1,     0, 15676,  1461,     0,     0,\
   -24116,     0,     5,     0, -8538, 30602,-17404, 30602,  4668,  1203,\
     4134,     1, -5968,  1203,   174,   613,   297,   888,-22384,     0,\
        0,     0, 11640,  1182,-21096,     0,   174,   613,   297,   888,\
       -1,   255,-30824,  1201,     2,     0,   255,     0,     0,     0,\
    29108, 26715,-13748,   942,     1,     0,     0,     0, 11640,  1182,\
      205,   841,   205,   841,   205,   841,   205,   841,   205,   841   
    /* 0: Output FIR coefficients for 2G/3G Normal mode */
    /* 1: Output FIR coefficients for 2G/3G/VoIP Headset mode */
    /* 2: Output FIR coefficients for 2G/3G Handfree mode */
    /* 3: Output FIR coefficients for 2G/3G/VoIP BT mode */
    /* 4: Output FIR coefficients for VoIP Normal mode */
    /* 5: Output FIR coefficients for VoIP Handfree mode */
#define WB_Speech_Output_FIR_Coeff \
      -14,   198,  -130,   122,    29,   -66,    70,  -175,   175,  -149,\
       22,  -165,    19,   167,    78,    66,   217,  -252,   -35,  -302,\
     -293,   317,  -254,  1178,  -714,   718,  -414,  -677,   282,  -927,\
      589,   291,  1051,  -469,  -294,   120,   589, -3219,  2506, -1937,\
     5445, -7122,  6842, -7490, 30607, 30607, -7490,  6842, -7122,  5445,\
    -1937,  2506, -3219,   589,   120,  -294,  -469,  1051,   291,   589,\
     -927,   282,  -677,  -414,   718,  -714,  1178,  -254,   317,  -293,\
     -302,   -35,  -252,   217,    66,    78,   167,    19,  -165,    22,\
     -149,   175,  -175,    70,   -66,    29,   122,  -130,   198,   -14,\
                                      \
    32767,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
                                      \
    32767,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
                                      \
    32767,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
        0,     0,     0,     0,     0,     0,     0,     0,     0,     0,\
                                      \
      -54,    -1,     0,     0,  3162,   182,    -1,    -1,-24184,-32289,\
     1113, 15647, 28559, 30518,-31786, 26760,     2,     0,-31788, 26760,\
     1413,     0,    21,     0,   512,     0,-31792, 26760,   512,     0,\
   -31786, 26760,     5,     0,     0,     0, 21227, 26712,-29224, 26760,\
    11674, 26694,-29224, 26760, 11828, 26702,  3162,   182,-29224, 26760,\
   -12224,   942,-12876,   942,-12848,   942, 11851, 26702,-12848,   942,\
        0,     0,     0,     0,     2,     0,-31786, 26760,-31788, 26760,\
        0,     0,  3162,   182,     0,     0,     0,     0, 14128,   182,\
       54,     0,-14648,   942,-15044, 30728, -9268, 30736,   842,-19245,\
                                      \
       21,     0,    54,     0,-14992, 30728, 29576,   820, 29576,   820,\
    29576,   820, 26052,   801,     0,     0,     0,   641,     0,   641,\
        0,     0,-31856, 26747,-14744,   942,-31856, 26747,    67,    86,\
       73,    82,    84,    76,    86,    68,    67,   104,   105,   108,\
      100,    48,    50,    52,    48,    48,    48,    48,    48,     0,\
        0,     0,     0,   820, 29384,   820,-27877, 30723, 29384,   820,\
       33,    82, 12257,     0,  3584,   137, -5096,   632,  3162,   182,\
    28464,   820,     1,     0, 29376,   820,-15072,   942, -8541, 30678,\
    29384,   820,     0,   719,  3162,   182, 29376,   820,-15052,   942   
#endif 
