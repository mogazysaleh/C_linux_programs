//this program prints the info about partitions on an MBR disk

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _LARGEFILE64_SOURCE
#define SECTOR_SIZE 512
#define PARTITIONS_COUNT 4
#define PARTITIONS_OFFSET 0x01BE
#define ID_OFFSET 0x04
#define LBA_OFFSET 0x08
#define SECTORS_COUNT_OFFSET 0x0C
#define EXT_ID_CHS 0x05
#define EXT_ID_LBA 0x0F

int main(int argc, char** argv)
{
    FILE *fptr;
    unsigned char buffer[512];
    unsigned char EBR_buffer[512];
    unsigned int *firstSector;
    unsigned int *logicalSector;
    unsigned int *numberOfSectors;
    unsigned int *logicalSectorsCnt;
    unsigned char id;
    unsigned char LogicalID;
    unsigned int EBR_position;
    unsigned long long EBRSector;
    unsigned int *nextEBR;
    int j;

    if(argc != 2)
    {
        printf("Please enter the name of device only\n");
        exit(EXIT_FAILURE);
    }

    //opening the disk file
    if((fptr = fopen64(argv[1], "r")) == NULL)
    {
        perror("");
        exit(EXIT_FAILURE);
    }
    
    //reading MBR sector into buffer
    fread(buffer, sizeof(char), SECTOR_SIZE, fptr);


    //printing partitions
    printf("%-12s%-15s%-20s%-14s%-17s%-5s\n", "PARTITION", "FIRST SECTOR", "NUMBER OF SECTORS", "LAST SECTOR", "TOTAL SIZE(MB)", "ID");
    for(int i = 0; i < PARTITIONS_COUNT; i++)
    {
        //musl be a valid entry
        if( !(buffer[PARTITIONS_OFFSET + (i * 16)] == 0x80 || buffer[PARTITIONS_OFFSET + (i * 16)] == 0x00) )
            continue;

        //number of first sector of the partition
        firstSector = (unsigned int *)(&buffer[PARTITIONS_OFFSET + (i * 16) + LBA_OFFSET]);

        //do not print unused partitions
        if(!(*firstSector)) 
            continue;

        //number of sectors the partition occupates
        numberOfSectors = (unsigned int *)(&buffer[PARTITIONS_OFFSET + (i * 16) + SECTORS_COUNT_OFFSET]);

        //partition type
        id = buffer[PARTITIONS_OFFSET + (i * 16) + ID_OFFSET];

        //printing the partition stats
        printf("%-12d%-15u%-20u%-14u%-17u%-5x\n", i+1, *firstSector,
        *numberOfSectors, (*firstSector)+(*numberOfSectors) - 1, (*numberOfSectors)/2/1024, id);

        //printing logical partitions (if exist)
        if(id == EXT_ID_CHS || id == EXT_ID_LBA)
        {
            j = i + 1;
            EBRSector = (long long)(*firstSector);
            do
            {
                j++;
                long long sect =  EBRSector * (long long)(SECTOR_SIZE);
                fseek(fptr, sect, SEEK_SET);
                fread(EBR_buffer, sizeof(char), SECTOR_SIZE, fptr);

                LogicalID = EBR_buffer[PARTITIONS_OFFSET + ID_OFFSET];
                logicalSector = (unsigned int *)(&EBR_buffer[PARTITIONS_OFFSET + LBA_OFFSET]);
                logicalSectorsCnt = (unsigned int *)(&EBR_buffer[PARTITIONS_OFFSET + SECTORS_COUNT_OFFSET]);


                printf("%-12d%-15llu%-20u%-14llu%-17u%-5x\n", j, (long long)(*logicalSector) + EBRSector, *logicalSectorsCnt,
                (long long)(*logicalSector)+ (long long)(*logicalSectorsCnt) - 1 + EBRSector, (*logicalSectorsCnt)/2/1024, LogicalID);

                //getting next EBR
                nextEBR = (unsigned int *)(&EBR_buffer[0x01CE + 8]);
                EBRSector = EBRSector + *nextEBR;
            } while (*nextEBR);

        }
    }

    //closing the file stream
    fclose(fptr);
}