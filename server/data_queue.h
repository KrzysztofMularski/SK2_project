#ifndef DATA_QUEUE_H
#define DATA_QUEUE_H

#include <stdlib.h>
#include <stdio.h>

#define ISIZE 5
#define DSIZE 20

struct Queue_Data_Position
{
    int client_id;
    int i;
    int j;
    int imat_order;
    char* mat_order;
    char** mat_1_row;
    char** mat_2_col;
    struct Queue_Data_Position* next;
};

typedef struct Queue_Data_Position* Data_Position;

Data_Position createDataPosition()
{
    Data_Position temp;
    temp = (Data_Position)malloc(sizeof(struct Queue_Data_Position));
    temp->next = NULL;
    return temp;
}

Data_Position addDataPosition(int client_id, int i, int j, Data_Position head, int iorder, char* order, char** row, char** col)
{
    Data_Position temp, p;
    temp = createDataPosition();
    temp->client_id = client_id;
    temp->i = i;
    temp->j = j;
    temp->imat_order = iorder;
    temp->mat_order = order;
    temp->mat_1_row = row;
    temp->mat_2_col = col;

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

Data_Position removeDataHead(Data_Position head)
{
    if (head == NULL)
        return head;
    Data_Position head2 = head->next;
    free(head);
    return head2;
}

void printDataQueue(Data_Position head)
{
    if(head == NULL)
    {
        printf("Head is null\n");
        return;
    }
    if(head != NULL)
    {
        printf("Head:\n");
        printf("  mat_order: %s\n", head->mat_order);
        printf("  row: ");
        for(int i=0; i<head->imat_order; i++)
        {
            printf("%s, ", head->mat_1_row[i]);
        }
        printf("\n  col: ");
        for(int i=0; i<head->imat_order; i++)
        {
            printf("%s, ", head->mat_2_col[i]);
        }
        printf("\n");
    }
    Data_Position p = head;
    while(p->next != NULL)
    {
        printf("Node:\n");
        printf("  mat_order: %s\n", p->next->mat_order);
        printf("  row: ");
        for(int i=0; i<p->next->imat_order; i++)
        {
            printf("%s, ", p->next->mat_1_row[i]);
        }
        printf("\n  col: ");
        for(int i=0; i<p->next->imat_order; i++)
        {
            printf("%s, ", p->next->mat_2_col[i]);
        }
        printf("\n");
        p = p->next;
    }
    printf("End of queue !\n");
}

#endif //DATA_QUEUE_H