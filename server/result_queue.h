#ifndef RESULT_QUEUE_H
#define RESULT_QUEUE_H

#include <stdlib.h>
#include <stdio.h>

struct Queue_Results_Position
{
    int row;
    int col;
    char* number;
    struct Queue_Results_Position* next;
};

typedef struct Queue_Results_Position* Result_Position;

Result_Position createResultPosition()
{
    Result_Position temp;
    temp = (Result_Position)malloc(sizeof(struct Queue_Results_Position));
    temp->next = NULL;
    return temp;
}

Result_Position addResultPosition(Result_Position head, int i, int j, char* num)
{
    Result_Position temp, p;
    temp = createResultPosition();
    temp->row = i;
    temp->col = j;
    temp->number = num;
    if(head == NULL)
    {
        head = temp;
    }
    else
    {
        p = head;
        while(p->next != NULL)
        {
            p = p->next;
        }
        p->next = temp;
    }
    return head;
}

Result_Position removeResultHead(Result_Position head)
{
    if (head == NULL)
        return head;
    Result_Position head2 = head->next;
    free(head);
    return head2;
}

void printResultQueue(Result_Position head)
{
    if(head == NULL)
    {
        printf("Head is null\n");
        return;
    }
    if(head != NULL)
    {
        printf("Head:\n");
        printf("  row: %d\n", head->row);
        printf("  col: %d\n", head->col);
        printf("  number: %s\n", head->number);
    }
    Result_Position p = head;
    while(p->next != NULL)
    {
        printf("Node:\n");
        printf("  row: %d\n", p->next->row);
        printf("  col: %d\n", p->next->col);
        printf("  number: %s\n", p->next->number);
        p = p->next;
    }
    printf("End of queue !\n");
}

#endif //RESULT_QUEUE_H