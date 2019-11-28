#include"only_atof.h"
#include"stdlib.h"

double only_atof(const char *str)
{
	int i = 0;
	double k = 1.0;
	int flag = 0;
	double ret = 0.0;
	if(NULL == str){
		return 0.0;
	}
	while(*(str+i)){
		if('.' == *(str+i)){
			flag = 1;
			i++;
		}
		if(0 == flag){
			ret = ret*10 + ((*(str+i))-'0');
			i++;
		}else{
			k = k*10.0;
			ret = ret + ((double)((*(str+i))-'0'))/k;
			i++;
		}
	}
	return ret;
}

