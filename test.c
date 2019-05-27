#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>

#include<time.h>
#include <fcntl.h>

#define FILE_A "a.bin"
#define FILE_B "b.bin"

struct vector{
	int num;
}vector;
FILE *fd1,*fd2;

struct vector a, b;
int val;

int get_elementA(int index){
	int element;
	int i;
	fd1 = fopen(FILE_A, "rb");
	rewind(fd1);
	printf("sizeof int %ld\n",sizeof(int));
	fseek(fd1, index * sizeof(struct vector), SEEK_SET);
	fread(&a, sizeof(struct vector), 1, fd1);
	element = a.num;
	fclose(fd1);fclose(fd2);
	return element;
}

void 
read_binary_file(){
	int i;
	fd1 = fopen(FILE_A, "rb");
	fd2 = fopen(FILE_B, "rb");
	for (i=0;i<68; i++) {
		fread(&a,sizeof(struct vector),1,fd1);
		fread(&b,sizeof(struct vector),1,fd2);
		printf("a%d = %d\t", i, a.num);
		printf("b%d = %d\t", i, b.num);
		printf("\n");
	}
	fclose(fd1); fclose(fd2);
}

int 
main(int argc, char *argv[]){
	read_binary_file();
	//rewind(fd1); rewind(fd2);
	int elem = get_elementA(20); 
	printf("elem = %d\n",elem);
	
}
