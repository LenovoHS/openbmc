#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <string.h>
#include <poll.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#define MAX_STR_LEN 256

#define GPIOBASE 280

#define GPIO_BMC_PCH_SYSPWROK 36      // GPIOE4
#define GPIO_CPLD_CONF_DONE 67        // GPIOI3
#define GPIO_BMC_RSMRST_N 204         // GPIOZ4
#define GPIO_BMC_POST_CMPLT_N 207     // GPIOZ7

#define GPIO_VALUE_H 1
#define GPIO_VALUE_L 0

#define SYSFS_GPIO_PATH "/sys/class/gpio"

#define I2C_BUS_CPLD "/dev/i2c-1"
#define I2C_ADDR_CPLD 0x44 // 7-bit

#define CPLD_REG_GET_SLPS3_N_EVT_STS 0x11
#define CPLD_REG_CLR_SLPS3_N_EVT_STS 0x12
#define RELEASE_SYSPWROK 1
#define CLEAR_EVENT 0

#define NO_TIMEOUT -1

int exportGPIO (int num) {
    int fd, len;
    char buf[MAX_STR_LEN];

    memset(buf, 0, sizeof(buf));

    fd = open(SYSFS_GPIO_PATH "/export", O_WRONLY);
    if (fd < 0) {
        printf("Open %s/export failed\n", SYSFS_GPIO_PATH);
        return fd;
    }

    len = snprintf(buf, sizeof(buf), "%d", (GPIOBASE + num));
    write(fd, buf, len);

    close(fd);

    return 0;
}

int setGPIOValue (int num, int value) {
    int fd;
    char file_name[MAX_STR_LEN];

    memset(file_name, 0, sizeof(file_name));
    snprintf(file_name, sizeof(file_name), SYSFS_GPIO_PATH "/gpio%d/value", (GPIOBASE + num));

    fd = open(file_name, O_WRONLY);
    if (fd < 0) {
        printf("Open %s/gpio%d/value failed\n", SYSFS_GPIO_PATH, (GPIOBASE + num));
        return fd;
    }

    if (value)
        write(fd, "1", 2);
    else
        write(fd, "0", 2);

    close(fd);

    return 0;
}

int getGPIOValue (int num) {
    int fd;
    char file_name[MAX_STR_LEN];
    char rbuf;

    memset(file_name, 0, sizeof(file_name));
    snprintf(file_name, sizeof(file_name), SYSFS_GPIO_PATH "/gpio%d/value", (GPIOBASE + num));

    fd = open(file_name, O_RDONLY);
    if (fd < 0) {
        printf("Open %s/gpio%d/value failed\n", SYSFS_GPIO_PATH, (GPIOBASE + num));
        return fd;
    }

    if(read(fd, &rbuf, 1) < 0) {
        printf("Failed to read value");
        return -1;
    }
    close(fd);

    return atoi(&rbuf);
}

int setGPIODirection (int num, char *dir) {
    int fd;
    char file_name[MAX_STR_LEN];

    memset(file_name,  0, sizeof(file_name));
    snprintf(file_name, sizeof(file_name), SYSFS_GPIO_PATH  "/gpio%d/direction", (GPIOBASE + num));

    fd = open(file_name, O_WRONLY);
    if (fd < 0) {
        printf("Open %s/gpio%d/direction failed\n", SYSFS_GPIO_PATH, (GPIOBASE + num));
        return fd;
    }

    write(fd, dir, strlen(dir) + 1); 

    close(fd);

    return 0;
}

int setGPIOEdge (int num, char *edge_type) {
    int fd;
    char file_name[MAX_STR_LEN];

    memset(file_name,  0, sizeof(file_name));
    snprintf(file_name, sizeof(file_name), SYSFS_GPIO_PATH "/gpio%d/edge", (GPIOBASE + num));

    fd = open(file_name, O_WRONLY);
    if (fd < 0) {
        printf("Open %s/gpio%d/edge failed\n", SYSFS_GPIO_PATH, (GPIOBASE + num));
        return fd;
    }

    write(fd, edge_type, strlen(edge_type) + 1);

    close(fd);

    return 0;
}

int i2cMasterWriteRead(int fd, uint16_t addr, uint8_t *wbuf, uint16_t req_len, uint8_t *rbuf, uint16_t rsp_len) {
    struct i2c_rdwr_ioctl_data data;
    struct i2c_msg msg[2];
    int ret = 0;

    msg[0].len = req_len;
    msg[0].buf = wbuf;
    msg[0].addr = addr;
    msg[0].flags = 0;        // Master write

    msg[1].len = rsp_len;
    msg[1].buf = rbuf;
    msg[1].addr = addr;
    msg[1].flags = I2C_M_RD;

    data.msgs = msg;
    data.nmsgs = 2;

    ret = ioctl(fd, I2C_RDWR, &data);
    if (ret < 0) {
        printf("Failed to set CPLD reg 0x%02X , ret: %d\n", wbuf[0], ret);
        return ret;
    }
    return 0;
}

int i2cMasterWrite(int fd, uint16_t addr, uint8_t *wbuf, uint16_t len) {
    struct i2c_rdwr_ioctl_data data;
    struct i2c_msg msg;
    int ret = 0;

    msg.len = len;
    msg.buf = wbuf;
    msg.addr = addr;
    msg.flags = 0;        // Master write

    data.msgs = &msg;
    data.nmsgs = 1;

    ret = ioctl(fd, I2C_RDWR, &data);
    if (ret < 0) {
        printf("Failed to set CPLD reg 0x%02X , ret: %d\n", wbuf[0], ret);
        return ret;
    }
    return 0;
}

int verifyBIOS() {
    // TODO: Varify BIOS
    //printf("Prepare to varify BIOS...\n");
    //sleep(1);  // Simulate process of verifying BIOS

    return 0;
}

int getSLPS3Sts() {
    int fd;
    uint8_t rbuf[2], wbuf[2];

    fd = open(I2C_BUS_CPLD, O_RDWR);
    if (fd < 0){
        //printf("Open %s failed\n", I2C_BUS_CPLD);
        return -1;
    }

    wbuf[0] = CPLD_REG_GET_SLPS3_N_EVT_STS;
    i2cMasterWriteRead(fd, I2C_ADDR_CPLD, wbuf, 1, rbuf, 1);

    if (rbuf[0] == 0x1) {
        printf("SLPS3 triggered\n");
        return 1;
    } else {
        return 0;
    }
}

int releaseSYSPWROK() {
    int fd, ret = 0;
    uint8_t wbuf[2];

    fd = open(I2C_BUS_CPLD, O_RDWR);
    if (fd < 0){
        //printf("Open %s failed\n", I2C_BUS_CPLD);
        return -1;
    }

    // Set CPLD reg 12 bit-0 as 1 to release SYS_PWROK
    wbuf[0] = CPLD_REG_CLR_SLPS3_N_EVT_STS;
    wbuf[1] = RELEASE_SYSPWROK;
    ret = i2cMasterWrite(fd, I2C_ADDR_CPLD, wbuf, 2);

    return ret;
}

int clearSLPS3Event() {
    int fd, ret = 0;
    uint8_t wbuf[2];

    fd = open(I2C_BUS_CPLD, O_RDWR);
    if (fd < 0){
        //printf("Open %s failed\n", I2C_BUS_CPLD);
        return -1;
    }

    // Clear CPLD reg 12 bit-0 to 0
    wbuf[0] = CPLD_REG_CLR_SLPS3_N_EVT_STS;
    wbuf[1] = CLEAR_EVENT;
    ret = i2cMasterWrite(fd, I2C_ADDR_CPLD, wbuf, 2);

    return ret;
}

int main(int argc, char *argv[]) {
    int ret = 0;
    bool is_slps3_triggered = false;
    int is_cpld_config;

    printf("Power sequence control service running...\n");

    exportGPIO(GPIO_CPLD_CONF_DONE);

    is_cpld_config = getGPIOValue(GPIO_CPLD_CONF_DONE);
    if (is_cpld_config == 1) {
        printf("system initialized already, skip to monitoring !\n");
        goto skip;
    }

    // Export necessary GPIOs
    exportGPIO(GPIO_BMC_PCH_SYSPWROK);
    exportGPIO(GPIO_BMC_RSMRST_N);
    exportGPIO(GPIO_BMC_POST_CMPLT_N);

    //Set BMC_RSMRST_N(Z4) as output/low to hold RSMRST
    setGPIODirection(GPIO_BMC_RSMRST_N, "out");
    setGPIOValue(GPIO_BMC_RSMRST_N, GPIO_VALUE_L);

    //Set BMC_PCH_SYSPWROK(Z6) as output/low to hold SYS_PWROK
    setGPIODirection(GPIO_BMC_PCH_SYSPWROK, "out");
    setGPIOValue(GPIO_BMC_PCH_SYSPWROK, GPIO_VALUE_L);

    ret = verifyBIOS();
    if (ret == 0) {
        printf("BIOS Varification PASS.\n");
        setGPIOValue(GPIO_BMC_RSMRST_N, GPIO_VALUE_H);
    } else {
        printf("BIOS Varification FAILED.\n");
        return -1;
    }

    //Release BMC_PCH_SYSPWROK(Z6) to let host up
    setGPIOValue(GPIO_BMC_PCH_SYSPWROK, GPIO_VALUE_H);

	//Set BMC_POST_CMPLT_N(Z7) as output/low to let CPLD up
    setGPIODirection(GPIO_BMC_POST_CMPLT_N, "out");
    setGPIOValue(GPIO_BMC_POST_CMPLT_N, GPIO_VALUE_L);

skip:
    while (1) {
        is_slps3_triggered = getSLPS3Sts();

        if (is_slps3_triggered) {
            ret = verifyBIOS();
            if (ret == 0) {
                //printf("BIOS Varification PASS.\n");
                releaseSYSPWROK();
                usleep(100000);   // 100 ms
                clearSLPS3Event();
            } else {
                //printf("BIOS Varification FAILED.\n");
                return -1;
            }
        }
        sleep(3);
    }

    return 0;
}
