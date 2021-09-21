#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

typedef struct dirent dirent;
typedef struct tm datetime;
typedef struct stat statStruct;
typedef struct passwd passwd;
typedef struct group group;
typedef long unsigned lu;



bool i_flag = false;
bool d_flag = false;
bool p_flag = false;
bool l_flag = false;
bool s_flag = false;
bool t_flag = false;

lu inum;
int perm;
int size_int;
int sizeComp = 0;
int size_multiplier;
char type;



char *months[] = {"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"};

void print_BFS(char *rootDir);
void print_DFS(char *rootDir);
bool checkSize(lu size);
void printPermissions(mode_t mode);
bool checkPerm(mode_t mode);
void printEntryInfo(char *path);

int main(int argc, char *argv[])
{
    char rootDir[PATH_MAX];
    char opt;
    char size_arg[10];
    char size_str[10];
    int size_length;

    while((opt = getopt(argc, argv, "i:dp:ls:t:")) != -1)
    {
        switch (opt)
        {
        case 'i':
            i_flag = true;
            inum = atoi(optarg);
            break;
        case 'd':
            d_flag = true;
            break;
        case 'p':
            p_flag = true;
            if(strlen(optarg) != 3)
            {
                puts("Invalid mode\n");
                exit(EXIT_FAILURE);
            }
            perm = ((optarg[0] - '0') << 6) + ((optarg[0] - '0') << 3) + ((optarg[0] - '0'));
            break;
        case 'l':
            l_flag = true;
            break;
        case 's':
            s_flag = true;
            strcpy(size_arg, optarg);
            break;
        case 't':
            t_flag = true;
            if(strlen(optarg) > 1)
            {
                puts("-t only accepts one character as argument: either f or d");
                exit(EXIT_FAILURE);
            }
            type = *optarg;
            if(!(type == 'f' || type == 'd'))
            {
                puts("-t only accepts one character as argument: either f or d");
                exit(EXIT_FAILURE);
            }
            break;
        default:
            exit(EXIT_FAILURE);
            break;
        }
    }

    if(optind >= argc)
        strcpy(rootDir, ".");
    else
        strcpy(rootDir, argv[optind]);

    if(s_flag)
    {
        size_length = strlen(size_arg);
        size_int = atoi(size_arg);
        if(size_arg[0] == '+') sizeComp = 1;
        else if(size_arg[0] == '-')
        {
            sizeComp = -1;
            size_int = size_int * -1;
        }

        if(size_arg[size_length - 1] == 'M') size_multiplier = 1024 * 1024;
        else if(size_arg[size_length - 1] == 'G') size_multiplier = 1024 * 1024 * 1024;
        else if(size_arg[size_length - 1] == 'b') size_multiplier = 512;
        else if(size_arg[size_length - 1] == 'c') size_multiplier = 1;
        else
        {
            printf("You must specify the unit of size\n");
            exit(EXIT_FAILURE);
        }
    }
    if(d_flag)
        print_DFS(rootDir);
    else
        print_BFS(rootDir);
}

void print_BFS(char* rootDir)
{
    DIR* dir;
    dirent *dt;
    char newDir[PATH_MAX];

    printEntryInfo(rootDir);
    
    if((dir = opendir(rootDir)) == NULL)
        return;

    if(rootDir[strlen(rootDir) - 1] != '/')
        strcat(rootDir, "/");


    while (dt = readdir(dir))
    {
        if((strcmp(dt->d_name, ".") == 0) || (strcmp(dt->d_name, "..") == 0))
            continue;
        strcpy(newDir, rootDir);
        strcat(newDir, dt->d_name);
        print_BFS(newDir);
    }
}

void print_DFS(char *rootDir)
{
    DIR* dir;
    dirent *dt;
    char newDir[PATH_MAX];
    
    if((dir = opendir(rootDir)) == NULL)
    {
        printEntryInfo(rootDir);
        return;
    }

    while (dt = readdir(dir))
    {
        if((strcmp(dt->d_name, ".") == 0) || (strcmp(dt->d_name, "..") == 0))
            continue;
        strcpy(newDir, rootDir);
        if(newDir[strlen(newDir) - 1] != '/')
            strcat(newDir, "/");
        strcat(newDir, dt->d_name);
        print_DFS(newDir);
    }
    
    printEntryInfo(rootDir);
}

void printEntryInfo(char *path)
{
    statStruct st;
    datetime *dt;

    if(stat(path, &st) == -1)
        return;
    

    if(i_flag && (st.st_ino != inum))
        return;

    if(t_flag && (S_ISDIR(st.st_mode) != (type == 'd')))
        return;
    
    if(s_flag && (checkSize((st.st_size + size_multiplier - 1) / size_multiplier) == false))
        return;

    if(p_flag && (checkPerm(st.st_mode) == false) ) 
        return;
    if(l_flag)
    {
        //printing inode
        printf("%10lu ", st.st_ino);

        //printing number of blocks
        printf("%9lu ", (st.st_size + 1023) / 1024);

        //printing file type
        if(S_ISDIR(st.st_mode))
            printf("d");
        else
            printf("-");

        //printing permissions
        printPermissions(st.st_mode);
        printf("  ");
        
        //printing number of links
        printf("%3lu ", st.st_nlink);

        //printing username
        printf("%s ", ((passwd*)getpwuid(st.st_uid))->pw_name);

        //printing group name
        printf("%s ", ((passwd*)getgrgid(st.st_gid))->pw_name);

        //printing size
        printf("%10lu ", st.st_size);

        //printing date and time
        dt = localtime(&(st.st_mtime));
        printf("%3s %2d %02d:%02d ", months[dt->tm_mon], dt->tm_mday, dt->tm_hour, dt->tm_min);
    }

    //printing name
    printf("%s\n", path);
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

bool checkSize(lu size)
{
    switch (sizeComp)
    {
    case 0:
        if(size == (unsigned long)size_int) return true;
        else return false;
        break;
    case 1:
        if(size > (unsigned long)size_int) return true;
        else return false;
        break;
    case -1:
        if(size < (unsigned long)size_int) return true;
        else return false;
        break;
    }
}

bool checkPerm(mode_t mode)
{
    if((mode &  S_IRUSR) != (perm & S_IRUSR) ) return false;
    if((mode &  S_IWUSR) != (perm & S_IWUSR) ) return false;
    if((mode &  S_IXUSR) != (perm & S_IXUSR) ) return false;
    if((mode &  S_IRGRP) != (perm & S_IRGRP) ) return false;
    if((mode &  S_IWGRP) != (perm & S_IWGRP) ) return false;
    if((mode &  S_IXGRP) != (perm & S_IXGRP) ) return false;
    if((mode &  S_IROTH) != (perm & S_IROTH) ) return false;
    if((mode &  S_IWOTH) != (perm & S_IWOTH) ) return false;
    if((mode &  S_IXOTH) != (perm & S_IXOTH) ) return false;

    return true;
}