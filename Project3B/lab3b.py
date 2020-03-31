#!/usr/bin/python3

import sys #for opening csv file
import csv 
import string

inconsistent = False
blockList = [] #[used1/free0/unreferenced-1, inodeNum(is used), dup, offset, level]
first_non_reserved_block = 0
numBlocks = 0
errorType = ["INVALID", "RESERVED", "DUPLICATE"]


def inodeConsistencyAudit():
    free = []               #list of Free inodes
    alloc= []               #list of allocated inodes
    link = {}               #dictionary of link associated with inode number
    parent = {}             #dictionary of parentInodeNumber with inode number
    superblock = {}
    with open(sys.argv[1], 'r') as file:
        csvfile = csv.reader(file, delimiter=',')
        #print(csvfile)
        for line in csvfile:
            #print("###")
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
                    parent[int(line[3])] = int(line[1])
        file.seek(0)      
        #csvfile = csv.reader(file, delimiter=',')
        for line in csvfile:
            #print("***")
            if line[0] == 'INODE':
                alloc.append(int(line[1]))
                i = int(line[1] )        #inode number
                if i in free:
                    print("ALLOCATED INODE {} ON FREELIST".format(i))
                    inconsistent = True
                if i in link:
                    if link[i] != int(line[6]):
                        inconsistent = True
                        print("INODE {} HAS {} LINKS BUT LINKCOUNT IS {}".format(i, link[i], line[6]))
                else:
                    print("INODE {} HAS {} LINKS BUT LINKCOUNT IS {}".format(i, 0, line[6]))

        file.seek(0)
        for line in csvfile:
            #print("&&&")
            if line[0] == 'DIRENT':
                i = int(line[3])            # current inode number
                parent_i = int(line[1])
                if i<1 or i>superblock['numInodes']:
                    inconsisten = True
                    print("DIRECTORY INODE {} NAME {} INVALID INODE {}".format(parent_i, line[6], i))
                if line[6] == "'..'" and int(line[3])!=parent[int(line[1])]:
                    inconsistent = True
                    print("DIRECTORY INODE {} NAME '..' LINK TO INODE {} SHOULD BE {}".format(parent_i, line[3], parent[int(line[1])]))
                if line[6] == "'.'" and int(line[3]) != int(line[1]):
                    inconsistent = True
                    print("DIRECTORY INODE {} NAME '.' LINK TO INODE {} SHOULD BE {}".format(parent_i, line[3], line[1]))
                if i not in alloc and i>1 and i<superblock['numInodes']:
                    inconsistent = True
                    print("DIRECTORY INODE {} NAME {} UNALLOCATED INODE {}".format(parent_i, line[6], i))

        file.seek(0)          
        for i in range(superblock['fistInode'], 1+superblock['numInodes']):
            if i not in alloc:
                if i not in free:
                    print("UNALLOCATED INODE {} NOT ON FREELIST".format(i))
                    inconsistent = True
            
            
            
            
                    
            
            
                













def printErr(block, inode, offset, level, type):
    global inconsistent
    inconsistent = True
    if (level == 0):
       print("{} BLOCK {} IN INODE {} AT OFFSET {}".format(errorType[type], block, inode, offset), file=sys.stdout)
    elif (level == 1):
       print("{} INDIRECT BLOCK {} IN INODE {} AT OFFSET {}".format(errorType[type], block, inode, offset), file=sys.stdout)
    elif (level == 2):
       print("{} DOUBLE INDIRECT BLOCK {} IN INODE {} AT OFFSET {}".format(errorType[type], block, inode, offset), file=sys.stdout)
    elif (level == 3):
       print("{} TRIPLE INDIRECT BLOCK {} IN INODE {} AT OFFSET {}".format(errorType[type], block, inode, offset), file=sys.stdout)
#end

def blockConsistencyAudits():
    global inconsistent
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
                        if (blockNum < 0 or blockNum >= numBlocks):
                            printErr(blockNum, inodeNum, offset, level, 0)
                        elif (blockNum > 0):
                            if (blockNum < first_non_reserved_block):
                                printErr(blockNum, inodeNum, offset, level, 1)
                            if (blockList[blockNum][0] == 0): #on freelist
                                if (blockList[blockNum][2] == 0): #not duplicated, only on freelist
                                    print("ALLOCATED BLOCK {} ON FREELIST".format(blockNum), file=sys.stdout)
                                    inconsistent = True
                                    blockList[blockNum][1] = inodeNum
                                    blockList[blockNum][2] = 1
                                    blockList[blockNum][3] = offset
                                    blockList[blockNum][4] = level
                                elif (blockList[blockNum][2] == 1): #duplicated for the first time & on freelist
                                    print("ALLOCATED BLOCK {} ON FREELIST".format(blockNum), file=sys.stdout)
                                    printErr(blockNum, blockList[blockNum][1], blockList[blockNum][3], blockList[blockNum][4], 2)
                                    printErr(blockNum, inodeNum, offset, level, 2)
                                    blockList[blockNum][2] = 2
                                elif (blockList[blockNum][2] == 2): #duplicated more than once
                                    print("ALLOCATED BLOCK {} ON FREELIST".format(blockNum), file=sys.stdout)
                                    printErr(blockNum, inodeNum, offset, level, 2)
                            elif (blockList[blockNum][0] == -1): #update
                                blockList[blockNum][0] = 1
                                blockList[blockNum][1] = inodeNum
                                blockList[blockNum][2] = 1
                                blockList[blockNum][3] = offset
                                blockList[blockNum][4] = level
                            elif (blockList[blockNum][0] == 1):
                                if (blockList[blockNum][2] == 1): #duplicated for the first time & on freelist
                                    printErr(blockNum, blockList[blockNum][1], blockList[blockNum][3], blockList[blockNum][4], 2)
                                    printErr(blockNum, inodeNum, offset, level, 2)
                                    blockList[blockNum][2] = 2
                                elif (blockList[blockNum][2] == 2): #duplicated more than once
                                    printErr(blockNum, inodeNum, offset, level, 2)
            elif (line[0] == "INDIRECT"):
                inodeNum = int(line[1])
                blockNum = int(line[5])
                level = int(line[2])
                offset = int(line[3])
                if (blockNum < 0 or blockNum >= numBlocks):
                    printErr(blockNum, inodeNum, offset, level, 0)
                elif (blockNum > 0):
                    if (blockNum < first_non_reserved_block):
                        printErr(blockNum, inodeNum, offset, level, 1)
                    if (blockList[blockNum][0] == 0): #on freelist
                        if (blockList[blockNum][2] == 0): #not duplicated, only on freelist
                            print("ALLOCATED BLOCK {} ON FREELIST".format(blockNum), file=sys.stdout)
                            inconsistent = True
                            blockList[blockNum][1] = inodeNum
                            blockList[blockNum][2] = 1
                            blockList[blockNum][3] = offset
                            blockList[blockNum][4] = level
                        elif (blockList[blockNum][2] == 1): #duplicated for the first time & on freelist
                            print("ALLOCATED BLOCK {} ON FREELIST".format(blockNum), file=sys.stdout)
                            printErr(blockNum, blockList[blockNum][1], blockList[blockNum][3], blockList[blockNum][4], 2)
                            printErr(blockNum, inodeNum, offset, level, 2)
                            blockList[blockNum][2] = 2
                        elif (blockList[blockNum][2] == 2): #duplicated more than once
                            print("ALLOCATED BLOCK {} ON FREELIST".format(blockNum), file=sys.stdout)
                            printErr(blockNum, inodeNum, offset, level, 2)
                    elif (blockList[blockNum][0] == -1): #update
                        blockList[blockNum][0] = 1
                        blockList[blockNum][1] = inodeNum
                        blockList[blockNum][2] = 1
                        blockList[blockNum][3] = offset
                        blockList[blockNum][4] = level
                    elif (blockList[blockNum][0] == 1):
                        if (blockList[blockNum][2] == 1): #duplicated for the first time & on freelist
                            printErr(blockNum, blockList[blockNum][1], blockList[blockNum][3], blockList[blockNum][4], 2)
                            printErr(blockNum, inodeNum, offset, level, 2)
                            blockList[blockNum][2] = 2
                        elif (blockList[blockNum][2] == 2): #duplicated more than once
                            printErr(blockNum, inodeNum, offset, level, 2)

    for i in range(first_non_reserved_block, numBlocks):
        if (blockList[i][0] == -1):
            print("UNREFERENCED BLOCK {}".format(i), file=sys.stdout)
            inconsistent = True

#end 




def main():
    global numBlocks
    global blockList
    global first_non_reserved_block

    if (len(sys.argv) != 2):
        print("Invalid argument. Please follow the usage: ./lab3b fileName", file=sys.stderr)
        sys.exit(1)
    try:
        with open(sys.argv[1], 'r') as file:
            csvfile = csv.reader(file, delimiter=',')
            for line in csvfile:
                if (line[0] == "SUPERBLOCK"):
                    numBlocks = int(line[1])
                    blockList = [[-1, 0, 0, 0, 0] for j in range(int(line[1]))]
                    reservedBlockNum = int(int(line[2]) * int(line[4]) / int(line[3]))
                elif (line[0] == "GROUP"):
                    firstBlockNum = int(line[8])
                elif (line[0] == "BFREE"):
                    blockList[(int(line[1]))][0] = 0
        
        first_non_reserved_block = int(firstBlockNum + reservedBlockNum)

    except IOError:
        print("Failed to open file.", file=sys.stderr)
        sys.exit(1)


    
    blockConsistencyAudits()
    inodeConsistencyAudit()
    if (inconsistent):
        sys.exit(2)
    else:
        sys.exit(0)



if __name__ == "__main__":
    main()
