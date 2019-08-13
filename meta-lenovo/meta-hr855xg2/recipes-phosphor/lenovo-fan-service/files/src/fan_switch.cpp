#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define GPIOBASE 280
#define GPIO_PDB_FAN_TACH_SEL 88
#define SYSFS_GPIO_PATH "/sys/class/gpio/"

#define TIME_INTERVAL 60

#define PDB_FAN1 1
#define PDB_FAN2 0

#define MAX_STR_LEN 256

int exportGPIO (int num) {
    int fd, len;
    char buf[MAX_STR_LEN];

    memset(buf, 0, sizeof(buf));

    fd = open(SYSFS_GPIO_PATH "export", O_WRONLY);
    if (fd < 0) {
        printf("Open %sexport failed\n", SYSFS_GPIO_PATH);
        return fd;
    }

    len = snprintf(buf, sizeof(buf), "%d", (GPIOBASE + num));
    write(fd, buf, len);

    close(fd);

    return 0;
}

int setGPIOValue (int num, bool value) {
    int fd;
    char file_name[MAX_STR_LEN];

    memset(file_name, 0, sizeof(file_name));
    snprintf(file_name, sizeof(file_name), SYSFS_GPIO_PATH "gpio%d/value", (GPIOBASE + num));

    fd = open(file_name, O_WRONLY);
    if (fd < 0) {
        printf("Open %sgpio%d/value failed\n", SYSFS_GPIO_PATH, (GPIOBASE + num));
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
    snprintf(file_name, sizeof(file_name), SYSFS_GPIO_PATH  "gpio%d/direction", (GPIOBASE + num));

    fd = open(file_name, O_WRONLY);
    if (fd < 0) {
        printf("Open %sgpio%d/direction failed\n", SYSFS_GPIO_PATH, (GPIOBASE + num));
        return fd;
    }

    write(fd, dir, strlen(dir) + 1); 

    close(fd);

    return 0;
}

int main(int argc, char *argv[])
{
    bool value = PDB_FAN1;

    exportGPIO(GPIO_PDB_FAN_TACH_SEL);
    setGPIODirection(GPIO_PDB_FAN_TACH_SEL, "out");

    while(1) {
        setGPIOValue(GPIO_PDB_FAN_TACH_SEL, value);
        value ^= 1;
        sleep(TIME_INTERVAL);
    }

    return 0;
}
