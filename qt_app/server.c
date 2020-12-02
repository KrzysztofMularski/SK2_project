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
#include "queue.h"
#include "calc_unit.h"

#define SERVER_PORT 1234
#define MAX_CLIENTS_ONLINE 3
#define MAX_QUEUE_SIZE 100
#define MAX_NUMBER_SIZE 100
#define ROW_SIZE 1024

int server_slots[MAX_CLIENTS_ONLINE]; // tablica deskryptorów aktualnie obsługiwanych klientów
int current_online_clients = 0; // klienci obsługiwani
int current_queue_size = 0; // klienci czekający w kolejce
// online + queue = liczba wszystkich klientów połączonych z serwerem
pthread_mutex_t current_clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_descriptors_array();

struct thread_data_t
{
    int my_socket;
    int id_in_desc_array;
};


//void *ThreadBehavior(void *t_data)
void ThreadBehavior(struct thread_data_t* t_data)
{ // kontrola błędów u klienta, tutaj tylko prawidłowo podane dane
    //pthread_detach(pthread_self());
    struct thread_data_t *th_data = (struct thread_data_t*)t_data;
    // odczyt danych od klienta
    int mat_order;
    double** M1;
    double** M2;
    // stopień macierzy
    char mat_size[MATRIX_ORDER];
    read(th_data->my_socket, mat_size, MATRIX_ORDER);
    write(th_data->my_socket, ".", 2);
    sscanf(mat_size, "%i", &mat_order);
    // Macierz 1 i 2
    M1 = (double**)malloc(mat_order*sizeof(double*));
    M2 = (double**)malloc(mat_order*sizeof(double*));
    for(int i=0; i<mat_order; i++)
    {
        M1[i] = (double*)malloc(mat_order*sizeof(double));
        M2[i] = (double*)malloc(mat_order*sizeof(double));
    }
    char number[MAX_NUMBER_SIZE];
    double dnumber;
    char *eptr;
    for (int i=0; i<mat_order; i++)
    {
        for (int j=0; j<mat_order; j++)
        {
            read(th_data->my_socket, number, MAX_NUMBER_SIZE);
            M1[i][j] = strtod(number, &eptr);
        }
        write(th_data->my_socket, ".", 2);
    }
    write(th_data->my_socket, "'", 2);
    for (int i=0; i<mat_order; i++)
    {
        for (int j=0; j<mat_order; j++)
        {
            read(th_data->my_socket, number, MAX_NUMBER_SIZE);
            M2[i][j] = strtod(number, &eptr);
        }
        write(th_data->my_socket, ".", 2);
    }
    // przekazanie danych do jednostek obliczeniowych
    

}

void handleConnection(int connection_socket_descriptor, int id)
{
    int create_result = 0;
    pthread_t thread1;

    struct thread_data_t* t_data = malloc(sizeof(struct thread_data_t));
    t_data->my_socket = connection_socket_descriptor;
    t_data->id_in_desc_array = id;

    ThreadBehavior(t_data);
    //create_result = pthread_create(&thread1, NULL, ThreadBehavior, (void *)t_data);
    if (create_result){
       printf("Błąd przy próbie utworzenia wątku, kod błędu: %d\n", create_result);
       exit(-1);
    }
}

int main(int argc, char* argv[])
{
    pthread_t calc_unit;
    int create_result = pthread_create(&calc_unit, NULL, calculation_unit, NULL);
    if (create_result){
       printf("Błąd przy próbie utworzenia wątku, kod błędu: %d\n", create_result);
       exit(-1);
    }
    struct sockaddr_in server_address;

    memset(&server_address, 0, sizeof(struct sockaddr));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(SERVER_PORT);

    int server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_descriptor < 0)
    {
        fprintf(stderr, "%s: Błąd przy próbie utworzenia gniazda..\n", argv[0]);
        exit(1);
    }
    char reuse_addr_val = 1;
    setsockopt(server_socket_descriptor, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse_addr_val, sizeof(reuse_addr_val));

    int bind_result = bind(server_socket_descriptor, (struct sockaddr*)&server_address, sizeof(struct sockaddr));
    if (bind_result < 0)
    {
        fprintf(stderr, "%s: Błąd przy próbie dowiązania adresu IP i numeru portu do gniazda.\n", argv[0]);
        exit(1);
    }

    int listen_result = listen(server_socket_descriptor, MAX_CLIENTS_ONLINE);
    if (listen_result < 0) {
        fprintf(stderr, "%s: Błąd przy próbie ustawienia wielkości kolejki.\n", argv[0]);
        exit(1);
    }

    init_descriptors_array('c'); // ustawia wartość wszystkich elementów tablicy deskryptorów na -1
    Node queue; // inicjalizowanie kolejki klientów, którzy czekają na miejsce na serwerze
    int connection_socket_descriptor;
    while(1)
    {
        if (current_queue_size < MAX_QUEUE_SIZE)
        {
            connection_socket_descriptor = accept(server_socket_descriptor, NULL, NULL);
            if (connection_socket_descriptor < 0)
            {
                fprintf(stderr, "%s: Błąd przy próbie utworzenia gniazda dla połączenia.\n", argv[0]);
                exit(1);
            }
            if (current_online_clients < MAX_CLIENTS_ONLINE)
            {
                //dodawanie do wolnego slotu
                int j = -1;
                for (int i=0; i<MAX_CLIENTS_ONLINE; i++)
                    if (server_slots[i] == -1)
                    {
                        j = i; // j - nr slotu, który zostanie zajęty
                        break;
                    }
                server_slots[j] = connection_socket_descriptor;
                // klient połączony
                handleConnection(connection_socket_descriptor, j);
            }
            else
            {
                //dodawanie do kolejki
                queue = addNode(queue, connection_socket_descriptor);
            }
        }
        else
        {
            //cała kolejka zajęta
            //to do: zrobić jakieś nieaktywne czekanie
        }
    }
}

void init_descriptors_array(char obj)
{
    if (obj == 'c') // clients
        for (int i=0; i<MAX_CLIENTS_ONLINE; i++)
            server_slots[i] = -1;
    else if (obj == 'u')
        for (int i=0; i<MAX_UNITS_NUMBER; i++)
            units[i] = -1;
}