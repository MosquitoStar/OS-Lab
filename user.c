#include <stdio.h>
#include <string.h>

int main(){
	int ch;
	int output=0;
	FILE *fp;
	
	//����־�ļ� 
	fp=fopen("/var/log/kern.log","r");
	if(fp==NULL){//���򲻿������������Ϣ ����-1 
		printf("Can't open log file!\n");
		return -1;
	}
	
	//��ȡ��־�ļ�
	fseek(fp,0,SEEK_SET);//�ļ�ָ���ض�λ���ļ�ͷ 
	ch=fgetc(fp);
	//�ҵ��ں�ģ�������¼��ͷ
	while(ch!=EOF){
		//�����¼��ͷ��������Ϊ"&#&" 
		if(ch=='&'){
			ch=fgetc(fp);
			if(ch=='#'){
				ch=fgetc(fp);
				if(ch=='&'){
					printf("Find my module log head!\n");
					break;
				}
			}
		}
		ch=fgetc(fp);
	}
	//��ӡ���ں�ģ��������¼
	while(ch!=EOF){
		if(ch=='@'){
			output=1;
		}else if(ch=='\n'){
			output=0;
		}else if(ch=='&'){//�����¼��β��������Ϊ"&*&" 
			ch=fgetc(fp);
			if(ch=='*'){
				ch=fgetc(fp);
				if(ch=='&'){
					printf("My module log ends!\n");
					break;
				}
			}
		}else if(output){
			printf("%c",ch);
		}
		ch=fgetc(fp);
	}
	
	//�ر���־�ļ�
	fclose(fp); 
	return 0;
}
