#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    if(argc< 2){
        printf("usage: %s path1 [path2 ...]\n", argv[0]);
        exit(1);
    }

    struct stat filestat;
        
    for(int i=1; i<argc; i++){
        if (lstat(argv[i], &filestat) == -1) {
            //perror("lstat");
            printf("%s: path error\n", argv[i]);
            continue;;
        }

        printf("%s: ", argv[i]);

        switch (filestat.st_mode & __S_IFMT){
            case __S_IFIFO:
                printf("named pipe (fifo)\n");               
                break;
            case __S_IFCHR:
                printf("character special file\n");  
                break;
            case __S_IFDIR:  
                printf("directory\n");               
                break;
            case __S_IFBLK:  
                printf("block special\n");            
                break;
            case __S_IFREG:  
                printf("regular file\n");            
                break;
            case __S_IFLNK:  
                printf("symbolic link\n");                
                break;
            case __S_IFSOCK: 
                printf("socket\n");                 
                break;
            default:       
                printf("unknown?\n");                
                break;
        }
    } 

    return 0; 
}