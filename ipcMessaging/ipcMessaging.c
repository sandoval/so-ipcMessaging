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
#include <unistd.h>

typedef struct nodeInfo nodeInfo;
typedef struct nodeList nodeList;
typedef struct messageList messageList;

struct nodeInfo {
    int id;
    int msgid;
    nodeList* connectedNodes;
};

struct nodeList {
    nodeInfo *node;
    nodeList *next;
};

struct messageList {
    message *message;
    messageList *next;
    messageList *previous;
};

int queueIdForNode(nodeInfo node);
nodeList* initNodeList();
nodeList* calculateConnectedNodes(nodeInfo node);
int freeNodeList(nodeList* nodes);

//Determines to which node the message should be sent.
nodeInfo* nextNodeInRoute(message* message);

//Connects to node by getting a reference to it's message queue.
int connectNode(nodeInfo* node);

//Method that executes periodically to check and treat new messages.
void watchdog();

//Internal method that sends messages to destination or next node in route.
//May be used for routing existing messages and doesn't set source.
int _sendMessage(message* msg);

//UNUSED
//Methods that handle message lists.
messageList *initMessageList();
message *takeMessageOfType(int type, messageList *list);
messageList *putMessage(message *msg, messageList *list);

nodeInfo thisNode;
long currentMessageId = 0;

int setup(int nodeId) {
    thisNode.id = nodeId;
    thisNode.msgid = msgget(queueIdForNode(thisNode), IPC_CREAT | 0600);
    
    if (thisNode.msgid == -1) {
        printf("Failed to create/retrieve queue for node %d: errno %d\n", thisNode.id, errno);
        exit(1);
    }
    thisNode.connectedNodes = calculateConnectedNodes(thisNode);
    
    return 0;
}

int tearDown() {
    msgctl(thisNode.msgid, IPC_RMID, NULL);
    freeNodeList(thisNode.connectedNodes);
    return 0;
}

void listenForMessages(int mtype, void (*callback)(message*)) {
    message* msg = malloc(sizeof(message));
    ssize_t errorCheck;

    while ((errorCheck = msgrcv(thisNode.msgid, msg, sizeof(message) - sizeof(long int), mtype, 0)) != (ssize_t)-1) {
        if (msg->mdata.destination != thisNode.id) {
            _sendMessage(msg);
        } else {
            callback(msg);
        }
    }
    if (errorCheck == (ssize_t)-1) {
        printf("An error occured while trying to receive message: %d\n", errno);
    }
    free(msg);
}

int sendMessage(message *msg) {
    msg->mdata.source = thisNode.id;
    return _sendMessage(msg);
}

int _sendMessage(message* msg) {
    nodeInfo* nodeToSend;
    
    //Cannot send a message to itself.
    if (msg->mdata.destination == thisNode.id) {
        printf("Cannot send message to itself!\n");
        return -1;
    }
    
    nodeToSend = nextNodeInRoute(msg);
    //printf("[NODE %d] Sending message (%d->%d) to node %d\n", thisNode.id, msg->mdata.source, msg->mdata.destination, nodeToSend->id);
    //Unknown routing error.
    if (nodeToSend == NULL) {
        printf("Unknown routing error!\n");
        return -2;
    }
    
    
    if (connectNode(nodeToSend) != 0) {
        printf("Couldn't connect to node %d!\n", nodeToSend->id);
        return -3;
    }
    
    if (msg->mdata.source == thisNode.id) {
        msg->mdata.messageId = currentMessageId++;
    }

    if(msgsnd(nodeToSend->msgid, msg, sizeof(message) - sizeof(long int), 0) == -1) {
        printf("Error sending message!\n");
        return -4;
    }
    
    return 0;
}

nodeInfo* nextNodeInRoute(message* message) {
    int destination;
    nodeList *currentNode, *selectedNode = NULL;
    //This is the node to which the message is destined.
    if (message->mdata.destination == thisNode.id)
        return NULL;
    
    destination = message->mdata.destination;
    //Translation needed in order to guide the message to node zero if it's destination is the printer!
    if ((destination == INT_MAX && thisNode.id != 0) || (thisNode.id == INT_MAX))
        destination = 0;
    
    //Searches for direct connections to destination node. Returns the node if found said connection.
    currentNode = thisNode.connectedNodes;
    while (currentNode != NULL) {
        if (currentNode->node->id == destination)
            return currentNode->node;
        currentNode = currentNode->next;
    }
    
    currentNode = thisNode.connectedNodes;
    if (destination > thisNode.id) {
        //Destination node has greater id than this node. We must route it to the connected node with the greatest id.
        while (currentNode != NULL) {
            if ((currentNode->node->id != INT_MAX) && (currentNode->node->id > thisNode.id) && (currentNode->node->id < destination)) {
                //First match.
                selectedNode = currentNode;
                break;
            }
            currentNode = currentNode->next;
        }
        while ((selectedNode != NULL) && (currentNode != NULL)) {
            //Best match.
            if ((currentNode->node->id != INT_MAX) && (currentNode->node->id > selectedNode->node->id) && (currentNode->node->id < destination))
                selectedNode = currentNode;
            currentNode = currentNode->next;
        }
    } else {
        //Destination node has lower id than this node. We must route it to the connected node with the lowest id.
        while (currentNode != NULL) {
            if (currentNode->node->id < thisNode.id && currentNode->node->id > destination) {
                selectedNode = currentNode;
                break;
            }
            currentNode = currentNode->next;
        }
        while ((selectedNode != NULL) && (currentNode != NULL)) {
            if (currentNode->node->id < selectedNode->node->id && currentNode->node->id > destination)
                selectedNode = currentNode;
            currentNode = currentNode->next;
        }
    }
    
    if (selectedNode != NULL)
        return selectedNode->node;
    return NULL;
}

int connectNode(nodeInfo* node) {
    if (node == NULL)
        return -1;
    
    if (node->msgid != -1)
        return 0;
    else {
        int i = 2;
        node->msgid = msgget(queueIdForNode(*node), 0);
        while ((node->msgid == -1) && (i > 0)) {
            sleep(1);
            node->msgid = msgget(queueIdForNode(*node), 0);
            i--;
        }
    }
    if (node->msgid != -1)
        return 0;
    else
        return -1;
}

int freeNodeList(nodeList* nodes) {
    nodeList* nextNode;
    while (nodes != NULL) {
        nextNode = nodes->next;
        if (nodes->node != NULL)
            free(nodes->node);
        free(nodes);
        nodes = nextNode;
    }
    return 0;
}

nodeList* calculateConnectedNodes(nodeInfo node) {
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

messageList *initMessageList() {
    messageList *list = malloc(sizeof(messageList));
    list->message = NULL;
    list->next = NULL;
    list->previous = NULL;
    return list;
}

message *takeMessageOfType(int type, messageList *list) {
    message *msg = NULL;
    while (list != NULL) {
        if (list->message->mtype == type) {
            if (list->next != NULL)
                list->next->previous = list->previous;
            if (list->previous != NULL)
                list->previous->next = list->next;
            msg = list->message;
            free(list);
        }
    }
    return msg;
}

messageList *putMessage(message *msg, messageList *list) {
    messageList* element = initMessageList();
    element->message = msg;
    if (list == NULL)
        return element;
    while (list->next != NULL) {
        list = list->next;
    }
    element->previous = list;
    list->next = element;
    return element;
}

nodeInfo* initNode() {
    nodeInfo* node = malloc(sizeof(nodeInfo));
    node->connectedNodes = NULL;
    node->msgid = -1;
    return node;
}

nodeList* initNodeList() {
    nodeList* list = malloc(sizeof(nodeList));
    list->node = initNode();
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