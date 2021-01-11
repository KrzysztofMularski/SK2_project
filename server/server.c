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
#include "data_queue.h"
#include "result_queue.h"
#include "calc_unit.h"
#include "constants.h"

Node queue; // inicjalizowanie kolejki klientów, którzy czekają na miejsce na serwerze
int server_slots[MAX_CLIENTS_ONLINE]; // tablica deskryptorów aktualnie obsługiwanych klientów
int current_online_clients = 0; // klienci obsługiwani
int current_queue_size = 0; // klienci czekający w kolejce
// online + queue = liczba wszystkich klientów połączonych z serwerem
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER; //current_online_clients
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_full_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t client_gone = PTHREAD_COND_INITIALIZER;


struct thread_data_t
{
    int my_socket;
    int id_in_desc_array;
    int mat_order;
};

void* ResultBehavior(void* t_data)
{
    // to do:
    // po nieoczekiwanym rozłączeniu z klientem, ResultBehavior nadal będzie próbować pobrać wszystkie dane
    // może sygnał, albo zmienna i mutex przekazywane przez strukturę do ResultBehavior, które będą stopować odczytywanie wyników
    struct thread_data_t *th_data = (struct thread_data_t*)t_data;
    struct result_data* r_data;
    
    int iorder = th_data->mat_order;
    //str: i;j;number
    //str = 2;3;5.43
    char* str = (char*)malloc((ISIZE+ISIZE+DSIZE+3)*sizeof(char));
    
    for (int i=0; i<iorder*iorder; i++)
    {
        r_data = get_result(th_data->id_in_desc_array);
        sprintf(str, "%i;%i;%sE", r_data->i, r_data->j, r_data->number);
        write(th_data->my_socket, str, strlen(str));
        sleep(SLOWDOWN);
    }
    free(str);
    
    /*
    sprintf(str, "%d;%d;%sE", 1, 0, "10");
    write(th_data->my_socket, str, strlen(str));
    sleep(SLOWDOWN);
    sprintf(str, "%d;%d;%sE", 0, 0, "To");
    write(th_data->my_socket, str, strlen(str));
    sleep(SLOWDOWN);
    sprintf(str, "%d;%d;%sE", 1, 1, "fejk");
    write(th_data->my_socket, str, strlen(str));
    sleep(SLOWDOWN);
    sprintf(str, "%d;%d;%sE", 0, 1, "0.1");
    write(th_data->my_socket, str, strlen(str));
    sleep(SLOWDOWN);
    free(str);
    */
    //free(r_data);
    return NULL;
}


void *ThreadBehavior(void *t_data)
{ // kontrola błędów u klienta, tutaj tylko prawidłowo podane dane
    pthread_detach(pthread_self());
    struct thread_data_t *th_data = (struct thread_data_t*)t_data;
    // odczyt danych od klienta
    char* mat_order = (char*)malloc(ISIZE*sizeof(char));
    char*** M1;
    char*** M2;
    int M12mallocFlag = 0;
    char* line;
    char* number = (char*)malloc(DSIZE*sizeof(char));
    int imat_order;
    for(int one=0; one<1; one++)
    {
        write(th_data->my_socket, "o\n", 2);
        //printf("o sent\n");
        //fflush(stdout);
        sleep(SLOWDOWN);
        int size = read(th_data->my_socket, mat_order, ISIZE);
        //printf("mat_order: %s\n", mat_order);
        //fflush(stdout);
        if (!size)
            break;
        mat_order[size] = '\0';
        sscanf(mat_order, "%i", &imat_order);
        
        // deklaracja macierzy 1 i 2
        M1 = (char***)malloc(imat_order*sizeof(char**));
        M2 = (char***)malloc(imat_order*sizeof(char**));
        for(int i=0; i<imat_order; i++)
        {
            M1[i] = (char**)malloc(imat_order*sizeof(char*));
            M2[i] = (char**)malloc(imat_order*sizeof(char*));
            for(int j=0; j<imat_order; j++)
            {
                M1[i][j] = (char*)malloc(DSIZE*sizeof(char));
                M2[i][j] = (char*)malloc(DSIZE*sizeof(char));
            }
        }
        line = (char*)malloc(imat_order*DSIZE*sizeof(char));
        M12mallocFlag = 1;
        // pobieranie wartości macierzy od klienta

        for (int i=0; i<imat_order; i++)
        {
            write(th_data->my_socket, "r\n", 2);
            //printf("r sent\n");
            //fflush(stdout);
            sleep(SLOWDOWN);
            size = read(th_data->my_socket, line, imat_order*(DSIZE+1));
            if (!size)
                break;
            //printf("## line: %s\n", line);
            //fflush(stdout);
            line[size] = '\0';
            char* token = strtok(line, "|");
            //printf("## Separated line: ");
            for (int j=0; j<imat_order; j++)
            {
                //printf( " %s", token );
                strcpy(M1[i][j], token);
                token = strtok(NULL, "|");
            }
            //printf("\n");
            //fflush(stdout);
            if (!size)
                break;
        }
        if (!size)
            break;

        for (int i=0; i<imat_order; i++)
        {
            write(th_data->my_socket, "c\n", 2);
            //printf("c sent\n");
            //fflush(stdout);
            sleep(SLOWDOWN);
            size = read(th_data->my_socket, line, imat_order*(DSIZE+1));
            if (!size)
                break;
            //printf("## line: %s\n", line);
            fflush(stdout);
            line[size] = '\0';
            char* token = strtok(line, "|");
            //printf("## Separated line: ");
            for (int j=0; j<imat_order; j++)
            {
                //printf( " %s", token );
                strcpy(M2[i][j], token);
                token = strtok(NULL, "|");
            }
            //printf("\n");
            //fflush(stdout);
            if (!size)
                break;
        }
        if (!size)
            break;
        sleep(SLOWDOWN);
        //printf("## 180\n"); fflush(stdout);
        //nowy wątek do odbierania wyników
        pthread_t result_thread;
        // otrzymywanie wyniku dla klienta
        th_data->mat_order = imat_order;
        int create_result = pthread_create(&result_thread, NULL, ResultBehavior, (void *)th_data);
        if (create_result){
        printf("Błąd przy próbie utworzenia wątku, kod błędu: %d\n", create_result);
        exit(-1);
        }
        // przekazanie danych do jednostek obliczeniowych
        //printf("## 191\n"); fflush(stdout);
        for (int i=0; i<imat_order; i++)
        {
            for (int j=0; j<imat_order; j++)
            {
                //wysyłanie danych do obliczenia
                //printf("## 197\n"); fflush(stdout);
                put_data_in_queue(th_data->id_in_desc_array, i, j, imat_order, mat_order, M1[i], M2[j]);
            }
        }
        
        pthread_join(result_thread, NULL);
        //printf("## 204\n");
        //fflush(stdout);
    }
    //printf("## 207\n");
    //fflush(stdout);
    char ck[2];
    read(th_data->my_socket, ck, 2);
    close(th_data->my_socket);
    pthread_mutex_lock(&clients_mutex);
    server_slots[th_data->id_in_desc_array] = -1;
    current_online_clients--;
    printf("current_online_clients: %d\n", current_online_clients);
    pthread_mutex_unlock(&clients_mutex);
    printf("Współpraca z klientem %d zakończyła się powodzeniem!\n", th_data->id_in_desc_array);
    pthread_cond_signal(&client_gone);
    //printf("freeing mat_order\n");
    free(mat_order);
    //printf("freeing number\n");
    free(number);

    if (M12mallocFlag)
    {
        for(int i=0; i<imat_order; i++)
        {
            for(int j=0; j<imat_order; j++)
            {
                //printf("freeing Mij\n");
                //fflush(stdout);
                free(M1[i][j]);
                free(M2[i][j]);
            }
            //printf("freeing Mi\n");
            //fflush(stdout);
            free(M1[i]);
            free(M2[i]);
        }
        //printf("freeing M\n");
        //fflush(stdout);
        free(M1);
        free(M2);
        //printf("freeing line\n");
        free(line);
    }
    //printf("freeing th_data\n");
    free(th_data);

    pthread_exit(NULL);
}

void handleConnection(int connection_socket_descriptor, int id)
{
    int create_result = 0;
    pthread_t thread1;

    struct thread_data_t* t_data = malloc(sizeof(struct thread_data_t));
    t_data->my_socket = connection_socket_descriptor;
    t_data->id_in_desc_array = id;

    create_result = pthread_create(&thread1, NULL, ThreadBehavior, (void *)t_data);
    if (create_result){
       printf("Błąd przy próbie utworzenia wątku, kod błędu: %d\n", create_result);
       exit(-1);
    }
}

void putting_client_in(int desc)
{
    pthread_mutex_lock(&clients_mutex);
    //dodawanie do wolnego slotu
    int j = -1;
    for (int i=0; i<MAX_CLIENTS_ONLINE; i++)
        if (server_slots[i] == -1)
        {
            j = i; // j - nr slotu, który zostanie zajęty
            break;
        }
    server_slots[j] = desc;
    
    current_online_clients++;
    pthread_mutex_unlock(&clients_mutex);
    // klient połączony
    printf("----------------------------------------\n");
    printf("Klient %d o deskryptorze %d połączony!\n", j, desc);
    printf("current_online_clients: %d\n", current_online_clients);
    printf("current_queue_size: %d\n", current_queue_size);
    handleConnection(desc, j);
}

void *queue_management()
{
    int desc;
    int i = 1;
    char str[10];
    Node p;
    //kiedy klient zostanie obsłużony, kolejna osoba z kolejki wchodzi w slot serwera
    while(1)
    {
        pthread_mutex_lock(&queue_mutex);
        pthread_cond_wait(&client_gone, &queue_mutex);
        if (current_queue_size == 0)
        {
            pthread_mutex_unlock(&queue_mutex);
            continue;
        }
        desc = queue->descriptor;
        write(desc, "0E", 2);
        queue = removeHead(queue);
        current_queue_size--;
        i = 1;
        p = queue;

        if(queue != NULL)
        {
            sprintf(str, "%dE", i);
            write(queue->descriptor, str, strlen(str));
            i++;
            p = queue;
            while(p->next != NULL)
            {
                sprintf(str, "%dE", i);
                write(p->next->descriptor, str, strlen(str));
                i++;
                p = p->next;
            }
        }

        pthread_mutex_unlock(&queue_mutex);

        putting_client_in(desc);
    }
}

int main(int argc, char* argv[])
{
    printf("Start serwera\n");
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
    
    int connection_socket_descriptor;

    pthread_t queue_manager;
    create_result = pthread_create(&queue_manager, NULL, queue_management, NULL);
    if (create_result)
    {
        printf("Błąd przy próbie utworzenia wątku, kod błędu: %d\n", create_result);
        exit(-1);
    }
    
    init_results_queue_arr();
    while(1)
    {
        pthread_mutex_lock(&queue_mutex);
        if (current_queue_size >= MAX_QUEUE_SIZE) //znak ">" dla bezpieczeństwa
        {
            pthread_mutex_unlock(&queue_mutex);
            //cała kolejka zajęta
            pthread_mutex_lock(&queue_full_mutex);
            pthread_cond_wait(&client_gone, &queue_full_mutex);
            pthread_mutex_unlock(&queue_full_mutex);
        }
        if (current_queue_size < MAX_QUEUE_SIZE)
        {
            pthread_mutex_unlock(&queue_mutex);
            printf("Oczekiwanie na klienta..\n");
            connection_socket_descriptor = accept(server_socket_descriptor, NULL, NULL);
            if (connection_socket_descriptor < 0)
            {
                fprintf(stderr, "%s: Błąd przy próbie utworzenia gniazda dla połączenia.\n", argv[0]);
                exit(1);
            }
            
            if (current_online_clients < MAX_CLIENTS_ONLINE)
            {
                putting_client_in(connection_socket_descriptor);
            }
            else
            {
                //dodawanie do kolejki
                pthread_mutex_lock(&queue_mutex);
                printf("Klient %d dodany do kolejki!\n", connection_socket_descriptor);
                queue = addNode(queue, connection_socket_descriptor);
                current_queue_size++;

                int n = current_queue_size;
                
                printf("Obecna długość kolejki: %d\n", current_queue_size);
                pthread_mutex_unlock(&queue_mutex);

                write(connection_socket_descriptor, "q\n", 2);
                char str[10];
                sprintf(str, "%dE", n);
                printf("----- n: %d\n", n);
                fflush(stdout);
                write(connection_socket_descriptor, str, strlen(str));
            }
        }
    }
}