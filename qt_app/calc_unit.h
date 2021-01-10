#ifndef CALC_UNIT_H
#define CALC_UNIT_H

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define UNIT_PORT 1233
#define MAX_UNITS_NUMBER 100
#define MATRIX_ORDER 10 // stopień macierzy

int units[MAX_UNITS_NUMBER];
int current_units_online = 0;

pthread_mutex_t units_mutex = PTHREAD_MUTEX_INITIALIZER;

//void *UnitBehavior(void *t_data)
UnitBehavior(t_data)
{
    //pthread_detach(pthread_self());
    struct thread_data_t *th_data = (struct thread_data_t*)t_data;
    //odczyt danych z serwera
    int mat_order;
    double* row;
    double* col;
    //stopień macierzy
    char mat_size[MATRIX_ORDER];
    read(th_data->my_socket, mat_size, MATRIX_ORDER);
    write(th_data->my_socket, ".", 2);
    sscanf(mat_size, "%i", &mat_order);
    //wiersz i kolumna
    row = (double*)malloc(mat_order*sizeof(double));
    col = (double*)malloc(mat_order*sizeof(double));
}

void handleUnit(int unit_socket_descriptor, int id)
{
    int create_result = 0;
    pthread_t calc_unit;

    struct thread_data_t* t_data = malloc(sizeof(struct thread_data_t));
    t_data->my_socket = unit_socket_descriptor;
    t_data->id_in_desc_array = id;

    UnitBehavior(t_data);
    //create_result = pthread_create(&calc_unit, NULL, UnitBehavior, (void *)t_data);
    if (create_result){
       printf("Błąd przy próbie utworzenia wątku, kod błędu: %d\n", create_result);
       exit(-1);
    }
}

void *calculation_unit(void)
{
    struct sockaddr_in unit_address;

    memset(&unit_address, 0, sizeof(struct sockaddr));
    unit_address.sin_family = AF_INET;
    unit_address.sin_addr.s_addr = htonl(INADDR_ANY);
    unit_address.sin_port = htons(UNIT_PORT);

    int unit_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (unit_socket_descriptor < 0)
    {
        fprintf(stderr, "UNIT: Błąd przy próbie utworzenia gniazda..\n");
        exit(1);
    }
    char reuse_addr_val = 1;
    setsockopt(unit_socket_descriptor, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse_addr_val, sizeof(reuse_addr_val));

    int bind_result = bind(unit_socket_descriptor, (struct sockaddr*)&unit_address, sizeof(struct sockaddr));
    if (bind_result < 0)
    {
        fprintf(stderr, "UNIT: Błąd przy próbie dowiązania adresu IP i numeru portu do gniazda.\n");
        exit(1);
    }

    int listen_result = listen(unit_socket_descriptor, MAX_UNITS_NUMBER);
    if (listen_result < 0) {
        fprintf(stderr, "UNIT: Błąd przy próbie ustawienia wielkości kolejki.\n");
        exit(1);
    }

    init_descriptors_array('u');
    int unit_socket_descriptor;
    while(1)
    {
        if (current_units_online < MAX_UNITS_NUMBER)
        {
            //obsłużenie jednostki
            unit_socket_descriptor = accept(unit_socket_descriptor, NULL, NULL);
            if (unit_socket_descriptor < 0)
            {
                fprintf(stderr, "UNIT: Błąd przy próbie utworzenia gniazda dla połączenia.\n");
                exit(1);
            }
            //szukanie wolnego slotu
            int j = -1;
            pthread_mutex_lock(&units_mutex);
            for (int i=0; i<MAX_UNITS_NUMBER; i++)
                if (units[i] == -1)
                {
                    j = i; //j - nr slotu, który zostanie zajęty
                    break;
                }
                units[j] = unit_socket_descriptor;
                //jednostka połączona
                current_units_online++;
                pthread_mutex_lock(&units_mutex);
                handleUnit(unit_socket_descriptor, j);
        }
        else
        {
            //czekanie aż się zwolni miejsce na jednostkę
        }
    }
}

#endif //CALC_UNIT_H