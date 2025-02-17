/* arch/arm/mach-rk30/board-rk30-sdk.c
 *
 * Copyright (C) 2012 ROCKCHIP, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/skbuff.h>
#include <linux/spi/spi.h>
#include <linux/mmc/host.h>
#include <linux/ion.h>
#include <linux/cpufreq.h>
#include <linux/clk.h>
#include <mach/dvfs.h>

#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/flash.h>
#include <asm/hardware/gic.h>

#include <mach/board.h>
#include <mach/hardware.h>
#include <mach/io.h>
#include <mach/gpio.h>
#include <mach/iomux.h>
#include <linux/fb.h>
#include <linux/regulator/machine.h>
#include <linux/rfkill-rk.h>
#include <linux/sensor-dev.h>
#include <mach/yfmach.h>

#ifdef CONFIG_RK30_PWM_REGULATOR
#include <linux/regulator/rk29-pwm-regulator.h>
#endif

#if defined(CONFIG_HDMI_RK30)
	#include "../../../drivers/video/rockchip/hdmi/rk_hdmi.h"
#endif

#if defined(CONFIG_SPIM_RK29)
#include "../../../drivers/spi/rk29_spim.h"
#endif
#if defined(CONFIG_MU509)
#include <linux/mu509.h>
#endif
#if defined(CONFIG_MW100)
#include <linux/mw100.h>
#endif
#if defined(CONFIG_MT6229)
#include <linux/mt6229.h>
#endif
#if defined(CONFIG_SEW868)
#include <linux/sew868.h>
#endif
#if defined(CONFIG_ANDROID_TIMED_GPIO)
#include "../../../drivers/staging/android/timed_gpio.h"
#endif

#ifdef  CONFIG_THREE_FB_BUFFER
#define RK30_FB0_MEM_SIZE 12*SZ_1M
#else
#define RK30_FB0_MEM_SIZE 8*SZ_1M
#endif

#ifdef CONFIG_VIDEO_RK29
#include "yfcam.c"
#endif /* CONFIG_VIDEO_RK29 */

#if defined(CONFIG_TOUCHSCREEN_GT82X_IIC)
#define TOUCH_RESET_PIN  RK30_PIN4_PD0
#define TOUCH_PWR_PIN    INVALID_GPIO
int goodix_init_platform_hw(void)
{
	int ret;
	
	rk30_mux_api_set(GPIO4D0_SMCDATA8_TRACEDATA8_NAME, GPIO4D_GPIO4D0);
	rk30_mux_api_set(GPIO4C2_SMCDATA2_TRACEDATA2_NAME, GPIO4C_GPIO4C2);
	printk("%s:0x%x,0x%x\n",__func__,rk30_mux_api_get(GPIO4D0_SMCDATA8_TRACEDATA8_NAME),rk30_mux_api_get(GPIO4C2_SMCDATA2_TRACEDATA2_NAME));

	if (TOUCH_PWR_PIN != INVALID_GPIO) {
		ret = gpio_request(TOUCH_PWR_PIN, "goodix power pin");
		if (ret != 0) {
			gpio_free(TOUCH_PWR_PIN);
			printk("goodix power error\n");
			return -EIO;
		}
		gpio_direction_output(TOUCH_PWR_PIN, 0);
		gpio_set_value(TOUCH_PWR_PIN, GPIO_LOW);
		msleep(100);
	}

	if (TOUCH_RESET_PIN != INVALID_GPIO) {
		ret = gpio_request(TOUCH_RESET_PIN, "goodix reset pin");
		if (ret != 0) {
			gpio_free(TOUCH_RESET_PIN);
			printk("goodix gpio_request error\n");
			return -EIO;
		}
		gpio_direction_output(TOUCH_RESET_PIN, 1);
                msleep(100);
		//gpio_set_value(TOUCH_RESET_PIN, GPIO_LOW);
		//msleep(100);
		//gpio_set_value(TOUCH_RESET_PIN, GPIO_HIGH);
		//msleep(500);
	}
	return 0;
}

struct goodix_platform_data goodix_info = {
	.model = 8105,
	.irq_pin = RK30_PIN4_PC2,
	.rest_pin = TOUCH_RESET_PIN,
	.init_platform_hw = goodix_init_platform_hw,
};
#endif

static struct spi_board_info board_spi_devices[] = {
};

/***********************************************************
*	rk30  backlight
************************************************************/
#ifdef CONFIG_BACKLIGHT_RK29_BL
#define PWM_ID            2
#define PWM_MUX_NAME      GPIO0D6_PWM2_NAME
#define PWM_MUX_MODE      GPIO0D_PWM2
#define PWM_MUX_MODE_GPIO GPIO0D_GPIO0D6
#define PWM_GPIO 	  RK30_PIN0_PD6
#define PWM_EFFECT_VALUE  0

#define LCD_DISP_ON_PIN

#ifdef  LCD_DISP_ON_PIN
//#define BL_EN_MUX_NAME    GPIOF34_UART3_SEL_NAME
//#define BL_EN_MUX_MODE    IOMUXB_GPIO1_B34

#define BL_EN_PIN         RK30_PIN6_PB3
#define BL_EN_VALUE       GPIO_HIGH
static int s_bl_suspend;
#endif
static int rk29_backlight_io_init(void)
{
	int ret = 0;
	msleep(100);
	rk30_mux_api_set(PWM_MUX_NAME, PWM_MUX_MODE);
#ifdef  LCD_DISP_ON_PIN
	// rk30_mux_api_set(BL_EN_MUX_NAME, BL_EN_MUX_MODE);

	ret = gpio_request(BL_EN_PIN, NULL);
	if (ret != 0) {
		gpio_free(BL_EN_PIN);
	}

	gpio_direction_output(BL_EN_PIN, BL_EN_VALUE);
#endif
	return ret;
}
static int rk_fb_io_disable(void);
static int rk29_backlight_io_deinit(void)
{
	int ret = 0;
#ifdef  LCD_DISP_ON_PIN
	gpio_direction_output(BL_EN_PIN, !BL_EN_VALUE);
	gpio_free(BL_EN_PIN);
#endif
	rk30_mux_api_set(PWM_MUX_NAME, PWM_MUX_MODE_GPIO);
	rk_fb_io_disable();
	return ret;
}

static int rk29_backlight_pwm_suspend(void)
{
	int ret = 0;
	s_bl_suspend = 1;
	rk30_mux_api_set(PWM_MUX_NAME, PWM_MUX_MODE_GPIO);
	if (gpio_request(PWM_GPIO, NULL)) {
		printk("func %s, line %d: request gpio fail\n", __FUNCTION__, __LINE__);
		return -1;
	}
	gpio_direction_output(PWM_GPIO, GPIO_LOW);
#ifdef  LCD_DISP_ON_PIN
	gpio_direction_output(BL_EN_PIN, !BL_EN_VALUE);
#endif
	return ret;
}

static int rk29_backlight_pwm_resume(void)
{
	msleep(100);
	gpio_free(PWM_GPIO);
	rk30_mux_api_set(PWM_MUX_NAME, PWM_MUX_MODE);
#ifdef  LCD_DISP_ON_PIN
	msleep(100);
	gpio_direction_output(BL_EN_PIN, BL_EN_VALUE);
#endif
	s_bl_suspend = 0;
	return 0;
}

static struct rk29_bl_info rk29_bl_info = {
	.pwm_id = PWM_ID,
	.bl_ref = PWM_EFFECT_VALUE,
	.io_init = rk29_backlight_io_init,
	.io_deinit = rk29_backlight_io_deinit,
	.pwm_suspend = rk29_backlight_pwm_suspend,
	.pwm_resume = rk29_backlight_pwm_resume,
};

static struct platform_device rk29_device_backlight = {
	.name	= "rk29_backlight",
	.id 	= -1,
	.dev	= {
		.platform_data  = &rk29_bl_info,
	}
};

#endif

#ifdef CONFIG_RK29_SUPPORT_MODEM

#define RK30_MODEM_POWER        RK30_PIN4_PD1
#define RK30_MODEM_POWER_IOMUX  rk29_mux_api_set(GPIO4D1_SMCDATA9_TRACEDATA9_NAME, GPIO4D_GPIO4D1)

static int rk30_modem_io_init(void)
{
    printk("%s\n", __FUNCTION__);
    RK30_MODEM_POWER_IOMUX;

	return 0;
}

static struct rk29_io_t rk30_modem_io = {
    .io_addr    = RK30_MODEM_POWER,
    .enable     = GPIO_HIGH,
    .disable    = GPIO_LOW,
    .io_init    = rk30_modem_io_init,
};

static struct platform_device rk30_device_modem = {
	.name	= "rk30_modem",
	.id 	= -1,
	.dev	= {
		.platform_data  = &rk30_modem_io,
	}
};
#endif
#if defined(CONFIG_MU509)
static int mu509_io_init(void)
{

	rk30_mux_api_set(GPIO2B6_LCDC1DATA14_SMCADDR18_TSSYNC_NAME, GPIO2B_GPIO2B6);
        rk30_mux_api_set(GPIO4D2_SMCDATA10_TRACEDATA10_NAME, GPIO4D_GPIO4D2);
	rk30_mux_api_set(GPIO2B7_LCDC1DATA15_SMCADDR19_HSADCDATA7_NAME, GPIO2B_GPIO2B7);
	rk30_mux_api_set(GPIO2C0_LCDCDATA16_GPSCLK_HSADCCLKOUT_NAME, GPIO2C_GPIO2C0);
	return 0;
}

static int mu509_io_deinit(void)
{
	
	return 0;
}
 
struct rk29_mu509_data rk29_mu509_info = {
	.io_init = mu509_io_init,
  	.io_deinit = mu509_io_deinit,
	.modem_power_en = RK30_PIN6_PB2,//RK30_PIN4_PD1,
	.bp_power = RK30_PIN2_PB6,//RK30_PIN4_PD1,
	.bp_reset = RK30_PIN4_PD2,
	.ap_wakeup_bp = RK30_PIN2_PB7,
	.bp_wakeup_ap = RK30_PIN6_PA0, 
};
struct platform_device rk29_device_mu509 = {	
        .name = "mu509",	
    	.id = -1,	
	.dev		= {
		.platform_data = &rk29_mu509_info,
	}    	
    };
#endif
#if defined(CONFIG_MW100)
static int mw100_io_init(void)
{
	 rk30_mux_api_set(GPIO2B6_LCDC1DATA14_SMCADDR18_TSSYNC_NAME, GPIO2B_GPIO2B6);
	 rk30_mux_api_set(GPIO4D2_SMCDATA10_TRACEDATA10_NAME, GPIO4D_GPIO4D2);
	 rk30_mux_api_set(GPIO2B7_LCDC1DATA15_SMCADDR19_HSADCDATA7_NAME, GPIO2B_GPIO2B7);
	 rk30_mux_api_set(GPIO2C0_LCDCDATA16_GPSCLK_HSADCCLKOUT_NAME, GPIO2C_GPIO2C0);
	return 0;
}

static int mw100_io_deinit(void)
{
	
	return 0;
}
 
struct rk29_mw100_data rk29_mw100_info = {
	.io_init = mw100_io_init,
  	.io_deinit = mw100_io_deinit,
	.modem_power_en = RK30_PIN6_PB2,
	.bp_power = RK30_PIN2_PB6,
	.bp_reset = RK30_PIN4_PD2,
	.ap_wakeup_bp = RK30_PIN2_PB7,
	.bp_wakeup_ap = RK30_PIN6_PA0,
};
struct platform_device rk29_device_mw100 = {	
        .name = "mw100",	
    	.id = -1,	
	.dev		= {
		.platform_data = &rk29_mw100_info,
	}    	
    };
#endif
#if defined(CONFIG_MT6229)
static int mt6229_io_init(void)
{
	 rk30_mux_api_set(GPIO2B6_LCDC1DATA14_SMCADDR18_TSSYNC_NAME, GPIO2B_GPIO2B6);
	 rk30_mux_api_set(GPIO4D2_SMCDATA10_TRACEDATA10_NAME, GPIO4D_GPIO4D2);
	 rk30_mux_api_set(GPIO2B7_LCDC1DATA15_SMCADDR19_HSADCDATA7_NAME, GPIO2B_GPIO2B7);
	 rk30_mux_api_set(GPIO2C0_LCDCDATA16_GPSCLK_HSADCCLKOUT_NAME, GPIO2C_GPIO2C0);
	return 0;
}

static int mt6229_io_deinit(void)
{
	
	return 0;
}
 
struct rk29_mt6229_data rk29_mt6229_info = {
	.io_init = mt6229_io_init,
  	.io_deinit = mt6229_io_deinit,
	.modem_power_en = RK30_PIN6_PB2,
	.bp_power = RK30_PIN2_PB7,
	.bp_reset = RK30_PIN4_PD2,
	.ap_wakeup_bp = RK30_PIN2_PC0,
	.bp_wakeup_ap = RK30_PIN6_PA0,
};
struct platform_device rk29_device_mt6229 = {	
        .name = "mt6229",	
    	.id = -1,	
	.dev		= {
		.platform_data = &rk29_mt6229_info,
	}    	
    };
#endif
#if defined(CONFIG_SEW868)
static int sew868_io_init(void)
{
	rk30_mux_api_set(GPIO2B6_LCDC1DATA14_SMCADDR18_TSSYNC_NAME, GPIO2B_GPIO2B6);
    rk30_mux_api_set(GPIO4D2_SMCDATA10_TRACEDATA10_NAME, GPIO4D_GPIO4D2);
	rk30_mux_api_set(GPIO4D4_SMCDATA12_TRACEDATA12_NAME, GPIO4D_GPIO4D4);
	return 0;
}
static int sew868_io_deinit(void)
{
	return 0;
}
struct rk30_sew868_data rk30_sew868_info = {
	.io_init = sew868_io_init,
  	.io_deinit = sew868_io_deinit,
	.bp_power = RK30_PIN6_PB2, 
	.bp_power_active_low = 1,
	.bp_sys = RK30_PIN2_PB6, 
	.bp_reset = RK30_PIN4_PD2, 
	.bp_reset_active_low = 1,
	.bp_wakeup_ap = RK30_PIN4_PD4, 
	.ap_wakeup_bp = NULL,
};

struct platform_device rk30_device_sew868 = {	
        .name = "sew868",	
    	.id = -1,	
	.dev		= {
		.platform_data = &rk30_sew868_info,
	}    	
    };
#endif

/*MMA8452 gsensor*/
#if defined (CONFIG_GS_MMA8452)
#define MMA8452_INT_PIN   RK30_PIN4_PC0

static int mma8452_init_platform_hw(void)
{
	rk30_mux_api_set(GPIO4C0_SMCDATA0_TRACEDATA0_NAME, GPIO4C_GPIO4C0);

	return 0;
}

static struct sensor_platform_data mma8452_info = {
	.type = SENSOR_TYPE_ACCEL,
	.irq_enable = 1,
	.poll_delay_ms = 30,
        .init_platform_hw = mma8452_init_platform_hw,
        .orientation = {-1, 0, 0, 0, 0, 1, 0, -1, 0},
};
#endif
#if defined (CONFIG_GS_LIS3DH)
#define LIS3DH_INT_PIN   RK30_PIN4_PC0

static int lis3dh_init_platform_hw(void)
{
        rk30_mux_api_set(GPIO4C0_SMCDATA0_TRACEDATA0_NAME, GPIO4C_GPIO4C0);

        return 0;
}

static struct sensor_platform_data lis3dh_info = {
	.type = SENSOR_TYPE_ACCEL,
	.irq_enable = 1,
	.poll_delay_ms = 30,
        .init_platform_hw = lis3dh_init_platform_hw,
	.orientation = {-1, 0, 0, 0, 0, 1, 0, -1, 0},
};
#endif
#if defined (CONFIG_COMPASS_AK8975)
static struct sensor_platform_data akm8975_info =
{
	.type = SENSOR_TYPE_COMPASS,
	.irq_enable = 1,
	.poll_delay_ms = 30,
	.m_layout = 
	{
		{
			{1, 0, 0},
			{0, 1, 0},
			{0, 0, 1},
		},

		{
			{1, 0, 0},
			{0, 1, 0},
			{0, 0, 1},
		},

		{
			{1, 0, 0},
			{0, 1, 0},
			{0, 0, 1},
		},

		{
			{1, 0, 0},
			{0, 1, 0},
			{0, 0, 1},
		},
	}
};

#endif

#if defined(CONFIG_GYRO_L3G4200D)

#include <linux/l3g4200d.h>
#define L3G4200D_INT_PIN  RK30_PIN4_PC3

static int l3g4200d_init_platform_hw(void)
{
	rk30_mux_api_set(GPIO4C3_SMCDATA3_TRACEDATA3_NAME, GPIO4C_GPIO4C3);
	
	return 0;
}

static struct sensor_platform_data l3g4200d_info = {
	.type = SENSOR_TYPE_GYROSCOPE,
	.irq_enable = 1,
	.poll_delay_ms = 30,
	.orientation = {0, 1, 0, -1, 0, 0, 0, 0, 1},
	.init_platform_hw = l3g4200d_init_platform_hw,
	.x_min = 40,//x_min,y_min,z_min = (0-100) according to hardware
	.y_min = 40,
	.z_min = 20,
};

#endif

#ifdef CONFIG_LS_CM3217
static struct sensor_platform_data cm3217_info = {
	.type = SENSOR_TYPE_LIGHT,
	.irq_enable = 0,
	.poll_delay_ms = 500,
};

#endif

static int s_lcd_en_pol = 0;
#ifdef CONFIG_FB_ROCKCHIP

#define LCD_CS_MUX_NAME    GPIO4C7_SMCDATA7_TRACEDATA7_NAME
#define LCD_CS_PIN         RK30_PIN4_PC7
#define LCD_CS_VALUE       GPIO_HIGH

#define LCD_STB_MUX_NAME   GPIO4D2_SMCDATA10_TRACEDATA10_NAME
#define LCD_STB_PIN        RK30_PIN4_PD2
#define LCD_STB_VALUE      GPIO_HIGH

#define LCD_EN_MUX_NAME    GPIO4C7_SMCDATA7_TRACEDATA7_NAME
#define LCD_EN_PIN         RK30_PIN6_PB4
#define LCD_EN_VALUE       s_lcd_en_pol

static int rk_fb_io_init(struct rk29_fb_setting_info *fb_setting)
{
	int ret = 0;
	rk30_mux_api_set(LCD_CS_MUX_NAME, GPIO4C_GPIO4C7);
	ret = gpio_request(LCD_CS_PIN, NULL);
	if (ret != 0)
	{
		gpio_free(LCD_CS_PIN);
		printk(KERN_ERR "request lcd cs pin fail!\n");
		return -1;
	}
	else
	{
		gpio_direction_output(LCD_CS_PIN, LCD_CS_VALUE);
	}
	ret = gpio_request(LCD_EN_PIN, NULL);
	if (ret != 0)
	{
		gpio_free(LCD_EN_PIN);
		printk(KERN_ERR "request lcd en pin fail!\n");
		return -1;
	}
	else
	{
		gpio_direction_output(LCD_EN_PIN, !LCD_EN_VALUE);
		msleep(10);
		gpio_direction_output(LCD_EN_PIN, LCD_EN_VALUE);
		msleep(10);
	}
	ret = gpio_request(LCD_STB_PIN, NULL);
	if (ret != 0)
	{
		gpio_free(LCD_STB_PIN);
		printk(KERN_ERR "request lcd std pin fail!\n");
		return -1;
	}
	else
	{
		gpio_direction_output(LCD_STB_PIN, LCD_STB_VALUE);
	}

	return 0;
}
static int rk_fb_io_disable(void)
{
	gpio_set_value(LCD_STB_PIN, LCD_STB_VALUE? 0:1);
	gpio_set_value(LCD_CS_PIN, LCD_CS_VALUE? 0:1);
	mdelay(30);
	gpio_set_value(LCD_EN_PIN, LCD_EN_VALUE? 0:1);
	return 0;
}
static int rk_fb_io_enable(void)
{
	gpio_set_value(LCD_EN_PIN, LCD_EN_VALUE);
	mdelay(30);
	gpio_set_value(LCD_CS_PIN, LCD_CS_VALUE);
	gpio_set_value(LCD_STB_PIN, LCD_STB_VALUE);
	return 0;
}

#if defined(CONFIG_LCDC0_RK30)
struct rk29fb_info lcdc0_screen_info = {
	.prop	   = PRMRY,		//primary display device
	.io_init   = rk_fb_io_init,
	.io_disable = rk_fb_io_disable,
	.io_enable = rk_fb_io_enable,
	.set_screen_info = set_lcd_info,
};
#endif

#if defined(CONFIG_LCDC1_RK30)
struct rk29fb_info lcdc1_screen_info = {
	#if defined(CONFIG_HDMI_RK30)
	.prop		= EXTEND,	//extend display device
	.lcd_info  = NULL,
	.set_screen_info = hdmi_init_lcdc,
	#endif
};
#endif

static struct resource resource_fb[] = {
	[0] = {
		.name  = "fb0 buf",
		.start = 0,
		.end   = 0,//RK30_FB0_MEM_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.name  = "ipp buf",  //for rotate
		.start = 0,
		.end   = 0,//RK30_FB0_MEM_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[2] = {
		.name  = "fb2 buf",
		.start = 0,
		.end   = 0,//RK30_FB0_MEM_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
};

static struct platform_device device_fb = {
	.name		= "rk-fb",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(resource_fb),
	.resource	= resource_fb,
};
#endif

#ifdef CONFIG_ANDROID_TIMED_GPIO
static struct timed_gpio timed_gpios[] = {
	{
		.name = "vibrator",
		.gpio = RK30_PIN0_PA4,
		.max_timeout = 1000,
		.active_low = 0,
		.adjust_time =20,      //adjust for diff product
	},
};

static struct timed_gpio_platform_data rk29_vibrator_info = {
	.num_gpios = 1,
	.gpios = timed_gpios,
};

static struct platform_device rk29_device_vibrator = {
	.name = "timed-gpio",
	.id = -1,
	.dev = {
		.platform_data = &rk29_vibrator_info,
	},

};
#endif

#ifdef CONFIG_LEDS_GPIO_PLATFORM
static struct gpio_led rk29_leds[] = {
	{
		.name = "button-backlight",
		.gpio = RK30_PIN4_PD7,
		.default_trigger = "timer",
		.active_low = 0,
		.retain_state_suspended = 0,
		.default_state = LEDS_GPIO_DEFSTATE_OFF,
	},
};

static struct gpio_led_platform_data rk29_leds_pdata = {
	.leds = rk29_leds,
	.num_leds = ARRAY_SIZE(rk29_leds),
};

static struct platform_device rk29_device_gpio_leds = {
	.name	= "leds-gpio",
	.id	= -1,
	.dev	= {
		.platform_data  = &rk29_leds_pdata,
	},
};
#endif

#ifdef CONFIG_RK_IRDA
#define IRDA_IRQ_PIN           RK30_PIN6_PA1

static int irda_iomux_init(void)
{
	int ret = 0;

	//irda irq pin
	ret = gpio_request(IRDA_IRQ_PIN, NULL);
	if (ret != 0) {
		gpio_free(IRDA_IRQ_PIN);
		printk(">>>>>> IRDA_IRQ_PIN gpio_request err \n ");
	}
	gpio_pull_updown(IRDA_IRQ_PIN, PullDisable);
	gpio_direction_input(IRDA_IRQ_PIN);

	return 0;
}

static int irda_iomux_deinit(void)
{
	gpio_free(IRDA_IRQ_PIN);
	return 0;
}

static struct irda_info rk29_irda_info = {
	.intr_pin = IRDA_IRQ_PIN,
	.iomux_init = irda_iomux_init,
	.iomux_deinit = irda_iomux_deinit,
	//.irda_pwr_ctl = bu92747guw_power_ctl,
};

static struct platform_device irda_device = {
#ifdef CONFIG_RK_IRDA_NET
	.name = "rk_irda",
#else
	.name = "bu92747_irda",
#endif
	.id = -1,
	.dev = {
		.platform_data = &rk29_irda_info,
	}
};
#endif

#ifdef CONFIG_ION
#define ION_RESERVE_SIZE        (80 * SZ_1M)
static struct ion_platform_data rk30_ion_pdata = {
	.nr = 1,
	.heaps = {
		{
			.type = ION_HEAP_TYPE_CARVEOUT,
			.id = ION_NOR_HEAP_ID,
			.name = "norheap",
			.size = ION_RESERVE_SIZE,
		}
	},
};

static struct platform_device device_ion = {
	.name = "ion-rockchip",
	.id = 0,
	.dev = {
		.platform_data = &rk30_ion_pdata,
	},
};
#endif

/**************************************************************************************************
 * SDMMC devices,  include the module of SD,MMC,and sdio.noted by xbw at 2012-03-05
**************************************************************************************************/
#ifdef CONFIG_SDMMC_RK29
#include "board-rk30-sdk-sdmmc.c"

#if defined(CONFIG_SDMMC0_RK29_WRITE_PROTECT)
#define SDMMC0_WRITE_PROTECT_PIN	RK30_PIN3_PB7	//According to your own project to set the value of write-protect-pin.
#endif

#if defined(CONFIG_SDMMC1_RK29_WRITE_PROTECT)
#define SDMMC1_WRITE_PROTECT_PIN	RK30_PIN3_PC7	//According to your own project to set the value of write-protect-pin.
#endif

#define RK29SDK_WIFI_SDIO_CARD_DETECT_N    RK30_PIN6_PB2

#endif //endif ---#ifdef CONFIG_SDMMC_RK29

#ifdef CONFIG_SDMMC0_RK29
static int rk29_sdmmc0_cfg_gpio(void)
{
#ifdef CONFIG_SDMMC_RK29_OLD
	rk30_mux_api_set(GPIO3B1_SDMMC0CMD_NAME, GPIO3B_SDMMC0_CMD);
	rk30_mux_api_set(GPIO3B0_SDMMC0CLKOUT_NAME, GPIO3B_SDMMC0_CLKOUT);
	rk30_mux_api_set(GPIO3B2_SDMMC0DATA0_NAME, GPIO3B_SDMMC0_DATA0);
	rk30_mux_api_set(GPIO3B3_SDMMC0DATA1_NAME, GPIO3B_SDMMC0_DATA1);
	rk30_mux_api_set(GPIO3B4_SDMMC0DATA2_NAME, GPIO3B_SDMMC0_DATA2);
	rk30_mux_api_set(GPIO3B5_SDMMC0DATA3_NAME, GPIO3B_SDMMC0_DATA3);

	rk30_mux_api_set(GPIO3B6_SDMMC0DETECTN_NAME, GPIO3B_GPIO3B6);

	rk30_mux_api_set(GPIO3A7_SDMMC0PWREN_NAME, GPIO3A_GPIO3A7);
	gpio_request(RK30_PIN3_PA7, "sdmmc-power");
	gpio_direction_output(RK30_PIN3_PA7, GPIO_LOW);

#else
	rk29_sdmmc_set_iomux(0, 0xFFFF);

	rk30_mux_api_set(GPIO3B6_SDMMC0DETECTN_NAME, GPIO3B_SDMMC0_DETECT_N);

#if defined(CONFIG_SDMMC0_RK29_WRITE_PROTECT)
	gpio_request(SDMMC0_WRITE_PROTECT_PIN, "sdmmc-wp");
	gpio_direction_input(SDMMC0_WRITE_PROTECT_PIN);
#endif

#endif

	return 0;
}

#define CONFIG_SDMMC0_USE_DMA
struct rk29_sdmmc_platform_data default_sdmmc0_data = {
	.host_ocr_avail =
	    (MMC_VDD_25_26 | MMC_VDD_26_27 | MMC_VDD_27_28 | MMC_VDD_28_29 |
	     MMC_VDD_29_30 | MMC_VDD_30_31 | MMC_VDD_31_32 | MMC_VDD_32_33 |
	     MMC_VDD_33_34 | MMC_VDD_34_35 | MMC_VDD_35_36),
	.host_caps =
	    (MMC_CAP_4_BIT_DATA | MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED),
	.io_init = rk29_sdmmc0_cfg_gpio,

#if !defined(CONFIG_SDMMC_RK29_OLD)
	.set_iomux = rk29_sdmmc_set_iomux,
#endif

	.dma_name = "sd_mmc",
#ifdef CONFIG_SDMMC0_USE_DMA
	.use_dma = 1,
#else
	.use_dma = 0,
#endif
	.detect_irq = RK30_PIN3_PB6,	// INVALID_GPIO
	.enable_sd_wakeup = 0,

#if defined(CONFIG_SDMMC0_RK29_WRITE_PROTECT)
	.write_prt = SDMMC0_WRITE_PROTECT_PIN,
#else
	.write_prt = INVALID_GPIO,
#endif
};
#endif // CONFIG_SDMMC0_RK29

#ifdef CONFIG_SDMMC1_RK29
#define CONFIG_SDMMC1_USE_DMA
static int rk29_sdmmc1_cfg_gpio(void)
{
#if defined(CONFIG_SDMMC_RK29_OLD)
	rk30_mux_api_set(GPIO3C0_SMMC1CMD_NAME, GPIO3C_SMMC1_CMD);
	rk30_mux_api_set(GPIO3C5_SDMMC1CLKOUT_NAME, GPIO3C_SDMMC1_CLKOUT);
	rk30_mux_api_set(GPIO3C1_SDMMC1DATA0_NAME, GPIO3C_SDMMC1_DATA0);
	rk30_mux_api_set(GPIO3C2_SDMMC1DATA1_NAME, GPIO3C_SDMMC1_DATA1);
	rk30_mux_api_set(GPIO3C3_SDMMC1DATA2_NAME, GPIO3C_SDMMC1_DATA2);
	rk30_mux_api_set(GPIO3C4_SDMMC1DATA3_NAME, GPIO3C_SDMMC1_DATA3);
	//rk30_mux_api_set(GPIO3C6_SDMMC1DETECTN_NAME, GPIO3C_SDMMC1_DETECT_N);

#else

#if defined(CONFIG_SDMMC1_RK29_WRITE_PROTECT)
	gpio_request(SDMMC1_WRITE_PROTECT_PIN, "sdio-wp");
	gpio_direction_input(SDMMC1_WRITE_PROTECT_PIN);
#endif

#endif

	return 0;
}

struct rk29_sdmmc_platform_data default_sdmmc1_data = {
	.host_ocr_avail =
	    (MMC_VDD_25_26 | MMC_VDD_26_27 | MMC_VDD_27_28 | MMC_VDD_28_29 |
	     MMC_VDD_29_30 | MMC_VDD_30_31 | MMC_VDD_31_32 | MMC_VDD_32_33 |
	     MMC_VDD_33_34),

#if !defined(CONFIG_USE_SDMMC1_FOR_WIFI_DEVELOP_BOARD)
	.host_caps = (MMC_CAP_4_BIT_DATA | MMC_CAP_SDIO_IRQ |
		      MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED),
#else
	.host_caps =
	    (MMC_CAP_4_BIT_DATA | MMC_CAP_MMC_HIGHSPEED | MMC_CAP_SD_HIGHSPEED),
#endif

	.io_init = rk29_sdmmc1_cfg_gpio,

#if !defined(CONFIG_SDMMC_RK29_OLD)
	.set_iomux = rk29_sdmmc_set_iomux,
#endif

	.dma_name = "sdio",
#ifdef CONFIG_SDMMC1_USE_DMA
	.use_dma = 1,
#else
	.use_dma = 0,
#endif

#if !defined(CONFIG_USE_SDMMC1_FOR_WIFI_DEVELOP_BOARD)
#ifdef CONFIG_WIFI_CONTROL_FUNC
	.status = rk29sdk_wifi_status,
	.register_status_notify = rk29sdk_wifi_status_register,
#endif
#if 0
	.detect_irq = RK29SDK_WIFI_SDIO_CARD_DETECT_N,
#endif

#if defined(CONFIG_SDMMC1_RK29_WRITE_PROTECT)
	.write_prt = SDMMC1_WRITE_PROTECT_PIN,
#else
	.write_prt = INVALID_GPIO,
#endif

#else
	.detect_irq = INVALID_GPIO,
	.enable_sd_wakeup = 0,
#endif

};
#endif //endif--#ifdef CONFIG_SDMMC1_RK29

/**************************************************************************************************
 * the end of setting for SDMMC devices
**************************************************************************************************/

#ifdef CONFIG_BATTERY_RK30_ADC
static struct rk30_adc_battery_platform_data rk30_adc_battery_platdata = {
        .dc_det_pin      = RK30_PIN6_PA5,
        .batt_low_pin    = RK30_PIN6_PA0,
        .charge_set_pin  = INVALID_GPIO,
        .charge_ok_pin   = RK30_PIN6_PA6,
        .dc_det_level    = GPIO_LOW,
        .charge_ok_level = GPIO_HIGH,
};

static struct platform_device rk30_device_adc_battery = {
        .name   = "rk30-battery",
        .id     = -1,
        .dev = {
                .platform_data = &rk30_adc_battery_platdata,
        },
};
#endif

#ifdef CONFIG_RK29_VMAC
#define PHY_PWR_EN_GPIO	RK30_PIN1_PD6
#include "board-rk30-sdk-vmac.c"
#endif

#ifdef CONFIG_RFKILL_RK
// bluetooth rfkill device, its driver in net/rfkill/rfkill-rk.c
static struct rfkill_rk_platform_data rfkill_rk_platdata = {
    .type               = RFKILL_TYPE_BLUETOOTH,

    .poweron_gpio       = { // BT_REG_ON
        .io             = RK30_PIN3_PC7,
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = GPIO3C7_SDMMC1WRITEPRT_NAME,
            .fgpio      = GPIO3C_GPIO3C7,
        },
    },

    .reset_gpio         = { // BT_RST
        .io             = RK30_PIN3_PD1, // set io to INVALID_GPIO for disable it
        .enable         = GPIO_LOW,
        .iomux          = {
            .name       = GPIO3D1_SDMMC1BACKENDPWR_NAME,
            .fgpio      = GPIO3D_GPIO3D1,
        },
    },

    .wake_gpio          = { // BT_WAKE, use to control bt's sleep and wakeup
        .io             = RK30_PIN3_PC6, // set io to INVALID_GPIO for disable it
        .enable         = GPIO_HIGH,
        .iomux          = {
            .name       = GPIO3C6_SDMMC1DETECTN_NAME,
            .fgpio      = GPIO3C_GPIO3C6,
        },
    },

    .wake_host_irq      = { // BT_HOST_WAKE, for bt wakeup host when it is in deep sleep
        .gpio           = {
            .io         = RK30_PIN6_PA7, // set io to INVALID_GPIO for disable it
            .enable     = GPIO_LOW,      // set GPIO_LOW for falling, set 0 for rising
            .iomux      = {
                .name   = NULL,
            },
        },
    },

    .rts_gpio           = { // UART_RTS, enable or disable BT's data coming
        .io             = RK30_PIN1_PA3, // set io to INVALID_GPIO for disable it
        .enable         = GPIO_LOW,
        .iomux          = {
            .name       = GPIO1A3_UART0RTSN_NAME,
            .fgpio      = GPIO1A_GPIO1A3,
            .fmux       = GPIO1A_UART0_RTS_N,
        },
    },
};

static struct platform_device device_rfkill_rk = {
    .name   = "rfkill_rk",
    .id     = -1,
    .dev    = {
        .platform_data = &rfkill_rk_platdata,
    },
};
#endif

#if CONFIG_RK30_PWM_REGULATOR
const static int pwm_voltage_map[] = {
	1000000, 1025000, 1050000, 1075000, 1100000, 1125000, 1150000, 1175000, 1200000, 1225000, 1250000, 1275000, 1300000, 1325000, 1350000, 1375000, 1400000
};

static struct regulator_consumer_supply pwm_dcdc1_consumers[] = {
	{
		.supply = "vdd_core",
	}
};

struct regulator_init_data pwm_regulator_init_dcdc[1] =
{
	{
		.constraints = {
			.name = "PWM_DCDC1",
			.min_uV = 600000,
			.max_uV = 1800000,	//0.6-1.8V
			.apply_uV = true,
			.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,
		},
		.num_consumer_supplies = ARRAY_SIZE(pwm_dcdc1_consumers),
		.consumer_supplies = pwm_dcdc1_consumers,
	},
};
static struct pwm_platform_data pwm_regulator_info[1] = {
	{
		.pwm_id = 3,
		.pwm_gpio = RK30_PIN0_PD7,
		.pwm_iomux_name = GPIO0D7_PWM3_NAME,
		.pwm_iomux_pwm = GPIO0D_PWM3,
		.pwm_iomux_gpio = GPIO0D_GPIO0D6,
		.pwm_voltage = 1100000,
		.suspend_voltage = 1050000,
		.min_uV = 1000000,
		.max_uV	= 1400000,
		.coefficient = 455,	//45.5%
		.pwm_voltage_map = pwm_voltage_map,
		.init_data	= &pwm_regulator_init_dcdc[0],
	},
};

struct platform_device pwm_regulator_device[1] = {
	{
		.name = "pwm-voltage-regulator",
		.id = 0,
		.dev		= {
			.platform_data = &pwm_regulator_info[0],
		}
	},
};
#endif

static struct platform_device *devices[] __initdata = {
#ifdef CONFIG_FB_ROCKCHIP
	&device_fb,
#endif
#ifdef CONFIG_ION
	&device_ion,
#endif
#ifdef CONFIG_ANDROID_TIMED_GPIO
	&rk29_device_vibrator,
#endif
#ifdef CONFIG_LEDS_GPIO_PLATFORM
	&rk29_device_gpio_leds,
#endif
#ifdef CONFIG_RK_IRDA
	&irda_device,
#endif
#ifdef CONFIG_WIFI_CONTROL_FUNC
	&rk29sdk_wifi_device,
#endif
#ifdef CONFIG_RK29_SUPPORT_MODEM
	&rk30_device_modem,
#endif
#if defined(CONFIG_MU509)
	&rk29_device_mu509,
#endif
#if defined(CONFIG_MW100)
	&rk29_device_mw100,
#endif
#if defined(CONFIG_MT6229)
	&rk29_device_mt6229,
#endif
#if defined(CONFIG_SEW868)
	&rk30_device_sew868,
#endif
#ifdef CONFIG_BATTERY_RK30_ADC
 	&rk30_device_adc_battery,
#endif
#ifdef CONFIG_RFKILL_RK
	&device_rfkill_rk,
#endif
#ifdef CONFIG_RK30_PWM_REGULATOR
	&pwm_regulator_device[0],
#endif
};

// i2c
#ifdef CONFIG_I2C0_RK30
static struct i2c_board_info __initdata i2c0_info[] = {
#if defined (CONFIG_GS_MMA8452)
	{
		.type	        = "gs_mma8452",
		.addr	        = 0x1d,
		.flags	        = 0,
		.irq	        = MMA8452_INT_PIN,
		.platform_data = &mma8452_info,
	},
#endif
#if defined (CONFIG_GS_LIS3DH)
	{
		.type	        = "gs_lis3dh",
		.addr	        = 0x19,   //0x19(SA0-->VCC), 0x18(SA0-->GND)
		.flags	        = 0,
		.irq	        = LIS3DH_INT_PIN,
		.platform_data = &lis3dh_info,
	},
#endif
#if defined (CONFIG_COMPASS_AK8975)
	{
		.type          = "ak8975",
		.addr          = 0x0d,
		.flags         = 0,
		.irq           = RK30_PIN4_PC1,
		.platform_data = &akm8975_info,
	},
#endif
#if defined (CONFIG_GYRO_L3G4200D)
	{
		.type          = "l3g4200d_gryo",
		.addr          = 0x69,
		.flags         = 0,
		.irq           = L3G4200D_INT_PIN,
		.platform_data = &l3g4200d_info,
	},
#endif
#if defined (CONFIG_SND_SOC_RK1000)
	{
		.type          = "rk1000_i2c_codec",
		.addr          = 0x60,
		.flags         = 0,
	},
	{
		.type          = "rk1000_control",
		.addr          = 0x40,
		.flags         = 0,
	},
#endif
#if defined (CONFIG_SND_SOC_RT5631)
        {
                .type                   = "rt5631",
                .addr                   = 0x1a,
                .flags                  = 0,
        },
#endif
#if defined (CONFIG_SND_RK29_SOC_ES8323)
        {
                .type                   = "es8323",
                .addr                   = 0x10,
                .flags                  = 0,
        },
#endif
#ifdef CONFIG_MFD_RK610
		{
			.type			= "rk610_ctl",
			.addr			= 0x40,
			.flags			= 0,
		},
#ifdef CONFIG_RK610_TVOUT
		{
			.type			= "rk610_tvout",
			.addr			= 0x42,
			.flags			= 0,
		},
#endif
#ifdef CONFIG_RK610_HDMI
		{
			.type			= "rk610_hdmi",
			.addr			= 0x46,
			.flags			= 0,
			.irq			= RK29_PIN5_PA2,
		},
#endif
#ifdef CONFIG_SND_SOC_RK610
		{//RK610_CODEC addr  from 0x60 to 0x80 (0x60~0x80)
			.type			= "rk610_i2c_codec",
			.addr			= 0x60,
			.flags			= 0,
		},
#endif
#endif

};
#endif
#define PMIC_TYPE_WM8326	1
#define PMIC_TYPE_TPS65910	2
int __sramdata g_pmic_type =  0;
#ifdef CONFIG_I2C1_RK30
#ifdef CONFIG_MFD_WM831X_I2C
#include "board-rk30-sdk-wm8326.c"
#endif
#ifdef CONFIG_MFD_TPS65910
#define TPS65910_HOST_IRQ        RK30_PIN6_PA4
#include "board-rk30-sdk-tps65910.c"
#endif

static struct i2c_board_info __initdata i2c1_info[] = {
#if defined (CONFIG_MFD_WM831X_I2C)
	{
		.type          = "wm8326",
		.addr          = 0x34,
		.flags         = 0,
		.irq           = RK30_PIN6_PA4,
		.platform_data = &wm831x_platdata,
	},
#endif
#if defined (CONFIG_MFD_TPS65910)
	{
        .type           = "tps65910",
        .addr           = TPS65910_I2C_ID0,
        .flags          = 0,
        .irq            = TPS65910_HOST_IRQ,
    	.platform_data = &tps65910_data,
	},
#endif
};
#endif

void __sramfunc board_pmu_suspend(void)
{
	#if defined (CONFIG_MFD_WM831X_I2C)
       if(g_pmic_type == PMIC_TYPE_WM8326)
       board_pmu_wm8326_suspend();
	#endif
	#if defined (CONFIG_MFD_TPS65910)
       if(g_pmic_type == PMIC_TYPE_TPS65910)
       board_pmu_tps65910_suspend(); 
    #endif
}

void __sramfunc board_pmu_resume(void)
{
	#if defined (CONFIG_MFD_WM831X_I2C)
       if(g_pmic_type == PMIC_TYPE_WM8326)
       board_pmu_wm8326_resume();
	#endif
	#if defined (CONFIG_MFD_TPS65910)
       if(g_pmic_type == PMIC_TYPE_TPS65910)
       board_pmu_tps65910_resume(); 
	#endif
}

int __sramdata gpio0d7_iomux,gpio0d7_do,gpio0d7_dir,gpio0d7_en;

void __sramfunc rk30_pwm_logic_suspend_voltage(void)
{
#ifdef CONFIG_RK30_PWM_REGULATOR

//	int gpio0d7_iomux,gpio0d7_do,gpio0d7_dir,gpio0d7_en;
	sram_udelay(10000);
	gpio0d7_iomux = readl_relaxed(GRF_GPIO0D_IOMUX);
	gpio0d7_do = grf_readl(GRF_GPIO0H_DO);
	gpio0d7_dir = grf_readl(GRF_GPIO0H_DIR);
	gpio0d7_en = grf_readl(GRF_GPIO0H_EN);

	writel_relaxed((1<<30), GRF_GPIO0D_IOMUX);
	grf_writel((1<<31)|(1<<15), GRF_GPIO0H_DIR);
	grf_writel((1<<31)|(1<<15), GRF_GPIO0H_DO);
	grf_writel((1<<31)|(1<<15), GRF_GPIO0H_EN);
#endif 
}
void __sramfunc rk30_pwm_logic_resume_voltage(void)
{
#ifdef CONFIG_RK30_PWM_REGULATOR
	writel_relaxed((1<<30)|gpio0d7_iomux, GRF_GPIO0D_IOMUX);
	grf_writel((1<<31)|gpio0d7_en, GRF_GPIO0H_EN);
	grf_writel((1<<31)|gpio0d7_dir, GRF_GPIO0H_DIR);
	grf_writel((1<<31)|gpio0d7_do, GRF_GPIO0H_DO);
	sram_udelay(10000);

#endif

}
extern void pwm_suspend_voltage(void);
extern void pwm_resume_voltage(void);
void  rk30_pwm_suspend_voltage_set(void)
{
#ifdef CONFIG_RK30_PWM_REGULATOR
	pwm_suspend_voltage();
#endif
}
void  rk30_pwm_resume_voltage_set(void)
{
#ifdef CONFIG_RK30_PWM_REGULATOR
	pwm_resume_voltage();
#endif
}


#ifdef CONFIG_I2C2_RK30
static struct i2c_board_info __initdata i2c2_info[] = {
#if defined (CONFIG_TOUCHSCREEN_GT82X_IIC)
	{
		.type          = "Goodix-TS",
		.addr          = 0x5d,
		.flags         = 0,
		.irq           = RK30_PIN4_PC2,
		.platform_data = &goodix_info,
	},
#endif
#if defined (CONFIG_LS_CM3217)
	{
		.type          = "lightsensor",
		.addr          = 0x10,
		.flags         = 0,
		.platform_data = &cm3217_info,
	},
#endif
};
#endif

#ifdef CONFIG_I2C3_RK30
static struct i2c_board_info __initdata i2c3_info[] = {
};
#endif

#ifdef CONFIG_I2C4_RK30
static struct i2c_board_info __initdata i2c4_info[] = {
};
#endif

#ifdef CONFIG_I2C_GPIO_RK30
#define I2C_SDA_PIN     INVALID_GPIO// RK30_PIN2_PD6   //set sda_pin here
#define I2C_SCL_PIN     INVALID_GPIO//RK30_PIN2_PD7   //set scl_pin here
static int rk30_i2c_io_init(void)
{
        //set iomux (gpio) here
        //rk30_mux_api_set(GPIO2D7_I2C1SCL_NAME, GPIO2D_GPIO2D7);
        //rk30_mux_api_set(GPIO2D6_I2C1SDA_NAME, GPIO2D_GPIO2D6);

        return 0;
}
struct i2c_gpio_platform_data default_i2c_gpio_data = {
       .sda_pin = I2C_SDA_PIN,
       .scl_pin = I2C_SCL_PIN,
       .udelay = 5, // clk = 500/udelay = 100Khz
       .timeout = 100,//msecs_to_jiffies(100),
       .bus_num    = 5,
       .io_init = rk30_i2c_io_init,
};
static struct i2c_board_info __initdata i2c_gpio_info[] = {
};
#endif

static void __init rk30_i2c_register_board_info(void)
{
#ifdef CONFIG_I2C0_RK30
	i2c_register_board_info(0, i2c0_info, ARRAY_SIZE(i2c0_info));
#endif
#ifdef CONFIG_I2C1_RK30
	i2c_register_board_info(1, i2c1_info, ARRAY_SIZE(i2c1_info));
#endif
#ifdef CONFIG_I2C2_RK30
	i2c_register_board_info(2, i2c2_info, ARRAY_SIZE(i2c2_info));
#endif
#ifdef CONFIG_I2C3_RK30
	i2c_register_board_info(3, i2c3_info, ARRAY_SIZE(i2c3_info));
#endif
#ifdef CONFIG_I2C4_RK30
	i2c_register_board_info(4, i2c4_info, ARRAY_SIZE(i2c4_info));
#endif
#ifdef CONFIG_I2C_GPIO_RK30
	i2c_register_board_info(5, i2c_gpio_info, ARRAY_SIZE(i2c_gpio_info));
#endif
}
//end of i2c

#define POWER_ON_PIN RK30_PIN6_PB0   //power_hold
#define POWER_IND_PIN RK30_PIN0_PD1   //power indicator
#define CHARGE_PIN RK30_PIN4_PD5   //charge level pin
static int s_charge_pol = 0;
int charge_mode = 0;
void rk30_setgpio_resume_board(void)
{
	gpio_direction_output(POWER_IND_PIN, GPIO_LOW);
	gpio_direction_output(CHARGE_PIN, s_charge_pol ? 1 : 0);
	charge_mode = 0;
}
void rk30_setgpio_suspend_board(void)
{
	gpio_direction_output(POWER_IND_PIN, GPIO_HIGH);
	gpio_direction_output(CHARGE_PIN, s_charge_pol ? 0 : 1);
	charge_mode = 1;
}

static void rk30_pm_power_off(void)
{
	printk(KERN_ERR "rk30_pm_power_off start...\n");
	gpio_direction_output(POWER_ON_PIN, GPIO_LOW);
	gpio_direction_output(POWER_IND_PIN, GPIO_HIGH);
	#if defined(CONFIG_MFD_WM831X)	
	if(g_pmic_type == PMIC_TYPE_WM8326)
	{
		wm831x_set_bits(Wm831x,WM831X_GPIO_LEVEL,0x0001,0x0000);  //set sys_pwr 0
		wm831x_device_shutdown(Wm831x);//wm8326 shutdown
	}
	#endif
	#if defined(CONFIG_MFD_TPS65910)
	if(g_pmic_type == PMIC_TYPE_TPS65910)
	{
		tps65910_device_shutdown();//tps65910 shutdown
	}
	#endif

	while (1);
}
int pmu_data_read(int index)
{
#ifdef CONFIG_MFD_TPS65910
	if(g_pmic_type == PMIC_TYPE_TPS65910) {
		return tps65910_data_read((u8)index);
	}
#endif
	return -ENODEV;
}

int pmu_data_write(int index, u8 value)
{
#ifdef CONFIG_MFD_TPS65910
	if(g_pmic_type == PMIC_TYPE_TPS65910) {
		return tps65910_data_write((u8)index, value);
	}
#endif
	return -ENODEV;
}

static unsigned s_acc_dir = -1;
static char * s_acc_name;
static int s_acc_off[3];
static int s_acc_units;
//for calibration
static int s_acc_cal = -1;
static int s_acc_val[3];
static int s_acc_count;

int acc_supported(char * name)
{
	int supported = 0;
	if(s_acc_dir == -1) {
		const char * accs = env_get_str("acc_supproted", 0);
		supported = !accs || strstr(accs, name);
	}
	return supported;
}

void acc_register(char * name, int units)
{
	if(s_acc_dir == -1) {
		char dir_name[32] = "acc_dir_";
		strcat(dir_name, name);
		if(env_cpy_u32s(dir_name, &s_acc_dir, 1) != 1) {
			s_acc_dir = env_get_u32("acc_dir", -1);
		}
		if(s_acc_dir == -1) {
			s_acc_dir = 0;
		}
		s_acc_name = name;
		s_acc_units = units;
		printk("acc %s registered,dir %d ,units %d\n", name, s_acc_dir, s_acc_units);
	}
}

void acc_report(struct input_dev *input, int x, int y, int z)
{
	int t;
	if(s_acc_cal < 0) return;
	switch(s_acc_dir) {
		case 1:
			t = x;
			x = -y;
			y = t;
			break;
		case 2:
			x = -x;
			y = -y;
			break;
		case 3:
			t = x;
			x = y;
			y = -t;
			break;
		case 4:
			x = -x;
			z = -z;
			break;
		case 5:
			t = x;
			x = y;
			y = t;
			z = -z;
			break;
		case 6:
			y = -y;
			z = -z;
			break;
		case 7:
			t = x;
			x = -y;
			y = -t;
			z = -z;
			break;
	}
	if(s_acc_cal) {
		s_acc_val[0] += x;
		s_acc_val[1] += y;
		s_acc_val[2] += z;
		if(s_acc_cal == 1) {
			s_acc_val[0] /= s_acc_count;
			s_acc_val[1] /= s_acc_count;
			s_acc_val[2] /= s_acc_count;
			//always update new off
			s_acc_off[0] = -s_acc_val[0];
			s_acc_off[1] = -s_acc_val[1];
			s_acc_off[2] = s_acc_units - s_acc_val[2];
			printk("acc new offset %d %d %d\n", s_acc_off[0], s_acc_off[1], s_acc_off[2]);
			if(sys_data_write(SYS_DATA_MARK, sizeof(s_acc_off), s_acc_off) == sizeof(s_acc_off)) {
				s_acc_count = 0;
			}
			else {
				printk("udpate new offset fail\n");
				s_acc_count = -1;
			}
		}
		s_acc_cal--;
	}
	x += s_acc_off[0];
	y += s_acc_off[1];
	z += s_acc_off[2];
	input_report_abs(input, ABS_X, x);
	input_report_abs(input, ABS_Y, y);
	input_report_abs(input, ABS_Z, z);
	input_sync(input);
}

static unsigned s_ctp_ok = 0;
int ctp_supported(char * name)
{
	int supported = 0;
	if(!s_ctp_ok) {
		const char * ctps = env_get_str("ctp_supproted", 0);
		supported = !ctps || strstr(ctps, name);
	}
	return supported;
}

void ctp_register(char * name)
{
	if(!s_ctp_ok) {
		s_ctp_ok = 1;
		printk("ctp %s registered\n", name);
	}
}

#include <linux/proc_fs.h>
static int charger_read(char *page, char **start, off_t off,int count, int *eof, void *data)
{
	//int value = gpio_get_value(CHARGE_PIN);
	int value = pmu_data_read(PMU_BAT_CAP);
	sprintf(page, "%d\n", ((value <= 0 || (value & 0x80)) ? 1 : 0));
	return 2;
}

static int charger_write(struct file *file, const char *buffer,unsigned long count, void *data)
{
	if(count && buffer) {
		int value = *buffer == '1';
		printk("set charge mode %d\n", value);
		if(value) {
			//gpio_set_value(BL_EN_PIN, !BL_EN_VALUE);
			gpio_direction_output(CHARGE_PIN, s_charge_pol ? 0 : 1);
		}
		else {
			gpio_direction_output(CHARGE_PIN, s_charge_pol ? 1 : 0);
			//gpio_set_value(BL_EN_PIN, BL_EN_VALUE);
		}
		charge_mode = value;
	}
	return count;
}

static int acc_info_read(char *page, char **start, off_t off,int count, int *eof, void *data)
{
	if(s_acc_name) {
		if(s_acc_cal == -1) {
			int buf[3];
			int len = sys_data_read(SYS_DATA_MARK, sizeof(buf), buf);
			if(len == sizeof(buf)) {
				memcpy(s_acc_off, buf, sizeof(buf));
				printk("acc offset %d %d %d\n", s_acc_off[0], s_acc_off[1], s_acc_off[2]);
			}
			s_acc_cal = 0;
		}
		count = sprintf(page, "name:%s\nunits:%d\ndir:%d\noffset:%d %d %d\n", 
		s_acc_name, s_acc_units, s_acc_dir,
		s_acc_off[0], s_acc_off[1], s_acc_off[2]);
	}
	else {
		count = -1;
	}
	return count;
}

static int acc_info_write(struct file *file, const char *buffer,unsigned long count, void *data)
{
	if(count && buffer && s_acc_dir != -1) {
		s_acc_dir = *buffer - '0';
		printk("set acc direction to %d\n", s_acc_dir);
	}
	else {
		count = -1;
	}
	return count;
}

static int acc_cal_read(char *page, char **start, off_t off,int count, int *eof, void *data)
{
	return sprintf(page, "%d\n", s_acc_cal ? s_acc_cal : s_acc_count);
}

static int acc_cal_write(struct file *file, const char *buffer,unsigned long count, void *data)
{
	int ret = -1;
	if(count && buffer && s_acc_cal == 0) {
		int temp = simple_strtol(buffer, NULL, 0);
		if(temp > 0) {
			printk("acc calibrating %d\n", temp);
			s_acc_val[0] = s_acc_val[1] = s_acc_val[2] = 0;
			s_acc_cal = s_acc_count = temp;
			ret = count;
		}
	}
	return ret;
}

static int backlight_read(char *page, char **start, off_t off,int count, int *eof, void *data)
{
	//int value = gpio_get_value(CHARGE_PIN);
	int value = gpio_get_value(BL_EN_PIN);
	sprintf(page, "%d\n", value ? 1 : 0);
	return 2;
}

static int backlight_write(struct file *file, const char *buffer,unsigned long count, void *data)
{
	if(count && buffer && !s_bl_suspend) {
		int value = *buffer == '1';
		if(value) {
			gpio_set_value(BL_EN_PIN, BL_EN_VALUE);
		}
		else {
			gpio_set_value(BL_EN_PIN, !BL_EN_VALUE);
		}
	}
	return count;
}

static struct list_head power_callbacks;
static int last_status;
static int last_capacity = -1;
void power_register_callback(struct power_callback * callback)
{
	list_add(&(callback->list), &power_callbacks);
	if(last_capacity != -1) {
		callback->callback(callback->param, last_status, last_capacity);
	}
}
EXPORT_SYMBOL(power_register_callback);

void power_unregister_callback(struct power_callback * callback)
{
	list_del(&(callback->list));
}
EXPORT_SYMBOL(power_unregister_callback);

void power_changed(int mask, int value)
{
	struct list_head * pos;
	struct power_callback * callback;
	if(mask == STATUS_CAPACITY) {
		last_capacity = value;
	}
	else {
		last_status = value;
	}
	list_for_each(pos, &power_callbacks) {
		callback = list_entry(pos,struct power_callback, list);
		if(callback->mask & mask) {
			callback->callback(callback->param, last_status, last_capacity);
		}
	}
}
EXPORT_SYMBOL(power_changed);

//android init
static char * s_initrc_buffer = NULL;
static int s_initrc_length = 0;
static int s_buffer_length = 0;

static ssize_t init_rc_read(struct file *file, char __user *buf,
		      size_t len, loff_t * offset)
{
	printk("read init rc\n");
	return simple_read_from_buffer(buf, len, offset,s_initrc_buffer,s_initrc_length);
}

//should support read_file(can't use seq_xxx)
static const struct file_operations init_rc_proc_fops = {
	.owner = THIS_MODULE,
	.read = init_rc_read,
};

static int __init on_init_rc(char * name,char * value,void * param)
{
	int value_length = strlen(value);
	if(s_initrc_length + 1 + value_length > s_buffer_length) {
		s_initrc_buffer = krealloc(s_initrc_buffer, s_buffer_length+4096, GFP_KERNEL);
		if(s_initrc_buffer == NULL) {
			printk("memory out for init_rc");
			return -ENOMEM;
		}
		s_buffer_length += 4096;
	}
	memcpy(s_initrc_buffer + s_initrc_length, value, value_length);
	s_initrc_length += value_length;
	s_initrc_buffer[s_initrc_length++] = '\n';
	return 0;
}

static void __init proc_setup(void)
{
	struct proc_dir_entry * entry = create_proc_entry("charger", S_IRUGO|S_IWUGO, 0);
	if (entry) {
		entry->read_proc = charger_read;
		entry->write_proc = charger_write;
	} else {
		printk("create proc charger failed");
	}
	entry = create_proc_entry("acc_info", S_IRUGO|S_IWUGO, 0);
	if (entry) {
		entry->read_proc = acc_info_read;
		entry->write_proc = acc_info_write;
	} else {
		printk("create proc acc_info failed");
	}
	entry = create_proc_entry("acc_cal", S_IRUGO|S_IWUGO, 0);
	if (entry) {
		entry->read_proc = acc_cal_read;
		entry->write_proc = acc_cal_write;
	} else {
		printk("create proc acc_cal failed");
	}
	entry = proc_create("init.rc", 0, NULL, &init_rc_proc_fops);
	if (entry) {
		env_enum_str("init",NULL,on_init_rc);
		entry->size = s_initrc_length;
	} else {
		printk("create proc init.rc failed");
	}
	entry = create_proc_entry("backlight", S_IRUGO|S_IWUGO, 0);
	if (entry) {
		entry->read_proc = backlight_read;
		entry->write_proc = backlight_write;
	} else {
		printk("create proc backlight failed");
	}
}

//2012-12-4 23:37:17 liqiangman@yftech.com
//backlight should shutdown before lcd
static int __init rk30_late_init(void)
{
#ifdef CONFIG_BACKLIGHT_RK29_BL
	platform_device_register(&rk29_device_backlight);
#endif
	return 0;
}

subsys_initcall(rk30_late_init);

static void __init misc_setup(void)
{
	s_lcd_en_pol = env_get_u32("lcd_en_pol", 0);
	//rk29_bl_info.min_brightness = env_get_u32("lcd_bl_min", 65);
	//rk29_bl_info.max_brightness = env_get_u32("lcd_bl_max", 235);
	//rk29_bl_info.pwm_clock = env_get_u32("lcd_pwm_clock", 20000);
	INIT_LIST_HEAD(&power_callbacks);
}

static void __init machine_rk30_board_init(void)
{
	avs_init();
	gpio_request(POWER_ON_PIN, "poweronpin");
	gpio_direction_output(POWER_ON_PIN, GPIO_HIGH);
	gpio_request(POWER_IND_PIN, "powerindpin");
	gpio_direction_output(POWER_IND_PIN, GPIO_LOW);
	gpio_request(CHARGE_PIN, "chargepin");
	s_charge_pol = env_get_u32("power_charge_pol", 0);
	gpio_direction_output(CHARGE_PIN, s_charge_pol ? 1 : 0);

	pm_power_off = rk30_pm_power_off;
	
	rk30_i2c_register_board_info();
	spi_register_board_info(board_spi_devices, ARRAY_SIZE(board_spi_devices));
	platform_add_devices(devices, ARRAY_SIZE(devices));
	//board_usb_detect_init(RK30_PIN6_PA3);

#ifdef CONFIG_WIFI_CONTROL_FUNC
	rk29sdk_wifi_bt_gpio_control_init();
#endif
	misc_setup();
	proc_setup();
}

static void __init rk30_reserve(void)
{
#ifdef CONFIG_ION
	rk30_ion_pdata.heaps[0].base = board_mem_reserve_add("ion", ION_RESERVE_SIZE);
#endif
#ifdef CONFIG_FB_ROCKCHIP
	resource_fb[0].start = board_mem_reserve_add("fb0", RK30_FB0_MEM_SIZE);
	resource_fb[0].end = resource_fb[0].start + RK30_FB0_MEM_SIZE - 1;
	resource_fb[1].start = board_mem_reserve_add("ipp buf", RK30_FB0_MEM_SIZE);
	resource_fb[1].end = resource_fb[1].start + RK30_FB0_MEM_SIZE - 1;
	resource_fb[2].start = board_mem_reserve_add("fb2", RK30_FB0_MEM_SIZE);
	resource_fb[2].end = resource_fb[2].start + RK30_FB0_MEM_SIZE - 1;
#endif
#ifdef CONFIG_VIDEO_RK29
	rk30_camera_request_reserve_mem();
#endif
	board_mem_reserved();
}

/**
 * dvfs_cpu_logic_table: table for arm and logic dvfs 
 * @frequency	: arm frequency
 * @cpu_volt	: arm voltage depend on frequency
 * @logic_volt	: logic voltage arm requests depend on frequency
 * comments	: min arm/logic voltage
 */
/*
0003d860  00118c30  0011edd8  
0007b0c0  00118c30  0011edd8  
000c7380  0011edd8  00124f80  
000f6180  0011edd8  00124f80  
00124f80  00137478  001312d0  
001368c0  00137478  001312d0  
00159b40  00149970  001312d0  
fffffffe  00000000  00000000  
*/
static struct dvfs_arm_table dvfs_cpu_logic_table[] = {
	{.frequency = 252 * 1000,	.cpu_volt = 1150 * 1000,	.logic_volt = 1175 * 1000},//0.975V/1.000V
	{.frequency = 504 * 1000,	.cpu_volt = 1150 * 1000,	.logic_volt = 1175 * 1000},//0.975V/1.000V
	{.frequency = 816 * 1000,	.cpu_volt = 1175 * 1000,	.logic_volt = 1200 * 1000},//1.000V/1.025V
	{.frequency = 1008 * 1000,	.cpu_volt = 1175 * 1000,	.logic_volt = 1200 * 1000},//1.025V/1.050V
	{.frequency = 1200 * 1000,	.cpu_volt = 1275 * 1000,	.logic_volt = 1250 * 1000},//1.100V/1.050V
	{.frequency = 1272 * 1000,	.cpu_volt = 1275 * 1000,	.logic_volt = 1250 * 1000},//1.150V/1.100V
	{.frequency = 1416 * 1000,	.cpu_volt = 1350 * 1000,	.logic_volt = 1250 * 1000},//1.225V/1.100V
	{.frequency = 1512 * 1000,	.cpu_volt = 1350 * 1000,	.logic_volt = 1250 * 1000},//1.300V/1.150V
	{.frequency = 1608 * 1000,	.cpu_volt = 1425 * 1000,	.logic_volt = 1300 * 1000},//1.325V/1.175V
	{.frequency = CPUFREQ_TABLE_END},
};

static struct cpufreq_frequency_table dvfs_gpu_table[] = {
	{.frequency = 266 * 1000,	.index = 1050 * 1000},
	{.frequency = 400 * 1000,	.index = 1275 * 1000},
	{.frequency = CPUFREQ_TABLE_END},
};

static struct cpufreq_frequency_table dvfs_ddr_table[] = {
	{.frequency = 300 * 1000,	.index = 1050 * 1000},
	{.frequency = 400 * 1000,	.index = 1125 * 1000},
	{.frequency = CPUFREQ_TABLE_END},
};

#define DVFS_CPU_TABLE_SIZE	(ARRAY_SIZE(dvfs_cpu_logic_table))
static struct cpufreq_frequency_table cpu_dvfs_table[DVFS_CPU_TABLE_SIZE];
static struct cpufreq_frequency_table dep_cpu2core_table[DVFS_CPU_TABLE_SIZE];

void __init board_clock_init(void)
{
	rk30_clock_data_init(periph_pll_default, codec_pll_default, RK30_CLOCKS_DEFAULT_FLAGS);
	dvfs_set_arm_logic_volt(dvfs_cpu_logic_table, cpu_dvfs_table, dep_cpu2core_table);
	dvfs_set_freq_volt_table(clk_get(NULL, "gpu"), dvfs_gpu_table);
	dvfs_set_freq_volt_table(clk_get(NULL, "ddr"), dvfs_ddr_table);
}

void __init yftech_fixup(struct machine_desc *desc, struct tag *tags,
			char **cmdline, struct meminfo *mi)
{
	env_fixup();
	rk30_fixup(desc, tags, cmdline, mi);
}

MACHINE_START(RK30, "RK30board")
	.boot_params	= PLAT_PHYS_OFFSET + 0x800,
	.fixup		= yftech_fixup,
	.reserve	= &rk30_reserve,
	.map_io		= rk30_map_io,
	.init_irq	= rk30_init_irq,
	.timer		= &rk30_timer,
	.init_machine	= machine_rk30_board_init,
MACHINE_END
