#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>

#include<time.h>
#include <fcntl.h>

#define FILE_A "a.bin"
#define FILE_B "b.bin"

int T,N;

struct vector{
	int num;
}vector;

struct vector a, b;
sem_t *S;



static void *
thread_function(void *arg){
	int *id = (int *)arg;	
	sem_wait(S);
	printf("thread %d\n",*id);
}

int 
main(int argc, char *argv[]){

	T = atoi(argv[1]);
	N = atoi(argv[2]);
	S = (sem_t *)malloc(sizeof(sem_t));
	sem_init(S, 0, 0);
	
	/*create T threads*/
	pthread_t *th;
	th = (pthread_t *)malloc(T * sizeof(pthread_t));
	int i;
	int *pi;
	for(i=0;i<T;i++){
		pi = (int *)malloc(sizeof(int));
		*pi = i;
		sleep(1);
		pthread_create(&th[i],NULL,thread_function, (void *)pi);
	}
	
	int j=0;
	FILE *fd1, *fd2;	
	while(j<10){
		sleep(1);
		
		int k, r, *vecA, *vecB;
		/*generate r between(1-N)*/
		srand(time(NULL));
		r = ((rand() % N) + 1);
		vecA = (int *)malloc(r * sizeof(int));
		vecB = (int *)malloc(r * sizeof(int));
		/*fill vector A & B*/
		srand(time(NULL));
		for(k=0; k<r; k++){
			vecA[k] = ((rand() % 1000));
		}
		srand(time(NULL));
		for(k=0; k<r; k++){
			vecB[k] = ((rand() % 1000));
		}
		
		/*write to files*/
		fd1 = fopen(FILE_A, "w");
		fd2 = fopen(FILE_B, "w");
		for(k=0; k<r; k++){
			a.num = vecA[k];
			b.num = vecB[k];
			fwrite(&a, sizeof(struct vector),1,fd1);
			fwrite(&b, sizeof(struct vector),1,fd2);
		}
		fclose(fd1); fclose(fd2);
		/*read back what's been written*/
		fd1 = fopen(FILE_A, "r");
		fd2 = fopen(FILE_B, "r");
		for (k=1;k<=r; k++) {
		    fread(&a,sizeof(struct vector),1,fd1);
		    fread(&b,sizeof(struct vector),1,fd2);
		    printf("%d\t",a.num);
		}
		fclose(fd1); fclose(fd2);
		
		j++;
	}
	for(i = 0; i < T; i++)
		sem_post(S);
	/*print result*/
	
	/************/
	for(i=0;i<T;i++){
		pthread_join(th[i],NULL);
	}

	return 0;
}
