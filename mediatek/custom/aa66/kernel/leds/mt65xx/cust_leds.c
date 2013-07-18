#include <cust_leds.h>
#include <mach/mt_pwm.h>

#include <linux/kernel.h>
#include <mach/pmic_mt6329_hw_bank1.h> 
#include <mach/pmic_mt6329_sw_bank1.h> 
#include <mach/pmic_mt6329_hw.h>
#include <mach/pmic_mt6329_sw.h>
#include <mach/upmu_common_sw.h>
#include <mach/upmu_hw.h>

#define ERROR_BL_LEVEL 0xFFFFFFFF

extern int mtkfb_set_backlight_level(unsigned int level);
extern int mtkfb_set_backlight_pwm(int div);

unsigned int brightness_mapping(unsigned int level)
{
	if(level ==1 )return 999;
    
	if(level>=20 && level<=255) { 
    
		return ( (level-20)/35 + 1 ); 
	}
    else if(level>0 && level<20) { 
		return 0;
	}
#if 0
	if(level ==1 )return 999;
	
	if(level>=30 && level<=255) { 
    
		return ( (level-14)/10 -1 ); 
	}
	else if(level>0 && level<30) { 
		return 0;
	}
#endif	
	return ERROR_BL_LEVEL;
}

unsigned int Cust_SetBacklight(int level, int div)
{
    mtkfb_set_backlight_pwm(div);
    mtkfb_set_backlight_level(brightness_mapping(level));
    return 0;
}

static struct cust_mt65xx_led cust_led_list_pvt[MT65XX_LED_TYPE_TOTAL] = {
	//{"red",               MT65XX_LED_MODE_NONE, -1,{0}},
	{"red",               MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK4,{0}},
	//{"green",             MT65XX_LED_MODE_NONE, -1,{0}},
	{"green",            MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK5,{0}},
	//{"blue",              MT65XX_LED_MODE_NONE, -1,{0}},
	{"blue",              MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK0,{0}}, 
	{"jogball-backlight", MT65XX_LED_MODE_NONE, -1,{0}},
	{"keyboard-backlight",MT65XX_LED_MODE_NONE, -1,{0}},
	{"button-backlight",  MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_BUTTON,{0}},
	//{"lcd-backlight",     MT65XX_LED_MODE_CUST, (int)Cust_SetBacklight,{0}},
	{"lcd-backlight",     MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_LCD_BOOST,{0}},
};

static struct cust_mt65xx_led cust_led_list_dvt[MT65XX_LED_TYPE_TOTAL] = {
	//{"red",               MT65XX_LED_MODE_NONE, -1,{0}},
	{"red",               MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK0,{0}},
	//{"green",             MT65XX_LED_MODE_NONE, -1,{0}},
	{"green",            MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK4,{0}},
	//{"blue",              MT65XX_LED_MODE_NONE, -1,{0}},
	{"blue",              MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK5,{0}}, 
	{"jogball-backlight", MT65XX_LED_MODE_NONE, -1,{0}},
	{"keyboard-backlight",MT65XX_LED_MODE_NONE, -1,{0}},
	{"button-backlight",  MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_BUTTON,{0}},
	//{"lcd-backlight",     MT65XX_LED_MODE_CUST, (int)Cust_SetBacklight,{0}},
	{"lcd-backlight",     MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_LCD_BOOST,{0}},
};
extern unsigned int HWID;

struct cust_mt65xx_led *get_cust_led_list(void)
{
   if(HWID>=2 )
       return cust_led_list_pvt;
   else 
	return cust_led_list_dvt;
}



