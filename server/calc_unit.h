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
#include "data_queue.h"
#include "constants.h"

int units[MAX_UNITS_NUMBER];
int current_units_online = 0;
Data_Position data_queue = NULL;
Result_Position result_queue[MAX_CLIENTS_ONLINE];
extern pthread_mutex_t clients_mutex;
extern int server_slots[MAX_CLIENTS_ONLINE];

pthread_mutex_t units_mutex = PTHREAD_MUTEX_INITIALIZER; //units[], current_units_online
pthread_cond_t unit_gone = PTHREAD_COND_INITIALIZER;
pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER; //data_queue
pthread_cond_t data_arrived_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t result_mutex[MAX_CLIENTS_ONLINE]; //result_queue[MAX_CLIENTS_ONLINE];
pthread_cond_t result_arrived_cond[MAX_CLIENTS_ONLINE];

//temp
pthread_mutex_t result_mut = PTHREAD_MUTEX_INITIALIZER;

struct unit_data
{
    int my_descriptor;
};

struct result_data
{
    int i;
    int j;
    char number[DSIZE];
};

void init_descriptors_array(char obj)
{
    if (obj == 'c') // clients
    {
        pthread_mutex_lock(&clients_mutex);
        for (int i=0; i<MAX_CLIENTS_ONLINE; i++)
            server_slots[i] = -1;
        pthread_mutex_unlock(&clients_mutex);
    }
    else if (obj == 'u') // units
    {
        pthread_mutex_lock(&units_mutex);
        for (int i=0; i<MAX_UNITS_NUMBER; i++)
            units[i] = -1;
        pthread_mutex_unlock(&units_mutex);
    } 
}

void init_results_queue_arr()
{
    for (int i=0; i<MAX_CLIENTS_ONLINE; i++)
        result_queue[i] = NULL;
}

void init_m_u()
{
    for (int i=0; i<MAX_CLIENTS_ONLINE; i++)
    {
        pthread_mutex_init(&result_mutex[i], NULL);
        pthread_cond_init(&result_arrived_cond[i], NULL);
    }
}

void put_data_in_queue(int client_id, int i, int j, int imat_order, char* mat_order, char** row, char** col)
{
    pthread_mutex_lock(&data_mutex);
    data_queue = addDataPosition(client_id, i, j, data_queue, imat_order, mat_order, row, col);
    pthread_cond_signal(&data_arrived_cond);
    pthread_mutex_unlock(&data_mutex);
}

struct result_data* get_result(int client_id)
{
    struct result_data* temp = (struct result_data*)malloc(sizeof(struct result_data));
    //pthread_mutex_lock(&result_mutex[client_id]);
    pthread_mutex_lock(&result_mut);
    if(result_queue[client_id] == NULL)
        pthread_cond_wait(&result_arrived_cond[client_id], &result_mut);
        //pthread_cond_wait(&result_arrived_cond[client_id], &result_mutex[client_id]);
    temp->i = result_queue[client_id]->row;
    temp->j = result_queue[client_id]->col;
    strcpy(temp->number, result_queue[client_id]->number);
    //temp->number = result_queue[client_id]->number;
    result_queue[client_id] = removeResultHead(result_queue[client_id]);
    
    pthread_mutex_unlock(&result_mut);
    //pthread_mutex_unlock(&result_mutex[client_id]);

    return temp;
}

void *UnitBehavior(void *u_data)
{
    pthread_detach(pthread_self());
    struct unit_data* un_data = (struct unit_data*)u_data;
    int my_desc = un_data->my_descriptor;

    int client_id;
    int i;
    int j;
    int iorder;
    char* order;
    char** row;
    char** col;

    char* checkpoint = (char*)malloc(CHECKPOINT_SIZE*sizeof(char));
    char* result = (char*)malloc(DSIZE*sizeof(char));
    int size = 1;
    while(1)
    {
        pthread_mutex_lock(&data_mutex);
        //czekanie na dane
        if(data_queue == NULL)
        {
            pthread_cond_wait(&data_arrived_cond, &data_mutex);
        }
        pthread_mutex_unlock(&data_mutex);
        
        //sprawdzanie czy jednostka jest aktywna
        write(my_desc, "?", 1);
        size = read(my_desc, checkpoint, 1);
        
        if (!size)
        {
            break;
        }
        else if (checkpoint[0] != 'o')
        {
            break;
        }
        //odczyt danych z kolejki danych do przetworzenia
        pthread_mutex_lock(&data_mutex);
        if (data_queue != NULL)
        {
            client_id = data_queue->client_id;
            i = data_queue->i;
            j = data_queue->j;
            iorder = data_queue->imat_order;
            order = data_queue->mat_order;
            row = data_queue->mat_1_row;
            col = data_queue->mat_2_col;

            data_queue = removeDataHead(data_queue);
            pthread_mutex_unlock(&data_mutex);
        }
        else
        {
            pthread_mutex_unlock(&data_mutex);
            continue;
        }
        //wysyłanie danych do jednostki obliczeniowej
        write(my_desc, order, strlen(order));

        for (int k=0; k<iorder; k++)
        {
            size = read(my_desc, checkpoint, 1);
            if (!size) break;
            else if (checkpoint[0] != 'r') break;
            write(my_desc, row[k], strlen(row[k]));
        }
        if (!size) break;

        for (int k=0; k<iorder; k++)
        {
            size = read(my_desc, checkpoint, 1);
            if (!size) break;
            else if (checkpoint[0] != 'c') break;
            write(my_desc, col[k], strlen(col[k]));
        }
        if (!size) break;
        size = read(my_desc, checkpoint, 1);
        if (!size) break;
        else if (checkpoint[0] != 'E') break;
        //odczyt obliczonego wyniku
        result[0] = '\0';
        write(my_desc, "s", 1);
        size = read(my_desc, result, DSIZE);
        if (!size) break;

        result[size] = '\0';

        //wysyłanie potwierdzenia otrzymania wyniku
        write(my_desc, "E", 1);

        //umieszczanie wyniku w kolejce wyników konkretnego klienta
        pthread_mutex_lock(&result_mut);
        //pthread_mutex_lock(&result_mutex[client_id]);
        result_queue[client_id] = addResultPosition(result_queue[client_id], i, j, result);

        pthread_mutex_unlock(&result_mut);
        //pthread_mutex_unlock(&result_mutex[client_id]);
        pthread_cond_signal(&result_arrived_cond[client_id]);
    }
    free(checkpoint);
    free(un_data);
    
    pthread_mutex_lock(&units_mutex);
    current_units_online--;
    printf("current_units_online: %d\n", current_units_online);
    pthread_mutex_unlock(&units_mutex);

    printf("Współpraca z jednostką %d zakończyła się powodzeniem\n", my_desc);
    fflush(stdout);
    close(my_desc);
    return NULL;
}

void handleUnit(int unit_socket_descriptor)
{
    int create_result = 0;
    pthread_t calc_unit;

    struct unit_data* u_data = malloc(sizeof(struct unit_data));
    u_data->my_descriptor = unit_socket_descriptor;

    create_result = pthread_create(&calc_unit, NULL, UnitBehavior, (void*) u_data);
    if (create_result){
       printf("Błąd przy próbie utworzenia wątku, kod błędu: %d\n", create_result);
       exit(-1);
    }
}

void* calculation_unit()
{
    init_m_u();
    struct sockaddr_in unit_address;

    memset(&unit_address, 0, sizeof(struct sockaddr));
    unit_address.sin_family = AF_INET;
    unit_address.sin_addr.s_addr = htonl(INADDR_ANY);
    unit_address.sin_port = htons(UNIT_PORT);

    int unit_server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (unit_server_socket_descriptor < 0)
    {
        fprintf(stderr, "UNIT: Błąd przy próbie utworzenia gniazda..\n");
        exit(1);
    }
    char reuse_addr_val = 1;
    setsockopt(unit_server_socket_descriptor, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse_addr_val, sizeof(reuse_addr_val));

    int bind_result = bind(unit_server_socket_descriptor, (struct sockaddr*)&unit_address, sizeof(struct sockaddr));
    if (bind_result < 0)
    {
        fprintf(stderr, "UNIT: Błąd przy próbie dowiązania adresu IP i numeru portu do gniazda.\n");
        exit(1);
    }

    int listen_result = listen(unit_server_socket_descriptor, MAX_UNITS_NUMBER);
    if (listen_result < 0) {
        fprintf(stderr, "UNIT: Błąd przy próbie ustawienia wielkości kolejki.\n");
        exit(1);
    }

    init_descriptors_array('u');
    int unit_socket_descriptor;
    while(1)
    {
        pthread_mutex_lock(&units_mutex);
        if (current_units_online >= MAX_UNITS_NUMBER)
        {
            //czekanie aż się zwolni miejsce na jednostkę
            pthread_cond_wait(&unit_gone, &units_mutex);
        }
        if (current_units_online < MAX_UNITS_NUMBER)
        {
            pthread_mutex_unlock(&units_mutex);
            //obsłużenie jednostki
            unit_socket_descriptor = accept(unit_server_socket_descriptor, NULL, NULL);
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
            pthread_mutex_unlock(&units_mutex);
            handleUnit(unit_socket_descriptor);
            printf("Dołączono jednostkę o desc: %d\n", unit_socket_descriptor);
            fflush(stdout);
        }
    }
}

#endif //CALC_UNIT_H