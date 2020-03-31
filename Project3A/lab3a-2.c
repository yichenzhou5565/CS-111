#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include "ext2_fs.h"

/* Globla Variables */
int fd;
struct ext2_super_block super_block;
unsigned int block_size;


/* Functions */
void superblock();          /* Print out superblock summary */
void group();               /* Print out group summary */
void freeblock(unsigned int blk, int group_no);           /* If free, print summary */
void free_inode(unsigned int blk, int group_no, unsigned int inode_table);          /* If inode free, print summary */
void inode();               /* If inode allocated, print summary */
void directory_entry();
void single_indirect();
void double_indirect();
void triple_indirect();

void get_time (uint32_t timestamp, char* buf) {
    time_t rawtime = timestamp;
    struct tm *info;
    time(&rawtime);
    info = gmtime(&rawtime);
    strftime(buf, 80, "%m/%d/%y %H:%M:%S", &info);
    //reference: https://www.tutorialspoint.com/c_standard_library/c_function_gmtime.htm
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Invalid Number of arguments.\n");
        exit(1);
    }
    
    /* Open the image */
    fd = open(argv[1], O_RDONLY);
    if (fd < 0)
    {
        fprintf(stderr, "Error in open() system call. %s\n", strerror(errno));
        exit(1);
    }
    
    /* Print summaries */
    superblock();
    group();
    
    return (0);
}

void superblock()
{
    if (pread(fd, &super_block, sizeof(struct ext2_super_block), 1024) == -1)
    {
        fprintf (stderr, "Error in pread() system call. %s\n", strerror(errno));
        exit(1);
    }
    block_size = EXT2_MIN_BLOCK_SIZE << super_block.s_log_block_size;
    fprintf(stdout, "SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n",
            super_block.s_blocks_count,      /* 2.number of blocks */
            super_block.s_inodes_count,      /* 3.number of inodes */
            EXT2_MIN_BLOCK_SIZE << super_block.s_log_block_size,
                                             /* 4.block size */
            super_block.s_inode_size,        /* 5.inode size */
            super_block.s_blocks_per_group,  /* 6.block per group */
            super_block.s_inodes_per_group,  /* 7.inode per group */
            super_block.s_first_ino          /* 8.first non-reserved inode */
            );
}

void group()
{
    int group_num = 1+super_block.s_blocks_count / super_block.s_blocks_per_group;
    int i=0;
    while ( i < group_num )
    {
        struct ext2_group_desc group_desc;
        
        /* read */
        if (pread(fd, &group_desc, sizeof(struct ext2_group_desc), 2048+i*(sizeof(group_desc))) < 0)
        {
            fprintf (stderr, "Error in pread() systen call. %s\n", strerror(errno));
            exit(1);
        }
        
        /* total number of blocks and number of inodes */
        int block_num = super_block.s_blocks_per_group;
        int inode_num = super_block.s_inodes_count;
        if (i + 1 == group_num)
        {
            block_num = super_block.s_blocks_count - (group_num-1)*block_num;
            inode_num = super_block.s_inodes_count - (group_num-1)*inode_num;
        }
        
        /* Output */
        fprintf(stdout, "GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n",
                i,                      /* 2.group number */
                block_num,              /* 3.# of blk in this group */
                inode_num,              /* 4.# of inode in this group */
                group_desc.bg_free_blocks_count,    /* 5.free blk cnt */
                group_desc.bg_free_inodes_count,    /* 6.free inode cnt */
                group_desc.bg_block_bitmap,         /* 7.blk bitmap */
                group_desc.bg_inode_bitmap,         /* 8.inode bitmap */
                group_desc.bg_inode_table           /* 9.1st blk of inode */
                );
        
        /* print freeblock for THIS group */
        freeblock(group_desc.bg_block_bitmap, i);
        
        /* print free inode for THIS group */
        free_inode(group_desc.bg_inode_bitmap, i, group_desc.bg_inode_table);
        
        i++;
    }
}


void freeblock(unsigned int blk, int group_no)
{
    /*  1: used
        0: free */
    int bsize = EXT2_MIN_BLOCK_SIZE << super_block.s_log_block_size;
    int j = 0;
    
    /* freeblock number */
    int free_blk_no = group_no * super_block.s_blocks_per_group + super_block.s_first_data_block;
    
    /* read the whole block */
    char* buffer = (char*)malloc(sizeof(bsize));
    if (pread(fd, buffer, bsize, 1024+bsize*(blk-1))<0)
    {
        fprintf(stderr, "Error in pread() system call. %s\n", strerror(errno));
        exit(1);
    }
    
    while (j < bsize)
    {
        int k = 0;
        while ( k < 8 )         /* 1 byte = 8 bits */
        {
            if ((buffer[j] & 1) == 0 )
            {
                fprintf(stdout, "BFREE,%d\n", free_blk_no);
            }
            buffer[j] = buffer[j] >> 1;
            free_blk_no += 1;
            k++;
        }
        
        j++;
    }
    free(buffer);
}


void free_inode(unsigned int blk, int group_no, unsigned int inode_table)
{
    /* inode number */
    int free_inode_no = group_no * super_block.s_inodes_per_group + 1;
    int tmp = free_inode_no;
    int isize = super_block.s_inodes_per_group >> 3;
    int bsize = EXT2_MIN_BLOCK_SIZE << super_block.s_log_block_size;
    int j = 0;
    
    /* read */
    char* buffer = malloc(isize);
    if ( pread(fd, buffer, isize, 1024+bsize*(blk-1)) < 0 )
    {
        fprintf(stderr, "Error in pread() system call. %s\n", strerror(errno));
        exit(1);
    }
    
    while (j < isize)
    {
        int k = 0;
        while ( k < 8 )
        {
            if ((buffer[j] & 1) == 0 )
            {
                fprintf(stdout, "IFREE,%d\n", free_inode_no);
            }
            else
            {
                unsigned int inode_num = free_inode_no;
                unsigned int inode_index = free_inode_no - tmp;
                inode(inode_table, inode_num, inode_index);
            }
            buffer[j] = buffer[j] >> 1;
            free_inode_no += 1;
            k++;
        }
        j++;
    }
    
    free(buffer);
}

//inode table, inode_number, index in inode table
void inode(unsigned int inode_table, unsigned int inode_num, unsigned int inode_index)
{
    /*
    struct ext2_inode inode;
    unsigned long offset = 1024 + (inode_table-1) * block_size + inode_index * sizeof(inode);
    pread(fd,&inode,sizeof(struct ext2_inode), offset);
    
    char filetype = '?'; //anything else
    uint16_t type = inode.i_mode & 0xF000;
    if (type == 0xa000) { //symbolic link
        filetype = 's';
    } else if (type == 0x8000) { //regular file
        filetype = 'f';
    } else if (type == 0x4000) { //directory
        filetype = 'd';
    }
    uint16_t mode = inode.i_mode & 0x0FFF; //"%o"
    uint16_t owner = inode.i_uid;
    uint16_t group = inode.i_gid;
    uint16_t link_count = inode.i_links_count;
    char ctime[32];
    get_time (inode.i_ctime, ctime);
    char mtime[32];
    get_time (inode.i_mtime, mtime);
    char atime[32];
    get_time (inode.i_atime, atime);
    uint32_t file_size = inode.i_size;
    uint32_t numBlocks = inode.i_blocks;
    fprintf(stdout, "INODE,%d,%c,%o,%u,%u,%u,%s,%s,%s,%d,%d", inode_num, type, mode, owner, group, link_count, ctime, mtime, atime, file_size, numBlocks);
    
    int i;
    for (i = 0; i < 15; i++) { //block addresses
        fprintf(stdout, ",%d", inode.i_block[i]);
    }
    fprintf(stdout, "\n");
    
    for (i = 0; i < 12; i++) { //scan first 12 direct entries
        if (inode.i_block[i] != 0 && filetype == 'd') {
            directory_entry(inode_num, inode.i_block[i]);
        }
    }
    if(inode.i_block[12] != 0) //13th
        single_indirect();
    if(inode.i_block[13] != 0) //14th
        double_indirect();
    if(inode.i_block[14] != 0) //15th
        triple_indirect();
     */
     
}
 

void directory_entry() {
    
}

void single_indirect() {
    
}

void triple_indirect() {
    
}
