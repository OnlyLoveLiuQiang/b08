#include"only_strstr.h"
#include"stdlib.h"

char *only_strstr(const char *haystack, const char *needle)
{
	int i;
	int needle_len = 0;
	int haystack_len = 0;
	if(needle[0] == '\0') return (char *)haystack;
do0:
//	while((*haystack) != needle[0])
	while((*haystack) && ((*haystack) != needle[0]))
		haystack++;
	i = 0;
	while((*(haystack+i))){
		haystack_len++;
		i++;
	}
	i = 0;
	while((*(needle+i))){
		needle_len++;
		i++;
	}
	if(haystack_len < needle_len){
		return NULL;
	}
	if(only_memcpm(haystack,needle,needle_len) == 0){
		return (char *)haystack;
	}else{
		haystack +=1;
		haystack_len = 0;
		needle_len = 0;
	}
	goto do0;
}

long only_memcpm(const char *s1,const char *s2,int len)
{ 
	unsigned long int a0;
	unsigned long int b0;
	long int srcp1 = (long int)s1;
	long int srcp2 = (long int)s2;
	unsigned long int res;
	while(len != 0){
		a0 = ((char *)srcp1)[0];
		b0 = ((char *)srcp2)[0];
		srcp1 +=1;
		srcp2 +=1;
		res = a0 - b0;
		if(res != 0) return res;
		len -=1;
	}
	return 0;
}
