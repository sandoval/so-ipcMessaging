//
//  main.c
//  ipcMessaging
//
//  Created by Daniel Sandoval on 02/11/2013.
//  Copyright (c) 2013 Departamento de Ciência da Computação - Universidade de Brasília. All rights reserved.
//

#include <stdio.h>
#include "ipcMessaging.h"

int main(int argc, const char * argv[])
{
    nodeList* nodes;
    nodeInfo node;
    for (int i = 0; i < 9; i++) {
        node.id = i;
        nodes = connectedNodes(node);
    }
    return 0;
}

