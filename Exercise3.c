#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>
#include <time.h>
/*
 * Starvation-free producer/consumer bounded-buffer problem with readers.
 * Implemented here are semaphores to protect from concurrent reading and writing, but also semaphores that indicate whether the buffer is full or empty.
 * These new semaphores give priority to producers, allowing them to skip line, and populate the buffer when it is empty.
 * Similarly, consumers are given priority once the buffer is full.
 */


#define BUFFER_SIZE 64
#define NUMBER_PRODUCERS 3
#define NUMBER_CONSUMERS 3
#define NUMBER_READERS 20
#define OPERATIONS_PER_PRODUCER 10
#define OPERATIONS_PER_CONSUMER 10
#define OPERATIONS_PER_READER 5


int buffer[BUFFER_SIZE];
int itemsInBuffer = 0;
int readersInBuffer = 0;

sem_t mutex, wrt, full, empty, fullPriority, emptyPriority;

void* producerFunction (void *arg) {
    int producerId = *(int *)arg;

    for (int op = 0; op < OPERATIONS_PER_PRODUCER; op++) {
        sem_wait(&full);         // Wait if buffer is full
        sem_wait(&fullPriority); // Get priority over readers if full
        sem_wait(&wrt);          // Critical section

        int item = rand() % 100;
        buffer[itemsInBuffer] = item;
        itemsInBuffer = itemsInBuffer + 1;
        printf("[Producer %d] Added %d (items in buffer: %d)\n", producerId, item, itemsInBuffer);

        sem_post(&wrt);
        sem_post(&fullPriority);
        sem_post(&empty);
        usleep(10000);
    }

    return NULL;
}

void* consumerFunction (void *arg) {
    int consumerId = *(int *)arg;

    for (int op = 0; op < OPERATIONS_PER_CONSUMER; op++) {
        sem_wait(&empty);         // Wait if buffer is empty
        sem_wait(&emptyPriority); // Get priority over readers if empty
        sem_wait(&wrt);           // Critical section

        itemsInBuffer = itemsInBuffer - 1;
        int item = buffer[itemsInBuffer];
        printf("[Consumer %d] Removed %d (items in buffer: %d)\n", consumerId, item, itemsInBuffer);

        sem_post(&wrt);
        sem_post(&emptyPriority);
        sem_post(&full);
        usleep(12000);
    }

    return NULL;
}

void* readerFunction (void *arg) {
    int readerId = *(int *)arg;

    for (int op = 0; op < OPERATIONS_PER_READER; op++) {
        sem_wait(&fullPriority);  // Blocked if full
        sem_post(&fullPriority);
        sem_wait(&emptyPriority); // Blocked if buffer is empty
        sem_post(&emptyPriority);

        sem_wait(&mutex);
        readersInBuffer = readersInBuffer + 1;
        if(readersInBuffer == 1) { // First reader blocks writers
            sem_wait(&wrt);
        }
        sem_post(&mutex);

        int readItem = -1;
        if(itemsInBuffer > 0) {
            readItem = buffer[itemsInBuffer - 1];
            printf("[Reader %d] Read %d (items in buffer: %d)\n", readerId, readItem, itemsInBuffer);
        } else {
            printf("[Reader %d] Skipped, buffer empty\n", readerId);
        }

        sem_wait(&mutex);
        readersInBuffer = readersInBuffer - 1;
        if(readersInBuffer == 0) { // Last reader releases writers
            sem_post(&wrt);
        }
        sem_post(&mutex);
        usleep(8000);
    }

    return NULL;
}

int main() {
    srand((unsigned int)time(NULL));
    sem_init(&mutex, 0, 1);
    sem_init(&wrt, 0, 1);
    sem_init(&full, 0, BUFFER_SIZE);
    sem_init(&empty, 0, 0);
    sem_init(&fullPriority, 0, 1);
    sem_init(&emptyPriority, 0, 1);
    pthread_t producerthreads[NUMBER_PRODUCERS];
    pthread_t consumerthreads[NUMBER_CONSUMERS];
    pthread_t readerthreads[NUMBER_READERS];
    int producerIds[NUMBER_PRODUCERS];
    int consumerIds[NUMBER_CONSUMERS];
    int readerIds[NUMBER_READERS];

    for (int i = 0; i < NUMBER_PRODUCERS; i++) {
        producerIds[i] = i;
    }
    for (int i = 0; i < NUMBER_CONSUMERS; i++) {
        consumerIds[i] = i;
    }
    for (int i = 0; i < NUMBER_READERS; i++) {
        readerIds[i] = i;
    }

    int producersCreated = 0;
    int consumersCreated = 0;
    int readersCreated = 0;
    int totalToCreate = NUMBER_PRODUCERS + NUMBER_CONSUMERS + NUMBER_READERS;

    for (int i = 0; i < totalToCreate; i++) {
        int roll = rand() % 100;

        if (roll < 60 && readersCreated < NUMBER_READERS) {
            pthread_create(&readerthreads[readersCreated], NULL, readerFunction, &readerIds[readersCreated]);
            readersCreated++;
        } else if (roll < 80 && producersCreated < NUMBER_PRODUCERS) {
            pthread_create(&producerthreads[producersCreated], NULL, producerFunction, &producerIds[producersCreated]);
            producersCreated++;
        } else if (consumersCreated < NUMBER_CONSUMERS) {
            pthread_create(&consumerthreads[consumersCreated], NULL, consumerFunction, &consumerIds[consumersCreated]);
            consumersCreated++;
        } else if (readersCreated < NUMBER_READERS) {
            pthread_create(&readerthreads[readersCreated], NULL, readerFunction, &readerIds[readersCreated]);
            readersCreated++;
        } else if (producersCreated < NUMBER_PRODUCERS) {
            pthread_create(&producerthreads[producersCreated], NULL, producerFunction, &producerIds[producersCreated]);
            producersCreated++;
        } else {
            pthread_create(&consumerthreads[consumersCreated], NULL, consumerFunction, &consumerIds[consumersCreated]);
            consumersCreated++;
        }
    }

    for (int i = 0; i < NUMBER_PRODUCERS; i++) {
        pthread_join(producerthreads[i], NULL);
    }
    for (int i = 0; i < NUMBER_CONSUMERS; i++) {
        pthread_join(consumerthreads[i], NULL);
    }
    for (int i = 0; i < NUMBER_READERS; i++) {
        pthread_join(readerthreads[i], NULL);
    }

    sem_destroy(&mutex);
    sem_destroy(&wrt);
    sem_destroy(&full);
    sem_destroy(&empty);
    sem_destroy(&fullPriority);
    sem_destroy(&emptyPriority);
    return 0;
}