#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define TARGET_TEMPERATURE 52

struct _pid{
    float targettemp;       //target
    float curtemp;          // current temp
    float err;              // offset between target temp and current temp
    float err_last;        
    float kp,ki,kd;         
    float speed;            // control variable
    float integral;
}pid;

void PID_init(){
     pid.targettemp = 0.0;
     pid.curtemp = 0.0;
     pid.err = 0.0;
     pid.err_last = 0.0;
     pid.speed = 0.0;
     pid.integral = 0.0;
     pid.kp = 0.2;
     pid.ki = 0.015;
     pid.kd = 0.2;
 }


//void PID_init(char** temp_array);
float PID_process(float target_temp, char** temp_array, char* Busid);
char* getBusId(char *c);
char* dec2hex(char* hex,int dec);

int main(){

    char* temperature_array[] = {
 			 "/sys/class/hwmon/hwmon0/temp1_input",
                         "/sys/class/hwmon/hwmon0/temp2_input",
                         "/sys/class/hwmon/hwmon0/temp3_input",
                         "/sys/class/hwmon/hwmon0/temp4_input",
                         "/sys/class/hwmon/hwmon0/temp5_input",
                         "/sys/class/hwmon/hwmon0/temp6_input",
                         "/sys/class/hwmon/hwmon0/temp7_input",
                         "/sys/class/hwmon/hwmon0/temp8_input",
                         "/sys/class/hwmoin/hwmon0/temp9_input",
                         "i/sys/class/hwmon/hwmon0/temp10_nput",
                         "/sys/class/hwmon/hwmon0/temp11_input"};

     char Busid[10], cmd_wakeup_fan[100];
     FILE *fp, *outfp;

     if(getBusId(Busid)){
	printf("Found the i2c adapter\n");
     }
     else{
	printf("Error: Can not foun i2c adapter\n");
	return 0;
     }
     
     snprintf(cmd_wakeup_fan,sizeof(cmd_wakeup_fan),"i2cset -y %s 0x70 0x4", Busid);
     printf("Ini_cmd = %s\n", cmd_wakeup_fan);
     fp = popen(cmd_wakeup_fan,"r");
     fclose(fp);

     PID_init();
     //outfp = fopen("out.txt","w");
     //int i = 1000;
     while(1)
     {
         float temp = PID_process(TARGET_TEMPERATURE, temperature_array, Busid);
	 //fprintf(outfp,"%f\n",speed);
         printf("Temperature = %f\n",temp);
         //i = i - 1;
     }
 return 0;
 }



float get_max_temp(char** t_array){
        double value;
        int TEMP_IDX_MAX = 11;
        FILE *f;

        double max_temp = 0;
        for(int i = 0; i < TEMP_IDX_MAX; i++){
                if((f = fopen(t_array[i], "r"))){
                        int temp = 0;
                        temp = fscanf(f, "%lf", &value);
                        if( max_temp < value ){
                                max_temp = value;
                        }
                        temp = fclose(f);
                }
        }
        return max_temp;
}

char* dec2hex(char* hex,int dec) {
        sprintf(hex, "%x", dec);
        //printf("hex = %s",hex);
        return hex;
}

float PID_process(float temp, char** t_array, char* id){
     FILE *fp1,*fp2,*fp3,*fp4,*fp5;
     char cmd[100],hex[100],fan1[100],fan2[100],fan3[100],fan4[100],fan5[100];
     int speed;

     pid.targettemp = temp;
     pid.err = pid.targettemp - pid.curtemp;
     pid.integral += pid.err;
     pid.speed = pid.kp*pid.err + pid.ki*pid.integral + pid.kd*(pid.err - pid.err_last);
     pid.speed = -1 * pid.speed;
     if(pid.speed <= 1) pid.speed = 16.0;
     if(pid.speed > 112) pid.speed = 112.0;
     pid.err_last = pid.err;
     speed = (int)pid.speed;
     printf("Speed = %d ",speed);
     /**********generate command**********/
     dec2hex(hex,speed);
     snprintf(fan1,sizeof(fan1),"i2cset -y %s 0x66 0x00 0x%s",id, hex);
     snprintf(fan2,sizeof(fan2),"i2cset -y %s 0x66 0x01 0x%s",id, hex);
     snprintf(fan3,sizeof(fan3),"i2cset -y %s 0x66 0x02 0x%s",id, hex);
     snprintf(fan4,sizeof(fan4),"i2cset -y %s 0x66 0x03 0x%s",id, hex);
     snprintf(fan5,sizeof(fan5),"i2cset -y %s 0x66 0x04 0x%s",id, hex);	
     /**********excute cmd**********/
     fp1 = popen(fan1, "r");
     fclose(fp1);
     fp2 = popen(fan2, "r");
     fclose(fp2);
     fp3 = popen(fan3, "r");
     fclose(fp3);
     fp4 = popen(fan4, "r");
     fclose(fp4);
     fp5 = popen(fan5, "r");
     fclose(fp5);
     sleep(1);
     pid.curtemp = (float)get_max_temp(t_array)/1000;
     //printf("TEMPUTRE = %d",(int)pid.curtemp);
     return (float)pid.curtemp;
}

char* getBusId(char *c){
        char id[2];
        FILE *fp;
        fp = popen("i2cdetect -l | grep \"I801\" | awk '{print $1}' | cut -d '-' -f 2", "r");
        fgets(id,sizeof(id),fp);
        strcpy(c,id);
        fclose(fp);
        return c;
}


