#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <cstdlib>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
using namespace std;

FILE *archivefile;

bool c = false;
bool x = false;
bool v = false;

int create(char* path){
	//printf("c=%d v=%d\n", c, v);
	struct stat finfo;
	
	DIR *d;
	
	struct dirent *de;
	
	d=opendir(path);
	
	if(d == NULL) { perror("Couldn't open directory"); exit(1); }

	for(de = readdir(d); de != NULL; de = readdir(d)){
		if(stat(de->d_name, &finfo) == 0){
			fwrite(&finfo, sizeof(struct stat), 1, archivefile);
			//string s = de->d_name;
			fprintf(archivefile, "%s\n", de->d_name);
			//fwrite(s, s.size(), 1, archivefile);
		}
		else{
			printf("%s: file not found\n", de->d_name);
		}
	}
	
	return 1;
}

int extract(){
	//printf("x=%d v=%d\n", x, v);
	return 1;
}

int main(int argc, char *argv[]){
	//int flag;
	//printf("%s\n", argv[1]);
	if(strcmp(argv[1], "c")==0){
		c=true;
	}
	else if(strcmp(argv[1], "cv")==0){
		c=true;
		v=true;
	}
	else if(strcmp(argv[1], "x")==0){
		x=true;
	}
	else if(strcmp(argv[1], "xv")==0){
		x=true;
		v=true;
	}
	else{
		printf("usage to create an archive: tar c[v] ARCHIVE DIRECTORY\n");
                printf("usage to extract an archive: tar x[v] ARCHIVE\n");
		exit(-1);
	}
	
	if(c){
		if(argc!=4){
                	printf("usage to create an archive: tar c[v] ARCHIVE DIRECTORY\n");
			exit(-1);
		}
		else{
			printf("%s\n", argv[3]);
			printf("%s\n", argv[2]);
			if((archivefile=fopen(argv[2], "w"))==NULL){
				printf("Could not open output archive file\n");
				return -1;
			}
			create(argv[2]);
		}
	}
	else if(x){
        	if(argc!=3){
                        printf("usage to extract an archive: tar x[v] ARCHIVE\n");
			exit(-1);
                }
		else{
			extract();
		}
	}

	return 1;
}
