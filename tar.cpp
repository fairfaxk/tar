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
#include <linux/limits.h>
#include <set>
#include <list>

using namespace std;

FILE *archivefile;

list<string> dirs;

bool c = false;
bool x = false;
bool v = false;

void breakList(string path){
    string s = "";
    
    for(int i = 0; i<path.length(); i++){
        char c = path[i];
        if(c=='/'){
            dirs.push_back(s);
            s="";
        }
        else if(c==path[path.length()-1]){
            s+=c;
            dirs.push_back(s);
        }
        else{
            s+=c;
        }
    }
}

int create(char* path){
	struct stat finfo;
	
	DIR *d;
	
	struct dirent *de;

	set<ino_t> inodes;
	
	d=opendir(path);

	if(d == NULL) { perror("Couldn't open directory"); exit(1); }
	
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
				if(S_ISDIR(finfo.st_mode)){
					create((char *) s.c_str());
				}
				else if(S_ISREG(finfo.st_mode)){
	                               if(v){
         	                               printf("%s:processing\n", s.c_str());
                 	               }

					FILE *file;
				
					//write struct stat to archive
					fwrite(&finfo, sizeof(struct stat), 1, archivefile);
				
					//write file name to archive
					fprintf(archivefile, "%s\n", s.c_str());
					
					//Check to make sure we haven't written inode info for hard links
					if(inodes.find(finfo.st_ino) == inodes.end()){
      						inodes.insert(finfo.st_ino);

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
						fclose(file);
					}
				}
			}
			else{
				perror("file not found\n");
				exit(1);
			}
		}
	}
	closedir(d);
	
	return 1;
}

int extract(){
	struct stat finfo;
	char filename[PATH_MAX];

	fread(&finfo, sizeof(struct stat), 1, archivefile);
	fscanf(archivefile, "%s\n", filename);

	string pathto = string(filename);

	breakList(pathto);
	
	//Checks if the input directory is a subdir and creates the parents if it is
	if(!dirs.empty()){
        	std::list<string>::iterator it;
        	for (it = dirs.begin(); it != dirs.end(); ++it){
			if(mkdir(it->c_str(), 755)!=0){
				perror("could not make directory");
				exit(1);
			}
        	}
	} 
	
	//Creates the input directory
	if(mkdir(pathto.c_str(), finfo.st_mode)!=0){
		perror("could not make directory");
		exit(1);
	}
	
	while(fread(&finfo, sizeof(struct stat), 1, archivefile)){
                fscanf(archivefile, "%s\n", filename);

                if(v){
                	printf("processing:%s\n", filename);
                }
		if(S_ISDIR(finfo.st_mode)){
			if(mkdir(filename, finfo.st_mode)!=0){
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
			}
			else{
				perror("Could not read content of file");
				exit(1);
			}
			fclose(file);
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
			
			string path = string(argv[3]);

			//If path ends in /, drop the / (for formatting)
        		if(path[path.size()-1]=='/'){
                		path = path.substr(0, path.size()-1);
        		}		
	
			create((char*)path.c_str());
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

	fclose(archivefile);

	return 1;
}
