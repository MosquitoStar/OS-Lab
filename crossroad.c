#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <windows.h>

//宏定义 车辆方向 从北开始按逆时针编号（0~3） 
#define north 0			//北 
#define west 1			//西 
#define south 2			//南 
#define east 3			//东 
#define MAXCARNUM 100	//最大的车辆（线程）数量 

//保存车辆信息的数据结构 
typedef struct carInfo * Info;
struct carInfo{
	int direction;//车辆来自的方向 
	int number;//车辆的编号 
};

//队列Queue数据结构与操作
typedef struct queue *Queue;
struct queue{
	int capacity;//队列最大容量 
	int size;//队列现存元素数量 
	int front;//队列头索引 
	int rear;//队列尾索引 
	int *element;//数组方式实现队列 
};
Queue createQueue(int maxElements);//创建一个最大容量为maxElements的队列 
void clearQueue(Queue q);//清空队列 释放内存 
void enqueue(int x,Queue q);//将元素x放入队列末尾 
int dequeue(Queue q);//弹出队列开头的元素 
int front(Queue q);//获取队列开头的元素，但不弹出

pthread_cond_t queue[4];	//各个方向车辆队列条件变量
pthread_cond_t first[4];	//各个方向下次通行车辆的条件变量
pthread_mutex_t dirMut[4];	//访问各个方向的条件变量的互斥量 
Queue waitQ[4];				//各个方向等待过马路的队列 
pthread_mutex_t waitQMut[4];//访问各个方向等待队列的互斥量
pthread_mutex_t cross;		//访问公有资源a、b、c、d的互斥量 
pthread_cond_t deadLock;	//死锁检测线程唤醒的条件变量
pthread_mutex_t deadLockMut;//访问死锁检测线程唤醒条件变量的互斥量 
pthread_t deadLockCheck;	//死锁检测线程
int flag[4];		//本方向前面是否有车辆（有车辆为1，否则为0） 
int waiting[4];		//各个方向的车是否在等待（无等待为0，否则为1） 

//每个车辆线程的函数 
void* carFrom(void* info){
	Info i=(Info)info;	//获取车辆信息 
	int myDir=i->direction;	//设置来自方向 
	int myNo=i->number;	//设置车辆（线程）编号 
	char* dirS;	//根据车辆信息设置方向字符串 
	if(myDir==north)
		dirS="North";	//若为北边来车 则设为North 
	else if(myDir==south)
		dirS="South";	//若为南边来车 则设为South
	else if(myDir==east)
		dirS="East";	//若为东边来车 则设为East 
	else if(myDir==west)
		dirS="West";	//若为西边来车 则设为West 
	pthread_mutex_lock(&waitQMut[myDir]); 
	enqueue(myNo,waitQ[myDir]);	//将本车辆加入等待队列中 
	pthread_mutex_unlock(&waitQMut[myDir]);
	while(front(waitQ[myDir])!=myNo);//如果本车辆不在等待队列最前方则等待 
	printf("Car %d from %s arrives at crossing.\n",myNo,dirS);
	pthread_mutex_lock(&dirMut[myDir]);
	while(flag[myDir]){//等待本方向正在过马路的车辆发信号 
		pthread_cond_wait(&queue[myDir],&dirMut[myDir]);
	}
	flag[myDir]=1;//设置本方向车辆正在过马路条件 
	if(waitQ[(myDir+1)%4]->size!=0){//如果右边有车辆也要过马路 
		waiting[myDir]=1;//设置本方向车辆正在等待过马路条件 
		if(waiting[0]&&waiting[1]&&waiting[2]&&waiting[3]){
			printf("DEADLOCK: car jam detected, signalling North to go.\n");
			pthread_mutex_lock(&deadLockMut);
			pthread_cond_signal(&deadLock);//如果四个方向都在等待，则触发死锁信号 
			pthread_mutex_unlock(&deadLockMut);
		}
		pthread_cond_wait(&first[myDir],&dirMut[myDir]);//等待右边车辆发信号给自己 
	}
	waiting[myDir]=0;//取消本方向车辆正在等待过马路条件
	pthread_mutex_lock(&cross);
	printf("Car %d from %s leaving crossing.\n",myNo,dirS);
	Sleep(100);//过马路时间1s (Windows)
//	sleep(1000);//过马路时间1s (Linux)
	pthread_mutex_unlock(&cross);
	pthread_mutex_lock(&dirMut[(myDir+3)%4]);
	pthread_cond_signal(&first[(myDir+3)%4]);//发出信号给左边的车辆通行 
	pthread_mutex_unlock(&dirMut[(myDir+3)%4]);
	pthread_mutex_lock(&waitQMut[myDir]);
	dequeue(waitQ[myDir]);	//本车辆从等待队列中退出 
	pthread_cond_signal(&queue[myDir]);	//发出信号给本方向下一辆车 
	flag[myDir]=0;	//取消本方向车辆正在过马路条件
	pthread_mutex_unlock(&waitQMut[myDir]);
	pthread_mutex_unlock(&dirMut[myDir]);
}
//死锁检测线程函数 
void* checkDeadLock(){
	while(1){
		pthread_mutex_lock(&cross);
		pthread_cond_wait(&deadLock,&cross);//等待死锁的时候被唤醒 
		//printf("deadlockcheck wake up and signal north.\n");
		pthread_mutex_lock(&dirMut[north]);//获取访问北边条件变量的锁 
		pthread_cond_signal(&first[north]);//发出信号让北边车辆先行 
		pthread_mutex_unlock(&dirMut[north]);//释放访问北边条件变量的锁
		pthread_mutex_unlock(&cross);
	}
}

int main(){
	//初始化所有条件变量和互斥量 创建四个队列 
	int no=1,i;
	char directions[MAXCARNUM];//读取车辆方向的字符串 
	pthread_t car[MAXCARNUM];//车辆线程 
	for(i=0;i<4;i++){
		pthread_cond_init(&queue[i],NULL);//初始化各个方向车辆队列条件变量 
		pthread_cond_init(&first[i],NULL);//初始化各个方向下次通行车辆的条件变量
		pthread_mutex_init(&dirMut[i],NULL);//初始化访问各个方向的条件变量的互斥量
		pthread_mutex_init(&waitQMut[i],NULL);//初始化访问各个方向等待队列的互斥量
		waitQ[i]=createQueue(MAXCARNUM);//创建最大容量为MAXCARNUM的队列 
		flag[i]=0;//初始化本方向前面是否有车辆条件 
		waiting[i]=0;//初始化各个方向的车是否在等待条件 
	}
	pthread_cond_init(&deadLock,NULL);//初始化死锁检测线程唤醒的条件变量 
	pthread_mutex_init(&deadLockMut,NULL);//初始化访问死锁检测线程唤醒条件变量的互斥量 
	pthread_mutex_init(&cross,NULL);//初始化公有资源a、b、c、d互斥量 

	int error=pthread_create(&deadLockCheck,NULL,checkDeadLock,NULL);//创建死锁检测线程 
	if(error!=0){
		printf("Can't create deadlock thread! %s\n",strerror(error));
	}
	scanf("%s",&directions);//读取新来车辆来自的方向
	for(i=0;i<strlen(directions);i++){
		Info thisInfo=(Info)malloc(sizeof(struct carInfo));//创建车辆（线程）信息 
		if(directions[i]=='n'){//如果输入为n，代表新来的车是从北边来的 
			thisInfo->direction=north;
		}else if(directions[i]=='s'){//如果输入为s，代表新来的车是从南边来的
			thisInfo->direction=south;
		}else if(directions[i]=='e'){//如果输入为e，代表新来的车是从东边来的
			thisInfo->direction=east;
		}else if(directions[i]=='w'){//如果输入为w，代表新来的车是从西边来的
			thisInfo->direction=west;
		}
		thisInfo->number=no++;//给车辆编号 
		int err=pthread_create(&car[i],NULL,carFrom,thisInfo);//创建车辆（线程） 
		if(err!=0){
			printf("Can't create car %d! %s\n",thisInfo->number,strerror(err));
		}
	}
	for(i=0;i<strlen(directions);i++){
		pthread_join(car[i],NULL);//等待所有已经创建的线程结束 
	} 
	for(i=0;i<4;i++){
		clearQueue(waitQ[i]);//清空等待队列 
	}
	return 0;
} 

//创建一个最大容量为maxElements的队列 
Queue createQueue(int maxElements){
	Queue q=(Queue)malloc(sizeof(struct queue));
	q->element=(int*)malloc(sizeof(int)*maxElements);//分配数组空间 
	q->capacity=maxElements;//最大容量为maxElements 
	q->size=0;//初始容量为0 
	q->front=1;//头指针指向1位置 
	q->rear=0;//尾指针指向0位置 
	return q; 
}
//清空队列 释放内存 
void clearQueue(Queue q){
	free(q->element);//释放内部数组空间 
	free(q);//释放队列指针 
} 
//求value的下一个队列索引 
static int succ(int value,Queue q){
	if(++value==q->capacity) 
		value=0;//如果已经到达了数组末尾则下一个索引在数组开头 
	return value;
}
//将元素x放入队列末尾 
void enqueue(int x,Queue q){
	if(q->size==q->capacity)
		return;		//若队列已满 则不能再往里添加元素 
	else{
		q->size++;	//队列元素数量加一 
		q->rear=succ(q->rear,q);//队列尾指针后移一位 
		q->element[q->rear]=x;//将元素插入队列尾 
	}
}
//弹出队列开头的元素 
int dequeue(Queue q){
	int res;
	if(q->size==0)
		return -1;	//若队列为空 则不能从队列里弹出元素 
	else{
		res=q->element[q->front];//取出队列开头元素 
		q->size--;	//队列元素数量减一 
		q->front=succ(q->front,q);//队列头指针后移一位 
		return res;
	}
}
//获取队列开头的元素，但不弹出
int front(Queue q){
	if(q->size==0)	//若队列为空 返回-1 
		return -1;
	else			//若队列不为空 返回头值 
		return q->element[q->front];
}
