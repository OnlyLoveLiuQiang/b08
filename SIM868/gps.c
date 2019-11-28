#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"gps.h"
#include"checksum.h"
#include"usart.h"
#include"only_atoi.h"
#include"only_strstr.h"
#include"only_atof.h"
const unsigned char mon_table[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

void gps_buf_extract(char *RxBuffer3_p,char *GpsData_p,char *GPGGA_p,char *GPRMC_p)
{
	char *st = NULL;
	int i = 0;
	//提取一包完整的GPS数据
	st = NULL;
	st = only_strstr((const char *)RxBuffer3_p,(const char *)"NG");
	if(st != NULL){
		for(i=0;i<1024;i++){
			GpsData_p[i] = st[i];
		}	
	}
	//清除GPGGA数组
	for(i=0;i<128;i++){
		GPGGA_p[i] = '\0';
	}
	//提取GNGGA数据
	st = NULL;
	i = 0;
	st = only_strstr((const char *)RxBuffer3_p,(const char *)"NG");
	if(st != NULL){
		while('$' != st[i]){
			GPGGA_p[i] = st[i];
			i++;
		}
	}
	//清除GPRMC数组
	for(i=0;i<128;i++){
		GPRMC_p[i] = '\0';
	}
	//提取GNRMC数据
	st = NULL;
	i = 0;
	st = only_strstr((const char *)RxBuffer3_p,(const char *)"NR");
	if(st != NULL){
	while('$' != st[i]){
		GPRMC_p[i] = st[i];
			i++;
		}
	}
}

//提取GPS数据
int gps_extract(char *str,GPS_SINGLE_DATA *gps_single_data_p,int n,int flag)
{
	int i;
	char *strp = NULL;
	//find the data 
	for(i=0;i<n;i++){
		str = only_strstr((const char *)str,(const char *)",");
		if(0x00000000 == str){
			return 0;
		}
		str = str+1;
	}
	strp = str;
	if(',' == *strp){
		gps_single_data_p->dataflag = 0;
		return 1;
	}
	gps_single_data_p->dataflag = 1;
	//find the point and find the len 
	while(',' != (*str)){
		gps_single_data_p->len++;
		if('.' == (*str)){
			gps_single_data_p->point = gps_single_data_p->len;
		}
		str++;
	}
	//fill data 
	for(i=0;i<gps_single_data_p->len;i++){
		gps_single_data_p->data[i] = strp[i];
	}
	if(1 == flag){
		gps_single_data_p->flag = *(str+1);
	}
	return 0;
}

void gps_view(GPS_SINGLE_DATA *gps_single_data_p)
{
	u_p("data = %s\n",gps_single_data_p->data);
	u_p("len = %d\n",gps_single_data_p->len);
	u_p("point = %d\n",gps_single_data_p->point);
	u_p("dataflag = %d\n",gps_single_data_p->dataflag);
	u_p("flag = %c\n",gps_single_data_p->flag);
}

void gps_single_data_clean(GPS_SINGLE_DATA * gps_single_data_p)
{
	int i;
	for(i=0;i<16;i++){
		gps_single_data_p->data[i] = '\0';
	}
	gps_single_data_p->flag = '\0';
	gps_single_data_p->len = 0;
	gps_single_data_p->point = 0;
}

void data_init(unsigned char *data)
{
#if DEBUG 
	printf("----- data_init debug start -----\n");
#endif 
	short i = 72;
	data[0] = 0xAA;
	data[1] = 0x44;
	data[2] = 0x16;
	data[3] = (unsigned char)(0x00ff&i);
	data[4] = (unsigned char)(0x00ff&(i>>8));
#if DEBUG 
	printf("----- data_init debug end -----\n");
#endif 
}

void reg_fill(unsigned char *data,unsigned char *reg)
{
#if DEBUG 
	printf("----- reg_fill debug start-----\n");
#endif 
	int i;
	data = data + 5;
	for(i=0;i<13;i++){
		data[i] = reg[i];
	}
#if DEBUG 
	printf("reg = %s\n",data);
#endif
#if DEBUG 
	printf("----- reg_fill debug end -----\n");
#endif 
}

void cpn_fill(unsigned char *data,unsigned char *cpn)
{
#if DEBUG 
	printf("----- cpn_fill debug start -----\n");
#endif 
	int i;
	data = data+18;
	for(i=0;i<13;i++){
		data[i] = cpn[i];
	}
#if DEBUG
	printf("cpn = %s\n",data);
#endif
#if DEBUG 
	printf("----- cpn_fill debug end -----\n");
#endif 
}

void id_fill(unsigned char *data,unsigned char *id)
{
#if DEBUG 
	printf("----- id_fill debug start -----\n");
#endif 
	int i;
	data = data+31;
	for(i=0;i<13;i++){
		data[i] = id[i];
	}
#if DEBUG 
	printf("id = %s\n",data);
#endif
#if DEBUG 
	printf("----- id_fill debug end -----\n");
#endif 
}

int is_leap_time(int year)
{
	if(year%4==0){
		if(year%100==0){
			if(year%400==0) return 1;
			else return 0;
		}else return 1;
	}else return 0;
}

void time_fill(char *gps,unsigned char *data,UTC *time_p)
{
	int i = 0;
	char year_array[8] = " ";
	char month_array[8] = " ";
	char day_array[8] = " ";
	char hour_array[8] = " ";
	char minute_array[8] = " ";
	char second_array[8] = " ";
	int year = 0;
	int month = 0;
	int day = 0;
	int hour = 0;
	int minute = 0;
	float second = 0.0;
	unsigned long long nu = 0;
	unsigned long long num = 0;
	
	GPS_SINGLE_DATA gps_single_data;
	
	gps_single_data_clean(&gps_single_data);
	//year-month-day 
	gps_extract(gps,&gps_single_data,9,0);
	//find the day 
	for(i=0;i<2;i++){
		day_array[i] = gps_single_data.data[i];
	}
	//find the month 
	for(i=0;i<2;i++){
		month_array[i] = gps_single_data.data[i+2];
#if DEBUG 
		printf("month_array = %s\n",month_array);
#endif 
	}
	//find the year 
	for(i=0;i<2;i++){
		year_array[i] = gps_single_data.data[i+4];
#if DEBUG 
		printf("year_array = %s\n",year_array);
#endif
	}
	gps_single_data_clean(&gps_single_data);		
	//find hour-minute-second 
	gps_extract(gps,&gps_single_data,1,0);
	//find the hour 
	for(i=0;i<2;i++){
		hour_array[i] = gps_single_data.data[i];
#if DEBUG 
		printf("hour_array = %s\n",hour_array);
#endif 
	}
	//find the minute 
	for(i=0;i<2;i++){
		minute_array[i] = gps_single_data.data[i+2];
#if DEBUG 
		printf("minute_array = %s\n",minute_array);
#endif 
	}
	//find the second 
	for(i=0;i<(gps_single_data.len-4);i++){
		second_array[i] = gps_single_data.data[i+4];
#if DEBUG 
		printf("second_array = %s\n",second_array);
#endif 
	}	
	//process the data 
	time_p->year = only_atoi(year_array);
	time_p->year = time_p->year + 2000;
	time_p->month = only_atoi(month_array);
	time_p->day = only_atoi(day_array);
	time_p->hour = only_atoi(hour_array);
	time_p->hour = time_p->hour;
	time_p->minute = only_atoi(minute_array);
	time_p->second = atof(second_array);
#if 0
	u_p("%d - %d -%d %d:%d:%f\n",time_p->year,time_p->month,time_p->day,time_p->hour,time_p->minute,time_p->second);
#endif
	year = time_p->year;
	month = time_p->month;
	day = time_p->day;
	hour = time_p->hour;
	minute = time_p->minute;
	second = time_p->second;
#if UNIX_TIME

	for(i=1970;i<year;i++){
		if(is_leap_time(i))
			nu = nu + 31622400;
		else 
			nu = nu + 31536000;
	}
	month -=1;
	for(i=0;i<month;i++){
		nu += (long long)mon_table[i]*86400;
		if(is_leap_time(year)&&i==1)nu += 86400;
	}
	nu += (unsigned long long)(day-1)*86400;
	nu += (unsigned long long)hour*3600;
	nu += (unsigned long long)minute*60;
	num = (nu*1000) + (unsigned long long)second*1000;
	//08:00:00 <===> 28 800 000
	num = num - 28800000;

#endif 

#if DEBUG 
	printf("nu = %lld\n",nu);
	printf("num = %lld\n",num);
#endif 

#if UTC_TIME

#endif 

#if 0
	u_p("Unix_time = %lld\n",num);
#endif 

	data[56] = (unsigned char)((num>>0)&0xff);
	data[57] = (unsigned char)((num>>8)&0xff);
	data[58] = (unsigned char)((num>>16)&0xff);
	data[59] = (unsigned char)((num>>24)&0xff);
	data[60] = (unsigned char)((num>>32)&0xff);
	data[61] = (unsigned char)((num>>40)&0xff);
	data[62] = (unsigned char)((num>>48)&0xff);
	data[63] = (unsigned char)((num>>56)&0xff);
#if DEBUG 
	printf("----- time_fill debug end -----\n");
#endif 
}

void longitude_fill(unsigned char *data,GPS_SINGLE_DATA *gps_single_data_p)
{ 
	int i = 0;
	char degree_array[8] = " ";
	char minute_array[16] = " ";
	int degree = 0;
	float minute = 0.0;
	float nu = 0.0;
	int num = 0;
	if(6 == gps_single_data_p->point){
		//find degree 
		for(i=0;i<3;i++){
			degree_array[i] = gps_single_data_p->data[i];
		}
		//find minute 
		for(i=0;i<(gps_single_data_p->len-3);i++){
			minute_array[i] = gps_single_data_p->data[i+3];			
		}			
	}else if(5 == gps_single_data_p->point){
		//find degree                                                    
		for(i=0;i<2;i++){                                                
			degree_array[i] = gps_single_data_p->data[i];
		}		
		//find minute                                                    
        for(i=0;i<(gps_single_data_p->len-2);i++){                           
            minute_array[i] = gps_single_data_p->data[i+2];
		}		
	}else if(4 == gps_single_data_p->point){
		//find degree                                                  
		for(i=0;i<1;i++){                                            
			degree_array[i] = gps_single_data_p->data[i];
		}		
		//find minute                                                 
		for(i=0;i<(gps_single_data_p->len-1);i++){                          
			minute_array[i] = gps_single_data_p->data[i+1];
        }		
	}
#if 0
	u_p("degree_array = %s\n",degree_array);
	u_p("minute_array = %s\n",minute_array);
#endif 

	degree = only_atoi(degree_array);
	minute = atof(minute_array);		
	nu = degree + minute/60;
	num = (int)(nu*10000000);
	if('W' == gps_single_data_p->flag){
		num = -num;
	}	
	data[44] = (unsigned char)((num>>0)&0xff);
	data[45] = (unsigned char)((num>>8)&0xff);
	data[46] = (unsigned char)((num>>16)&0xff);
	data[47] = (unsigned char)((num>>24)&0xff);	
#if 0
	u_p("degree = %d\n",degree);
	u_p("longitude = %d\n",num);
#endif 
}

void latitude_fill(unsigned char *data,GPS_SINGLE_DATA *gps_single_data_p)
{
	int i = 0;
	char degree_array[8] = " ";
	char minute_array[16] = " ";
	int degree = 0;
	float minute = 0.0;
	float nu = 0.0;
	int num = 0;
	if(5 == gps_single_data_p->point){
		for(i=0;i<2;i++){
			degree_array[i] = gps_single_data_p->data[i];
		}
		for(i=0;i<(gps_single_data_p->len-2);i++){
			minute_array[i] = gps_single_data_p->data[i+2];
		}
	}else if(4 == gps_single_data_p->point){
		for(i=0;i<1;i++){
			degree_array[i] = gps_single_data_p->data[i];
		}
		for(i=0;i<(gps_single_data_p->len-2);i++){
			minute_array[i] = gps_single_data_p->data[i+1];
		}
	}	
#if 0
	u_p("degree_array = %s\n",degree_array);
	u_p("minute_array = %s\n",minute_array);
#endif
	degree = only_atoi(degree_array);
	minute = atof(minute_array);
	nu = degree + minute/60;
	num = (int)(nu*10000000);
	if('S' == gps_single_data_p->flag){
		num = -num;
	}
#if 0
	u_p("latitude = %d\n",num);
#endif
	data[48] = (unsigned char)((num>>0)&0xff);
	data[49] = (unsigned char)((num>>8)&0xff);
	data[50] = (unsigned char)((num>>16)&0xff);
	data[51] = (unsigned char)((num>>24)&0xff);
}

void elevation_fill(unsigned char *data,GPS_SINGLE_DATA *gps_single_data_p)
{
#if DEBUG 
	printf("----- elevation_fill debug start -----\n");
#endif 
	int num = 0;
	float elevation = 0.0;
	elevation = atof(gps_single_data_p->data);
	num = (int)(elevation*1000);
#if DEBUG 
	u_p("elevation = %d\n",num);
#endif
	data[52] = (unsigned char)((num>>0)&0xff);
	data[53] = (unsigned char)((num>>8)&0xff);
	data[54] = (unsigned char)((num>>16)&0xff);
	data[55] = (unsigned char)((num>>24)&0xff);
}

void speed_fill(unsigned char *data,GPS_SINGLE_DATA *gps_single_data_p)
{
	float speed = 0.0;
	float nu = 0.0;
	unsigned short num = 0;
	speed = atof(gps_single_data_p->data);
	nu = speed*((1.85*10000)/(60*60));
	num = (unsigned char)nu;
#if 0 
	u_p("speed = %d\n",num);
#endif
	data[64] = (unsigned char)((num>>0)&0xff);
	data[65] = (unsigned char)((num>>8)&0xff);
#if DEBUG 
	printf("----- elevation_fill debug end -----\n");
#endif 
}

void direction_fill(unsigned char *data,GPS_SINGLE_DATA *gps_single_data_p)
{
#if DEBUG 
	printf("----- direction_fill debug start -----\n");
#endif 
	float direction = 0.0;
//	float nu = 0.0;
	unsigned short num = 0;
	direction = atof(gps_single_data_p->data);
	num = (unsigned short)direction;
#if 0
	u_p("direction = %d\n",num);
#endif
	data[66] = (unsigned char)((num>>0)&0xff);
	data[67] = (unsigned char)((num>>8)&0xff);
#if DEBUG 
	printf("----- direction_fill debug end -----\n");
#endif 
}

void other_fill(unsigned char *data,GPS_SINGLE_DATA *gps_single_data_p)
{
#if DEBUG 
	printf("----- other_fill debug start -----\n");
#endif 
	data[71] = 5;
#if DEBUG 
	printf("----- other_fill debug end -----\n");
#endif 
}

void data_parsing(unsigned char *data,GPS_DATA *gps_data_p)
{
#if DEBUG 
	printf("----- data_parsing debug start -----\n");
#endif 
	int i = 0;
	//head 
	for(i=0;i<3;i++){
		gps_data_p->head[i] = data[i];
	}
	//len 
	gps_data_p->len = gps_data_p->len & 0x0000;	
	gps_data_p->len = (gps_data_p->len<<0) | data[4];
	gps_data_p->len = (gps_data_p->len<<8) | data[3];
#if DEBUG 
	printf("len = %d\n",gps_data_p->len);
#endif
	//reg 
	for(i=0;i<13;i++){
		gps_data_p->reg[i] = data[i+5];
	}
#if DEBUG 
	printf("reg = %s\n",gps_data_p->reg);
#endif 
	//cpn 
	for(i=0;i<13;i++){
		gps_data_p->cpn[i] = data[i+18];
	}
#if DEBUG 
	printf("cpn = %s\n",gps_data_p->cpn);
#endif 
	//id 
	for(i=0;i<13;i++){
		gps_data_p->id[i] = data[i+31];
	}
#if DEBUG 
	printf("id = %s\n",gps_data_p->id);
#endif 
	//longitude 
	gps_data_p->longitude = gps_data_p->longitude&0x00000000;
	gps_data_p->longitude = (gps_data_p->longitude<<0) | data[47];
	gps_data_p->longitude = (gps_data_p->longitude<<8) | data[46];
	gps_data_p->longitude = (gps_data_p->longitude<<8) | data[45];
	gps_data_p->longitude = (gps_data_p->longitude<<8) | data[44];
#if DEBUG 
	printf("longitude = %d\n",gps_data_p->longitude);
#endif
	//latitude  
	gps_data_p->latitude = gps_data_p->latitude&0x00000000;
	gps_data_p->latitude = (gps_data_p->latitude<<0) | data[51];
	gps_data_p->latitude = (gps_data_p->latitude<<8) | data[50];
	gps_data_p->latitude = (gps_data_p->latitude<<8) | data[49];
	gps_data_p->latitude = (gps_data_p->latitude<<8) | data[48];
#if DEBUG 
	printf("latitude = %d\n",gps_data_p->latitude);
#endif 
	//elevation 
	gps_data_p->elevation = gps_data_p->elevation&0x00000000;
	gps_data_p->elevation = (gps_data_p->elevation<<0) | data[55];
	gps_data_p->elevation = (gps_data_p->elevation<<8) | data[54];
	gps_data_p->elevation = (gps_data_p->elevation<<8) | data[53];
	gps_data_p->elevation = (gps_data_p->elevation<<8) | data[52];
#if DEBUG 
	printf("elevation = %d\n",gps_data_p->elevation);
#endif
	//time 
	gps_data_p->time = gps_data_p->time &0x0000000000000000;
	gps_data_p->time = (gps_data_p->time<<0) | data[63];
	gps_data_p->time = (gps_data_p->time<<8) | data[62];
	gps_data_p->time = (gps_data_p->time<<8) | data[61];
	gps_data_p->time = (gps_data_p->time<<8) | data[60];
	gps_data_p->time = (gps_data_p->time<<8) | data[59];
	gps_data_p->time = (gps_data_p->time<<8) | data[58];
	gps_data_p->time = (gps_data_p->time<<8) | data[57];
	gps_data_p->time = (gps_data_p->time<<8) | data[56];
#if DEBUG 
	printf("time = %lld\n",gps_data_p->time);
#endif 
	//speed 
	gps_data_p->speed = gps_data_p->speed & 0x0000;
	gps_data_p->speed = (gps_data_p->speed<<0) | data[65];
	gps_data_p->speed = (gps_data_p->speed<<8) | data[64];
#if DEBUG 
	printf("speed = %hd\n",gps_data_p->speed);
#endif 
	//direction 
	gps_data_p->direction = gps_data_p->direction & 0x0000;
	gps_data_p->direction = (gps_data_p->direction<<0) | data[67];
	gps_data_p->direction = (gps_data_p->direction<<8) | data[66];
#if DEBUG 
	printf("direction = %hd\n",gps_data_p->direction);
#endif 

	//other 
	gps_data_p->other = gps_data_p->other & 0x00;
	gps_data_p->other = gps_data_p->other | data[71];
#if DEBUG 
	printf("----- data_parsing debug end -----\n");
#endif 
}

void data_encryption(unsigned char *data,unsigned char encryption,int begin,int end)
{
	int i = begin;
	int j = end;
	for(;i<j;i++){
		data[i] = data[i]^encryption;
	}
}

void crc_fill(unsigned char *data,int len)
{
	unsigned short crc;
	crc = crc_16((const unsigned char *)data,len);
	data[74] = (unsigned char)((crc>>0)&0xff);
	data[75] = (unsigned char)((crc>>8)&0xff);
}

