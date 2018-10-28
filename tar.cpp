#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <linux/limits.h>
#include <set>
#include <list>
#include <map>
#include <unistd.h>
#include <utime.h>
#include <time.h>



using namespace std;

FILE *archivefile;

set<ino_t> inodes;

list<string> dirs;

bool c = false;
bool x = false;
bool v = false;

void breakList(string path){
    string s = "";
    
    for(unsigned int i = 0; i<path.length(); i++){
        char c = path[i];
        if(c=='/'){
            dirs.push_back(s);
            s="";
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
	
	d=opendir(path);

	if(d == NULL) { perror("Couldn't open directory"); exit(1); }
	
	if(stat(path, &finfo) == 0){
	        if(v){
        	        printf("%s:processing\n", path);
        	}
		//inodes.insert(finfo.st_ino);
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

	map<ino_t, string> inodesMap;
	
	map<string, struct stat> directories;
	
	fread(&finfo, sizeof(struct stat), 1, archivefile);
	fscanf(archivefile, "%s\n", filename);

	breakList(string(filename));
	
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
	
	if(v){
		printf("processing:%s\n", filename);
	}
	//Creates the input directory
	if(mkdir(filename, finfo.st_mode)!=0){
		perror("could not make directory");
		exit(1);
	}
	
	directories.insert(std::pair<string,struct stat>(string(filename), finfo));
	
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
			directories.insert(std::pair<string,struct stat>(string(filename), finfo));
		}
		else if(S_ISREG(finfo.st_mode)){
			//Checks to see if this is a hard link to an existing file. Writes the file if it's not a link
			std::map<ino_t,string>::iterator it;
			it = inodesMap.find(finfo.st_ino);
			//printf("%s\n", it->second);
			if(it == inodesMap.end()){
				FILE *file;
				//printf("file:%s inode:%lu size:%ld\n", filename, finfo.st_ino, finfo.st_size);
				char buf[finfo.st_size];
			
				//read content from archive file and write it to new file in directory
				if(fread(buf,finfo.st_size,1,archivefile)>=0){
					if((file=fopen(filename, "w"))!=NULL){
						fwrite(buf, finfo.st_size, 1, file);
						//printf("Adding %lu:%s to map\n", finfo.st_ino, filename);
						inodesMap.insert(std::pair<ino_t,string>(finfo.st_ino,string(filename)));
					}
				}
				else{
					perror("Could not read content of file");
					exit(1);
				}
				fclose(file);
			}
			//If it is a hard link, create it
			else{
				//printf("Linking %s to %s on %lu\n", filename, it->second.c_str(), it->first );
				if(link(it->second.c_str(), filename)<0){
					perror("could not create link");
					exit(1);
				}	
			}
			//set permissions and stuff for the file
			struct utimbuf times;
			times.actime = finfo.st_atime;
			times.modtime = finfo.st_mtime;

			if(utime(filename, &times) != 0) { perror("utime"); exit(1); }
	
			if(chmod(filename, finfo.st_mode) != 0) { perror("chmod"); exit(1); }
		}
	}
	
	//set permissions and stuff on all the directories
	map<string, struct stat>::iterator it;
	
	for(it=directories.begin(); it!=directories.end(); it++){
		struct utimbuf times;
		times.actime = it->second.st_atime;
		times.modtime = it->second.st_mtime;
		
		if(utime(it->first.c_str(), &times) != 0) { perror("utime"); exit(1); }
		
		if(chmod(it->first.c_str(), it->second.st_mode) != 0) { perror("chmod"); exit(1); }
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
