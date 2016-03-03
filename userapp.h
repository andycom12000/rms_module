#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

void timer_register()
{
	char buffer[100] = {'\0'};
	pid_t pid = getpid();
	sprintf(buffer, "echo %d > /proc/mp1/mp1", pid);
	system(buffer);
	return;
}
