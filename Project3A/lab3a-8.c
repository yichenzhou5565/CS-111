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
uint32_t block_size;


/* Functions */
void superblock();          /* Print out superblock summary */
void group();               /* Print out group summary */
void freeblock(unsigned int blk);           /* If free, print summary */
void free_inode(unsigned int blk, int group_no, unsigned int inode_table);          /* If inode free, print summary */
void inode();               /* If inode allocated, print summary */
void directory_entry();
void single_indirect();
void double_indirect();
void triple_indirect();

void get_time (uint32_t timestamp, char* buf) {
    time_t rawtime = timestamp;
    struct tm* info;
    info = gmtime(&rawtime);
    strftime(buf, 32, "%m/%d/%y %H:%M:%S", info);
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
    int group_num = super_block.s_blocks_count / super_block.s_blocks_per_group;
    if (group_num == 0){
        group_num = 1;
    }
    
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
        freeblock(group_desc.bg_block_bitmap);
        
        /* print free inode for THIS group */
        free_inode(group_desc.bg_inode_bitmap, i, group_desc.bg_inode_table);
        
        i++;
    }
}


void freeblock(unsigned int blk)
{
    /*  1: used
        0: free */
    int bsize = EXT2_MIN_BLOCK_SIZE << super_block.s_log_block_size;
    int j = 0;
    
    /* freeblock number */
//    int free_blk_no = group_no * super_block.s_blocks_per_group + super_block.s_first_data_block;
    
    /* read the whole block */
    //char* buffer = (char*)malloc(sizeof(bsize));
    char buffer[bsize];
    if (pread(fd, buffer, bsize, 1024+bsize*(blk-1))<0)
    {
        fprintf(stderr, "Error in pread() system call. %s\n", strerror(errno));
        exit(1);
    }
    uint8_t buf8;
    while (j < bsize)
    {
        pread(fd, &buf8, 1, (blk * bsize) + j);
        int k = 1;
        int tmp = 1;
        while ( k <= 8 )         /* 1 byte = 8 bits */
        {
            if ((buf8 & tmp) == 0 )
            {
                fprintf(stdout, "BFREE,%d\n", j * 8 + k );
            }
            tmp = tmp << 1;
            k++;
        }
        j++;
    }
    
//    if (buffer != NULL)
//        free(buffer);
    
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
        int k = 1;
        while ( k <= 8 )
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
    
    struct ext2_inode inode;
    unsigned long offset = inode_table * block_size + inode_index*sizeof(inode);
    pread(fd,&inode,sizeof(struct ext2_inode), offset);
    
    char filetype = '?'; //anything else
    if(inode.i_mode == 0){
        return;
    }
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
    if(link_count == 0){
        return;
    }
    char ctime[32];
    get_time (inode.i_ctime, ctime);
    char mtime[32];
    get_time (inode.i_mtime, mtime);
    char atime[32];
    get_time (inode.i_atime, atime);
    uint32_t file_size = inode.i_size;
    uint32_t numBlocks = inode.i_blocks;
    fprintf(stdout, "INODE,%d,%c,%o,%u,%u,%u,%s,%s,%s,%d,%d", inode_num, filetype, mode, owner, group, link_count, ctime, mtime, atime, file_size, numBlocks);
    
    int i;
    for (i = 0; i < 15; i++) { //block addresses
        fprintf(stdout, ",%d", inode.i_block[i]);
    }
    fprintf(stdout, "\n");
    
    if (filetype == 'd') {
        directory_entry(inode, inode_num);
    }
    if(inode.i_block[12] != 0) //13th
        single_indirect(inode_num, inode.i_block[12]);
    if(inode.i_block[13] != 0) //14th
        double_indirect(inode_num, inode.i_block[13]);
    if(inode.i_block[14] != 0) //15th
        triple_indirect(inode_num, inode.i_block[14]);
    
}
 

void directory_entry(struct ext2_inode inode, unsigned int parent_inode_num) {
    struct ext2_dir_entry directory;
    uint32_t i;
    for (i = 0; i < 12; i++) { //scan first 12 direct entries
        if (inode.i_block[i] != 0 ) {
            unsigned int byte_offset = 0;
            unsigned long offset = inode.i_block[i] * block_size;
            while (byte_offset < block_size) {
                pread(fd, &directory, sizeof(struct ext2_dir_entry), offset + byte_offset);
                if(directory.inode == 0){ //inode start at 1吧？？我不确定 我读的tutorial这么说的
                    byte_offset += directory.rec_len;
                    continue;
                }
                fprintf(stdout, "DIRENT,%d,%d,%d,%d,%d,'%s'\n", parent_inode_num, byte_offset,
                   directory.inode, directory.rec_len, directory.name_len,
                   directory.name);
                byte_offset += directory.rec_len; //update logical byte offset
            }
        }
    }
    
    if(inode.i_block[12] != 0) {
        uint32_t j, indir_block_num;
        for(j = 0; j < block_size/4; j++)
        {
            pread(fd, &indir_block_num, 4, inode.i_block[12] * block_size + j * 4);
            if(indir_block_num != 0)
            {
                unsigned int byte_offset = 0;
                unsigned long offset = indir_block_num * block_size;
                while (byte_offset < block_size) {
                    pread(fd, &directory, sizeof(struct ext2_dir_entry), offset + byte_offset);
                    if(directory.inode == 0){
                        byte_offset += directory.rec_len;
                        continue;
                    }
                    fprintf(stdout, "DIRENT,%d,%d,%d,%d,%d,'%s'\n", parent_inode_num, byte_offset,
                            directory.inode, directory.rec_len, directory.name_len,
                            directory.name);
                    byte_offset += directory.rec_len;//update logical byte offset
                }
            }
        }
    }
    
    if (inode.i_block[13] != 0) {
        uint32_t k, j, indir1, indir2;
        for (k = 0; k < block_size/4; k++){
            pread(fd, &indir1, 4, inode.i_block[13] * block_size + k * 4);
            if(indir1 != 0) {
                for (j = 0; j < block_size/4; j++) {
                    pread(fd, &indir2, 4, indir1 * block_size + j * 4);
                    if (indir2 != 0){
                        unsigned int byte_offset = 0;
                        long offset = indir2 * block_size;
                        while (byte_offset < block_size) {
                            pread(fd, &directory, sizeof(struct ext2_dir_entry), offset + byte_offset);
                            if(directory.inode == 0){
                                byte_offset += directory.rec_len;
                                continue;
                            }
                            fprintf(stdout, "DIRENT,%d,%d,%d,%d,%d,'%s'\n", parent_inode_num, byte_offset,
                                    directory.inode, directory.rec_len, directory.name_len,
                                    directory.name);
                            byte_offset += directory.rec_len;
                        }
                    }
                }
            }
        }
    }
    
    if (inode.i_block[14] != 0){
        uint32_t l, k, j, indir1, indir2, indir3;
        for (l = 0; l < block_size/4; l++){
            pread(fd, &indir1, 4, inode.i_block[14] * block_size + l * 4);
            if (indir1 != 0) {
                for (k = 0; k < block_size/4; k++){
                    pread(fd, &indir2, 4, indir1 * block_size + k * 4);
                    if(indir2 != 0) {
                        for (j = 0; j < block_size/4; j++) {
                            pread(fd, &indir3, 4, indir2 * block_size + j * 4);
                            if (indir3 != 0){
                                unsigned int byte_offset = 0;
                                unsigned long offset = indir3 * block_size;
                                while (byte_offset < block_size) {
                                    pread(fd, &directory, sizeof(struct ext2_dir_entry), offset + byte_offset);
                                    if(directory.inode == 0){
                                        byte_offset += directory.rec_len;
                                        continue;
                                    }
                                    fprintf(stdout, "DIRENT,%d,%d,%d,%d,%d,'%s'\n", parent_inode_num, byte_offset,
                                            directory.inode, directory.rec_len, directory.name_len,
                                            directory.name);
                                    byte_offset += directory.rec_len;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void single_indirect(unsigned int inode_num, unsigned int indir_block) {
    unsigned long offset = indir_block * block_size;
    uint32_t i, ref_block;
    for (i = 0; i < block_size/4; i++){
        pread(fd, &ref_block, 4, offset + i*4);
        if (ref_block != 0) {
            fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n", inode_num, 1, 12 + i, indir_block, ref_block);
        }
    }
}

void double_indirect(unsigned int inode_num, unsigned int indir_block) {
    unsigned long offset1 = indir_block * block_size;
    uint32_t i, j, indir1, ref_block;
    for(i = 0; i < block_size/4; i++){
        pread(fd, &indir1, 4, offset1 + i*4);
        if(indir1 != 0){
            fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n", inode_num, 2, 12 + block_size/4 + i * block_size/4, indir_block, indir1);
            unsigned long offset2 = indir1 * block_size;
            for(j = 0; j < block_size/4; j++){
                pread(fd, &ref_block, 4, offset2 + j*4);
                if (ref_block != 0) {
                    fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n", inode_num, 1, 12 + block_size/4 + i * block_size/4 + j, indir1, ref_block);
                }
            }
        }
    }
}

void triple_indirect(unsigned int inode_num, unsigned int indir_block) {
    unsigned long offset1 = indir_block * block_size;
    uint32_t i, j, k, indir1, indir2, ref_block;
    for(i = 0; i < block_size/4; i++){
        pread(fd, &indir2, 4, offset1 + i*4);
        if(indir2 != 0){
            fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n", inode_num, 3, 12 + block_size/4 + block_size/4 * block_size/4 + i * block_size/4 * block_size/4, indir_block, indir2);
            unsigned long offset2 = indir2 * block_size;
            for(j = 0; j < block_size/4; j++){
                pread(fd, &indir1, 4, offset2 + j*4);
                if(indir1 != 0){
                    fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n", inode_num, 2, 12 + block_size/4 + block_size/4 * block_size/4 + i * block_size/4 * block_size/4 + j*block_size/4, indir2, indir1);
                    unsigned long offset3 = indir1 * block_size;
                    for(k = 0; k < block_size/4; k++){
                        pread(fd, &ref_block, 4, offset3 + k*4);
                        if (ref_block != 0) {
                            fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n", inode_num, 1, 12 + block_size/4 + block_size/4 * block_size/4 + i * block_size/4 * block_size/4 + j*block_size/4 +k, indir1, ref_block);
                        }
                    }
                }

            }
        }
    }
}

