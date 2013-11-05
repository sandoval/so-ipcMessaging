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
    long int mtype;
    struct data {
        long messageId;
        int source;
        int destination;
        long referencedMessageId;
        char text[200];
    } mdata;
} message;

int sendMessage(message* msg);

int setup(int nodeId);

/**
 * Function that blocks waiting for message of type mtype.
 * mtype type of message to be received. Causes same behavior of mtype in msgrcv.
 * callback function that will be called when message is received.
 */
void listenForMessages(int mtype, void (*callback)(message*));

int tearDown();

#endif
