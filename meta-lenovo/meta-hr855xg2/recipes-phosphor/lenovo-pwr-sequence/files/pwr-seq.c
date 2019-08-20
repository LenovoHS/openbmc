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

    printf("Prepare to verify FPGA...\n");
    
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


int main(int argc, char *argv[]) {
    printf("Power sequence control service running...\n");
	
//	int PowerOKStatus;
//	exportGPIO(PWRGD_SYS_PWROK_BMC);
	
//	PowerOKStatus = getGPIOValue(PWRGD_SYS_PWROK_BMC);
	
	exportGPIO(PDB_RESTART_N);
	setGPIODirection(PDB_RESTART_N, "high");
	
//	if(PowerOKStatus == 0x31)
//	{
//		printf("do nothing when host is on\n");
//		return 0;
//	}
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
