//
//  ipcMessaging.h
//  ipcMessaging
//
//  Created by Daniel Sandoval on 02/11/2013.
//  Copyright (c) 2013 Departamento de Ciência da Computação - Universidade de Brasília. All rights reserved.
//

#ifndef ipcMessaging_ipcMessaging_h
#define ipcMessaging_ipcMessaging_h

typedef struct nodeInfo {
    int id;
    int msgid;
} nodeInfo;

typedef struct nodeList {
    nodeInfo *node;
    struct nodeList *next;
} nodeList;

nodeList* connectedNodes(nodeInfo node);

#endif
