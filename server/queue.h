#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <stdio.h>

struct Queue_Element
{
    int descriptor;
    struct Queue_Element* next;
};

typedef struct Queue_Element* Node;

Node createNode()
{
    Node temp; // declare a node
    temp = (Node)malloc(sizeof(struct Queue_Element)); // allocate memory using malloc()
    temp->next = NULL;// make next point to NULL
    return temp;//return the new node
}

Node addNode(Node head, int desc)
{
    Node temp, p;// declare two nodes temp and p
    temp = createNode();//createNode will return a new node with data = value and next pointing to NULL.
    temp->descriptor = desc; // add element's value to data part of node
    if(head == NULL)
    {
        head = temp;     //when linked list is empty
    }
    else
    {
        p = head;//assign head to p 
        while(p->next != NULL){
            p = p->next;//traverse the list until p is the last node.The last node always points to NULL.
        }
        p->next = temp;//Point the previous last node to the new node created.
    }
    return head;
}

Node removeHead(Node head)
{
    if (head == NULL)
        return head;
    Node head2 = head->next;
    free(head);
    return head2;
}

Node removeNode(Node head, int desc)
{
    if (head == NULL)
        return head;
    else if (head->descriptor == desc)
        return removeHead(head);
    else if (head->next == NULL)
        return head;
    else
    {
        Node p, n;
        p = head;
        while(p->next->descriptor != desc)
        {
            if (p->next->next == NULL)
                return head;
            else
                p = p->next;
        }
        n = p->next;
        p->next = p->next->next;
        free(n);
        return head;
    }
}

void printList(Node head)
{
    if(head == NULL)
    {
        printf("Head is null\n");
        return;
    }
    if(head != NULL)
        printf("Head: %i\n", head->descriptor);
    Node p = head;
    while(p->next != NULL)
    {
        printf("Node: %i\n", p->next->descriptor);
        p = p->next;
    }
    printf("End of queue !\n");
}

#endif // QUEUE_H