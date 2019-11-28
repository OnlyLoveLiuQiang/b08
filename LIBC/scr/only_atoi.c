#include "only_atoi.h"
#include"stdlib.h"

int only_atoi(const char *str)
{
	int ret = 0;
	unsigned char i = 0;
	char symbolFlag = 0;
	if(NULL == str){
		return 0;
	}
	if('-' == (*str)){
		symbolFlag = 1;
		i++;
	}
	while(*(str+i)){
		ret = ret*10 + ((*(str+i))-'0');
		i++;
	}
	if(1 == symbolFlag){
		ret = -ret;
	}
	return ret;
}
