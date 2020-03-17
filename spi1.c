/* This is a simple application for testing SPI communication on a RedPitaya
** Test GPIO port P
*/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <sys/mman.h>

/* Inline functions definition */
static int init_spi();
static int release_spi();
static int write_spi(int* write_data, int size);
int write_gpio(long number);
int pulse_gpio(long number);

/* Constants definition */
int spi_fd = -1;
int errmsg = 0;

int main(void){

    /* Sample data */
    //char *data = "REDPITAYA SPI TEST";	
	int dataA [6][1] = {{0x002c8018}, {0x08008029}, {0x00004e42}, {0x000004b3}, {0x00ec803c}, {0x00580005}};
	
	if(init_spi() < 0){
        printf("Initialization of SPI failed. Error: %s\n", strerror(errno));
        return -1;
    }
	
	/* Initiate the GPIO */
	errmsg = write_gpio(0);

	/*******  User added code block     **********/ 
	/* Write R0~R5 data */
	for(int i=0; i<6; ++i){
		if(write_spi(dataA[i], sizeof(dataA[i])) < 0){
			printf("Write dataA[%i] to SPI failed. Error: %s\n", i, strerror(errno));
			return -1;
		}
	}
	/****** End of user code block      ******** */
	
	/* Release resources */
    if(release_spi() < 0){
        printf("Relase of SPI resources failed, Error: %s\n", strerror(errno));
        return -1;
    }
	
    return 0;
}

static int init_spi(){

    /* MODES: mode |= SPI_LOOP;
    *        mode |= SPI_CPHA;
    *        mode |= SPI_CPOL;
    *                 mode |= SPI_LSB_FIRST;
    *        mode |= SPI_CS_HIGH;
    *        mode |= SPI_3WIRE;
    *        mode |= SPI_NO_CS;
    *        mode |= SPI_READY;
    *
    * multiple possibilities possible using | */
    int mode = 0;
//	    mode |= SPI_MODE_3;
		

    /* Opening file stream */
    spi_fd = open("/dev/spidev1.0", O_RDWR | O_NOCTTY);

    if(spi_fd < 0){
        printf("Error opening spidev0.1. Error: %s\n", strerror(errno));
        return -1;
    }

    /* Setting mode (CPHA, CPOL) */
    if(ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0){
        printf("Error setting SPI_IOC_RD_MODE. Error: %s\n", strerror(errno));
        return -1;
    }

    /* Setting SPI bus speed */
    int spi_speed = 1000000;

    if(ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed) < 0){
        printf("Error setting SPI_IOC_WR_MAX_SPEED_HZ. Error: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

static int release_spi(){

    /* Release the spi resources */
    close(spi_fd);

    return 0;
}

/* Write data to the SPI bus */
static int write_spi(int* write_buffer, int size){

    int write_spi = write(spi_fd, write_buffer, size);

    if(write_spi < 0){
        printf("Failed to write to SPI. Error: %s\n", strerror(errno));
        return -1;
    }
	//usleep(200);
	
	//set HIGH the GPIO 
	errmsg = pulse_gpio(16);
	
	usleep(200);

    return 0;
}

/*  open the file and write into gpio  */
//int write_gpio(int argc, char *argv[])
int write_gpio(long number)
{
  int fd_gpio;
  volatile void *cfg;

  errno = 0;
  //number = (argc == 2) ? strtol(argv[1], &end, 10) : -1;
  if(errno != 0 || number < 0 || number > 255)
  {
    fprintf(stderr, "Usage: gpio-output [0-255]\n");
    return EXIT_FAILURE;
  }

  if((fd_gpio = open("/dev/mem", O_RDWR)) < 0)
  {
    fprintf(stderr, "Cannot open /dev/mem.\n");
    return EXIT_FAILURE;
  }

  cfg = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd_gpio, 0x40001000);

  *(uint8_t *)(cfg + 2) = (uint8_t)number;
  
  /* Clean the variable */
  close(fd_gpio);

  return EXIT_SUCCESS;
}

int pulse_gpio(long number)
{
  int fd_gpio;
  volatile void *cfg;
  long m_zero = 0;

  errno = 0;

  if(errno != 0 || number < 0 || number > 255)
  {
    fprintf(stderr, "Usage: gpio-output [0-255]\n");
    return EXIT_FAILURE;
  }

  if((fd_gpio = open("/dev/mem", O_RDWR)) < 0)
  {
    fprintf(stderr, "Cannot open /dev/mem.\n");
    return EXIT_FAILURE;
  }

  cfg = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd_gpio, 0x40001000);

  *(uint8_t *)(cfg + 2) = (uint8_t)m_zero;
  *(uint8_t *)(cfg + 2) = (uint8_t)number;
   // usleep(.2);
  *(uint8_t *)(cfg + 2) = (uint8_t)m_zero;
  
  /* Clean the variable */
  close(fd_gpio);

  return EXIT_SUCCESS;
}
