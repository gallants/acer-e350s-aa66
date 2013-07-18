/*
 * MD218A voice coil motor driver
 *
 *
 */

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include "DRV201AF.h"
#include "../camera/kd_camera_hw.h"
//#include "kd_cust_lens.h"

//#include <mach/mt6573_pll.h>
//#include <mach/mt6573_gpt.h>
//#include <mach/mt6573_gpio.h>


#define DRV201AF_DRVNAME "DRV201AF"
#define DRV201AF_VCM_WRITE_ID           0x1C

#define DRV201AF_DEBUG
#ifdef DRV201AF_DEBUG
#define DRV201AFDB printk
#else
#define DRV201AFDB(x,...)
#endif

static spinlock_t g_DRV201AF_SpinLock;
/* Kirby: remove old-style driver
static unsigned short g_pu2Normal_DRV201AF_i2c[] = {DRV201AF_VCM_WRITE_ID , I2C_CLIENT_END};
static unsigned short g_u2Ignore_DRV201AF = I2C_CLIENT_END;

static struct i2c_client_address_data g_stDRV201AF_Addr_data = {
    .normal_i2c = g_pu2Normal_DRV201AF_i2c,
    .probe = &g_u2Ignore_DRV201AF,
    .ignore = &g_u2Ignore_DRV201AF
};*/

static struct i2c_client * g_pstDRV201AF_I2Cclient = NULL;

static dev_t g_DRV201AF_devno;
static struct cdev * g_pDRV201AF_CharDrv = NULL;
static struct class *actuator_class = NULL;

static int  g_s4DRV201AF_Opened = 0;
static long g_i4MotorStatus = 0;
static long g_i4Dir = 0;
static long g_i4Position = 0;
static unsigned long g_u4DRV201AF_INF = 0;
static unsigned long g_u4DRV201AF_MACRO = 1023;
static unsigned long g_u4TargetPosition = 0;
static unsigned long g_u4CurrPosition   = 0;
//static struct work_struct g_stWork;     // --- Work queue ---
//static XGPT_CONFIG	g_GPTconfig;		// --- Interrupt Config ---


extern s32 mt_set_gpio_mode(u32 u4Pin, u32 u4Mode);
extern s32 mt_set_gpio_out(u32 u4Pin, u32 u4PinOut);
extern s32 mt_set_gpio_dir(u32 u4Pin, u32 u4Dir);

extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);

static int s4DRV201AF_ReadReg(unsigned short * a_pu2Result)
{
	u8 get_byte[2]= {0};
    char puSendCmd[2] = {0x03 , 0x0 }; // current control register is 0x03(L) and 0x04(H)
    
    DRV201AFDB("s4DRV201AF_ReadReg ! \n");
	iReadRegI2C(puSendCmd , 1, get_byte, 2, DRV201AF_VCM_WRITE_ID);
    *a_pu2Result = ((get_byte[0]&0x03)<<8)|(get_byte[1]);

    return 0;
}

static int s4DRV201AF_WriteReg(u16 a_u2Data)
{
	char puSendCmd[3] = {0x03, (char)((a_u2Data >> 8) & 0x03), (char)(a_u2Data & 0xFF)};
	
	DRV201AFDB("s4DRV201AF_WriteReg ! \n");
	iWriteRegI2C(puSendCmd , 3, DRV201AF_VCM_WRITE_ID);
	
    return 0;
}



inline static int getDRV201AFInfo(__user stDRV201AF_MotorInfo * pstMotorInfo)
{
    stDRV201AF_MotorInfo stMotorInfo;
    stMotorInfo.u4MacroPosition   = g_u4DRV201AF_MACRO;
    stMotorInfo.u4InfPosition     = g_u4DRV201AF_INF;
    stMotorInfo.u4CurrentPosition = g_u4CurrPosition;
	if (g_i4MotorStatus == 1)	{stMotorInfo.bIsMotorMoving = TRUE;}
	else						{stMotorInfo.bIsMotorMoving = FALSE;}

	if (g_s4DRV201AF_Opened >= 1)	{stMotorInfo.bIsMotorOpen = TRUE;}
	else						{stMotorInfo.bIsMotorOpen = FALSE;}

    if(copy_to_user(pstMotorInfo , &stMotorInfo , sizeof(stDRV201AF_MotorInfo)))
    {
        DRV201AFDB("[DRV201AF] copy to user failed when getting motor information \n");
    }

    return 0;
}

inline static int moveDRV201AF(unsigned long a_u4Position)
{
    if((a_u4Position > g_u4DRV201AF_MACRO) || (a_u4Position < g_u4DRV201AF_INF))
    {
        DRV201AFDB("[DRV201AF] out of range \n");
        return -EINVAL;
    }

	if (g_s4DRV201AF_Opened == 1)
	{
		unsigned short InitPos;
	
		if(s4DRV201AF_ReadReg(&InitPos) == 0)
		{
			DRV201AFDB("[DRV201AF] Init Pos %6d \n", InitPos);
		
			g_u4CurrPosition = (unsigned long)InitPos;
		}
		else
		{
			g_u4CurrPosition = 0;
		}
		
		g_s4DRV201AF_Opened = 2;
	}

	if      (g_u4CurrPosition < a_u4Position)	{g_i4Dir = 1;}
	else if (g_u4CurrPosition > a_u4Position)	{g_i4Dir = -1;}
	else										{return 0;}

	if (1)
	{
		g_i4Position = (long)g_u4CurrPosition;
		g_u4TargetPosition = a_u4Position;

		if (g_i4Dir == 1)
		{
			//if ((g_u4TargetPosition - g_u4CurrPosition)<60)
			{		
				g_i4MotorStatus = 0;
				if(s4DRV201AF_WriteReg((unsigned short)g_u4TargetPosition) == 0)
				{
					g_u4CurrPosition = (unsigned long)g_u4TargetPosition;
				}
				else
				{
					DRV201AFDB("[DRV201AF] set I2C failed when moving the motor \n");
					g_i4MotorStatus = -1;
				}
			}
			//else
			//{
			//	g_i4MotorStatus = 1;
			//}
		}
		else if (g_i4Dir == -1)
		{
			//if ((g_u4CurrPosition - g_u4TargetPosition)<60)
			{
				g_i4MotorStatus = 0;		
				if(s4DRV201AF_WriteReg((unsigned short)g_u4TargetPosition) == 0)
				{
					g_u4CurrPosition = (unsigned long)g_u4TargetPosition;
				}
				else
				{
					DRV201AFDB("[DRV201AF] set I2C failed when moving the motor \n");
					g_i4MotorStatus = -1;
				}
			}
			//else
			//{
			//	g_i4MotorStatus = 1;		
			//}
		}
	}
	else
	{
	g_i4Position = (long)g_u4CurrPosition;
	g_u4TargetPosition = a_u4Position;
	g_i4MotorStatus = 1;
	}

    return 0;
}

inline static int setDRV201AFInf(unsigned long a_u4Position)
{
	g_u4DRV201AF_INF = a_u4Position;
	return 0;
}

inline static int setDRV201AFMacro(unsigned long a_u4Position)
{
	g_u4DRV201AF_MACRO = a_u4Position;
	return 0;	
}

////////////////////////////////////////////////////////////////
static long DRV201AF_Ioctl(
struct file * a_pstFile,
unsigned int a_u4Command,
unsigned long a_u4Param)
{
    long i4RetValue = 0;

    switch(a_u4Command)
    {
        case DRV201AFIOC_G_MOTORINFO :
			DRV201AFDB("[DRV201AF_Ioctl] DRV201AFIOC_G_MOTORINFO \n");
            i4RetValue = getDRV201AFInfo((__user stDRV201AF_MotorInfo *)(a_u4Param));
        break;

        case DRV201AFIOC_T_MOVETO :
			DRV201AFDB("[DRV201AF_Ioctl] DRV201AFIOC_T_MOVETO \n");
            i4RetValue = moveDRV201AF(a_u4Param);
        break;
 
 		case DRV201AFIOC_T_SETINFPOS :
			DRV201AFDB("[DRV201AF_Ioctl] DRV201AFIOC_T_SETINFPOS \n");
			 i4RetValue = setDRV201AFInf(a_u4Param);
		break;

 		case DRV201AFIOC_T_SETMACROPOS :
			DRV201AFDB("[DRV201AF_Ioctl] DRV201AFIOC_T_SETMACROPOS \n");
			 i4RetValue = setDRV201AFMacro(a_u4Param);
		break;
		
        default :
      	     DRV201AFDB("[DRV201AF] No CMD \n");
            i4RetValue = -EPERM;
        break;
    }

    return i4RetValue;
}
/*
static void DRV201AF_WORK(struct work_struct *work)
{
    g_i4Position += (25 * g_i4Dir);

    if ((g_i4Dir == 1) && (g_i4Position >= (long)g_u4TargetPosition))
	{
        g_i4Position = (long)g_u4TargetPosition;
        g_i4MotorStatus = 0;
    }

    if ((g_i4Dir == -1) && (g_i4Position <= (long)g_u4TargetPosition))
    {
        g_i4Position = (long)g_u4TargetPosition;
        g_i4MotorStatus = 0; 		
    }
	
    if(s4DRV201AF_WriteReg((unsigned short)g_i4Position) == 0)
    {
        g_u4CurrPosition = (unsigned long)g_i4Position;
    }
    else
    {
        DRV201AFDB("[DRV201AF] set I2C failed when moving the motor \n");
        g_i4MotorStatus = -1;
    }
}

static void DRV201AF_ISR(UINT16 a_input)
{
	if (g_i4MotorStatus == 1)
	{	
		schedule_work(&g_stWork);		
	}
}
*/
//Main jobs:
// 1.check for device-specified errors, device not ready.
// 2.Initialize the device if it is opened for the first time.
// 3.Update f_op pointer.
// 4.Fill data structures into private_data
//CAM_RESET
static int DRV201AF_Open(struct inode * a_pstInode, struct file * a_pstFile)
{
	char pwm_linear[2] = {0x06, 0x02};
	iWriteRegI2C(pwm_linear , 2, DRV201AF_VCM_WRITE_ID);
	
    spin_lock(&g_DRV201AF_SpinLock);

    if(g_s4DRV201AF_Opened)
    {
        spin_unlock(&g_DRV201AF_SpinLock);
        DRV201AFDB("[DRV201AF] the device is opened \n");
        return -EBUSY;
    }

    g_s4DRV201AF_Opened = 1;
		
    spin_unlock(&g_DRV201AF_SpinLock);

	// --- Config Interrupt ---
	//g_GPTconfig.num = XGPT7;
	//g_GPTconfig.mode = XGPT_REPEAT;
	//g_GPTconfig.clkDiv = XGPT_CLK_DIV_1;//32K
	//g_GPTconfig.u4Compare = 32*2; // 2ms
	//g_GPTconfig.bIrqEnable = TRUE;
	
	//XGPT_Reset(g_GPTconfig.num);	
	//XGPT_Init(g_GPTconfig.num, DRV201AF_ISR);

	//if (XGPT_Config(g_GPTconfig) == FALSE)
	//{
        //DRV201AFDB("[DRV201AF] ISR Config Fail\n");	
	//	return -EPERM;
	//}

	//XGPT_Start(g_GPTconfig.num);		

	// --- WorkQueue ---	
	//INIT_WORK(&g_stWork,DRV201AF_WORK);

    return 0;
}

//Main jobs:
// 1.Deallocate anything that "open" allocated in private_data.
// 2.Shut down the device on last close.
// 3.Only called once on last time.
// Q1 : Try release multiple times.
static int DRV201AF_Release(struct inode * a_pstInode, struct file * a_pstFile)
{
	unsigned int cnt = 0;

	if (g_s4DRV201AF_Opened)
	{
		moveDRV201AF(g_u4DRV201AF_INF);

		while(g_i4MotorStatus)
		{
			msleep(1);
			cnt++;
			if (cnt>1000)	{break;}
		}
		
    	spin_lock(&g_DRV201AF_SpinLock);

	    g_s4DRV201AF_Opened = 0;

    	spin_unlock(&g_DRV201AF_SpinLock);

    	//hwPowerDown(CAMERA_POWER_VCAM_A,"kd_camera_hw");

		//XGPT_Stop(g_GPTconfig.num);
	}

    return 0;
}

static const struct file_operations g_stDRV201AF_fops = 
{
    .owner = THIS_MODULE,
    .open = DRV201AF_Open,
    .release = DRV201AF_Release,
    .unlocked_ioctl = DRV201AF_Ioctl
};

inline static int Register_DRV201AF_CharDrv(void)
{
    struct device* vcm_device = NULL;

    //Allocate char driver no.
    if( alloc_chrdev_region(&g_DRV201AF_devno, 0, 1,DRV201AF_DRVNAME) )
    {
        DRV201AFDB("[DRV201AF] Allocate device no failed\n");

        return -EAGAIN;
    }

    //Allocate driver
    g_pDRV201AF_CharDrv = cdev_alloc();

    if(NULL == g_pDRV201AF_CharDrv)
    {
        unregister_chrdev_region(g_DRV201AF_devno, 1);

        DRV201AFDB("[DRV201AF] Allocate mem for kobject failed\n");

        return -ENOMEM;
    }

    //Attatch file operation.
    cdev_init(g_pDRV201AF_CharDrv, &g_stDRV201AF_fops);

    g_pDRV201AF_CharDrv->owner = THIS_MODULE;

    //Add to system
    if(cdev_add(g_pDRV201AF_CharDrv, g_DRV201AF_devno, 1))
    {
        DRV201AFDB("[DRV201AF] Attatch file operation failed\n");

        unregister_chrdev_region(g_DRV201AF_devno, 1);

        return -EAGAIN;
    }

    actuator_class = class_create(THIS_MODULE, "actuatordrv");
    if (IS_ERR(actuator_class)) {
        int ret = PTR_ERR(actuator_class);
        DRV201AFDB("Unable to create class, err = %d\n", ret);
        return ret;            
    }

    vcm_device = device_create(actuator_class, NULL, g_DRV201AF_devno, NULL, DRV201AF_DRVNAME);

    if(NULL == vcm_device)
    {
        return -EIO;
    }
    
    return 0;
}

inline static void Unregister_DRV201AF_CharDrv(void)
{
    //Release char driver
    cdev_del(g_pDRV201AF_CharDrv);

    unregister_chrdev_region(g_DRV201AF_devno, 1);
    
    device_destroy(actuator_class, g_DRV201AF_devno);

    class_destroy(actuator_class);
}

//////////////////////////////////////////////////////////////////////
/* Kirby: remove old-style driver
static int DRV201AF_i2c_attach(struct i2c_adapter * a_pstAdapter);
static int DRV201AF_i2c_detach_client(struct i2c_client * a_pstClient);
static struct i2c_driver DRV201AF_i2c_driver = {
    .driver = {
    .name = DRV201AF_DRVNAME,
    },
    //.attach_adapter = DRV201AF_i2c_attach,
    //.detach_client = DRV201AF_i2c_detach_client
};*/

/* Kirby: add new-style driver { */
static int DRV201AF_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info);
static int DRV201AF_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int DRV201AF_i2c_remove(struct i2c_client *client);
static struct i2c_board_info __initdata kd_lens_dev={ I2C_BOARD_INFO("DRV201AF", DRV201AF_VCM_WRITE_ID>>1)};
static const struct i2c_device_id DRV201AF_i2c_id[] = {{DRV201AF_DRVNAME,0},{}};   
//static unsigned short force[] = {IMG_SENSOR_I2C_GROUP_ID, DRV201AF_VCM_WRITE_ID, I2C_CLIENT_END, I2C_CLIENT_END};   
//static const unsigned short * const forces[] = { force, NULL };              
//static struct i2c_client_address_data addr_data = { .forces = forces,}; 
struct i2c_driver DRV201AF_i2c_driver = {                       
    .probe = DRV201AF_i2c_probe,                                   
    .remove = DRV201AF_i2c_remove,                           
    .detect = DRV201AF_i2c_detect,                           
    .driver.name = DRV201AF_DRVNAME,                 
    .id_table = DRV201AF_i2c_id,                             
    //.address_data = &addr_data,                        
};  

static int DRV201AF_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) {         
    strcpy(info->type, DRV201AF_DRVNAME);                                                         
    return 0;                                                                                       
}                                                                                                  
static int DRV201AF_i2c_remove(struct i2c_client *client) {
    return 0;
}
/* Kirby: } */


/* Kirby: remove old-style driver
int DRV201AF_i2c_foundproc(struct i2c_adapter * a_pstAdapter, int a_i4Address, int a_i4Kind)
*/
/* Kirby: add new-style driver {*/
static int DRV201AF_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
/* Kirby: } */
{
    int i4RetValue = 0;

    DRV201AFDB("[DRV201AF] Attach I2C \n");

    /* Kirby: remove old-style driver
    //Check I2C driver capability
    if (!i2c_check_functionality(a_pstAdapter, I2C_FUNC_SMBUS_BYTE_DATA))
    {
        DRV201AFDB("[DRV201AF] I2C port cannot support the format \n");
        return -EPERM;
    }

    if (!(g_pstDRV201AF_I2Cclient = kzalloc(sizeof(struct i2c_client), GFP_KERNEL)))
    {
        return -ENOMEM;
    }

    g_pstDRV201AF_I2Cclient->addr = a_i4Address;
    g_pstDRV201AF_I2Cclient->adapter = a_pstAdapter;
    g_pstDRV201AF_I2Cclient->driver = &DRV201AF_i2c_driver;
    g_pstDRV201AF_I2Cclient->flags = 0;

    strncpy(g_pstDRV201AF_I2Cclient->name, DRV201AF_DRVNAME, I2C_NAME_SIZE);

    if(i2c_attach_client(g_pstDRV201AF_I2Cclient))
    {
        kfree(g_pstDRV201AF_I2Cclient);
    }
    */
    /* Kirby: add new-style driver { */
    g_pstDRV201AF_I2Cclient = client;
    /* Kirby: } */

    //Register char driver
    i4RetValue = Register_DRV201AF_CharDrv();

    if(i4RetValue){

        DRV201AFDB("[DRV201AF] register char device failed!\n");

        /* Kirby: remove old-style driver
        kfree(g_pstDRV201AF_I2Cclient); */

        return i4RetValue;
    }

    spin_lock_init(&g_DRV201AF_SpinLock);

    DRV201AFDB("[DRV201AF] Attached!! \n");

    return 0;
}

/* Kirby: remove old-style driver
static int DRV201AF_i2c_attach(struct i2c_adapter * a_pstAdapter)
{

    if(a_pstAdapter->id == 0)
    {
    	 return i2c_probe(a_pstAdapter, &g_stDRV201AF_Addr_data ,  DRV201AF_i2c_foundproc);
    }

    return -1;

}

static int DRV201AF_i2c_detach_client(struct i2c_client * a_pstClient)
{
    int i4RetValue = 0;

    Unregister_DRV201AF_CharDrv();

    //detach client
    i4RetValue = i2c_detach_client(a_pstClient);
    if(i4RetValue)
    {
        dev_err(&a_pstClient->dev, "Client deregistration failed, client not detached.\n");
        return i4RetValue;
    }

    kfree(i2c_get_clientdata(a_pstClient));

    return 0;
}*/

static int DRV201AF_probe(struct platform_device *pdev)
{
    return i2c_add_driver(&DRV201AF_i2c_driver);
}

static int DRV201AF_remove(struct platform_device *pdev)
{
    i2c_del_driver(&DRV201AF_i2c_driver);
    return 0;
}

static int DRV201AF_suspend(struct platform_device *pdev, pm_message_t mesg)
{
//    int retVal = 0;
//    retVal = hwPowerDown(MT6516_POWER_VCAM_A,DRV201AF_DRVNAME);

    return 0;
}

static int DRV201AF_resume(struct platform_device *pdev)
{
/*
    if(TRUE != hwPowerOn(MT6516_POWER_VCAM_A, VOL_2800,DRV201AF_DRVNAME))
    {
        DRV201AFDB("[DRV201AF] failed to resume DRV201AF\n");
        return -EIO;
    }
*/
    return 0;
}

// platform structure
static struct platform_driver g_stDRV201AF_Driver = {
    .probe		= DRV201AF_probe,
    .remove	= DRV201AF_remove,
    .suspend	= DRV201AF_suspend,
    .resume	= DRV201AF_resume,
    .driver		= {
        .name	= "lens_actuator",
        .owner	= THIS_MODULE,
    }
};

static int __init DRV201AF_i2C_init(void)
{
	i2c_register_board_info(IMG_SENSOR_I2C_GROUP_ID, &kd_lens_dev, 1);
	
    if(platform_driver_register(&g_stDRV201AF_Driver)){
        DRV201AFDB("failed to register DRV201AF driver\n");
        return -ENODEV;
    }

    return 0;
}

static void __exit DRV201AF_i2C_exit(void)
{
	platform_driver_unregister(&g_stDRV201AF_Driver);
}

module_init(DRV201AF_i2C_init);
module_exit(DRV201AF_i2C_exit);

MODULE_DESCRIPTION("DRV201AF lens module driver");
MODULE_AUTHOR("Gipi Lin <Gipi.Lin@Mediatek.com>");
MODULE_LICENSE("GPL");
