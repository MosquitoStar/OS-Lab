#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <windows.h>

//�궨�� �������� �ӱ���ʼ����ʱ���ţ�0~3�� 
#define north 0			//�� 
#define west 1			//�� 
#define south 2			//�� 
#define east 3			//�� 
#define MAXCARNUM 100	//���ĳ������̣߳����� 

//���泵����Ϣ�����ݽṹ 
typedef struct carInfo * Info;
struct carInfo{
	int direction;//�������Եķ��� 
	int number;//�����ı�� 
};

//����Queue���ݽṹ�����
typedef struct queue *Queue;
struct queue{
	int capacity;//����������� 
	int size;//�����ִ�Ԫ������ 
	int front;//����ͷ���� 
	int rear;//����β���� 
	int *element;//���鷽ʽʵ�ֶ��� 
};
Queue createQueue(int maxElements);//����һ���������ΪmaxElements�Ķ��� 
void clearQueue(Queue q);//��ն��� �ͷ��ڴ� 
void enqueue(int x,Queue q);//��Ԫ��x�������ĩβ 
int dequeue(Queue q);//�������п�ͷ��Ԫ�� 
int front(Queue q);//��ȡ���п�ͷ��Ԫ�أ���������

pthread_cond_t queue[4];	//����������������������
pthread_cond_t first[4];	//���������´�ͨ�г�������������
pthread_mutex_t dirMut[4];	//���ʸ�����������������Ļ����� 
Queue waitQ[4];				//��������ȴ�����·�Ķ��� 
pthread_mutex_t waitQMut[4];//���ʸ�������ȴ����еĻ�����
pthread_mutex_t cross;		//���ʹ�����Դa��b��c��d�Ļ����� 
pthread_cond_t deadLock;	//��������̻߳��ѵ���������
pthread_mutex_t deadLockMut;//������������̻߳������������Ļ����� 
pthread_t deadLockCheck;	//��������߳�
int flag[4];		//������ǰ���Ƿ��г������г���Ϊ1������Ϊ0�� 
int waiting[4];		//��������ĳ��Ƿ��ڵȴ����޵ȴ�Ϊ0������Ϊ1�� 

//ÿ�������̵߳ĺ��� 
void* carFrom(void* info){
	Info i=(Info)info;	//��ȡ������Ϣ 
	int myDir=i->direction;	//�������Է��� 
	int myNo=i->number;	//���ó������̣߳���� 
	char* dirS;	//���ݳ�����Ϣ���÷����ַ��� 
	if(myDir==north)
		dirS="North";	//��Ϊ�������� ����ΪNorth 
	else if(myDir==south)
		dirS="South";	//��Ϊ�ϱ����� ����ΪSouth
	else if(myDir==east)
		dirS="East";	//��Ϊ�������� ����ΪEast 
	else if(myDir==west)
		dirS="West";	//��Ϊ�������� ����ΪWest 
	pthread_mutex_lock(&waitQMut[myDir]); 
	enqueue(myNo,waitQ[myDir]);	//������������ȴ������� 
	pthread_mutex_unlock(&waitQMut[myDir]);
	while(front(waitQ[myDir])!=myNo);//������������ڵȴ�������ǰ����ȴ� 
	printf("Car %d from %s arrives at crossing.\n",myNo,dirS);
	pthread_mutex_lock(&dirMut[myDir]);
	while(flag[myDir]){//�ȴ����������ڹ���·�ĳ������ź� 
		pthread_cond_wait(&queue[myDir],&dirMut[myDir]);
	}
	flag[myDir]=1;//���ñ����������ڹ���·���� 
	if(waitQ[(myDir+1)%4]->size!=0){//����ұ��г���ҲҪ����· 
		waiting[myDir]=1;//���ñ����������ڵȴ�����·���� 
		if(waiting[0]&&waiting[1]&&waiting[2]&&waiting[3]){
			printf("DEADLOCK: car jam detected, signalling North to go.\n");
			pthread_mutex_lock(&deadLockMut);
			pthread_cond_signal(&deadLock);//����ĸ������ڵȴ����򴥷������ź� 
			pthread_mutex_unlock(&deadLockMut);
		}
		pthread_cond_wait(&first[myDir],&dirMut[myDir]);//�ȴ��ұ߳������źŸ��Լ� 
	}
	waiting[myDir]=0;//ȡ�������������ڵȴ�����·����
	pthread_mutex_lock(&cross);
	printf("Car %d from %s leaving crossing.\n",myNo,dirS);
	Sleep(100);//����·ʱ��1s (Windows)
//	sleep(1000);//����·ʱ��1s (Linux)
	pthread_mutex_unlock(&cross);
	pthread_mutex_lock(&dirMut[(myDir+3)%4]);
	pthread_cond_signal(&first[(myDir+3)%4]);//�����źŸ���ߵĳ���ͨ�� 
	pthread_mutex_unlock(&dirMut[(myDir+3)%4]);
	pthread_mutex_lock(&waitQMut[myDir]);
	dequeue(waitQ[myDir]);	//�������ӵȴ��������˳� 
	pthread_cond_signal(&queue[myDir]);	//�����źŸ���������һ���� 
	flag[myDir]=0;	//ȡ�������������ڹ���·����
	pthread_mutex_unlock(&waitQMut[myDir]);
	pthread_mutex_unlock(&dirMut[myDir]);
}
//��������̺߳��� 
void* checkDeadLock(){
	while(1){
		pthread_mutex_lock(&cross);
		pthread_cond_wait(&deadLock,&cross);//�ȴ�������ʱ�򱻻��� 
		//printf("deadlockcheck wake up and signal north.\n");
		pthread_mutex_lock(&dirMut[north]);//��ȡ���ʱ��������������� 
		pthread_cond_signal(&first[north]);//�����ź��ñ��߳������� 
		pthread_mutex_unlock(&dirMut[north]);//�ͷŷ��ʱ���������������
		pthread_mutex_unlock(&cross);
	}
}

int main(){
	//��ʼ���������������ͻ����� �����ĸ����� 
	int no=1,i;
	char directions[MAXCARNUM];//��ȡ����������ַ��� 
	pthread_t car[MAXCARNUM];//�����߳� 
	for(i=0;i<4;i++){
		pthread_cond_init(&queue[i],NULL);//��ʼ������������������������ 
		pthread_cond_init(&first[i],NULL);//��ʼ�����������´�ͨ�г�������������
		pthread_mutex_init(&dirMut[i],NULL);//��ʼ�����ʸ�����������������Ļ�����
		pthread_mutex_init(&waitQMut[i],NULL);//��ʼ�����ʸ�������ȴ����еĻ�����
		waitQ[i]=createQueue(MAXCARNUM);//�����������ΪMAXCARNUM�Ķ��� 
		flag[i]=0;//��ʼ��������ǰ���Ƿ��г������� 
		waiting[i]=0;//��ʼ����������ĳ��Ƿ��ڵȴ����� 
	}
	pthread_cond_init(&deadLock,NULL);//��ʼ����������̻߳��ѵ��������� 
	pthread_mutex_init(&deadLockMut,NULL);//��ʼ��������������̻߳������������Ļ����� 
	pthread_mutex_init(&cross,NULL);//��ʼ��������Դa��b��c��d������ 

	int error=pthread_create(&deadLockCheck,NULL,checkDeadLock,NULL);//������������߳� 
	if(error!=0){
		printf("Can't create deadlock thread! %s\n",strerror(error));
	}
	scanf("%s",&directions);//��ȡ�����������Եķ���
	for(i=0;i<strlen(directions);i++){
		Info thisInfo=(Info)malloc(sizeof(struct carInfo));//�����������̣߳���Ϣ 
		if(directions[i]=='n'){//�������Ϊn�����������ĳ��Ǵӱ������� 
			thisInfo->direction=north;
		}else if(directions[i]=='s'){//�������Ϊs�����������ĳ��Ǵ��ϱ�����
			thisInfo->direction=south;
		}else if(directions[i]=='e'){//�������Ϊe�����������ĳ��ǴӶ�������
			thisInfo->direction=east;
		}else if(directions[i]=='w'){//�������Ϊw�����������ĳ��Ǵ���������
			thisInfo->direction=west;
		}
		thisInfo->number=no++;//��������� 
		int err=pthread_create(&car[i],NULL,carFrom,thisInfo);//�����������̣߳� 
		if(err!=0){
			printf("Can't create car %d! %s\n",thisInfo->number,strerror(err));
		}
	}
	for(i=0;i<strlen(directions);i++){
		pthread_join(car[i],NULL);//�ȴ������Ѿ��������߳̽��� 
	} 
	for(i=0;i<4;i++){
		clearQueue(waitQ[i]);//��յȴ����� 
	}
	return 0;
} 

//����һ���������ΪmaxElements�Ķ��� 
Queue createQueue(int maxElements){
	Queue q=(Queue)malloc(sizeof(struct queue));
	q->element=(int*)malloc(sizeof(int)*maxElements);//��������ռ� 
	q->capacity=maxElements;//�������ΪmaxElements 
	q->size=0;//��ʼ����Ϊ0 
	q->front=1;//ͷָ��ָ��1λ�� 
	q->rear=0;//βָ��ָ��0λ�� 
	return q; 
}
//��ն��� �ͷ��ڴ� 
void clearQueue(Queue q){
	free(q->element);//�ͷ��ڲ�����ռ� 
	free(q);//�ͷŶ���ָ�� 
} 
//��value����һ���������� 
static int succ(int value,Queue q){
	if(++value==q->capacity) 
		value=0;//����Ѿ�����������ĩβ����һ�����������鿪ͷ 
	return value;
}
//��Ԫ��x�������ĩβ 
void enqueue(int x,Queue q){
	if(q->size==q->capacity)
		return;		//���������� �������������Ԫ�� 
	else{
		q->size++;	//����Ԫ��������һ 
		q->rear=succ(q->rear,q);//����βָ�����һλ 
		q->element[q->rear]=x;//��Ԫ�ز������β 
	}
}
//�������п�ͷ��Ԫ�� 
int dequeue(Queue q){
	int res;
	if(q->size==0)
		return -1;	//������Ϊ�� ���ܴӶ����ﵯ��Ԫ�� 
	else{
		res=q->element[q->front];//ȡ�����п�ͷԪ�� 
		q->size--;	//����Ԫ��������һ 
		q->front=succ(q->front,q);//����ͷָ�����һλ 
		return res;
	}
}
//��ȡ���п�ͷ��Ԫ�أ���������
int front(Queue q){
	if(q->size==0)	//������Ϊ�� ����-1 
		return -1;
	else			//�����в�Ϊ�� ����ͷֵ 
		return q->element[q->front];
}
