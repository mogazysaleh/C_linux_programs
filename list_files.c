#include <dirent.h> //opendir(), readdir(), DIR, struct dirent
#include <stdio.h>
#include <stdlib.h> 
#include <stdbool.h> //to define the boolean in c
#include <unistd.h> // getopt()
#include <sys/stat.h> // stat()
#include <string.h> //strcpy(), strcat()
#include <pwd.h> //passwd
#include <grp.h> //grp
#include <time.h> //datetime
#include <sys/types.h>

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

typedef struct dirent dirent;
typedef struct stat statStruct;
typedef struct passwd passwd;
typedef struct group group;
typedef struct tm datetime;
typedef unsigned long ul;

bool a_flag = false;
bool i_flag = false;
bool l_flag = false;
int length_nlink = 0;
int length_size = 0;
int length_inode = 0;

typedef struct mystat{
     char name[PATH_MAX];
     statStruct st;
}fileStat;

int lengthOfInt(ul n)
{
    int length = 0;
    while(n != 0)
    {
        n /= 10;
        length++;
    }
    return length;
}

void printPermissions(mode_t mode)
{
    if(mode &  S_IRUSR ) printf("r"); else printf("-");
    if(mode &  S_IWUSR ) printf("w"); else printf("-");
    if(mode &  S_IXUSR ) printf("x"); else printf("-");
    if(mode &  S_IRGRP ) printf("r"); else printf("-");
    if(mode &  S_IWGRP ) printf("w"); else printf("-");
    if(mode &  S_IXGRP ) printf("x"); else printf("-");
    if(mode &  S_IROTH ) printf("r"); else printf("-");
    if(mode &  S_IWOTH ) printf("w"); else printf("-");
    if(mode &  S_IXOTH ) printf("x"); else printf("-");
}

//comparator function to be used in qsort() function
int comparator(const void *p1, const void *p2)
{
    fileStat *lp1=*(fileStat **)p1;
    fileStat *lp2=*(fileStat **)p2;

    return strcmp((lp1)->name, lp2->name);
}

void printEntry(fileStat* filePtr)
{
    datetime *dt;

    //skipping directories that start with '.' if the a flag is not present
    if(filePtr->name[0] == '.' && !a_flag)
        return;
    
    //printing inode if i option is present
    if(i_flag)
        printf("%*ld  ", length_inode, filePtr->st.st_ino);

    //printing long list if l option is present
    if(l_flag)
    {
        //printin file type
            if(S_ISDIR(filePtr->st.st_mode) ){
                printf("d");
            }
            else {
                printf("-");
            }
        
        //printing permissions
        printPermissions(filePtr->st.st_mode);
        printf("  ");

        //printing no of links
        printf("%*ld ", length_nlink, filePtr->st.st_nlink);

        //printing user name
        printf("%s ", ((passwd*)getpwuid(filePtr->st.st_uid))->pw_name);

        //printing group name
        printf("%s ", ((group*)getgrgid(filePtr->st.st_uid))->gr_name);

        //printing size
        printf("%*ld ", length_size, filePtr->st.st_size);

        //printing last modification date and time
        dt = localtime(&(filePtr->st.st_mtime));
        printf("%02d-%02d-%d %02d:%02d  ", dt->tm_mday, dt->tm_mon + 1, dt->tm_year + 1900, dt->tm_hour, dt->tm_min);

        //printing name of file
        printf("%s  \n",filePtr->name);
    }
    else
    {
        printf("%s  ", filePtr->name);
    }
        

}

int main(int argc, char **argv)
{
    DIR *dir;
    dirent *dp;
    statStruct st;
    fileStat **dirEntries;
    fileStat *entryPtr;
    char filePath[PATH_MAX];
    char dirPath[PATH_MAX];
    int dirEntriesCnt = 0;
    int opt;
    int index = 0;

    if(argc == 1)
    {
        printf("Please enter the directory\n");
        exit(EXIT_FAILURE);
    }

    //setting flags for options
    while((opt = getopt(argc, argv, "ail")) != -1)
    {
        switch (opt)
        {
        case 'a':
            a_flag = true;
            break;
        case 'i':
            i_flag = true;
            break;
        case 'l':
            l_flag = true;
            break;
        }
    }

    if(optind >= argc)
    {
        printf("Expected argument after options\nUsage: %s [-a] [-i] [-l] direcotry_name\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    //reading the directory and checking for its validity
    if((dir = opendir(argv[optind])) == NULL)
    {
        perror("");
        exit(EXIT_FAILURE);
    }

    strcpy(dirPath, argv[optind]);

    //getting the count of entries in the directory
    while(readdir(dir))
        dirEntriesCnt++;

    //returning pointer of the stream to the beginning
    rewinddir(dir);

    //allocating dynamic memorty for the fileStat struct with the number of entries
    dirEntries = (fileStat**)malloc(dirEntriesCnt * sizeof(fileStat*));



    //reading entries of the directory
    while(dp = readdir(dir))
    {
        //starting the file path with directory path
        strcpy(filePath, dirPath);

        //adding forward slash if not there
        if(filePath[strlen(filePath) - 1] != '/')
            strcat(filePath, "/");
        
        //adding the name of file to path
        strcat(filePath, dp->d_name);
        
        //stating the information about file
        if(stat(filePath, &st) == -1)
        {
            perror("");
            exit(EXIT_FAILURE);
        }

        dirEntries[index] = (fileStat*)malloc(sizeof(fileStat));
        strcpy(dirEntries[index]->name, dp->d_name);
        dirEntries[index]->st = st;
        index++;

        //getting lengths of the info while printing
        length_inode = max(lengthOfInt(st.st_ino),length_inode);
        length_nlink = max(lengthOfInt(st.st_nlink),length_nlink);
        length_size = max(lengthOfInt(st.st_size),length_size);

    }

    //sorting the entries based on name
    qsort(dirEntries, dirEntriesCnt, sizeof(fileStat*), comparator);

    //printing the entries
    for(int i = 0; i < dirEntriesCnt; i++)
    {
        printEntry(dirEntries[i]);
    }

    //adding a line break in case we are printing only one line
    if(!l_flag)
        printf("\n");
    
    //closing the direcetory stream
    closedir(dir);
}