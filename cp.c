#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <dirent.h>
#include <string.h>

typedef struct stat structStat;

void copyFile(char *srcPath, char *distPath)
{
    FILE *src;
    FILE *dist;
    structStat srcStat;
    char c;

    if(stat(srcPath, &srcStat) == -1)
        return;

    if((src = fopen(srcPath, "r")) == NULL)
        return;
    if((dist = fopen(distPath, "w")) == NULL)
        return;

    while((c = fgetc(src)) != EOF)
        fputc(c, dist);
    
    chmod(distPath, srcStat.st_mode); //This will make all file permissions the same which is different from cp
}

void addFilename(char *srcPath, char* distPath)
{
    char *path = basename(srcPath);
    if(distPath[strlen(distPath) - 1] != '/')
        strcat(distPath, "/");
    strcat(distPath, path);
}

int main(int argc, char **argv)
{
    structStat src;
    structStat dist;
    char distpath[PATH_MAX];

    if(argc != 3)
    {
        printf("USAGE: cp <source> <distination>\n");
        exit(EXIT_FAILURE);
    }
    
    if(stat(argv[1], &src) == -1)
    {
        printf("There is no file or directory named %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    if(S_ISDIR(src.st_mode)) //src is directory
    {

    }
    else //src is file
    {
        if(stat(argv[2], &dist) == -1) //distination is none
        {
            copyFile(argv[1], argv[2]);
        }
        else if(S_ISDIR(dist.st_mode)) //dist is a directory
        {
            strcpy(distpath, argv[2]);
            addFilename(argv[1], distpath);
            puts(distpath);
            copyFile(argv[1], distpath);
        }
        else //dist is a file
        {
            copyFile(argv[1], argv[2]);
        }
    }

}