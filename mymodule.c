#define MODULE
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/list.h>

//题目要求--遍历所有进程并输出以下内容 
//1.进程名字、id、状态、父进程名字
//2.统计进程个数
//3.分别统计处于下列状态状态进程个数： 
//TASK_RUNNING、TASK_INTERRUPTIBLE、TASK_UNINTERRUPTIBLE、TASK_ZOMBIE、TASK_STOPPED等等 

//提示： 
//1.进程名字：char comm[TASK_COMM_LEN];
//2.进程id：pit_t pid
//3.进程状态在两个变量中(state可运行性和exit_state退出状态)
//  state可能的值有TASK_RUNNING,TASK_INTERRUPTIBLE,TASK_UNINTERRUPTIBLE,
//  TASK_STOPPED,TASK_TRACED,TASK_NONINTERACTIVE
//  exit_state可能的值有EXIT_ZOMBIE,EXIT_DEAD
//4.父进程：struct task_struct *parent 
//5.如何遍历？ 用init_task得到0号进程 
//  for(p=&init_task;(p=next_task(p))!=&init_task;)

//注：printk中<1>代表打印信息的优先级，详情请百度 

//加载模块函数 
int init_module(void){
	int process_count=0;					//操作系统进程总数 
	int process_count_running=0;			//状态为TASK_RUNNING的进程总数 
	int process_count_interrupible=0;		//状态为TASK_INTERRUPTIBLE的进程总数 
	int process_count_uninterruptible=0;	//状态为TASK_UNINTERRUPTIBLE的进程总数
	int process_count_stopped=0;			//状态为TASK_STOPPED的进程总数
	int process_count_traced=0;				//状态为TASK_TRACED的进程总数
	int process_count_zombie=0;				//状态为EXIT_ZOMBIE的进程总数
	int process_count_dead=0;				//状态为EXIT_DEAD的进程总数
	int process_count_unknown=0;			//状态为未知的进程总数
	long process_state;						//进程的可运行性状态 
	long process_exit_state;				//进程的退出状态 
	struct task_struct *p=&init_task;		//遍历所有进程task_struct的指针 
	
	//开始标记 
	printk("<1>$#$MyProcessCounter begins\n");
	for(p=&init_task;(p=next_task(p))!=&init_task;){
		//输出进程名字和id 
		printk("@id: %d\n",p->pid);
		printk("@name: %s\n",p->comm);
		
		//输出父进程名字和id
		printk("@parent id: %d\n",p->parent->pid);
		printk("@parent name: %s\n",p->parent->comm); 
		
		//进程总数计数加一
		process_count++;
		
		//获取进程状态 
		process_state=p->state; 
		process_exit_state=p->exit_state;
		
		//判断进程的退出状态 
		switch(process_exit_state){
			case EXIT_ZOMBIE:
				process_count_zombie++;//状态为EXIT_ZOMBIE的进程总数计数加一 
				break;
			case EXIT_EXIT_DEAD:
				process_count_dead++;//状态为EXIT_DEAD的进程总数计数加一 
				break;
			default:
				break;
		}
		
		//若进程的状态为退出状态中的一种，则不可能为其它状态 
		if(process_exit_state){
			//输出进程状态
			printk("@state: %ld\n",process_exit_state); 
			continue;
		}
		
		//若进程的状态不为退出状态中的一种，则可能为其他状态
		switch(process_state){
			case TASK_RUNNING:
				process_count_running++;//状态为TASK_RUNNING的进程总数计数加一 
				break;
			case TASK_INTERRUPTIBLE:
				process_count_interruptible++;//状态为TASK_INTERRUPTIBLE的进程总数计数加一 
				break;
			case TASK_UNINTERRUPTIBLE:
				process_count_uninterruptible++;//状态为TASK_UNINTERRUPTIBLE的进程总数计数加一 
				break;
			case TASK_STOPPED:
				process_count_stopped++;//状态为TASK_STOPPED的进程总数计数加一 
				break;
			case TASK_TRACED:
				process_count_traced++;//状态为TASK_TRACED的进程总数计数加一 
				break;
			default:
				process_count_unknown++;//状态为未知的进程总数计数加一 
				break;
		}
		
		//输出进程状态
		printk("@state: %ld\n",process_state); 
	}
	
	//输出进程总数
	printk("@Total number=%d\n",process_count);
	//输出状态为TASK_RUNNING的进程总数
	printk("@TASK_RUNNING: %d\n",process_count_running);
	//输出状态为TASK_INTERRUPTIBLE的进程总数 
	printk("@TASK_INTERRUPTIBLE: %d\n",process_count_interruptible);
	//输出状态为TASK_UNINTERRUPTIBLE的进程总数 
	printk("@TASK_UNINTERRUPTIBLE: %d\n",process_count_uninterruptible);
	//输出状态为TASK_STOPPED的进程总数 
	printk("@TASK_STOPPED: %d\n",process_count_stopped);
	//输出状态为TASK_TRACED的进程总数 
	printk("@TASK_TRACED: %d\n",process_count_traced);
	//输出状态为EXIT_ZOMBIE的进程总数 
	printk("@EXIT_ZOMBIE: %d\n",process_count_zombie);
	//输出状态为EXIT_DEAD的进程总数 
	printk("@EXIT_DEAD: %d\n",process_count_dead);
	//输出状态为未知的进程总数
	printk("@UNKNOWN: %d\n",process_count_unknown);
	
	//结束标记 
	printk("<1>$*$MyProcessCounter ends\n");
	
	return 0;
} 
//卸载模块函数 
void cleanup_module(void){
	printk("<1>MyProcessCounter removed!\n");
}
MODULE_LICENSE("GPL");
