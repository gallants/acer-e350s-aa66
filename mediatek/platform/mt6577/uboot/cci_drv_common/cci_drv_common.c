#include <asm/arch/mt6577_gpio.h>
#include <asm/arch/cci_drv_common.h>
#include <asm/arch/mt65xx_typedefs.h>

/************************************************/
/*    Phase_ID:                                 */
/*              GPIO102  GPIO101  GPIO100       */
/*    EVT        0        0        0            */
/*    DVT1       0        0        1            */
/*    DVT2       0        1        0            */
/*    PVT        0        1        1            */
/*    MP         1        0        0            */
/*                                              */
/*    Project_ID:                               */
/*              GPIO105                         */
/*    AA66       0                              */
/*    AA67       1                              */
/*                                              */
/*    Band_ID:                                  */
/*              GPIO109                         */
/*    Band 1,5   0                              */
/*    Band 1,8   1                              */
/************************************************/
                                                    
//         GPIO100  GPIO101   GPIO102   GPIO105   GPIO109  
//         (HW_ID1) (HW_ID2)  (HW_ID3)  (HW_ID4)  (HW_ID5) 
//  0x00 |  0        0         0         0           0      HA01_DVT1_Band1,8                    
//  0x01 |  1        0         0         0           0      HA01_DVT2_Band1,8
//  0x02 |  0        1         0         0           0      HA01_PVT_Band1,8 
//  0x08 |  0        0         0         1           0      HA03_DVT1_Band1,8
//  0x09 |  1        0         0         1           0      HA03_DVT2_Band1,8
//  0x0A |  0        1         0         1           0      HA03_PVT_Band1,8
//  0x10 |  0        0         0         0           1      HA01_DVT1_Band2,5
//  0x11 |  1        0         0         0           1      HA01_DVT2_Band2,5
//  0x12 |  0        1         0         0           1      HA01_PVT_Band2,5       
//  0x18 |  0        0         0         1           1      HA03_DVT1_Band2,5
//  0x19 |  1        0         0         1           1      HA03_DVT2_Band2,5
//  0x1A |  0        1         0         1           1      HA03_PVT_Band2,5
                                                           
#if 0  //---start add by Rachel
static s32 platform_id = -1;

static void cci_get_platform_id(void)
{
	platform_id = 0;

	platform_id |= (mt_get_gpio_in(100)&0x01)?0x01:0x00;
	platform_id |= (mt_get_gpio_in(101)&0x01)?0x02:0x00;
	platform_id |= (mt_get_gpio_in(102)&0x01)?0x04:0x00;
	platform_id |= (mt_get_gpio_in(105)&0x01)?0x08:0x00;
	platform_id |= (mt_get_gpio_in(109)&0x01)?0x10:0x00;
}

bool cci_platform_is_evt(void)
{
        if(platform_id == -1)
          cci_get_platform_id();
        printf("*************************************************\n");
        printf("*  CCI-Uboot get platform id, platform_id = %d  *\n",platform_id); 
        printf("*************************************************\n");
        if((platform_id&0X07) == 0x00)
          return true;
        return false;
}

bool cci_platform_is_dvt1(void)
{
        if(platform_id == -1)
          cci_get_platform_id();
        printf("*************************************************\n");
        printf("*  CCI-Uboot get platform id, platform_id = %d  *\n",platform_id); 
        printf("*************************************************\n");
        if((platform_id&0X07) == 0x01)
          return true;
        return false;
}

bool cci_platform_is_dvt2(void)
{
        if(platform_id == -1)
          cci_get_platform_id();
        printf("*************************************************\n");
        printf("*  CCI-Uboot get platform id, platform_id = %d  *\n",platform_id); 
        printf("*************************************************\n");
        if((platform_id&0X07) == 0x02)
          return true;
        return false;
}

bool cci_platform_is_pvt(void)
{
        if(platform_id == -1)
          cci_get_platform_id();
        printf("*************************************************\n");
        printf("*  CCI-Uboot get platform id, platform_id = %d  *\n",platform_id); 
        printf("*************************************************\n");
        if((platform_id&0X07) == 0x03)
          return true;
        return false;
}

bool cci_platform_is_mp(void)
{
        if(platform_id == -1)
          cci_get_platform_id();
        printf("*************************************************\n");
        printf("*  CCI-Uboot get platform id, platform_id = %d  *\n",platform_id); 
        printf("*************************************************\n");
        if((platform_id&0X07) == 0x04)
          return true;
        return false;
}
#endif  //---end add by Rachel


//---start add by Rachel

static unsigned revision_gpio[] = {100,101,102,105,109};

static unsigned target_get_board_revision_no(void)
{
    unsigned revision = 0 , i = 0;

    for(i = 0 ;i < ARRAY_SIZE(revision_gpio) ; i++)
    {
        if(mt_get_gpio_in(revision_gpio[i])&0x01)
        {
            revision &= (1 << i);
        }
    }
    
    return revision;
}

extern char *CCI_UBOOT_PLATFORM_BOOTARG;

//unsigned* target_atag_revision()
void target_atag_revision()
{
    static s32 product_id = -1;
    
   product_id = target_get_board_revision_no();
   switch(product_id & 0x07)
   {
        case 0x00:
            CCI_UBOOT_PLATFORM_BOOTARG = "EVT";
            break;
            
        case 0x01:
            CCI_UBOOT_PLATFORM_BOOTARG = "DVT1";
            break;
            
        case 0x02:
            CCI_UBOOT_PLATFORM_BOOTARG = "DVT2";
            break;
            
        case 0x03:
            CCI_UBOOT_PLATFORM_BOOTARG = "PVT";
            break;
            
        case 0x04:
            CCI_UBOOT_PLATFORM_BOOTARG = "MP";
            break;
            
        case 0x05:
            CCI_UBOOT_PLATFORM_BOOTARG = "PM-B";
            break;
            
        case 0x06:
            CCI_UBOOT_PLATFORM_BOOTARG = "PM-W";
            break;
            
        default:
            break;
   }
   // return bd;
}

void bmt_charger_ov_check(void)
{
	aa66_bmt_charger_ov_check();
}

kal_bool pmic_chrdet_status(void)
{
    aa66_pmic_chrdet_status();
}

//---end add by Rachel
