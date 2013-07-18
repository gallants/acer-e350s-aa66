#ifndef _DRV201AF_H
#define _DRV201AF_H

#include <linux/ioctl.h>
//#include "kd_imgsensor.h"

#define DRV201AF_MAGIC 'A'
//IOCTRL(inode * ,file * ,cmd ,arg )


//Structures
typedef struct {
//current position
unsigned long u4CurrentPosition;
//macro position
unsigned long u4MacroPosition;
//Infiniti position
unsigned long u4InfPosition;
//Motor Status
bool          bIsMotorMoving;
//Motor Open?
bool          bIsMotorOpen;
} stDRV201AF_MotorInfo;

//Control commnad
//S means "set through a ptr"
//T means "tell by a arg value"
//G means "get by a ptr"             
//Q means "get by return a value"
//X means "switch G and S atomically"
//H means "switch T and Q atomically"
#define DRV201AFIOC_G_MOTORINFO _IOR(DRV201AF_MAGIC,0,stDRV201AF_MotorInfo)

#define DRV201AFIOC_T_MOVETO _IOW(DRV201AF_MAGIC,1,unsigned long)

#define DRV201AFIOC_T_SETINFPOS _IOW(DRV201AF_MAGIC,2,unsigned long)

#define DRV201AFIOC_T_SETMACROPOS _IOW(DRV201AF_MAGIC,3,unsigned long)

#else
#endif
