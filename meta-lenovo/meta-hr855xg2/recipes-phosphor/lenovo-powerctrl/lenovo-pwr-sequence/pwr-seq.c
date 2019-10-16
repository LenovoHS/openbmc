//# Copyright (c) 2019-present Lenovo
//# Licensed under BSD-3, see COPYING.BSD file for details.

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
#include <sys/mman.h>
#include <syslog.h>
#include <linux/gpio.h>
#include <stddef.h>
#include <assert.h>
#include <errno.h>

#define MAX_STR_LEN 256

#define GPIOBASE 280

#define BMC_FPGA_FLASH_MUX_SEL1    	52		//GPIOG4
#define CP_SPI_FLASH_NCONFIG		58		//GPIOH2
#define BMC_RESET_FPGA_N			59		//GPIOH3  no enable by now
#define FM_BIOS_SPI_SW_CTRL_R_0     200     //GPIOZ0
#define PCH_PWROK_BMC_FPGA            3     //GPIOA3 BMC notice FPGA finish verifying BIOS, FPGA can release SYS_PWROK.
#define PWRGD_SYS_PWROK_BMC          63     //GPIOH7
#define PDB_RESTART_N     			202 	// GPIO_Z2

#define  FPGA_READY					90	//GPIOL2
#define  INIT_DONE					33	//GPIOE1

#define GPIO_VALUE_H 1
#define GPIO_VALUE_L 0

#define SYSFS_GPIO_PATH "/sys/class/gpio"

#define NO_TIMEOUT -1

#define GPIO_OK           0x00
#define GPIO_ERROR        0x01
#define GPIO_OPEN_ERROR   0x02
#define GPIO_INIT_ERROR   0x04
#define GPIO_READ_ERROR   0x08
#define GPIO_WRITE_ERROR  0x10
#define GPIO_LOOKUP_ERROR 0x20

typedef struct {
  char* name;
  char* dev;
  uint16_t num;
  uint16_t chip_id;
  char* direction;
  int fd;
  bool irq_inited;
} GPIO;

void gpio_close(GPIO* gpio)
{
        if(gpio->fd < 0)
                return;

        close(gpio->fd);
        gpio->fd = -1;
}

int gpio_read(GPIO* gpio, uint8_t *value)
{
        assert (gpio != NULL);
        struct gpiohandle_data data;
        memset(&data, 0, sizeof(data));

        if (gpio->fd <= 0)
        {
                return GPIO_ERROR;
        }

        if (ioctl(gpio->fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) < 0)
        {
                return GPIO_READ_ERROR;
        }

        *value = data.values[0];

        return GPIO_OK;
}

int gpio_open(GPIO* gpio, uint8_t default_value)
{
        assert (gpio != NULL);

        char buf[255];
        sprintf(buf, "/dev/gpiochip%d", gpio->chip_id);
        gpio->fd = open(buf, 0);
        if (gpio->fd == -1)
        {
                return GPIO_OPEN_ERROR;
        }

        struct gpiohandle_request req;
        memset(&req, 0, sizeof(req));
        strncpy(req.consumer_label, "skeleton-gpio",  sizeof(req.consumer_label));

        if (gpio->direction == NULL)
        {
                gpio_close(gpio);
                return GPIO_OPEN_ERROR;
        }
        req.flags = (strcmp(gpio->direction,"in") == 0) ? GPIOHANDLE_REQUEST_INPUT
                                                                : GPIOHANDLE_REQUEST_OUTPUT;
        req.lineoffsets[0] = gpio->num;
        req.lines = 1;

        if (strcmp(gpio->direction,"out") == 0)
        {
                req.default_values[0] = default_value;
        }

        int rc = ioctl(gpio->fd, GPIO_GET_LINEHANDLE_IOCTL, &req);
        if (rc < 0)
        {
                gpio_close(gpio);
                return GPIO_OPEN_ERROR;
        }
        gpio_close(gpio);
        gpio->fd = req.fd;

        return GPIO_OK;
}

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

int getGPIOValue (int num) {
	int fd;
	char file_name[MAX_STR_LEN];
	char rbuf[MAX_STR_LEN];
	int value_str;
	
	printf("Prepare to getGPIOValue 111...\n");
	memset(file_name, 0, sizeof(file_name));
	snprintf(file_name, sizeof(file_name), SYSFS_GPIO_PATH "/gpio%d/value", (GPIOBASE + num));
	
	fd = open(file_name, O_RDONLY);
    if (fd < 0) {
        printf("Open %s/gpio%d/value failed\n", SYSFS_GPIO_PATH, (GPIOBASE + num));
        return fd;
    }
	printf("Prepare to getGPIOValue 222...\n");
	if(read(fd, rbuf,1) < 0)
	{
		printf("Failed to read value");
		return -1;
	}
	printf("Prepare to getGPIOValue 333...\n");
	close(fd);
	value_str = rbuf[0];
	
	printf("===debug===get gpio value%d===:%d:%d\n",value_str, rbuf[0],rbuf[1]);
	return value_str;
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


int verifyBIOS() {
   // int value = 0;
    printf("Prepare to verify BIOS...\n");
	
 //Switch BIOS SPI ROM TO BMC for BIOS verification
    setGPIODirection(FM_BIOS_SPI_SW_CTRL_R_0, "out");
  //  setGPIOValue(FM_BIOS_SPI_SW_CTRL_R_0, GPIO_VALUE_H);
	
	// value = getGPIODirection(FM_BIOS_SPI_SW_CTRL_R_0);
	// printf("====debug==value==%d\n", value);
	//printf("begin to verify BIOS...\n");
	sleep(10);
	
	//printf("finish verifying BIOS ...\n ");
	
//Switch BIOS SPI ROM to Host ==low

	 setGPIOValue(FM_BIOS_SPI_SW_CTRL_R_0, GPIO_VALUE_L);
	 //printf("Switch BIOS SPI rom to HOST ...\n ");
	 
	
    return 0;
}

int verifyFPGA() {

    fprintf(stderr, "Prepare to verify FPGA...\n");
    
    //TODO: verify FPGA SPI ROM 
    sleep(10);
	
    //switch FPGA SPI ROM TO FPGA
    setGPIODirection(BMC_FPGA_FLASH_MUX_SEL1, "out");
    setGPIOValue(BMC_FPGA_FLASH_MUX_SEL1, GPIO_VALUE_L);

    sleep(1);
    //Enable Nconfig to make FPGA boot
    setGPIODirection(CP_SPI_FLASH_NCONFIG, "out");
    setGPIOValue(CP_SPI_FLASH_NCONFIG, GPIO_VALUE_H);

    sleep(2);
	
    //Enable FPGA to reset
    //setGPIODirection(BMC_RESET_FPGA_N, "out");
    //setGPIOValue(BMC_RESET_FPGA_N, GPIO_VALUE_L);

   // printf("FPGA varification PASS\n");

    return 0;
}

int get_power_status(void)
{
        uint8_t pgood_state;
        GPIO power_gpio;
        power_gpio.name = "CPU_PGOOD";
        power_gpio.dev  = "/sys/class/gpio";
        power_gpio.num  = 195;
        power_gpio.chip_id = 0;
        power_gpio.direction = "in";
        int rc = gpio_open(&power_gpio, 0);
        if(rc != GPIO_OK)
        {
                gpio_close(&power_gpio);
                return 2;
        }
        rc = gpio_read(&power_gpio, &pgood_state);
        gpio_close(&power_gpio);
        if (rc != GPIO_OK) return 2;

        return pgood_state;
}

int main(int argc, char *argv[]) {
    fprintf(stderr, "Power sequence control service running...\n");
	
    int PowerOKStatus = get_power_status();
//	exportGPIO(PWRGD_SYS_PWROK_BMC);
	
//	PowerOKStatus = getGPIOValue(PWRGD_SYS_PWROK_BMC);
	
    exportGPIO(PDB_RESTART_N);
    setGPIODirection(PDB_RESTART_N, "high");
	
    if(1 == PowerOKStatus)
    {
        fprintf(stderr, "pwr seq: do nothing when host is on\n");
        return 0;
    }
    // Export necessary GPIOs
    exportGPIO(BMC_FPGA_FLASH_MUX_SEL1);
    exportGPIO(CP_SPI_FLASH_NCONFIG);
    exportGPIO(BMC_RESET_FPGA_N);
    exportGPIO(FM_BIOS_SPI_SW_CTRL_R_0);

    verifyBIOS();
	
    sleep(3);
	
    verifyFPGA();

  //  printf("Start monitoring...\n");

    return 0;
}
