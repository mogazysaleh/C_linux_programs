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

bool a_flag = false; //print entries that start with .
bool i_flag = false; //print inode
bool l_flag = false; //print long list format
bool r_flag = false; //reverse the sorting
bool u_flag = false; //display last access time
bool t_flag = false; //sort by time
bool c_flag = false; //display last status change time

char *months[12] = {"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"};

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

//comparator function to sort alphabetically according to name
int comparator_name(const void *p1, const void *p2)
{
    fileStat *lp1=*(fileStat **)p1;
    fileStat *lp2=*(fileStat **)p2;
    if(r_flag)
        return -1 * strcmp((lp1)->name, lp2->name);
    else
        return strcmp((lp1)->name, lp2->name);
}

//comparator function to sort by time
int comparator_time(const void *p1, const void *p2)
{
    fileStat *lp1=*(fileStat **)p1;
    fileStat *lp2=*(fileStat **)p2;
    time_t t1;
    time_t t2;
    if(u_flag) //sort by access time if u option is entered
    {
        t1 = lp1->st.st_atim.tv_sec;
        t2 = lp2->st.st_atim.tv_sec;
    }
    else if(c_flag) //sort by status change time if u option is entered
    {
        t1 = lp1->st.st_ctim.tv_sec;
        t2 = lp2->st.st_ctim.tv_sec;
    }
    else //sort by time of last modification
    {
        t1 = lp1->st.st_mtim.tv_sec;
        t2 = lp2->st.st_mtim.tv_sec;
    }

    //reverse if r option is entered
    if(r_flag)
        return t1 - t2;
    else
        return t2 - t1;
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
        if(u_flag)
            dt = localtime(&(filePtr->st.st_atime));
        else
            dt = localtime(&(filePtr->st.st_mtime));
        printf("%3s %2d %d %02d:%02d  ", months[dt->tm_mon], dt->tm_mday, dt->tm_year + 1900, dt->tm_hour, dt->tm_min);

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
    int (*comparator)(const void *, const void *);

    //setting flags for options
    while((opt = getopt(argc, argv, "ailrutc")) != -1)
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
        case 'r':
            r_flag = true;
            break;
        case 'u':
            u_flag = true;
            c_flag = false;
            break;
        case 't':
            t_flag = true;
            break;
        case 'c':
            c_flag = true;
            u_flag = false;
        default:
            exit(EXIT_FAILURE);
            break;
        }
    }

    if(optind >= argc)
    {
        strcpy(dirPath, ".");
    }
    else
    {
        strcpy(dirPath, argv[optind]);
    }
    
    //reading the directory and checking for its validity
    if((dir = opendir(dirPath)) == NULL)
    {
        perror("");
        exit(EXIT_FAILURE);
    }



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

    //sorting the entries based on name or time
    if(t_flag)
        comparator = comparator_time;
    else
        comparator = comparator_name;

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