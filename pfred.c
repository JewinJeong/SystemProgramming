#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/wait.h>

int pnum;
int interval;
int dataNum = 0;
int startPosition = 0;
int* inputNum;
int fp;
mqd_t des;

int ceilDivide(int n1, int n2){
	if(n1/(double)n2 > n1/n2){
        return n1/n2 +1;
    }
	return n1/n2;
}

void p_function(){
    int index = ceilDivide(10000,interval);
    int* countNum = (int*)malloc(sizeof(int)*index);
    if(des == -1){
        perror("open error\n");
        exit(0);
    }
    for(int i = 0; i<pnum; i++){
        mq_receive(des, (char*)countNum, 8192,0);
        for(int j = 0; j<index; j++){    
            inputNum[j] += countNum[j];
        }
    }
    mq_close(des);
    mq_unlink("/ku_pfred.c");
}
void c_function(int data){
    int cur = startPosition + (data)*5;
    char buffer[5];
    int num;
    int index = ceilDivide(10000,interval);
    int* countNum = (int*)malloc(sizeof(int)*index);
 

    for(int i = 0; i<index; i++){
        countNum[i] = 0;
    }
    while(pread(fp, buffer, 5, cur) == 5){
        cur += pnum *5;
        num = atoi(buffer);
        countNum[num/interval]++;
    }
    if(des == -1){
        perror("open error\n");
    }
    mq_send(des, (char*)countNum, sizeof(int)*index,0);
    free(countNum);
}

int main(int argc, char* argv[]){
	pnum = atoi(argv[1]);
	interval = atoi(argv[2]);
    char *filename = argv[3];
    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 8192;
    int status;
    des = mq_open("/ku_pfred.c", O_RDWR | O_CREAT, 0666, &attr);
	fp = open(filename, O_RDONLY);

	if(fp == -1){
		perror("Can't open!");
	}

	char buf=0;
	while(buf != '\n'){
		read(fp,&buf,1);
		startPosition++;
	}

	char* tmpBuf = (char*)malloc(sizeof(char)*startPosition);
	pread(fp,tmpBuf,startPosition,0);
	dataNum = atoi(tmpBuf);
	free(tmpBuf);

    char buffer[5];
    int index = ceilDivide(10000,interval);
    inputNum = (int*)malloc(sizeof(int)*index);

    int* children = (int*)malloc(sizeof(int)*pnum);

    for(int i = 0; i < pnum; i++){
        children[i] = fork();
        if(children[i] == 0){
            c_function(i);
            free(tmpBuf);
            free(inputNum);
            free(children);
            close(fp);
            exit(0);
        }
    }
    p_function();
    for(int i = 0; i<index; i++){
        printf("%d\n", inputNum[i]);
    }
    for(int i=0;i<pnum;i++){
        waitpid(children[i], &status, 0);
    }
    close(fp);
    free(tmpBuf);
    free(inputNum);
    free(children);
    
    return 0;
}
