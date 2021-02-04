#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <fcntl.h>

int tnum;
int interval;
int dataNum = 0;
int startPosition = 0;
int* inputNum;
int fp;

pthread_mutex_t* mutex;

int ceilDivide(int n1, int n2){
	if(n1/(double)n2 > n1/n2){
        return n1/n2 +1;
    }
	return n1/n2;
}
void *thread_function(void* data){ // start position
	int cur = startPosition+((int)((long)data))*5;
	char buffer[5];
	int num;
	int index = ceilDivide(10000,interval);
	int* countNum = (int*)malloc(sizeof(int)*index);
	for(int i = 0; i < index; i++){
		countNum[i] = 0;
	}
	while(pread(fp, buffer, 5, cur) == 5){
		cur += tnum * 5;
		num = atoi(buffer);
		countNum[num/interval]++;
	}
	for(int i = 0; i < index; i++){
		pthread_mutex_lock(&mutex[i]);
		inputNum[i] += countNum[i];
		pthread_mutex_unlock(&mutex[i]);
	}
    free(countNum);

}

int main(int argc, char* argv[]){
	tnum = atoi(argv[1]);
	interval = atoi(argv[2]);
	char *filename = argv[3];
	fp = open(filename, O_RDONLY);
    char buf=0;
    int index = ceilDivide(10000,interval);

	if(fp == -1){
		perror("Can't open!");
	}
	while(buf != '\n'){
		read(fp,&buf,1);
		startPosition++;
	}
	char* tmpBuf = (char*)malloc(sizeof(char)*startPosition);
	
	pread(fp,tmpBuf,startPosition,0);
	dataNum = atoi(tmpBuf);
	free(tmpBuf);
    
    
	pthread_t* p_thread = (pthread_t*)malloc(sizeof(pthread_t)*tnum);
	mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t)*index);
    inputNum = (int*)malloc(sizeof(int)*index);
	for(int i = 0; i < index; i++){
		inputNum[i] = 0;
	}
   
	for(int i = 0; i < index; i++){
		pthread_mutex_init(&mutex[i], NULL);
	}
   
	for(int i = 0; i< tnum-1; i++){
		pthread_create(&p_thread[i], NULL, thread_function,(void*)((long)i));
	}
    
	thread_function((void*)((long)(tnum-1)));

	for(int i = 0; i< tnum-1; i++){
		pthread_join(p_thread[i], NULL);
	}
	for(int i = 0; i < index; i++){
		printf("%d\n",inputNum[i]);
	}
	
	free(p_thread);
	free(mutex);
	free(inputNum);
	return 0;
}
