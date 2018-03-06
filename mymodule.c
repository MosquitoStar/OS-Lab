#define MODULE
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/list.h>

//��ĿҪ��--�������н��̲������������ 
//1.�������֡�id��״̬������������
//2.ͳ�ƽ��̸���
//3.�ֱ�ͳ�ƴ�������״̬״̬���̸����� 
//TASK_RUNNING��TASK_INTERRUPTIBLE��TASK_UNINTERRUPTIBLE��TASK_ZOMBIE��TASK_STOPPED�ȵ� 

//��ʾ�� 
//1.�������֣�char comm[TASK_COMM_LEN];
//2.����id��pit_t pid
//3.����״̬������������(state�������Ժ�exit_state�˳�״̬)
//  state���ܵ�ֵ��TASK_RUNNING,TASK_INTERRUPTIBLE,TASK_UNINTERRUPTIBLE,
//  TASK_STOPPED,TASK_TRACED,TASK_NONINTERACTIVE
//  exit_state���ܵ�ֵ��EXIT_ZOMBIE,EXIT_DEAD
//4.�����̣�struct task_struct *parent 
//5.��α����� ��init_task�õ�0�Ž��� 
//  for(p=&init_task;(p=next_task(p))!=&init_task;)

//ע��printk��<1>�����ӡ��Ϣ�����ȼ���������ٶ� 

//����ģ�麯�� 
int init_module(void){
	int process_count=0;					//����ϵͳ�������� 
	int process_count_running=0;			//״̬ΪTASK_RUNNING�Ľ������� 
	int process_count_interrupible=0;		//״̬ΪTASK_INTERRUPTIBLE�Ľ������� 
	int process_count_uninterruptible=0;	//״̬ΪTASK_UNINTERRUPTIBLE�Ľ�������
	int process_count_stopped=0;			//״̬ΪTASK_STOPPED�Ľ�������
	int process_count_traced=0;				//״̬ΪTASK_TRACED�Ľ�������
	int process_count_zombie=0;				//״̬ΪEXIT_ZOMBIE�Ľ�������
	int process_count_dead=0;				//״̬ΪEXIT_DEAD�Ľ�������
	int process_count_unknown=0;			//״̬Ϊδ֪�Ľ�������
	long process_state;						//���̵Ŀ�������״̬ 
	long process_exit_state;				//���̵��˳�״̬ 
	struct task_struct *p=&init_task;		//�������н���task_struct��ָ�� 
	
	//��ʼ��� 
	printk("<1>$#$MyProcessCounter begins\n");
	for(p=&init_task;(p=next_task(p))!=&init_task;){
		//����������ֺ�id 
		printk("@id: %d\n",p->pid);
		printk("@name: %s\n",p->comm);
		
		//������������ֺ�id
		printk("@parent id: %d\n",p->parent->pid);
		printk("@parent name: %s\n",p->parent->comm); 
		
		//��������������һ
		process_count++;
		
		//��ȡ����״̬ 
		process_state=p->state; 
		process_exit_state=p->exit_state;
		
		//�жϽ��̵��˳�״̬ 
		switch(process_exit_state){
			case EXIT_ZOMBIE:
				process_count_zombie++;//״̬ΪEXIT_ZOMBIE�Ľ�������������һ 
				break;
			case EXIT_EXIT_DEAD:
				process_count_dead++;//״̬ΪEXIT_DEAD�Ľ�������������һ 
				break;
			default:
				break;
		}
		
		//�����̵�״̬Ϊ�˳�״̬�е�һ�֣��򲻿���Ϊ����״̬ 
		if(process_exit_state){
			//�������״̬
			printk("@state: %ld\n",process_exit_state); 
			continue;
		}
		
		//�����̵�״̬��Ϊ�˳�״̬�е�һ�֣������Ϊ����״̬
		switch(process_state){
			case TASK_RUNNING:
				process_count_running++;//״̬ΪTASK_RUNNING�Ľ�������������һ 
				break;
			case TASK_INTERRUPTIBLE:
				process_count_interruptible++;//״̬ΪTASK_INTERRUPTIBLE�Ľ�������������һ 
				break;
			case TASK_UNINTERRUPTIBLE:
				process_count_uninterruptible++;//״̬ΪTASK_UNINTERRUPTIBLE�Ľ�������������һ 
				break;
			case TASK_STOPPED:
				process_count_stopped++;//״̬ΪTASK_STOPPED�Ľ�������������һ 
				break;
			case TASK_TRACED:
				process_count_traced++;//״̬ΪTASK_TRACED�Ľ�������������һ 
				break;
			default:
				process_count_unknown++;//״̬Ϊδ֪�Ľ�������������һ 
				break;
		}
		
		//�������״̬
		printk("@state: %ld\n",process_state); 
	}
	
	//�����������
	printk("@Total number=%d\n",process_count);
	//���״̬ΪTASK_RUNNING�Ľ�������
	printk("@TASK_RUNNING: %d\n",process_count_running);
	//���״̬ΪTASK_INTERRUPTIBLE�Ľ������� 
	printk("@TASK_INTERRUPTIBLE: %d\n",process_count_interruptible);
	//���״̬ΪTASK_UNINTERRUPTIBLE�Ľ������� 
	printk("@TASK_UNINTERRUPTIBLE: %d\n",process_count_uninterruptible);
	//���״̬ΪTASK_STOPPED�Ľ������� 
	printk("@TASK_STOPPED: %d\n",process_count_stopped);
	//���״̬ΪTASK_TRACED�Ľ������� 
	printk("@TASK_TRACED: %d\n",process_count_traced);
	//���״̬ΪEXIT_ZOMBIE�Ľ������� 
	printk("@EXIT_ZOMBIE: %d\n",process_count_zombie);
	//���״̬ΪEXIT_DEAD�Ľ������� 
	printk("@EXIT_DEAD: %d\n",process_count_dead);
	//���״̬Ϊδ֪�Ľ�������
	printk("@UNKNOWN: %d\n",process_count_unknown);
	
	//������� 
	printk("<1>$*$MyProcessCounter ends\n");
	
	return 0;
} 
//ж��ģ�麯�� 
void cleanup_module(void){
	printk("<1>MyProcessCounter removed!\n");
}
MODULE_LICENSE("GPL");
