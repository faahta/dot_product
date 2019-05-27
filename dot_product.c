#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>

#include<time.h>
#include <fcntl.h>

#define FILE_A "a.bin"
#define FILE_B "b.bin"

struct vector{
	int num;
}vector_t;

typedef struct element{
	int a,b;
}Element_t;

typedef struct thread{
	int vec_size, sum, count;
	bool *multiplied;
	sem_t *S1,*S2;
	pthread_mutex_t lock;
}Thread_t;

Thread_t th_data;
struct vector a, b;

int T, N;
sem_t *S;
FILE *fd1, *fd2;


Element_t 
get_element(int index){
	Element_t elem;
	int i;
	fd1 = fopen(FILE_A, "rb");
	fd2 = fopen(FILE_B, "rb");
	
	rewind(fd1); rewind(fd2);
	fseek(fd1, index * sizeof(struct vector), SEEK_SET);
	fseek(fd2, index * sizeof(struct vector), SEEK_SET);

	fread(&a,sizeof(struct vector),1,fd1);
	fread(&b,sizeof(struct vector),1,fd2);
	
	elem.a = a.num;
	elem.b = b.num;
	fclose(fd1);fclose(fd2);
	return elem;
}

void 
read_binary_file(){
	int i;
	fd1 = fopen(FILE_A, "rb");
	fd2 = fopen(FILE_B, "rb");
	for (i=0;i<th_data.vec_size; i++) {
		fread(&a,sizeof(struct vector),1,fd1);
		fread(&b,sizeof(struct vector),1,fd2);
		printf("a%d = %d\t", i, a.num);
		printf("b%d = %d\t", i, b.num);
		printf("\n");
	}
	fclose(fd1); fclose(fd2);
}

void
init(){
	S = (sem_t *)malloc(sizeof(sem_t));
	sem_init(S, 0, 0);	
	th_data.S1 = (sem_t *)malloc(sizeof(sem_t));
	sem_init(th_data.S1, 0, 1);
	th_data.S2 = (sem_t *)malloc(sizeof(sem_t));
	sem_init(th_data.S2, 0, 1);
	
	pthread_mutex_init(&th_data.lock, NULL);
	th_data.vec_size = 0;
	th_data.sum = 0;
	th_data.count = 0;
}
void 
setup(){
	th_data.multiplied = (bool *)malloc(th_data.vec_size * sizeof(bool));
	int i;
	for(i=0; i<th_data.vec_size; i++)
		th_data.multiplied[i] = 0;
}

static void *
thread_function(void *arg){
	pthread_detach(pthread_self());
	int *id = (int *)arg;	
	sem_wait(S);
	printf("thread %d\n",*id);
	int index;
	srand(time(NULL));
	
	while(th_data.count < th_data.vec_size){
		/*select index to multiply*/
		sleep(1);
		pthread_mutex_lock(&th_data.lock);
		index = (rand() % th_data.vec_size);
		Element_t elem;
		if(!th_data.multiplied[index]){
			elem = get_element(index);
			printf("thread %d index = %d a = %d b = %d\n",*id, index, elem.a,elem.b);
			th_data.sum += (elem.a * elem.b);
			/*mark index as multiplied*/
			th_data.multiplied[index] = 1;
			th_data.count++;
		}  	
		pthread_mutex_unlock(&th_data.lock);
	} 
	sem_post(th_data.S2);
	printf("thread %d exiting\n",*id);
	return NULL;
}

int 
main(int argc, char *argv[]){

	T = atoi(argv[1]);
	N = atoi(argv[2]);
	init();
	
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
	int *vecA, *vecB;
	while(j < 3){
		printf("=====================================================LOOP %d=========================================================\n",j);
		sleep(1);
		int k, r, x;
		/*generate r between(1-N)*/
		srand(time(NULL));
		r = ((rand() % N) + 1);
		th_data.vec_size += r;
		printf("r = %d\n",th_data.vec_size);
		vecA = (int *)malloc(r * sizeof(int));
		vecB = (int *)malloc(r * sizeof(int));
		sleep(2);
		/*fill vector A & B*/
		for(k=0; k<r; k++){
			x = (rand() % (1000 - 1 + 1)) + 1; 
			vecA[k] = x;
			//printf("a[%d] = %d\n", k, vecA[k]);
		}
		sleep(2);
		for(k=0; k<r; k++){
			x = (rand() % (1000 - 1 + 1)) + 1; 
			vecB[k] = x;
			//printf("b[%d] = %d\n", k, vecB[k]);
		}
		
		/*write to files*/
		if(j == 0){
			fd1 = fopen(FILE_A, "wb");
			fd2 = fopen(FILE_B, "wb");
		} else {
			fd1 = fopen(FILE_A, "ab");
			fd2 = fopen(FILE_B, "ab");
		}
		for(k=0; k<r; k++){
			a.num = vecA[k];
			b.num = vecB[k];
			fwrite(&a, sizeof(struct vector),1,fd1);
			fwrite(&b, sizeof(struct vector),1,fd2);
		}
		fclose(fd1); fclose(fd2);	
		j++;
	}
	/*check written binary file*/
	read_binary_file();
	 /*mark all indexes as NOT multiplied*/
	setup();
	/*notify all threads that the two vectors are ready*/
	for(i = 0; i < T; i++)
		sem_post(S);
		
	/*wait for their computation and print results*/
	int dot_prod=0;
	for(i=0; i<T; i++)
		sem_wait(th_data.S2);
		
	fd1 = fopen(FILE_A, "rb");
	fd2 = fopen(FILE_B, "rb");
	for (i=0;i<th_data.vec_size; i++) {
		fread(&a,sizeof(struct vector),1,fd1);
		fread(&b,sizeof(struct vector),1,fd2);
		dot_prod += (a.num * b.num);
	}
	fclose(fd1); fclose(fd2);
	
	printf("Dot product as computed by me: %d\n",dot_prod);
	printf("Dot product as computed by my niggas: %d\n",th_data.sum);
	if(dot_prod != th_data.su)
		printf("funny but true! I'd rather be 80 year old me than 20 year old you\n");
	pthread_exit((void *) pthread_self());

	return 0;
}
