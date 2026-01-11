#include <sd.h>
#include <io.h>
#include <terminal.h>
#include <delay.h>
#include <mem.h>
#include <storage.h>

extern unsigned char _end;

static unsigned int partitionlba = 0;

typedef struct {
    char            BS_JmpBoot[3];
    char            BS_OEMName[8];
    unsigned short  BPB_BytsPerSec;
    unsigned char   BPB_SecPerClus;
    unsigned short  BPB_RsvdSecCnt;
    unsigned char   BPB_NumFATs;
    unsigned short  BPB_RootEntCnt;
    unsigned short  BPB_TotSec16;
    unsigned char   BPB_Media;
    unsigned short  BPB_FATSz16;
    unsigned short  BPB_SecPerTrk;
    unsigned short  BPB_NumHeads;
    unsigned int    BPB_HiddSec;
    unsigned int    BPB_TotSec32;

    unsigned int    BPB_FATSz32;
    unsigned short  BPB_ExtFlags;
    unsigned short  BPB_FSVer;
    unsigned int    BPB_RootClus;
    unsigned short  BPB_FSInfo;
    unsigned short  BPB_BkBootSec;
    unsigned char   BPB_Reserved[12];
    unsigned char   BS_DrvNum;
    unsigned char   BS_Reserved;
    unsigned char   BS_BootSig;
    unsigned int    BS_VolID;
    unsigned char   BS_VolLab[11];
    unsigned char   BS_FilSysType[8];
} __attribute__((packed)) bpb_t;

typedef struct {
    char           name[8];
    unsigned char  ext[3];
    unsigned char  attr;
    unsigned char  nt_reserved;
    unsigned char  creation_time_tenths;
    unsigned short creation_time;
    unsigned short creation_date;
    unsigned short last_access_date;
    unsigned short cluster_high;
    unsigned short write_time;
    unsigned short write_date;
    unsigned short cluster_low;
    unsigned int   size;
} __attribute__((packed)) fatdir_t;

int fatGetPartition(uint8_t * buffer) {
    unsigned char *mbr=buffer;
    bpb_t *bpb=(bpb_t*)buffer;

    if (sdReadblock(0, buffer, 1)) {
        //check Master Boot Record for magic signature
        if (mbr[510] != 0x55 || mbr[511] != 0xAA) {
            logTerminal("ERROR: Magic Signature invalid in Master Boot Record (MBR)");
            return 0;
        }

        //check partition type, Fat 16 LBA, Fat 32 LBA
        if (mbr[0x1C2] != 0xE && mbr[0x1C2] != 0xC) {
            logTerminal("ERROR: Wrong partition type");
            return 0;
        }

        pushTerminal("Master Boot Record disk identifier: ");
        pushTerminalHex(*((unsigned int*)((unsigned long)buffer+0x1B8)));
        pushTerminal("\nFAT partition starts at: ");
        
        //partitionlba=*((unsigned int*)((unsigned long)&_end+0x1C6));
        //454 offset size: 4 Partition start sector in 32-bit LBA (1 - 0xFFFFFFFF).
        partitionlba = mbr[0x1C6] + (mbr[0x1C7]<<8) + (mbr[0x1C8]<<16) + (mbr[0x1C9]<<24);

        pushTerminalHex(partitionlba);
        pushTerminalChar('\n');

        if (!sdReadblock(partitionlba, buffer, 1)) {
            logTerminal("ERROR: Unable to read boot record");
            return 0;
        }

        //check file system type
        if (!(bpb->BS_FilSysType[0] == 'F' && bpb->BS_FilSysType[1] == 'A' && bpb->BS_FilSysType[2])) {
            
            logTerminal("ERROR: Unknown file system type");
            return 0;
        }
        pushTerminal("FAT type: ");
        //if the 16 bit sector per fat is 0 then it is FAT32
        pushTerminal(bpb->BPB_FATSz16 > 0 ? "FAT16\n" : "FAT32\n");

        return 1;
    }

    return 0;
}

void fatListDirectory(uint8_t * buffer) {
    bpb_t *bpb=(bpb_t*)buffer;
    fatdir_t *dir=(fatdir_t*)buffer;
    unsigned int rootSector, numClusters;

    //find root directory's lba
    rootSector = bpb->BPB_RsvdSecCnt + (bpb->BPB_FATSz32 * bpb->BPB_NumFATs);
    numClusters = (bpb->BPB_TotSec32 - bpb->BPB_RsvdSecCnt - bpb->BPB_NumFATs * bpb->BPB_FATSz32) / bpb->BPB_SecPerClus;

    //s = bpb->BPB_RootEntCnt;
    
    pushTerminal("FAT number of clusters: ");
    pushTerminalHex(numClusters);
    pushTerminalChar('\n');

    //s *= sizeof(fatdir_t);
    if (bpb->BPB_FATSz16 > 0) {logTerminal("ERROR: FAT 16 Unsupported"); return;} //FAT 16
        
    rootSector += (bpb->BPB_RootClus - 2) * bpb->BPB_SecPerClus;

    //add partition lba
    rootSector += partitionlba;

    pushTerminal("FAT root directory LBA: ");
    pushTerminalHex(rootSector);
    pushTerminalChar('\n');

    //load root directory
    if (sdReadblock(rootSector, buffer, numClusters/512 + 1)) {
        logTerminal("\nAttribute    Cluster    Size    Name");
        //go through each entry

        //no local iterator needed
        for (; dir->name[0] != 0; dir++) {
            //is valid
            if (dir->name[0] == 0xE5 || dir->attr == 0x0F) {continue;}
            if ((unsigned int)(dir->cluster_low + (unsigned int)(dir->cluster_high<<16)) > numClusters) {continue;}

            //attributes
            pushTerminalChar(dir->attr & 1  ? 'R' : '.'); //read only
            pushTerminalChar(dir->attr & 2  ? 'H' : '.'); //hidden
            pushTerminalChar(dir->attr & 4  ? 'S' : '.'); //system
            pushTerminalChar(dir->attr & 8  ? 'L' : '.'); //volume label
            pushTerminalChar(dir->attr & 16 ? 'D' : '.'); //directory
            pushTerminalChar(dir->attr & 32 ? 'A' : '.'); //archive
            pushTerminalChar(' ');

            //cluster
            pushTerminalHex((unsigned int)dir->cluster_high);
            pushTerminalChar(' ');
            pushTerminalHex((unsigned int)dir->cluster_low);
            pushTerminalChar(' ');

            //size
            pushTerminalHex(dir->size);
            pushTerminalChar(' ');

            //file name
            dir->attr = 0;
            pushTerminal(dir->name);
            //pushTerminalChar('.');
            //pushTerminal(dir->ext);
            pushTerminalChar('\n');

            push_storage_window(dir->name);
        }
    } else {
        logTerminal("ERROR: Unable to load root directory");
    }
}