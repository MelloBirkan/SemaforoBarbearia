#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define NUM_CLIENTES 7
#define TEMPO_MAX_CORTE 10
#define TEMPO_MIN_CORTE 4
#define NUM_CADEIRAS 5

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t cadeira_corte;
sem_t cadeira_espera;
sem_t barbeiro_disponivel;

int clientes_esperando = 0;
int cliente_atual = 0;

void *barbeiro(void *arg) {
    while (1) {
        sem_wait(&cadeira_corte);
        pthread_mutex_lock(&mutex);

        printf("Barbeiro cortando o cabelo do cliente %d\n", cliente_atual);
        int tempo_corte = rand() % (TEMPO_MAX_CORTE - TEMPO_MIN_CORTE + 1) + TEMPO_MIN_CORTE;
        sleep(tempo_corte);
        printf("Barbeiro terminou de cortar o cabelo do cliente %d\n", cliente_atual);

        pthread_mutex_unlock(&mutex);
        sem_post(&barbeiro_disponivel);
    }
    return NULL;
}

void *cliente(void *arg) {
    int id = *(int *)arg;

    pthread_mutex_lock(&mutex);
    if (clientes_esperando < NUM_CADEIRAS) {
        clientes_esperando++;
        printf("Cliente %d esperando na cadeira de espera\n", id);

        pthread_mutex_unlock(&mutex);
        sem_wait(&cadeira_espera);

        pthread_mutex_lock(&mutex);
        clientes_esperando--;
        cliente_atual = id;
        printf("Cliente %d sentado na cadeira de corte\n", id);

        pthread_mutex_unlock(&mutex);
        sem_post(&cadeira_corte);
        sem_wait(&barbeiro_disponivel);
        sem_post(&cadeira_espera);
    } else {
        printf("Cliente %d foi embora, pois todas as cadeiras de espera estão ocupadas\n", id);
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    pthread_t thr_barbeiro, thr_clientes[NUM_CLIENTES];
    int ids_clientes[NUM_CLIENTES];

    sem_init(&cadeira_corte, 0, 0);
    sem_init(&cadeira_espera, 0, NUM_CADEIRAS);
    sem_init(&barbeiro_disponivel, 0, 1);

    pthread_create(&thr_barbeiro, NULL, barbeiro, NULL);

    for (int i = 0; i < NUM_CLIENTES; i++) {
        ids_clientes[i] = i + 1;
        pthread_create(&thr_clientes[i], NULL, cliente, &ids_clientes[i]);
        sleep(2);
    }

    for (int i = 0; i < NUM_CLIENTES; i++) {
        pthread_join(thr_clientes[i], NULL);
    }

    return 0;
}
