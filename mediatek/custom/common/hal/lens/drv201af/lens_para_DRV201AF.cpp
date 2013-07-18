/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>
#include <utils/Errors.h>
#include <cutils/xlog.h>
//#include "msdk_nvram_camera_exp.h"
//#include "msdk_lens_exp.h"
#include "camera_custom_nvram.h"
#include "camera_custom_lens.h"

#include <utils/Errors.h>
#include <cutils/xlog.h>
#define LOGD(fmt, arg...) XLOGD(fmt, ##arg)


#include <fcntl.h>

#undef LOG_TAG
#define LOG_TAG "CAM_CUS_MSDK"

#define CAM_MSDK_LOG(fmt, arg...)    LOGD(LOG_TAG " "fmt, ##arg)
#define CAM_MSDK_ERR(fmt, arg...)    LOGE(LOG_TAG "Err: %5d: "fmt, __LINE__, ##arg)

const NVRAM_LENS_PARA_STRUCT DRV201AF_LENS_PARA_DEFAULT_VALUE =
{
    //Version
    NVRAM_CAMERA_LENS_FILE_VERSION,

    // Focus Range NVRAM
    {0, 1023},

    // AF NVRAM
    {
        {{
	    {// Exact Search
	    14,		// i4NormalNum
	    14,		// i4MarcoNum
    	{
             0,  25,  55, 80 ,110, 150, 190, 235, 285, 335, 385,
           445, 525, 605, 0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,   0,   0
              
	    }
    	}
	    }},

        {80},          //i4ZoomTable[0]
        30,             // i4AF_THRES_MAIN
        20,             // i4AF_THRES_SUB
        10000,          // i4AF_THRES_OFFSET
        5,              // THRES OFFSET GAIN for LOW LIGHT
        0,              // i4LV_THRES
        5,              // i4MATRIX_AF_DOF
        3,              // i4MATRIX_AF_WIN_NUM
        15000,          // i4AFC_THRES_OFFSET;
        10,              // i4AFC_STEPSIZE
        4,              // i4AFC_SPEED
        15,             // i4SCENE_CHANGE_THRES
        8,             // i4SCENE_CHANGE_CNT

        15,             // i4SPOT_PERCENT_X
        15,             // i4SPOT_PERCENT_Y
        15,             // i4MATRIX_PERCENT_X
        15,             // i4MATRIX_PERCENT_Y
        5,              // i4MATRIX_LOC_OFFSET

        20000,          // i4TUNE_PARA1 : THRES OFFSET for Matrix
        0,              // i4TUNE_PARA2 : Infinity position for normal
        0               // i4TUNE_PARA3 : Infinity position for macro
    },

    {0}
};

const NVRAM_LENS_PARA_STRUCT FOXAF_LENS_PARA_DEFAULT_VALUE =
{
    //Version
    NVRAM_CAMERA_LENS_FILE_VERSION,

    // Focus Range NVRAM
    {0, 1023},

    // AF NVRAM
    {
        {{
	    {// Exact Search
	    12,		// i4NormalNum
	    12,		// i4MarcoNum
    	{
             0,  25,  50, 75 ,100, 150, 200, 245, 290, 350, 410,
           480, 0, 0, 0,   0,   0,   0,   0,   0,
             0,   0,   0,   0,   0,   0,   0,   0,   0,   0
              
	    }
    	}
	    }},

       {50},          //i4ZoomTable[0]
        25,             // i4AF_THRES_MAIN
        20,             // i4AF_THRES_SUB
        10000,          // i4AF_THRES_OFFSET
        5,              // THRES OFFSET GAIN for LOW LIGHT
        0,              // i4LV_THRES
        5,              // i4MATRIX_AF_DOF
        3,              // i4MATRIX_AF_WIN_NUM
        15000,          // i4AFC_THRES_OFFSET;
        10,              // i4AFC_STEPSIZE
        4,              // i4AFC_SPEED
        15,             // i4SCENE_CHANGE_THRES
        7,             // i4SCENE_CHANGE_CNT

        15,             // i4SPOT_PERCENT_X
        15,             // i4SPOT_PERCENT_Y
        15,             // i4MATRIX_PERCENT_X
        15,             // i4MATRIX_PERCENT_Y
        5,              // i4MATRIX_LOC_OFFSET

        20000,          // i4TUNE_PARA1 : THRES OFFSET for Matrix
        0,              // i4TUNE_PARA2 : Infinity position for normal
        0               // i4TUNE_PARA3 : Infinity position for macro
    },

    {0}

};

UINT32 DRV201AF_getDefaultData(VOID *pDataBuf, UINT32 size)
{
    UINT32 dataSize = sizeof(NVRAM_LENS_PARA_STRUCT);
    int af_vendor_fd = -1;
	char af_vendor_name[10] = "unknow";
	CAM_MSDK_LOG("af get default data enter 33333333333333333333333\n");
    if ((pDataBuf == NULL) || (size < dataSize))
    {
        return 1;
    }
	
    af_vendor_fd = open("/proc/af_vendor",O_RDONLY);
    if(af_vendor_fd!=-1) {
        read(af_vendor_fd, af_vendor_name, 10);
        close(af_vendor_fd);
    }

    // copy from Buff to global struct
    if(strncmp(af_vendor_name,"TDK",3)==0)
    	{
        memcpy(pDataBuf, &DRV201AF_LENS_PARA_DEFAULT_VALUE, dataSize);
		CAM_MSDK_LOG("justin tdk enter11111111111111111111111(%s)\n",af_vendor_name);
    	}
    else
    	{
	    memcpy(pDataBuf, &FOXAF_LENS_PARA_DEFAULT_VALUE, dataSize);
	    CAM_MSDK_LOG("justin fox enter222222222222222222222(%s)\n",af_vendor_name);
    	}
    return 0;
}

PFUNC_GETLENSDEFAULT pDRV201AF_getDefaultData = DRV201AF_getDefaultData;


