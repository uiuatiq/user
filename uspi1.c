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
static int write_gpio(long number);
static int user_input(void);

/* Constants definition */
int spi_fd = -1;

int main(void){

    /* Sample data */
    //char *data = "REDPITAYA SPI TEST";	
	unsigned int option = 0;
	int data [6][1] = {{0x00000000}, {0x00000000}, {0x00000000}, {0x00000000}, {0x00000000}, {0x00000000}}; // Just initialize with all zero value
	int dataA [6][1] = {{0x18802c00}, {0x29800008}, {0x424e0000}, {0xb3040000}, {0x3c80ec00}, {0x05005800}}; //for 35Mhz
	//int dataB [6][1] = {{0x00004000}, {0x11800008}, {0x424e0000}, {0xb3040000}, {0x3c80ac00}, {0x05005800}}; //for 800Mhz

	int dataB [6][1] = {{0x18802c00}, {0x29800008}, {0x424e0000}, {0xb3040000}, {0x3c80ec00}, {0x05005800}}; //for 35Mhz
	if(init_spi() < 0){
            printf("Initialization of SPI failed. Error: %s\n", strerror(errno));
            return -1;
        }
	
	/*******  User added code block     **********/ 
	/* get User Input from terminal */
	do{
		option = user_input();
		
		switch(option){
			case 1 :memcpy(data,dataA,sizeof(dataA)); break;
			//case 2 : memcpy(data,dataB,sizeof(dataB)); break;
			case 2 :memcpy(data,dataB,sizeof(dataB)); break;
		       //	case 3 : strcpy(data,dataB); break;
		//	case 4 : strcpy(data,dataB); break;
			//case 5 : strcpy(data,dataB); break;
			//case 6 : strcpy(data,dataB); break;
			//case 7 : strcpy(data,dataB); break;
			//case 8 : strcpy(data,dataB); break;
			//case 9 : strcpy(data,dataB); break;
			case 99 : return 0;
			default: printf("Wrong input!\n"); continue;
		}
		
		printf("Sending register data ...\n");
		/* Write R0~R5 data */
		for(int i=0; i<6; ++i){
			if(write_spi(data[i], sizeof(data[i])) < 0){
				printf("Write data[%i] to SPI failed. Error: %s\n", i, strerror(errno));
				return -1;
			}
		}
		printf("Send register data complete.\n");
		usleep(200000);			// Optional delay
	} while(option != 99);
	/****** End of user code block      *********/
	
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
   // mode |= SPI_MODE_2;

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

    int errmsg = write_gpio(0);

    int write_spi = write(spi_fd, write_buffer, size);

    if(write_spi < 0){
        printf("Failed to write to SPI. Error: %s\n", strerror(errno));
        return -1;
    }
	
	//set HIGH the GPIO 
	errmsg = write_gpio(16);
	errmsg = write_gpio(0);
	
	usleep(200);
	
    return 0;
}

/*  open the file and write into gpio  */
static int write_gpio(long number)
{
  int fd_gpio;
  //char *end;      //|| end == argv[1] 
  volatile void *cfg;
  //long number;

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

/*  Ask for user input  */
static int user_input(void){
	
	unsigned int usr_inp = 0;
	unsigned int show_freq[10] = {35, 80, 3, 4, 5, 6, 7, 8, 9};
	
	printf("Choose from the following option: (1~9)\n");
	for (int i = 0; i < 9; i++) {
		printf("\t%d. For %d MHz frequency\n", (i+1), show_freq[i]);
	}
	printf("For exit the program, Enter: 99.\n");
	
	printf("Enter input: ");
	scanf("%d", &usr_inp);
	
	return usr_inp;
}
