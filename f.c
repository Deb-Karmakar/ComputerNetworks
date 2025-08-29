#include<stdio.h>
#include<stdlib.h>
int main()
{
	char rbuff[128]="files",sbuff[128];
	system("clear");
	sprintf(sbuff,"ls|grep ^%s>f3",rbuff);
	puts(sbuff);
	system(sbuff);
	return 0;
}
