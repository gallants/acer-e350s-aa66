//#include <platform/cust_leds.h>
#include <cust_leds.h>
//#include <asm/arch/mt6577_pwm.h>

extern int DISP_SetBacklight(int level);

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
	//{"lcd-backlight",     MT65XX_LED_MODE_CUST, (int)DISP_SetBacklight,{0}},
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
	//{"lcd-backlight",     MT65XX_LED_MODE_CUST, (int)DISP_SetBacklight,{0}},
	{"lcd-backlight",     MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_LCD_BOOST,{0}},
};

extern unsigned target_get_board_hwid_no(void);

struct cust_mt65xx_led *get_cust_led_list(void)
{
    if((target_get_board_hwid_no()& 0x07) >= 2)
        return cust_led_list_pvt;
    else
	return cust_led_list_dvt;
}

