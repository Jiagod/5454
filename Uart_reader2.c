#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h> 
#include <errno.h> 
#include <termios.h> 
#include <unistd.h>
int fd;
typedef struct gps_info{
	//char utc_time[80];
	float utc_time;
	float latitude_val;
	char lat_direction;
	float longtitude_val;
	char long_direction;
	int gps_qual;
	int sat_num;
	float hdop;
	float alt;
	char alt_val;
	float geosev;
	char geo_val;
	char checknum[20];	
}GPS_INFO;
void uart_init()
{
 	//printf("\n +----------------------------------+");
 	//printf("\n |        Serial Port Write         |");
 	//printf("\n +----------------------------------+");

/*------------------------------- Opening the Serial Port -------------------------------*/
  	fd = open("/dev/ttyS1",O_RDWR | O_NOCTTY| O_SYNC);      /* !!blocks the read  */
                                
 	if(fd == -1) printf("\n  Error! in Opening ttyUSB0  ");
 	else	printf("\n ttyS1 Opened Successfully ");


 /*---------- Setting the Attributes of the serial port using termios structure --------- */
    
	struct termios SerialPortSettings;/* Create the structure                          */
	tcgetattr(fd, &SerialPortSettings);         /* Get the current attributes of the Serial port */

    	cfsetispeed(&SerialPortSettings,B9600);        /* Set Read  Speed as 19200                       */
    	cfsetospeed(&SerialPortSettings,B9600);        /* Set Write Speed as 19200                       */
    	cfmakeraw(&SerialPortSettings);
    	tcsetattr(fd, TCSANOW, &SerialPortSettings);
    	SerialPortSettings.c_cc[VMIN] = 10; /* Read at least 10 characters */
    	SerialPortSettings.c_cc[VTIME] = 0; /* Wait indefinetly   */

	if((tcsetattr(fd,TCSANOW,&SerialPortSettings)) != 0)
 		printf("\n  ERROR ! in Setting attributes");
	else
 		printf("\n BaudRate = 9600 \n");
}
void uart_receive()
{
 	char read_buffer[150];   /* Buffer to store the data received              */

 	int  bytes_read = 0;    /* Number of bytes read by the read() system call */
 	int i = 0;

 	bytes_read = read(fd, read_buffer,sizeof(read_buffer)); /* Read the data                   */
	//read_buffer[bytes_read] = '\0';
	int j=0;
	//int i = 0;
	char* raw_info = NULL;
	if(( raw_info = strstr(read_buffer,"$GPGGA")) != NULL){
		for(i=0; i<strlen(raw_info);i++){
			if(raw_info[i] == '\n'){
				raw_info[i] = '\0';
			}
		}
	}
	GPS_INFO *gga_info = malloc(sizeof(GPS_INFO));
	if(raw_info != NULL){
		sscanf(raw_info,"$GPGGA,%f,%f,%c,%f,%c,%d,%d,%f,%f,%c,%f,%c,,%s",&(gga_info->utc_time),&(gga_info->latitude_val),&(gga_info->lat_direction),&(gga_info->longtitude_val),&(gga_info->long_direction),&(gga_info->gps_qual),&(gga_info->sat_num),&(gga_info->hdop),&(gga_info->alt),&(gga_info->alt_val),&(gga_info->geosev),&(gga_info->geo_val),gga_info->checknum);
	printf("\n");
	/*float utc_time_hr = gga_info->utc_time / 1000;
	float res_time = gga_info->utc_time % 1000;
	int gmt_time_hr = ((int)utc_time_hr + 8) % 24;
	float gmt_time = (float)gmt_time_hr * 1000 + res_time;*/
	printf(" Time = %f\n Latitude = %f\n Lattitude direction = %c\n Longtitude = %f\n Longtitude direction = %c\n GPS qual = %d\n Satellite number = %d\n Hdop = %f\n Alt = %f\n Alt value = %c\n Geosev = %f\n Geosev value = %c\n Check number = %s\n", gga_info->utc_time, gga_info->latitude_val,gga_info->lat_direction,gga_info->longtitude_val,gga_info->long_direction,gga_info->gps_qual,gga_info->sat_num,gga_info->hdop,gga_info->alt,gga_info->alt_val,gga_info->geosev,gga_info->geo_val,gga_info->checknum);
	}
	fflush(stdout);
	free(gga_info);

 //printf("\n +----------------------------------+\n\n\n");
}
void main(void)
{
  int fd; 
  uart_init();
  /*------------------------------- Write data to serial port -----------------------------*/
  //uart_write_commande(write_buffer); //Write function works well
  while(1){
  	uart_receive();
  }
  //close(fd);/* Close the Serial port */
}


