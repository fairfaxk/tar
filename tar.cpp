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
	struct stat finfo;
	
	DIR *d;
	
	struct dirent *de;

	d=opendir(path);

	if(d == NULL) { perror("Couldn't open directory"); exit(1); }
	
	//TODO:Implement this so it tracks if the base path is a subdirectory
	if(stat(path, &finfo) == 0){
	        if(v){
        	        printf("%s:processing\n", path);
        	}
		fwrite(&finfo, sizeof(struct stat), 1, archivefile);
		fprintf(archivefile, "%s\n", path);
	}

	for(de = readdir(d); de != NULL; de = readdir(d)){
		if(string(de->d_name)!="." && string(de->d_name)!=".."){	
			string s = string(path) + "/" + string(de->d_name);
		
			if(stat(s.c_str(), &finfo) == 0){
				if(v){
					printf("%s/%s:processing\n", path, de->d_name);
				}
				FILE *file;
				
				//write struct stat to archive
				fwrite(&finfo, sizeof(struct stat), 1, archivefile);
				
				//write file name to archive
				fprintf(archivefile, "%s\n", s.c_str());

				//Write contents of file to archive
                                if((file=fopen(s.c_str(), "r"))!=NULL){
                                	char c = fgetc(file); 
					while (c != EOF) 
    					{ 
       						fputc(c, archivefile); 
        					c = fgetc(file); 
    					} 
                                }
				else{
					perror("Couldn't write file");
					exit(1);
				}

			}
			else{
				perror("file not found\n");
				exit(1);
			}
		}
	}
	
	return 1;
}

int extract(){
	struct stat finfo;
	while(fread(&finfo, sizeof(struct stat), 1, archivefile)){
		char filename[255]; 
                fscanf(archivefile, "%s\n", filename);
                if(v){
                	printf("processing:%s\n", filename);
                }
		if(S_ISDIR(finfo.st_mode)){
			if(mkdir(filename, finfo.st_mode)==0){
				
			}
			else{
				perror("Could not make directory");
				exit(1);
			}
		}
		else if(S_ISREG(finfo.st_mode)){
			FILE *file;
			printf("file:%s inode:%lu size:%ld\n", filename, finfo.st_ino, finfo.st_size);
			char buf[finfo.st_size];
			
			//read content from archive file and write it to new file in directory
			if(fread(buf,finfo.st_size,1,archivefile)>=0){
				if((file=fopen(filename, "w"))!=NULL){
					fwrite(buf, finfo.st_size, 1, file);
				}
				//printf("%s", buf);
			}
			else{
				perror("Could not read content of file");
				exit(1);
			}
		}
	}
	return 1;
}

int main(int argc, char *argv[]){
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
				exit(-1);
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
                                exit(-1);
                        }
			extract();
		}
	}

	return 1;
}
