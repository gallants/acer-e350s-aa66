#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <cust_alsps.h>
//#include <mach/mt6577_pm_ldo.h>
static struct alsps_hw cust_alsps_hw = {
    .i2c_num    = 0,
	.polling_mode_ps =0,
	.polling_mode_als =1,
    .power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
    .power_vol  = VOL_DEFAULT,          /*LDO is not used*/
    .i2c_addr   = {0x0C, 0x48, 0x78, 0x00},
    .als_level  = { 100,  5210,10000,10000,10000,10000,10000,10000,10000,10000,10000,10000,10000,10000,20000},
    .als_value  = { 31, 1000, 8000, 8000, 8000, 8000, 8000, 8000, 8000, 8000, 8000, 8000, 8000, 8000, 8000, 8000},
    .ps_threshold = 5,	//3,
};
struct alsps_hw *get_cust_alsps_hw(void) {
    return &cust_alsps_hw;
}
int C_CUST_ALS_FACTOR=156;
