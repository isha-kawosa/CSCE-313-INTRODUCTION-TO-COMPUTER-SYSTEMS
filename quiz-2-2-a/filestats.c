#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <pwd.h>
#include <dirent.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    // todo
    if(argc !=2){
        printf("usage: %s directory_name\n", argv[0]);
        exit(1);
    }

    DIR *dir =opendir(argv[1]);
    if(!dir){
        printf("can’t open %s\n", argv[1]);
        exit(2);
    }

    struct dirent *entry;
    while((entry =readdir(dir))!=NULL){
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", argv[1], entry->d_name);

        struct stat filestat;
        if(lstat(path, &filestat)==-1){
            perror("lstat");
            continue;
        }
        
        if(S_ISLNK(filestat.st_mode)) {
            if(stat(path, &filestat) == -1){
                perror("stat");
                continue;
            }
        }


        printf("File name: %s\n", entry->d_name); //filestat.st_dev
        printf("inode number: %lu\n", (unsigned long)filestat.st_ino);
        printf("number of links: %lu\n", (unsigned long)filestat.st_nlink);
        printf("User ID of owner: %d\n", filestat.st_uid);
        printf("Group ID of owner: %d\n", filestat.st_gid);
        printf("Size in bytes: %lld bytes\n", (long long)filestat.st_size);
        printf("Last access: %s", ctime(&filestat.st_atime));
        printf("Last modification: %s", ctime(&filestat.st_mtime));
        printf("Last status change: %s", ctime(&filestat.st_ctime));
        printf("Number of disk blocks allocated: %lld\n", (long long)filestat.st_blocks);
        printf("Access mode in octal: %o\n", filestat.st_mode); 
    
    char permissions[10];
        snprintf(permissions, sizeof(permissions), "%c%c%c%c%c%c%c%c%c",
                 filestat.st_mode & S_IRUSR ? 'r' : '-',
                 filestat.st_mode & S_IWUSR ? 'w' : '-',
                 filestat.st_mode & S_IXUSR ? 'x' : '-',
                 filestat.st_mode & S_IRGRP ? 'r' : '-',
                 filestat.st_mode & S_IWGRP ? 'w' : '-',
                 filestat.st_mode & S_IXGRP ? 'x' : '-',
                 filestat.st_mode & S_IROTH ? 'r' : '-',
                 filestat.st_mode & S_IWOTH ? 'w' : '-',
                 filestat.st_mode & S_IXOTH ? 'x' : '-');
        printf("Access mode flags: %s\n\n", permissions);
    }
    closedir(dir);
    return 0;
}