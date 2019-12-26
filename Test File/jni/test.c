#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sched.h>

#include <linux/sched.h>

#define SCHED_WRR 6

//Use system call 157 to get the current scheduler
//Use system call 156 to set scheduler and priority

int main(void)
{
	struct sched_param pp; //store priority

	int prev,policy,pid,prio,prio_min,prio_max,flag=1;

	while(flag){
	printf("Please input the Choice of Scheduling algorithms (0-NORMAL,1-FIFO,2-RR,6-WRR):");
		scanf("%d",&policy);
		printf("Current scheduling algorithm is ");
		switch(policy) {
		case 0:
			printf("SCHED_NORMAL\n");
			flag=0;
			break;
		case 1:
			printf("SCHED_FIFO\n");
			flag=0;
			break;
		case 2:
			printf("SCHED_RR\n");
			flag=0;
			break;
		case 6:
			printf("SCHED_WRR\n");
			flag=0;
			break;
		}
		if(flag)
			printf("Invalid input! Please input policy again!\n");
	}

	printf("Please input the id of the testprocess :");
	scanf("%d",&pid);

	prio_min=syscall(160,SCHED_WRR); //priority min
	prio_max=syscall(159,SCHED_WRR); //priority max
	printf("Set Process's priority (%d-%d) :",prio_min,prio_max);
	scanf("%d",&prio);

	printf("current scheduler's priority is : %d\n",prio);
	prev=syscall(157,pid); //get scheduler

	printf("pre scheduler:");

	switch(prev) {
		case 0:
			printf("SCHED_NORMAL\n");
			break;
		case 1:
			printf("SCHED_FIFO\n");
			break;
		case 2:
			printf("SCHED_RR\n");
			break;
		case 6:
			printf("SCHED_WRR\n");
			break;
	}


	pp.sched_priority=prio;

	prev=syscall(156,pid,policy,&pp); //set scheduler & priority
	printf("set scheduler result: %d\n",prev);

	prev=syscall(157,pid); //get scheduler
	printf("current scheduler: %d\n", prev);

	return 0;
}
