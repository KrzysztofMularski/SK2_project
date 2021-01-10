#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define DSIZE 20
#define ISIZE 5

int main()
{
    struct sockaddr_in sa;
    int my_socket = socket(AF_INET, SOCK_STREAM, 0);
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(1233);
    inet_pton(AF_INET, "127.0.0.1", &(sa.sin_addr));

    if (connect(my_socket, (struct sockaddr*) & sa, sizeof(sa)) == -1)
    {
        printf("Can't connect\nExiting\n");
        exit(1);
    }
    int my_desc = my_socket;
    int client_id;
    int i;
    int j;
    double a, b;
    int iorder;
    double dresult;
    char** row;
    char** col;

    char checkpoint[2];
    char* result = (char*)malloc(DSIZE * sizeof(char));
    char* order = (char*)malloc(ISIZE * sizeof(char));
    int size = 1;
    while (1)
    {
        write(my_desc, 'o', 1);
        size = read(my_desc, order, ISIZE);
        if (!size) break;
        sscanf_s(order, "%d", &iorder);
        row = (char**)malloc(iorder * sizeof(char*));
        col = (char**)malloc(iorder * sizeof(char*));
        for (int i = 0; i < iorder; i++)
        {
            row[i] = (char*)malloc(DSIZE * sizeof(char));
            col[i] = (char*)malloc(DSIZE * sizeof(char));
        }
        for (int i = 0; i < iorder; i++)
        {
            write(my_desc, 'r', 1);
            size = read(my_desc, row[i], DSIZE);
            if (!size) break;
        }
        if (!size) break;
        for (int i = 0; i < iorder; i++)
        {
            write(my_desc, 'c', 1);
            size = read(my_desc, col[i], DSIZE);
            if (!size) break;
        }
        if (!size) break;
        dresult = 0;
        for (int i = 0; i < iorder; i++)
        {
            sscanf_s(row[i], "%lf", &a);
            sscanf_s(col[i], "%lf", &b);
            dresult += a * b;
        }
        result[0] = '\0';
        size = sprintf_s(result, "%lf", dresult);
        result[size] = '\0';
        //odczyt obliczonego wyniku
        size = read(my_desc, checkpoint, 1);
        if (!size) break;
        if (checkpoint[0] != 's') break;
        write(my_desc, result, strlen(result));


        for (int i = 0; i < iorder; i++)
        {
            free(row[i]);
            free(col[i]);
        }
        free(row);
        free(col);
    }
    free(result);
    free(order);
}