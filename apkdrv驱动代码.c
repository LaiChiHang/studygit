#include <linux/module.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/switch.h>
#include <linux/regulator/consumer.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h> 
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/i2c.h>
#include <mach/sys_config.h>
#include <asm/io.h>

extern void set_codec_fmtx_en    (int en  );
extern void set_codec_mute_flags (int mute);
extern int  get_codec_fmtx_en    (void);
extern int  get_codec_mute_flags (void);

extern void apkdrv_set_avin_switch(int );
extern int  apkdrv_get_avin_switch(void);
static int  kta0806_set_freq(int freq);
static int  kta0806_get_freq(void);

//++ for hwevent
static int apical_hwevent_state = 0;

static ssize_t switch_print_state(struct switch_dev *sdev, char *buf)
{
    return sprintf(buf, "%d\n", apical_hwevent_state);
}

static ssize_t switch_print_name(struct switch_dev *sdev, char *buf)
{
    return sprintf(buf, "%s\n", sdev->name);
}

static struct switch_dev g_sdev = {
    .name = "hwevent",
    .print_name = switch_print_name,
    .print_state = switch_print_state,
};

void apkdrv_report_bakcar_state(int backcar)
{
    int last = (apical_hwevent_state & (1 << 1)) << 16;
    int cur  = (backcar & (1 << 0)) << 1;
    apical_hwevent_state &= ~((1 << 1) | (1 << 17));
    apical_hwevent_state |=  (last | cur);
    switch_set_state(&g_sdev, apical_hwevent_state);
}
EXPORT_SYMBOL(apkdrv_report_bakcar_state);
//-- for hwevent

static int g_internal_gps_power = 0;
void apkdrv_set_internal_gps_pwr(int onoff, int flag)
{
    script_item_u             val;
    script_item_u            pwr1;
    script_item_u            pwr2;
    script_item_u          gonoff;
    script_item_u          wakeup;
    script_item_value_type_e type;
    int retry = 5;

    type = script_get_item("gps_para", "gps_used", &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT == type && val.val) {
        script_get_item("gps_para", "gps_pwr1"  , &pwr1  );
        script_get_item("gps_para", "gps_pwr2"  , &pwr2  );
        script_get_item("gps_para", "gps_onoff" , &gonoff);
        script_get_item("gps_para", "gps_wakeup", &wakeup);

        gpio_request(pwr1  .gpio.gpio, NULL);
        gpio_request(pwr2  .gpio.gpio, NULL);
        gpio_request(gonoff.gpio.gpio, NULL);
        gpio_request(wakeup.gpio.gpio, NULL);
        gpio_direction_input(wakeup.gpio.gpio);

        switch (onoff) {
        case 0: // power off
            while (gpio_get_value(wakeup.gpio.gpio) && --retry) {
                gpio_direction_output(gonoff.gpio.gpio, 0); msleep(50);
                gpio_set_value       (gonoff.gpio.gpio, 1); msleep(50);
            }
            gpio_direction_output(pwr1.gpio.gpio, 0);
            gpio_direction_output(pwr2.gpio.gpio, 0); msleep(50);
            break;
        case 1: // standby
            gpio_direction_output(pwr1.gpio.gpio, 1);
            gpio_direction_output(pwr2.gpio.gpio, 1); msleep(50);
            while (gpio_get_value(wakeup.gpio.gpio) && --retry) {
                gpio_direction_output(gonoff.gpio.gpio, 0); msleep(50);
                gpio_set_value       (gonoff.gpio.gpio, 1); msleep(50);
            }
            break;
        case 2: // power on
            gpio_direction_output(pwr1.gpio.gpio, 1);
            gpio_direction_output(pwr2.gpio.gpio, 1); msleep(50);
            while (!gpio_get_value(wakeup.gpio.gpio) && --retry) {
                gpio_direction_output(gonoff.gpio.gpio, 0); msleep(50);
                gpio_set_value       (gonoff.gpio.gpio, 1); msleep(50);
            }
            break;
        }

        gpio_free(pwr1  .gpio.gpio);
        gpio_free(pwr2  .gpio.gpio);
        gpio_free(gonoff.gpio.gpio);
        gpio_free(wakeup.gpio.gpio);

        // save gps power state
        if (flag) g_internal_gps_power = onoff;
    }
}
EXPORT_SYMBOL(apkdrv_set_internal_gps_pwr);

static int g_tmc_power = 0;
void apical_tmc_power(__u32 onoff, int flag)
{
    int ret = 0;
    struct regulator *tmc_pwr_ldo0 = NULL;
    struct regulator *tmc_pwr_ldo1 = NULL;
    struct regulator *tmc_pwr_ldo2 = NULL;

    tmc_pwr_ldo0 = regulator_get(NULL, "axp22_eldo1");
    if (tmc_pwr_ldo0) {
        if (onoff) {
            regulator_set_voltage(tmc_pwr_ldo0, 3000000, 3000000);
            ret = regulator_enable(tmc_pwr_ldo0);
            if (ret < 0) {
                printk("enable axp22_eldo1 regulator faile.\n");
            } else {
                printk("enable axp22_eldo1 regulator ok.\n");
                regulator_put(tmc_pwr_ldo0);
            }
        } else {
            ret = regulator_disable(tmc_pwr_ldo0);
            if (ret < 0) {
                printk("regulator_disable fail, return %d.\n", ret);
            } else {
                regulator_put(tmc_pwr_ldo0);
            }
        }
    } else {
       printk("get power regulator faile.\n");
    }

    tmc_pwr_ldo1 = regulator_get(NULL, "axp22_dldo3");
    if (tmc_pwr_ldo1) {
        if (onoff) {
            regulator_set_voltage(tmc_pwr_ldo1, 2800000, 2800000);
            ret = regulator_enable(tmc_pwr_ldo1);
            if (ret < 0) {
                printk("enable axp22_dldo3 regulator faile.\n");
            } else {
                printk("enable axp22_dldo3 regulator ok.\n");
                regulator_put(tmc_pwr_ldo1);
            }
        } else {
            ret = regulator_disable(tmc_pwr_ldo1);
            if (ret < 0) {
                printk("regulator_disable fail, return %d.\n", ret);
            } else {
                regulator_put(tmc_pwr_ldo1);
            }
        }
    } else {
       printk("get power regulator faile.\n");
    }

    tmc_pwr_ldo2 = regulator_get(NULL, "axp22_ldoio0");
    if (tmc_pwr_ldo2) {
        if (onoff) {
            regulator_set_voltage(tmc_pwr_ldo2, 2800000, 2800000);
            ret = regulator_enable(tmc_pwr_ldo2);
            if (ret < 0) {
                printk("enable axp22_ldoio0 regulator faile.\n");
            } else {
                printk("enable axp22_ldoio0 regulator ok.\n");
                regulator_put(tmc_pwr_ldo2);
            }
        } else {
            ret = regulator_disable(tmc_pwr_ldo2);
            if (ret < 0) {
                printk("regulator_disable fail, return %d.\n", ret);
            } else {
                regulator_put(tmc_pwr_ldo2);
            }
        }
    } else {
       printk("get power regulator faile.\n");
    }

    if (flag) g_tmc_power = onoff;
}
EXPORT_SYMBOL(apical_tmc_power);

static int g_avin_enable = 0;
int apkdrv_get_avin_enable(void)
{
    return g_avin_enable;
}
EXPORT_SYMBOL(apkdrv_get_avin_enable);

//++ inti powerup hardware work
static struct delayed_work apkdrv_init_work;
static void init_work_func(struct work_struct *work)
{
//  apical_tmc_power(1, 1);
    apkdrv_set_internal_gps_pwr(1, 1);
}
//-- inti powerup hardware work

//++ resume powerup hardware work
static struct delayed_work apkdrv_resume_work;
static void resume_work_func(struct work_struct *work)
{
//  apical_tmc_power(g_tmc_power, 0);
    apkdrv_set_internal_gps_pwr(g_internal_gps_power, 0);
}
//-- resume powerup hardware work

static int apical_open(struct inode *inode, struct file *filp)
{
    return 0;
}

static int apical_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static ssize_t apical_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    int  len = 0;
    char tmp[512] = {0};

    len = sprintf(tmp,
                "gpspwr      : %d\n"
                "usrmute     : %d\n"
                "sysmute     : %d\n"
                "hwevent     : %d\n"
                "avinen      : %d\n"
                "avinsw      : %d\n"
                "fmtx        : %d\n"
                "tmcpwr      : %d\n",
                g_internal_gps_power,
                (get_codec_mute_flags() & (1 << 0)) ? 1 : 0,
                (get_codec_mute_flags() & (1 << 1)) ? 1 : 0,
                apical_hwevent_state,
                g_avin_enable,
                apkdrv_get_avin_switch(),
                kta0806_get_freq(),
                g_tmc_power
            ) + 1;

    if (len < count && *f_pos == 0) {
        if (copy_to_user(buf, tmp, len)) {
            return -EFAULT;
        }
        else {
            *f_pos += len;
            return len;
        }
    }

    return 0;
}

static ssize_t apical_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    char tmp[512] = {0};
    char key[32];
    int  value;

    if (count > 512) return count;
    if (copy_from_user(tmp, buf, count)) {
        return -EFAULT;
    }
    else {
        sscanf(tmp, "%s %d", key, &value);
        printk("%s key = %s value = %d\n", tmp, key, value);
        if (strcmp(key, "gpspwr") == 0) {
            apkdrv_set_internal_gps_pwr(value, 1);
        }
        else if (strcmp(key, "usrmute") == 0) {
            int flags = get_codec_mute_flags();
            if (value) {
                flags |= (1 << 0);
            }
            else {
                flags &=~(1 << 0);
            }
            set_codec_mute_flags(flags);
        }
        else if (strcmp(key, "sysmute") == 0) {
            int flags = get_codec_mute_flags();
            if (value) {
                flags |= (1 << 1);
            }
            else {
                flags &=~(1 << 1);
            }
            set_codec_mute_flags(flags);
        }
        else if (strcmp(key, "avinen") == 0) {
            g_avin_enable = value;
        }
        else if (strcmp(key, "avinsw") == 0) {
            apkdrv_set_avin_switch(value);
        }
        else if (strcmp(key, "fmtx") == 0) {
            if (kta0806_get_freq() != value) {
                kta0806_set_freq(value);
            }
        }
        else if (strcmp(key, "tmcpwr") == 0) {
            apical_tmc_power(value, 1);
        }
        *f_pos += count;
        return count;
    }
}

static long apical_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    return 0;
}

static struct file_operations apical_fops =
{
    .owner          = THIS_MODULE,
    .open           = apical_open,
    .release        = apical_release,
    .read           = apical_read,
    .write          = apical_write,
    .unlocked_ioctl = apical_ioctl,
};

static struct miscdevice apical_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name  = "apical",
    .fops  = &apical_fops,
};

static int apical_probe(struct platform_device *pdev)
{
    switch_dev_register(&g_sdev);
    misc_register(&apical_misc);
    return 0;
}

static int apical_remove(struct platform_device *pdev) 
{
    switch_dev_unregister(&g_sdev);
    misc_deregister(&apical_misc);
    return 0;
}

static int apical_suspend(struct platform_device *pdev, pm_message_t mesg)
{
    cancel_delayed_work_sync(&apkdrv_resume_work);
    apkdrv_set_internal_gps_pwr(1, 0);
//  apical_tmc_power(0, 0);
    return 0;
}

static int apical_resume(struct platform_device *pdev)
{
    schedule_delayed_work(&apkdrv_resume_work, HZ/100);
    return 0;
}

static void apical_shutdown(struct platform_device *pdev)
{
}

static struct platform_device apical_device = {
    .name          = "apical",
    .id            = -1,
    .resource      = NULL,
    .num_resources = 0,
};

static struct platform_driver apical_driver = 
{
    .probe      = apical_probe,
    .remove     = apical_remove,
    .suspend    = apical_suspend,
    .resume     = apical_resume,
    .shutdown   = apical_shutdown,
    .driver     = {
        .name   = "apical",
        .owner  = THIS_MODULE,
    }
};

static int __init apical_board_init(void)
{
    INIT_DELAYED_WORK(&apkdrv_init_work  , init_work_func  ); // init init work for init powerup
    INIT_DELAYED_WORK(&apkdrv_resume_work, resume_work_func); // init resume work for resume powerup

    // schedule  init powerup work
    schedule_delayed_work(&apkdrv_init_work, HZ*1);

    platform_device_register(&apical_device);
    platform_driver_register(&apical_driver);
    return 0;
}

static void __exit apical_board_exit(void)
{
    cancel_delayed_work_sync(&apkdrv_init_work  );
    cancel_delayed_work_sync(&apkdrv_resume_work);

    platform_driver_unregister(&apical_driver);
    platform_device_unregister(&apical_device);
}

module_init(apical_board_init);
module_exit(apical_board_exit);


//++ for kta0806 fmtx ++//
#define KTA0806_SLAVEID         0x36
#define KTA0806_I2C_ADAPTER_NR  1
#define KTA0806_POWERUP_DELAY   500

#define RFGAIN     0xf  // 4bits
#define PGAVAL     0x00 // 5bits
#define SWMUTE     0x0  // 1bits
#define PLTADJ     0x0  // 1bits
#define PHTCNST    0x0  // 1bits
#define PA_BIAS    0x1  // 1bits
#define BASS_BOOST 0x0  // 2bits
#define LMTLVL     0x1  // 2bits
#define SW_MOD     0x0  // 1bits
#define SLNCDIS    0x0  // 1bits
#define SLNCTHH    0x5  // 3bits
#define SLNCTHL    0x0  // 3bits

static unsigned char kta0806_init_regs[][2] = {
    { 0x0B, (1 << 7) | (0 << 5) },
    { 0x00, 0x81 },
    { 0x01, ((RFGAIN & 0x3) << 6) | ((PGAVAL & 0x1c) << 1) | (0x3 << 0) },
    { 0x02, (0 << 7) | ((RFGAIN & (1 << 3)) << 3) | (SWMUTE << 3) | (PLTADJ << 2) | (PHTCNST << 0) },
    { 0x04, ((PGAVAL & 0x3) << 4) | (BASS_BOOST << 0) },
    { 0x0E, (PA_BIAS << 1) },
    { 0x10, (LMTLVL  << 3) },
    { 0x12, (SLNCDIS << 7) | (SLNCTHL << 4) | (SLNCTHH << 1) | (SW_MOD << 0) },
    { 0x13, ((RFGAIN & (1 << 2)) << 5) },
    { 0x14, (3 << 5) | (0 << 2) },
    { 0x16, (5 << 0) },
};
static int kta0806_power_delay = KTA0806_POWERUP_DELAY;

static int kta0806_write_regs(unsigned char addr, unsigned char *regs, int len)
{
    struct i2c_adapter *adapter;
    struct i2c_msg      msg;
    unsigned char       buf[257];
    unsigned char       slaveid;
    int                 ret;
    int                 retry;

    adapter = i2c_get_adapter(KTA0806_I2C_ADAPTER_NR);
    slaveid = KTA0806_SLAVEID;
    buf[0]  = addr;
    memcpy(&(buf[1]), regs, len);

    msg.addr  = slaveid;
    msg.flags = 0;
    msg.len   = len + 1;
    msg.buf   = buf;

    for (retry=0; retry<3; retry++) {
        ret = i2c_transfer(adapter, &msg, 1);
        if (ret < 0) {
            printk("%s error! slave = 0x%02x, addr = 0x%02x, len = %d\n ",__func__, slaveid, buf[0], len);
            msleep(10);
        }
        else break;
    }
    return ret;
}

static int kta0806_get_freq(void)
{
    if (kta0806_init_regs[0][1] & (1 << 7)) return 0;
    return (kta0806_init_regs[1][1] << 0) | ((kta0806_init_regs[2][1] & 0x7) << 8);
}

static int kta0806_set_freq(int freq)
{
    #define LOSC_VA_OUT_GATE  0xF1F00060
    void __iomem *sunxi_losc_reg = (void __iomem *)(LOSC_VA_OUT_GATE);
    unsigned long val  = 0;
    int   stb_changed  = 0;
    int   freq_changed = 0;
    int   i;

    if (freq == 0) {
        stb_changed = kta0806_init_regs[0][1] & (1 << 7) ? 0 : 1;
        freq_changed= 0;

        // enable standby
        kta0806_init_regs[0][1] |= (1 << 7); // reg0b
    } else if (freq == -1) {
        kta0806_power_delay = KTA0806_POWERUP_DELAY;
    } else {
        stb_changed = kta0806_init_regs[0][1] & (1 << 7) ? 1 : 0;
        freq_changed= freq != ((kta0806_init_regs[1][1] << 0) | ((kta0806_init_regs[2][1] & 0x7) << 8));

        // disable standby
        kta0806_init_regs[0][1] &=~(1 << 7); // reg0b

        // set freq
        kta0806_init_regs[1][1] = (freq >> 0) & 0xff; // reg00
        kta0806_init_regs[2][1]&= ~0x7;
        kta0806_init_regs[2][1]|= (freq >> 8) & 0x07; // reg01
    }

    if (!(kta0806_init_regs[0][1] & (1 << 7))) { // if disable standby
        // enable 32KHz clk output
        val  = readl(sunxi_losc_reg);
        val |= (1 << 0);
        writel(val, sunxi_losc_reg);

        // enable codec channel for fmtx
        set_codec_fmtx_en(1);

        // if powerup need delay
        if (kta0806_power_delay) {
            // powerup kta0806 chip
            kta0806_write_regs(kta0806_init_regs[0][0], &(kta0806_init_regs[0][1]), 1);

            msleep(kta0806_power_delay);
            kta0806_power_delay = 0;

            // write init regs
            for (i=3; i<sizeof(kta0806_init_regs)/sizeof(kta0806_init_regs[0]); i++) {
                kta0806_write_regs(kta0806_init_regs[i][0], &(kta0806_init_regs[i][1]), 1);
            }
        }
    }

    // update standby reg if needed
    if (stb_changed) {
        kta0806_write_regs(kta0806_init_regs[0][0], &(kta0806_init_regs[0][1]), 1);
    }

    // update freq regs if needed
    if (freq_changed) {
        kta0806_write_regs(kta0806_init_regs[1][0], &(kta0806_init_regs[1][1]), 1);
        kta0806_write_regs(kta0806_init_regs[2][0], &(kta0806_init_regs[2][1]), 1);
    }

    if ( (kta0806_init_regs[0][1] & (1 << 7))) { // if enable standby
        // disable 32KHz clk output
        val  = readl(sunxi_losc_reg);
        val &=~(1 << 0);
        writel(val, sunxi_losc_reg);
        // disable codec channel for fmtx
        set_codec_fmtx_en(0);
    }

    return 0;
}
//-- for kta0806 fmtx --//

