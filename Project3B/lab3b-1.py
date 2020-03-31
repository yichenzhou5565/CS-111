#!/usr/bin/python3

import sys #for opening csv file
import csv 
import string

superblock = {} #dictionary
blockBitMap = [] #[1/0/-1, inode, dup, offset, level] inodeNum:used 0:free -1:unreferenced; duplicated=1/0; level=0/1/2/3
first_non_reserved_block = 0

def inodeConsistencyAudit():
    free = []               #list of Free inodes
    alloc= []               #list of allocated inodes
    link = {}               #dictionary of link associated with inode number
    parent = {}             #dictionary of parentInodeNumber with inode number
    file = open(sys.argv[1])
    csvfile = reader(file, delimiter=',')
    for line in csv:
        if line[0] == 'SUPERBLOCK':
            superblock['numBlocks'] = int(line[1])
            superblock['numInodes'] = int(line[2])
            superblock['blockSize'] = int(line[3])
            superblock['inodeSize'] = int(line[4])
            superblock['fistInode'] = int(line[7])
        if line[0] == 'IFREE':
            free.append(int(line[1]))
        if line[0] == 'DIRENT':
            if int(line[3]) not in link:
                link[int(line[3])] = 1
            else:
                link[int(line[3])] += 1
            if int(line[3]) not in parent:
                parent[int(line[3])] = int(line[3])
        if line[0] == 'INODE':
            i = int(line[1] )        #inode number
            if i in free:
                print("ALLOCATED INODE {} ON FREELIST".format(i))
            
            
            
            
        
            
            
            
        
        






def printInvalid(block, inode, offset, level):
    if (level == 0):
       print("INVALID BLOCK {} IN INODE {} AT OFFSET {}".format(block, inode, offset), file=sys.stdout)
    elif (level == 1):
       print("INVALID INDIRECT BLOCK {} IN INODE {} AT OFFSET {}".format(block, inode, offset), file=sys.stdout)
    elif (level == 2):
       print("INVALID DOUBLE INDIRECT BLOCK {} IN INODE {} AT OFFSET {}".format(block, inode, offset), file=sys.stdout)
    elif (level == 3):
       print("INVALID TRIPLE INDIRECT BLOCK {} IN INODE {} AT OFFSET {}".format(block, inode, offset), file=sys.stdout)

def printReserved(block, inode, offset, level):
    if (level == 0):
        print("RESERVED BLOCK {} IN INODE {} AT OFFSET {}".format(block, inode, offset), file=sys.stdout)
    elif (level == 1):
        print("RESERVED INDIRECT BLOCK {} IN INODE {} AT OFFSET {}".format(block, inode, offset), file=sys.stdout)
    elif (level == 2):
        print("RESERVED DOUBLE INDIRECT BLOCK {} IN INODE {} AT OFFSET {}".format(block, inode, offset), file=sys.stdout)
    elif (level == 3):
        print("RESERVED TRIPLE INDIRECT BLOCK {} IN INODE {} AT OFFSET {}".format(block, inode, offset), file=sys.stdout)

def printDup(block, inode, offset, level):
    if (level == 0):
        print("DUPLICATE BLOCK {} IN INODE {} AT OFFSET {}".format(block, inode, offset), file=sys.stdout)
    elif (level == 1):
        print("DUPLICATE INDIRECT BLOCK {} IN INODE {} AT OFFSET {}".format(block, inode, offset), file=sys.stdout)
    elif (level == 2):
        print("DUPLICATE DOUBLE INDIRECT BLOCK {} IN INODE {} AT OFFSET {}".format(block, inode, offset), file=sys.stdout)
    elif (level == 3):
        print("DUPLICATE TRIPLE INDIRECT BLOCK {} IN INODE {} AT OFFSET {}".format(block, inode, offset), file=sys.stdout)
#end

def blockConsistencyAudits():
    with open(sys.argv[1], 'r') as file:
        csvfile = csv.reader(file, delimiter=',')
        for line in csvfile:
            if (line[0] == "INODE"):
                if (int(line[3]) != 0): #check mode, valid if != 0
                    inodeNum = int(line[1])
                    for i in range(15):
                        if (i < 12):
                            level = 0
                            offset = i
                        elif (i == 12):
                            level = 1
                            offset = 12
                        elif (i == 13):
                            level = 2
                            offset = 268 #1024/4 + 12
                        elif (i == 14):
                            level = 3
                            offset = 65804 #(1024/4)*(1024/4) + 1024/4 + 12
                        blockNum = int(line[12+i])
                        if (blockNum < 0 or blockNum >= superblock['numBlocks']):
                            printInvalid(blockNum, inodeNum, offset, level)
                        elif (blockNum > 0):
                            if (blockNum < first_non_reserved_block):
                                printReserved(blockNum, inodeNum, offset, level)
                            if (blockBitMap[blockNum][0] == 0): #on freelist
                                if (blockBitMap[blockNum][2] == 0): #not duplicated, only on freelist
                                    print("ALLOCATED BLOCK {} ON FREELIST".format(blockNum), file=sys.stdout)
                                    blockBitMap[blockNum][1] = inodeNum
                                    blockBitMap[blockNum][2] = 1
                                    blockBitMap[blockNum][3] = offset
                                    blockBitMap[blockNum][4] = level
                                elif (blockBitMap[blockNum][2] == 1): #duplicated for the first time & on freelist
                                    print("ALLOCATED BLOCK {} ON FREELIST".format(blockNum), file=sys.stdout)
                                    printDup(blockNum, blockBitMap[blockNum][1], blockBitMap[blockNum][3], blockBitMap[blockNum][4])
                                    printDup(blockNum, inodeNum, offset, level)
                                    blockBitMap[blockNum][2] = 2
                                elif (blockBitMap[blockNum][2] == 2): #duplicated more than once
                                    print("ALLOCATED BLOCK {} ON FREELIST".format(blockNum), file=sys.stdout)
                                    printDup(blockNum, inodeNum, offset, level)
                            elif (blockBitMap[blockNum][0] == -1): #update
                                blockBitMap[blockNum][0] = 1
                                blockBitMap[blockNum][1] = inodeNum
                                blockBitMap[blockNum][2] = 1
                                blockBitMap[blockNum][3] = offset
                                blockBitMap[blockNum][4] = level
                            elif (blockBitMap[blockNum][0] == 1):
                                if (blockBitMap[blockNum][2] == 1): #duplicated for the first time & on freelist
                                    printDup(blockNum, blockBitMap[blockNum][1], blockBitMap[blockNum][3], blockBitMap[blockNum][4])
                                    printDup(blockNum, inodeNum, offset, level)
                                    blockBitMap[blockNum][2] = 2
                                elif (blockBitMap[blockNum][2] == 2): #duplicated more than once
                                    printDup(blockNum, inodeNum, offset, level)
            elif (line[0] == "INDIRECT"):
                inodeNum = int(line[1])
                blockNum = int(line[5])
                level = int(line[2])
                offset = int(line[3])
                if (blockNum < 0 or blockNum >= superblock['numBlocks']):
                    printInvalid(blockNum, inodeNum, offset, level)
                elif (blockNum > 0):
                    if (blockNum < first_non_reserved_block):
                        printReserved(blockNum, inodeNum, offset, level)
                    if (blockBitMap[blockNum][0] == 0): #on freelist
                        if (blockBitMap[blockNum][2] == 0): #not duplicated, only on freelist
                            print("ALLOCATED BLOCK {} ON FREELIST".format(blockNum), file=sys.stdout)
                            blockBitMap[blockNum][1] = inodeNum
                            blockBitMap[blockNum][2] = 1
                            blockBitMap[blockNum][3] = offset
                            blockBitMap[blockNum][4] = level
                        elif (blockBitMap[blockNum][2] == 1): #duplicated for the first time & on freelist
                            print("ALLOCATED BLOCK {} ON FREELIST".format(blockNum), file=sys.stdout)
                            printDup(blockNum, blockBitMap[blockNum][1], blockBitMap[blockNum][3], blockBitMap[blockNum][4])
                            printDup(blockNum, inodeNum, offset, level)
                            blockBitMap[blockNum][2] = 2
                        elif (blockBitMap[blockNum][2] == 2): #duplicated more than once
                            print("ALLOCATED BLOCK {} ON FREELIST".format(blockNum), file=sys.stdout)
                            printDup(blockNum, inodeNum, offset, level)
                    elif (blockBitMap[blockNum][0] == -1): #update
                        blockBitMap[blockNum][0] = 1
                        blockBitMap[blockNum][1] = inodeNum
                        blockBitMap[blockNum][2] = 1
                        blockBitMap[blockNum][3] = offset
                        blockBitMap[blockNum][4] = level
                    elif (blockBitMap[blockNum][0] == 1):
                        if (blockBitMap[blockNum][2] == 1): #duplicated for the first time & on freelist
                            printDup(blockNum, blockBitMap[blockNum][1], blockBitMap[blockNum][3], blockBitMap[blockNum][4])
                            printDup(blockNum, inodeNum, offset, level)
                            blockBitMap[blockNum][2] = 2
                        elif (blockBitMap[blockNum][2] == 2): #duplicated more than once
                            printDup(blockNum, inodeNum, offset, level)

    for i in range(first_non_reserved_block, superblock['numBlocks']):
        if (blockBitMap[i][0] == -1):
            print("UNREFERENCED BLOCK {}".format(i), file=sys.stdout)

#end 




def main():
    global superblock
    global blockBitMap
    global first_non_reserved_block

    if (len(sys.argv) != 2):
        print("Invalid argument. Please follow the usage: ./lab3b fileName", file=sys.stderr)
        sys.exit(1)
    try:
        with open(sys.argv[1], 'r') as file:
            csvfile = csv.reader(file, delimiter=',')
            for line in csvfile:
                if (line[0] == "SUPERBLOCK"):
                    superblock['numBlocks'] = int(line[1])
                    superblock['numInodes'] = int(line[2])
                    superblock['blockSize'] = int(line[3])
                    superblock['inodeSize'] = int(line[4])
                    superblock['fistInode'] = int(line[7])
                    blockBitMap = [[-1, 0, 0, 0, 0] for j in range(int(line[1]))]
                    reservedBlockNum = int(int(line[2]) * int(line[4]) / int(line[3]))
                elif (line[0] == "GROUP"):
                    firstBlockNum = int(line[8])
                elif (line[0] == "BFREE"):
                    blockBitMap[(int(line[1]))][0] = 0
        
        first_non_reserved_block = int(firstBlockNum + reservedBlockNum)

    except IOError:
        print("Failed to open file.", file=sys.stderr)
        sys.exit(1)
    blockConsistencyAudits()

    



if __name__ == "__main__":
    main()
