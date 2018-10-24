#include <stdio.h>
#include <sstream>
#include <iostream>
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
	
	if(v){
		printf("%s:processing\n", path);
	}
	d=opendir(path);
	
	if(d == NULL) { perror("Couldn't open directory"); exit(1); }

	for(de = readdir(d); de != NULL; de = readdir(d)){
		if(string(de->d_name)!="." && string(de->d_name)!=".."){	
			string s = string(path) + "/" + string(de->d_name);
		
			if(stat(s.c_str(), &finfo) == 0){
				if(v){
					printf("%s/%s:processing\n", path, de->d_name);
				}
				fwrite(&finfo, sizeof(struct stat), 1, archivefile);
				fprintf(archivefile, "%s\n", de->d_name);
			}
			else{
				printf("%s: file not found\n", de->d_name);
			}
		}
	}
	
	return 1;
}

int extract(){
	//printf("x=%d v=%d\n", x, v);
	struct stat finfo;
	while(fread(&finfo, sizeof(struct stat), 1, archivefile)){
		char* filename;
		fscanf(archivefile, "%s\n", filename);
		printf("file:%s inode:%d size:%d\n", filename, finfo.st_ino, finfo.st_size);
	}
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
		perror("usage to create an archive: tar c[v] ARCHIVE DIRECTORY\nusage to extract an archive: tar x[v] ARCHIVE\n");
		exit(-1);
	}
	
	if(c){
		if(argc!=4){
                	perror("usage to create an archive: tar c[v] ARCHIVE DIRECTORY\n");
			exit(-1);
		}
		else{
			if((archivefile=fopen(argv[2], "w"))==NULL){
				perror("Could not open output archive file\n");
				return -1;
			}
			create(argv[3]);
		}
	}
	else if(x){
        	if(argc!=3){
                        perror("usage to extract an archive: tar x[v] ARCHIVE\n");
			exit(-1);
                }
		else{
                        if((archivefile=fopen(argv[2], "r"))==NULL){
                                perror("Could not open archive file\n");
                                return -1;
                        }
			extract();
		}
	}

	return 1;
}
