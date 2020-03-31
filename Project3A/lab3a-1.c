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

/* Functions */
void superblock();          /* Print out superblock summary */
void group();               /* Print out group summary */
void freeblock();           /* If free, print summary */
void free_inode();          /* If inode free, print summary */
void inode();               /* If inode allocated, print summary */

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
    freeblock();
    
    return (0);
}

void superblock()
{
    if (pread(fd, &super_block, sizeof(struct ext2_super_block), 1024) == -1)
    {
        fprintf (stderr, "Error in pread() system call. %s\n", strerror(errno));
        exit(1);
    }
    
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
            fprintf (stderr, "Error in pread() systen call.%s\n", strerror(errno));
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
        
        i++;
    }
}


void freeblock()
{
    /*  1: used
        0: free */
    
    int group_num = 1+super_block.s_blocks_count / super_block.s_blocks_per_group;
    int i=0;
    while (i < group_num)
    {
        int bsize = EXT2_MIN_BLOCK_SIZE << s_log_block_size;
        int j = 0;
        
        
        while (j < bsize)
        {
            int k = 0;
            while ( k < 8)      /* 1 byte = 8 bits */
            {
                
                
                k++;
            }
            
            j++;
        }
        
        i++;
    }
}


void free_inode()
{
    
}


void inode()
{
    
}
