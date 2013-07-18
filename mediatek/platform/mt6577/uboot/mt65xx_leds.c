/******************************************************************************
 * mt65xx_leds.c
 * 
 * Copyright 2010 MediaTek Co.,Ltd.
 * 
 * DESCRIPTION:
 *
 ******************************************************************************/

#include <common.h>
#include <asm/arch/mt65xx.h>
#include <asm/arch/mt65xx_typedefs.h>


// FIXME: should include power related APIs

#include <asm/arch/mt6577_pwm.h>
#include <asm/arch/mt6577_gpio.h>
#include <asm/arch/mt65xx_leds.h>
#include <asm/io.h>

#include <asm/arch/mt6577_pmic6329.h> 


#include <asm/arch/boot_mode.h>

/****************************************************************************
 * DEBUG MACROS
 ***************************************************************************/
int debug_enable = 0;
#define LEDS_DEBUG(format, args...) do{ \
		if(debug_enable) \
		{\
	//		printf(format,##args);\
		}\
	}while(0)
#define LEDS_INFO LEDS_DEBUG 	
/****************************************************************************
 * structures
 ***************************************************************************/
static int g_lastlevel[MT65XX_LED_TYPE_TOTAL] = {-1, -1, -1, -1, -1, -1, -1};
int backlight_PWM_div = CLK_DIV1;


//---start add by Rachel
struct nled_setting
{
	u8 nled_mode; //0, off; 1, on; 2, blink;
	u32 blink_on_time ;
	u32 blink_off_time;
};

struct mt65xx_led_data *blink_led_data;
//struct wake_lock leds_suspend_lock;
#define NLED_BLINK 2
#define NLED_OFF 0
#define NLED_ON 1
//---end add by Rachel


/****************************************************************************
 * function prototypes
 ***************************************************************************/

/* import functions */
// FIXME: should extern from pmu driver
//void pmic_backlight_on(void) {}
//void pmic_backlight_off(void) {}
//void pmic_config_interface(kal_uint16 RegNum, kal_uint8 val, kal_uint16 MASK, kal_uint16 SHIFT) {}

/* internal functions */
static int brightness_set_pwm(int pwm_num, enum led_brightness level,struct PWM_config *config_data);
static int led_set_pwm(int pwm_num, enum led_brightness level);
static int brightness_set_pmic(enum mt65xx_led_pmic pmic_type, enum led_brightness level);
static int brightness_set_gpio(int gpio_num, enum led_brightness level);
static int mt65xx_led_set_cust(struct cust_mt65xx_led *cust, int level);

/****************************************************************************
 * global variables
 ***************************************************************************/

/****************************************************************************
 * internal functions
 ***************************************************************************/
static int brightness_set_pwm(int pwm_num, enum led_brightness level,struct PWM_config *config_data)
{
struct pwm_spec_config pwm_setting;
	
	pwm_setting.pwm_no = pwm_num;
	pwm_setting.mode = PWM_MODE_FIFO; //new mode fifo and periodical mode
//	pwm_setting.clk_div = CLK_DIV1;
//	pwm_setting.clk_src = PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625;
	if(config_data->div)
	{
		pwm_setting.clk_div = config_data->div;
		backlight_PWM_div = config_data->div;
	}
   else
     pwm_setting.clk_div = CLK_DIV1;
	if(config_data->clock_source)
		pwm_setting.clk_src = PWM_CLK_NEW_MODE_BLOCK;
	else
		pwm_setting.clk_src = PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625;

	if(config_data->High_duration && config_data->low_duration)
		{
			pwm_setting.PWM_MODE_FIFO_REGS.HDURATION = config_data->High_duration;
			pwm_setting.PWM_MODE_FIFO_REGS.LDURATION = config_data->low_duration;
		}
	else
		{
			pwm_setting.PWM_MODE_FIFO_REGS.HDURATION = 4;
			pwm_setting.PWM_MODE_FIFO_REGS.LDURATION = 4;
		}
	
	pwm_setting.PWM_MODE_FIFO_REGS.IDLE_VALUE = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.GUARD_VALUE = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.STOP_BITPOS_VALUE = 63;
//	pwm_setting.PWM_MODE_FIFO_REGS.HDURATION = 4;
//	pwm_setting.PWM_MODE_FIFO_REGS.LDURATION = 4;
	pwm_setting.PWM_MODE_FIFO_REGS.GDURATION = 0;
	pwm_setting.PWM_MODE_FIFO_REGS.WAVE_NUM = 0;
	
	printf("[LEDS]uboot: backlight_set_pwm:duty is %d\n", level);
  
	if(level > 0)
	{
            pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA0 = ((1 << 30) - 1);
	    pwm_setting.PWM_MODE_FIFO_REGS.SEND_DATA1 = 0 ;
		pwm_set_spec_config(&pwm_setting);
        }
        else
 	{
 	    mt_set_pwm_disable(pwm_setting.pwm_no);
	    mt_power_off (pwm_setting.pwm_no);
        }
	
	
	return 0;
	
}


static int led_set_pwm(int pwm_num, enum led_brightness level)
{
	struct pwm_easy_config pwm_setting;
	pwm_setting.pwm_no = pwm_num;
	pwm_setting.clk_div = CLK_DIV1; 		
	pwm_setting.duration = 10;
    
    if(pwm_num != PWM3 && pwm_num != PWM4 && pwm_num != PWM5)
		pwm_setting.clk_src = PWM_CLK_OLD_MODE_32K;
	else
		pwm_setting.clk_src = PWM_CLK_NEW_MODE_BLOCK_DIV_BY_1625;
    
	if(level)
	{
		pwm_setting.duty = 30;
	}else
	{
		pwm_setting.duty = 0;
	}
	printf("[LEDS]uboot: brightness_set_pwm: level=%d, clk=%d \n\r", level, pwm_setting.clk_src);
	pwm_set_easy_config(&pwm_setting);
	
	return 0;
	
}

static int brightness_set_pmic(enum mt65xx_led_pmic pmic_type, enum led_brightness level)
{
	int temp_level;
	
	printf("[LEDS]LED PMIC#%d:%d\n", pmic_type, level);

		if (pmic_type == MT65XX_LED_PMIC_LCD_ISINK)
	{
			if(level)
			{
				//hwBacklightISINKTuning(1, PMIC_PWM_0, 0x3, 0);
				upmu_boost_isink_hw_sel(0x0);
				upmu_isinks_ch1_mode(0);
				upmu_isinks_ch1_step(3);
				upmu_isinks_ch1_cabc_en(0);	

				//hwBacklightISINKTuning(2, PMIC_PWM_0, 0x3, 0);
				upmu_isinks_ch2_mode(0);
				upmu_isinks_ch2_step(3);
				upmu_isinks_ch2_cabc_en(0);	

				//hwBacklightISINKTuning(3, PMIC_PWM_0, 0x3, 0);
				upmu_isinks_ch3_mode(0);
				upmu_isinks_ch3_step(3);
				upmu_isinks_ch3_cabc_en(0);	

				//hwPWMsetting(PMIC_PWM_0, 6, 0);
				upmu_isinks_dim0_duty(6);
				upmu_isinks_dim0_fsel(0);

				//hwBacklightISINKTurnOn(MT65XX_LED_PMIC_NLED_ISINK5);
				upmu_top2_bst_drv_ck_pdn(0x0);
				upmu_isinks_ch1_en(0x1);
				upmu_isinks_ch2_en(0x1);
				upmu_isinks_ch3_en(0x1);
			}
			else
			{
				upmu_isinks_ch1_en(0x0);
				upmu_isinks_ch2_en(0x0);
				upmu_isinks_ch3_en(0x0);
			}
			
			return 0;
		}
		else if(pmic_type == MT65XX_LED_PMIC_LCD_BOOST)
		{
			if (level) 
			{
				temp_level = level/17;
				printf("[LEDS]MT65XX_LED_PMIC_LCD_BOOST:level=%d  temp_level=%d\n", level, temp_level);
				//hwBacklightBoostTuning(PMIC_PWM_0, 0xA, 0);
				upmu_boost_isink_hw_sel(0x1);
				upmu_boost_mode(0);
				upmu_boost_vrsel(0xA);
				upmu_boost_cabc_en(0);
	
				//hwPWMsetting(PMIC_PWM_0, level, div);
				upmu_isinks_dim0_duty(temp_level);
				upmu_isinks_dim0_fsel(0);
				
				upmu_top2_bst_drv_ck_pdn(0x0);
				upmu_boost_en(0x1);
			}
			else 
			{	
				printf("[LEDS]MT65XX_LED_PMIC_LCD_BOOST:level=%d\n", level);
				//pmic_bank1_config_interface(0x3F, 0x00, 0xFF, 0x0);
				upmu_boost_en(0x0);
			}
		return 0;
	}
		else if(pmic_type == MT65XX_LED_PMIC_NLED_ISINK4)
	{

			if (level) 
			{
			     upmu_isinks_ch4_mode(3);   //register mode
                 upmu_isinks_ch4_step(0);  //0: 4ma  1:8ma
                 upmu_isinks_ch4_cabc_en(0);         
				
				upmu_top2_bst_drv_ck_pdn(0x0);
				upmu_isinks_ch4_en(0x1);
			}
			else 
			{
				upmu_isinks_ch4_en(0x0);
			}
		return 0;
	}
		else if(pmic_type == MT65XX_LED_PMIC_NLED_ISINK5)
		{
				
			if (level) 
			{
				upmu_isinks_ch5_mode(3);   //register mode
				upmu_isinks_ch5_step(0);  //0: 4ma  1:8ma
				upmu_isinks_ch5_cabc_en(0); 		
				
				upmu_top2_bst_drv_ck_pdn(0x0);
				upmu_isinks_ch5_en(0x1);
			}
			else 
			{
				upmu_isinks_ch5_en(0x0);
			}
			return 0;
		}
	    else if(pmic_type == MT65XX_LED_PMIC_NLED_ISINK0)
		{
				
			if (level) 
			{
				upmu_isinks_ch0_mode(3);
				upmu_isinks_ch0_step(0);   //0: 4ma  1:8ma
				upmu_isinks_ch0_cabc_en(0);	
				
				upmu_top2_bst_drv_ck_pdn(0x0);
				upmu_isinks_ch0_en(0x1);
			}
			else 
			{
				upmu_isinks_ch0_en(0x0);
			}
			return 0;
		}
		
	return -1;
}

static int brightness_set_gpio(int gpio_num, enum led_brightness level)
{
//	LEDS_INFO("LED GPIO#%d:%d\n", gpio_num, level);
	mt_set_gpio_mode(gpio_num, GPIO_MODE_00);// GPIO MODE
	mt_set_gpio_dir(gpio_num, GPIO_DIR_OUT);

	if (level)
		mt_set_gpio_out(gpio_num, GPIO_OUT_ONE);
	else
		mt_set_gpio_out(gpio_num, GPIO_OUT_ZERO);

	return 0;
}

static int mt65xx_led_set_cust(struct cust_mt65xx_led *cust, int level)
{
	if (level > LED_FULL)
		level = LED_FULL;
	else if (level < 0)
		level = 0;

	switch (cust->mode) {
		case MT65XX_LED_MODE_PWM:
			if(strcmp(cust->name,"lcd-backlight") == 0)
			{
			return brightness_set_pwm(cust->data, level,&cust->config_data);
			}
			else
			{
				return led_set_pwm(cust->data, level);
			}
		case MT65XX_LED_MODE_GPIO:
			return brightness_set_gpio(cust->data, level);
		case MT65XX_LED_MODE_PMIC:
			return brightness_set_pmic(cust->data, level);
		case MT65XX_LED_MODE_CUST:
			return ((cust_brightness_set)(cust->data))(level);
		case MT65XX_LED_MODE_NONE:
		default:
			break;
	}
	return -1;
}

/****************************************************************************
 * external functions
 ***************************************************************************/
int mt65xx_leds_brightness_set(enum mt65xx_led_type type, enum led_brightness level)
{
	struct cust_mt65xx_led *cust_led_list = get_cust_led_list();

	if (type < 0 || type >= MT65XX_LED_TYPE_TOTAL)
		return -1;

	if (level > LED_FULL)
		level = LED_FULL;
	else if (level < 0)
		level = 0;

	if (g_lastlevel[type] != level) {
		g_lastlevel[type] = level;
		printf("[LEDS]uboot: %s level is %d \n\r", cust_led_list[type].name, level);
		return mt65xx_led_set_cust(&cust_led_list[type], level);
	}
	else {
		return -1;
	}

}

//---start add by Rachel
//#if defined (CONFIG_ARCH_MT6577)
#define PMIC_PERIOD_NUM 15
int pmic_period_array[] = {250,500,1000,1250,1666,2000,2500,3333,4000,5000,6666,8000,10000,13333,16000};
u8 pmic_clksel_array[] = {0,0,0,0,0,0,1,1,1,2,2,2,3,3,3};
u8 pmic_freqsel_array[] = {24,26,28,29,30,31,29,30,31,29,30,31,29,30,31};

int find_time_index_pmic(int time_ms) {
	int i;
	for(i=0;i<PMIC_PERIOD_NUM;i++) {
		if(time_ms<=pmic_period_array[i]) {
			return i;
		} else {
			continue;
		}
	}
	return PMIC_PERIOD_NUM-1;
}

static int led_blink_pmic(enum mt65xx_led_pmic pmic_type, struct nled_setting* led) 
{
	int time_index = 0;
	int duty = 0;
	
	if((pmic_type != MT65XX_LED_PMIC_NLED_ISINK0 && pmic_type != MT65XX_LED_PMIC_NLED_ISINK4 && pmic_type!= MT65XX_LED_PMIC_NLED_ISINK5) || (led->nled_mode != NLED_BLINK)) 
	{
		return -1;
	}

	time_index = find_time_index_pmic(led->blink_on_time + led->blink_off_time);
	
	duty=32*led->blink_on_time/(led->blink_on_time + led->blink_off_time);
	
	switch(pmic_type){
	       case MT65XX_LED_PMIC_NLED_ISINK0:
			upmu_isinks_ch0_mode(1); //---Rachel PMIC_PWM_1
			upmu_isinks_ch0_step(0);  //0:4mA  1:8mA
			upmu_isinks_ch0_cabc_en(0);
			upmu_isinks_dim1_duty(duty);
			upmu_isinks_dim1_fsel(pmic_freqsel_array[time_index]);
			pmic_bank1_config_interface(0x2e, pmic_clksel_array[time_index], 0x3, 0x6);
			upmu_top2_bst_drv_ck_pdn(0x0);
			//hwBacklightISINKTurnOn(MT65XX_LED_PMIC_NLED_ISINK4);
			upmu_isinks_ch0_en(0x1);
		       break;
		       
		case MT65XX_LED_PMIC_NLED_ISINK4:
			upmu_isinks_ch4_mode(1); //---Rachel PMIC_PWM_1
			upmu_isinks_ch4_step(0);   //0:4mA  1:8mA
			upmu_isinks_ch4_cabc_en(0);
			upmu_isinks_dim1_duty(duty);
			upmu_isinks_dim1_fsel(pmic_freqsel_array[time_index]);
			pmic_bank1_config_interface(0x2e, pmic_clksel_array[time_index], 0x3, 0x6);
			upmu_top2_bst_drv_ck_pdn(0x0);
			//hwBacklightISINKTurnOn(MT65XX_LED_PMIC_NLED_ISINK4);
			upmu_isinks_ch4_en(0x1);
		       break;
		
		case MT65XX_LED_PMIC_NLED_ISINK5:
			upmu_isinks_ch5_mode(2); //--Rachel PMIC_PWM_2
			upmu_isinks_ch5_step(0);   //0:4mA  1:8mA
			upmu_isinks_ch5_cabc_en(0);
			upmu_isinks_dim2_duty(duty);
			upmu_isinks_dim2_fsel(pmic_freqsel_array[time_index]);
			pmic_bank1_config_interface(0x30, pmic_clksel_array[time_index], 0x3, 0x6);
			upmu_top2_bst_drv_ck_pdn(0x0);
			//hwBacklightISINKTurnOn(MT65XX_LED_PMIC_NLED_ISINK5);
			upmu_isinks_ch5_en(0x1);
		       break;
		
		default:
		       break;
	}
			
	return 0;
}
//#endif


int  mt65xx_blink_set(enum mt65xx_led_type type, unsigned long  delay_on, unsigned long  delay_off)
{
	//struct mt65xx_led_data *led_data = led_data_back;
	static int got_wake_lock = 0;
	struct nled_setting nled_tmp_setting = {0,0,0};

	struct cust_mt65xx_led *cust_led_list = get_cust_led_list();
	
	blink_led_data->cust = cust_led_list[type];
	
	// only allow software blink when delay_on or delay_off changed
	if (delay_on != blink_led_data->delay_on || delay_off != blink_led_data->delay_off ||type != blink_led_data->type ) 
	{
		blink_led_data->delay_on = delay_on;
		blink_led_data->delay_off = delay_off;
		blink_led_data->type = type;

		
		if (blink_led_data->delay_on && blink_led_data->delay_off)
		{ // enable blink
		        
			blink_led_data->level = 255; // when enable blink  then to set the level  (255)
			
			if(blink_led_data->cust.mode == MT65XX_LED_MODE_PWM && 
			     (blink_led_data->cust.data != PWM3 && blink_led_data->cust.data != PWM4 && blink_led_data->cust.data != PWM5))
			{
				nled_tmp_setting.nled_mode = NLED_BLINK;
				nled_tmp_setting.blink_off_time = blink_led_data->delay_off;
				nled_tmp_setting.blink_on_time = blink_led_data->delay_on;
				led_blink_pmic(blink_led_data->cust.data, &nled_tmp_setting);
				return 0;
			}
			
			//#if defined (CONFIG_ARCH_MT6575) || defined (CONFIG_ARCH_MT6575T)|| defined (CONFIG_ARCH_MT6577)
			else if((blink_led_data->cust.mode == MT65XX_LED_MODE_PMIC) && (blink_led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK4
				|| blink_led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK5 || blink_led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK0))
			{
				if(get_chip_eco_ver() == CHIP_E2) 
				{
					nled_tmp_setting.nled_mode = NLED_BLINK;
					nled_tmp_setting.blink_off_time = blink_led_data->delay_off;
					nled_tmp_setting.blink_on_time = blink_led_data->delay_on;
					led_blink_pmic(blink_led_data->cust.data, &nled_tmp_setting);
					return 0;
				} 
				else 
				{
				//	wake_lock(&leds_suspend_lock);
				}
			}
			//#endif		
			
			else if (!got_wake_lock) 
			{
			//	wake_lock(&leds_suspend_lock);
				got_wake_lock = 1;
			}
		}
		else if (!blink_led_data->delay_on && !blink_led_data->delay_off)
		{ // disable blink
			if(blink_led_data->cust.mode == MT65XX_LED_MODE_PWM && 
			(blink_led_data->cust.data != PWM3 && blink_led_data->cust.data != PWM4 && blink_led_data->cust.data != PWM5))
			{
				brightness_set_pmic(blink_led_data->cust.data, 0);
				return 0;
			}
			
			//#if defined (CONFIG_ARCH_MT6575) || defined (CONFIG_ARCH_MT6575T)|| defined (CONFIG_ARCH_MT6577)
			else if((blink_led_data->cust.mode == MT65XX_LED_MODE_PMIC) && (blink_led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK4
				|| blink_led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK5 || blink_led_data->cust.data == MT65XX_LED_PMIC_NLED_ISINK0))
			{
				if(get_chip_eco_ver() == CHIP_E2) 
				{
					brightness_set_pmic(blink_led_data->cust.data, 0);
					return 0;
				} 
				else 
				{
				//	wake_unlock(&leds_suspend_lock);
				}
			}
			//#endif	
			
			else if (got_wake_lock) 
			{
			//	wake_unlock(&leds_suspend_lock);
				got_wake_lock = 0;
			}
		}
		return -1;
	}

	// delay_on and delay_off are not changed
	return 0;
}
//---end add by Rachel



void leds_battery_full_charging(void)
{
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_RED, LED_OFF);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_GREEN, LED_FULL);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_BLUE, LED_OFF);
}

void leds_battery_low_charging(int flag)
{
        if(flag)
        {
               mt65xx_blink_set(MT65XX_LED_TYPE_RED, 0, 0);
	        mt65xx_leds_brightness_set(MT65XX_LED_TYPE_RED, LED_OFF);
	        mt65xx_leds_brightness_set(MT65XX_LED_TYPE_GREEN, LED_OFF);
               mt65xx_leds_brightness_set(MT65XX_LED_TYPE_BLUE, LED_FULL);
        }
        else
        {
	        mt65xx_leds_brightness_set(MT65XX_LED_TYPE_RED, LED_OFF);
	        mt65xx_leds_brightness_set(MT65XX_LED_TYPE_GREEN, LED_OFF);
               mt65xx_leds_brightness_set(MT65XX_LED_TYPE_BLUE, LED_OFF);
               mt65xx_blink_set(MT65XX_LED_TYPE_RED, 250, 500);
        }
}

void leds_battery_medium_charging(void)
{
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_RED, LED_FULL);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_GREEN, LED_OFF);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_BLUE, LED_OFF);
}

void leds_init(void)
{
	printf("[LEDS]uboot: leds_init: mt65xx_backlight_off \n\r");
	mt65xx_backlight_off();
}

void isink0_init(void)
{
	printf("[LEDS]uboot: isink0_init: turn on PMIC6329 isink0 \n\r");
	/*
	upmu_isinks_ch0_mode(3);   //register mode
	upmu_isinks_ch0_step(0);  //step 4ma,can customize
	upmu_isinks_ch0_cabc_en(0);	

	upmu_top2_bst_drv_ck_pdn(0x0);
	upmu_isinks_ch0_en(0x1);  //enable the ISINK0
*/
       upmu_isinks_ch5_mode(3);   //register mode
	upmu_isinks_ch5_step(1);  //0: 4ma  1:8ma
	upmu_isinks_ch5_cabc_en(0); 		
				
	upmu_top2_bst_drv_ck_pdn(0x0);
	upmu_isinks_ch5_en(0x1);
				
}

void leds_deinit(void)
{
    printf("[LEDS]uboot: leds_deinit: LEDS off \n\r");
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_RED, LED_OFF);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_GREEN, LED_OFF);
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_BLUE, LED_OFF);
}

void mt65xx_backlight_on(void)
{
	printf("[LEDS]uboot: mt65xx_backlight_on \n\r");
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_LCD, LED_FULL);
}

void mt65xx_backlight_off(void)
{
	printf("[LEDS]uboot: mt65xx_backlight_off \n\r");
	mt65xx_leds_brightness_set(MT65XX_LED_TYPE_LCD, LED_OFF);
}

