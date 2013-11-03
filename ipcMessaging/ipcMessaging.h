//
//  ipcMessaging.h
//  ipcMessaging
//
//  Created by Daniel Sandoval on 02/11/2013.
//  Copyright (c) 2013 Departamento de Ciência da Computação - Universidade de Brasília. All rights reserved.
//

#ifndef ipcMessaging_ipcMessaging_h
#define ipcMessaging_ipcMessaging_h

typedef struct message {
    long mtype;
    long messageId;
    int source;
    int destination;
    long referencedMessageId;
    char text[200];
} message;

int sendMessage(message* message);

int setup(int nodeId);

int tearDown();

#endif
