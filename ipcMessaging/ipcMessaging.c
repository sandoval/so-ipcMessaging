//
//  ipcMessaging.c
//  ipcMessaging
//
//  Created by Daniel Sandoval on 02/11/2013.
//  Copyright (c) 2013 Departamento de Ciência da Computação - Universidade de Brasília. All rights reserved.
//

#include "ipcMessaging.h"
#include <stdio.h>
#include <sys/msg.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>



int queueIdForNode(nodeInfo node);
nodeList* initNodeList();

nodeInfo thisNode;

int setup(int nodeId) {
    thisNode.id = nodeId;
    thisNode.msgid = msgget(queueIdForNode(thisNode), IPC_CREAT | 0006);
    if (thisNode.msgid == -1) {
        printf("Failed to create/retrieve queue for node %d: errno %d", thisNode.id, errno);
        exit(1);
    }
    
    return 0;
}

nodeList* connectedNodes(nodeInfo node) {
    nodeList *nodes, *currentNode;
    int currentNodeIsResolved = 0;
    const int meshSize = 3;
    
    currentNode = nodes = initNodeList();
    
    if (node.id == INT_MAX) {
        //This node is the printing server.
        //Only connect it to node zero.
        currentNode->node->id = 0;
        currentNodeIsResolved = 1;
    } else {
        if (node.id == 0) {
            //This is node zero. Connect it to printing server.
            currentNode->node->id = INT_MAX;
            currentNodeIsResolved = 1;
        }
        if (node.id%meshSize != meshSize - 1) {
            //This node is not on the right border. Connect it to it's neighbor on the right.
            if (currentNodeIsResolved) {
                currentNode->next = initNodeList();
                currentNode = currentNode->next;
            } else {
                currentNodeIsResolved = 1;
            }
            currentNode->node->id = node.id + 1;
        }
        if (node.id%meshSize != 0) {
            //This node is not on the left border. Connect it to it's neighbor on the left.
            if (currentNodeIsResolved) {
                currentNode->next = initNodeList();
                currentNode = currentNode->next;
            } else {
                currentNodeIsResolved = 1;
            }
            currentNode->node->id = node.id - 1;
        }
        if (node.id+meshSize < meshSize*meshSize) {
            //This node has a neighbor below him. Connect the neighbor!
            if (currentNodeIsResolved) {
                currentNode->next = initNodeList();
                currentNode = currentNode->next;
            } else {
                currentNodeIsResolved = 1;
            }
            currentNode->node->id = node.id + meshSize;
        }
        if (node.id-meshSize >= 0) {
            //This node has a neighbor above him. Connect the neighbor!
            if (currentNodeIsResolved) {
                currentNode->next = initNodeList();
                currentNode = currentNode->next;
            } else {
                currentNodeIsResolved = 1;
            }
            currentNode->node->id = node.id - meshSize;
        }
    }
    
    return nodes;
}

nodeList* initNodeList() {
    nodeList* list = malloc(sizeof(nodeList));
    list->node = malloc(sizeof(nodeInfo));
    list->next = NULL;
    return list;
}

//TODO: Sophisticate this function to return better queue ids.
int queueIdForNode(nodeInfo node) {
    register int ret = (node.id - 198233);
    if (ret > 0)
        return ret;
    else
        return -ret;
}