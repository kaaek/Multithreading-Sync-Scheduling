#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
/*
 * This exercise demonstrates multithreading scheduling using semaphores: a number M of bank accounts are shared among N threads.
 * Each bank account's balance is protected by a semaphore to avoid race conditions.
 * The global ledger is also protected by its own semaphore to avoid multiple threads writing to the same address at the same time.
 * Compiling & running prints a log including: all the accounts' initial balances, transaction logs, and final balances.
 */

#define NUMBER_ITERATIONS 10
#define NUMBER_BANK_ACCOUNTS 5
#define NUMBER_USER_THREADS 10
#define LOG_SIZE NUMBER_USER_THREADS*NUMBER_ITERATIONS

#define BALANCE_LOWER_BOUND 500
#define BALANCE_UPPER_BOUND 1500
#define TRANSCATION_LOWER_BOUND 50
#define TRANSCATION_UPPER_BOUND 300

#define BUFFER_SIZE 64

char transactionLog[LOG_SIZE][BUFFER_SIZE];
int logIndex = 0;

sem_t logSemaphore;

struct SharedBankAccount {
    int balance;
    sem_t semaphore;
};

struct SharedBankAccount accounts[NUMBER_BANK_ACCOUNTS];

void logDeposit(int accountIndex, int depositAmount){
    sem_t * semaphorePointer = &logSemaphore;
    sem_wait(semaphorePointer);
    snprintf(transactionLog[logIndex], BUFFER_SIZE, "Account %d: Deposit of amount $%d", accountIndex, depositAmount);
    logIndex = logIndex + 1;
    sem_post(semaphorePointer);
}

void logWithdrawSuccess(int accountIndex, int withdrawAmount){
    sem_t * semaphorePointer = &logSemaphore;
    sem_wait(semaphorePointer);
    snprintf(transactionLog[logIndex], BUFFER_SIZE, "Account %d: Withdraw succeeded of amount $%d", accountIndex, withdrawAmount);
    logIndex = logIndex + 1;
    sem_post(semaphorePointer);
}

void logWithdrawFailure(int accountIndex, int withdrawAmount){
    sem_t * semaphorePointer = &logSemaphore;
    sem_wait(semaphorePointer);
    snprintf(transactionLog[logIndex], BUFFER_SIZE, "Account %d: Withdraw failed (amount $%d)", accountIndex, withdrawAmount);
    logIndex = logIndex + 1;
    sem_post(semaphorePointer);
}

void accountInit(struct SharedBankAccount* account, int accountIndex){
    int initialBalance = rand() % (BALANCE_UPPER_BOUND - BALANCE_LOWER_BOUND + 1) + BALANCE_LOWER_BOUND;
    account->balance = initialBalance;
    printf("Account %d: Initial balance = $%d\n", accountIndex, initialBalance);
    sem_init(&(account->semaphore), 0, 1); // 0: Semaphore coordinates between threads (not processes), 1: initial value
}

int deposit(struct SharedBankAccount* account, int depositAmount, int accountIndex){
    sem_t * semaphorePointer = &(account->semaphore);
    sem_wait(semaphorePointer); // Enter the critical section
    int oldBalance = account->balance;
    int newBalance = oldBalance + depositAmount;  
    account->balance = newBalance;
    sem_post(semaphorePointer);
    logDeposit(accountIndex, depositAmount);
    return 1;
}

int withdraw(struct SharedBankAccount* account, int withdrawAmount, int accountIndex) {
    sem_t * semaphorePointer = &(account->semaphore);
    sem_wait(semaphorePointer); // Enter the critical section
    int oldBalance = account->balance;
    if (oldBalance >= withdrawAmount) {
        int newBalance = oldBalance - withdrawAmount;
        account->balance = newBalance;
        sem_post(semaphorePointer);
        logWithdrawSuccess(accountIndex, withdrawAmount);
        return 1; // Withdraw succeeded
    } else {
        sem_post(semaphorePointer);
        logWithdrawFailure(accountIndex, withdrawAmount);
        return 0; // Withdraw failed
    }
}

void* threadFunction(void* arg) {
    for(int i = 1; i <= NUMBER_ITERATIONS; i++) {
        int transactionChoice = rand() % 2; // Result is either 0: deposit or 1: withdraw
        int randomAccountIndex = rand() % NUMBER_BANK_ACCOUNTS;
        struct SharedBankAccount *randomBankAccount = &(accounts[randomAccountIndex]);
        int randomtransactionAmount = rand() % (TRANSCATION_UPPER_BOUND - TRANSCATION_LOWER_BOUND + 1) + TRANSCATION_LOWER_BOUND;
        if (transactionChoice == 0) {
            deposit(randomBankAccount, randomtransactionAmount, randomAccountIndex);
        } else {
            withdraw(randomBankAccount, randomtransactionAmount, randomAccountIndex);
        }
    }
    return NULL;
}

int main(){
    srand(time(NULL));

    sem_init(&(logSemaphore), 0, 1);

    for (int i = 0; i < NUMBER_BANK_ACCOUNTS; i++) {
        accountInit(&accounts[i], i);
    }

    pthread_t threads[NUMBER_USER_THREADS];
    for (int i = 0; i < NUMBER_USER_THREADS; i++) {
        pthread_create(&threads[i], NULL, threadFunction, NULL);
    }

    for (int i = 0; i < NUMBER_USER_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&(logSemaphore));

    for (int i = 0; i < NUMBER_BANK_ACCOUNTS; i++) {
        // accountInit(&accounts[i]);
        sem_destroy(&(accounts[i].semaphore));
    }

    for (int i = 0; i < logIndex; i++) {
        printf("%s\n", transactionLog[i]);
    }
    
    for (int i = 0; i < NUMBER_BANK_ACCOUNTS; i++) {
        printf("Account %d: Final balance = $%d\n", i, accounts[i].balance);
    }
}