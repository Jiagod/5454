#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define TARGET_TEMPERATURE 87

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
     pid.kp = 20;//default 0.2
     pid.ki = 0.8;//default 0.015
     pid.kd = 5;//default 0.2
 }


//void PID_init(char** temp_array);
float PID_process(float target_temp, /*char** temp_array,*/ char* Busid);
char* getBusId(char *c);
char* dec2hex(char* hex,int dec);

int main(){

     char Busid[10], cmd_wakeup_fan[100];
     FILE *fp, *outfp;

     if(getBusId(Busid)){
	printf("Found the i2c adapter\n");
     }
     else{
	printf("Error: Can not found i2c adapter\n");
	return 0;
     }
     
     snprintf(cmd_wakeup_fan,sizeof(cmd_wakeup_fan),"i2cset -y %s 0x70 0x20", Busid);
     printf("Ini_cmd = %s\n", cmd_wakeup_fan);
     fp = popen(cmd_wakeup_fan,"r");
     fclose(fp);

     PID_init();

     while(1)
     {
         float temp = PID_process(TARGET_TEMPERATURE, /*temperature_array,*/ Busid);
	 //fprintf(outfp,"%f\n",speed);
         //printf("Temperature = %f\n",temp);
         //i = i - 1;
     }
 return 0;
 }

float get_max_temp(void){
        float max = 0;
        int i;
        char** temp_array[6];
        char buffer[80];
        float f;
        for(i=0;i < 6;i++){
                temp_array[i] = (char *)malloc(sizeof(char)*100);
        }
        strcpy(temp_array[0],"FPGA");
        strcpy(temp_array[1],"Board[[:space:]]Temperature");
        strcpy(temp_array[2],"PKVL0[[:space:]]Core");
        strcpy(temp_array[3],"PKVL0[[:space:]]SerDes");
        strcpy(temp_array[4],"PKVL1[[:space:]]Core");
        strcpy(temp_array[5],"PKVL1[[:space:]]SerDes");
        i = 0;
        for(i;i<6;i++){
                FILE *fp1;
                char get_temp[100];
		snprintf(get_temp,sizeof(get_temp),"fpgainfo temp | grep %s | cut -d ':' -f 2 | cut -d ' ' -f 2", temp_array[i]);
                fp1 = popen(get_temp,"r");
                fgets(buffer, sizeof(buffer), fp1);
                f = atof(buffer);
                if(f > max) max = f;
                //printf("%f\t",f);
                pclose(fp1);

        }
        for(i=0;i < 6; i++){
                free(temp_array[i]);
        }
        return max;
}

char* dec2hex(char* hex,int dec) {
        sprintf(hex, "%x", dec);
        //printf("hex = %s",hex);
        return hex;
}

float PID_process(float temp, /*char** t_array*/ char* id){
     FILE *fp1,*fp2,*fp3,*fp4,*fp5;
     char cmd[100],hex[100],fan1[100],fan2[100],fan3[100],fan4[100],fan5[100];
     int speed;

     pid.targettemp = temp;
     pid.err = pid.targettemp - pid.curtemp;
     pid.integral += pid.err;
     pid.speed = pid.kp*pid.err + pid.ki*pid.integral + pid.kd*(pid.err - pid.err_last);
     pid.speed = -1 * pid.speed;//defa -1
     //printf("%f\n",pid.speed);
     if(pid.speed <= 128) pid.speed = 128;
     if(pid.speed > 255) pid.speed = 255.0;
     pid.err_last = pid.err;
     speed = (int)pid.speed;
     //printf("Speed = %d ",speed);
     /**********generate command**********/
     dec2hex(hex,speed);
     snprintf(fan4,sizeof(fan4),"i2cset -y %s 0x66 0x04 0x%s",id, hex);
     snprintf(fan5,sizeof(fan5),"i2cset -y %s 0x66 0x05 0x%s",id, hex);	
     /**********excute cmd**********/
     fp4 = popen(fan4, "r");
     fclose(fp4);
     fp5 = popen(fan5, "r");
     fclose(fp5);
     sleep(1);
     pid.curtemp = (float)get_max_temp();
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


