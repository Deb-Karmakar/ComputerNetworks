#include<stdio.h>
#include<stdlib.h>
int main()
{
	char rbuff[128]="f2",sbuff[128];
	system("clear");
	system("ls -l>f3");
	sprintf(sbuff,"ls|grep ^%s>f3",rbuff);
	puts(sbuff);
	system(sbuff);
	return 0;
}
