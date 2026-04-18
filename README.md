# Multithreading-Sync-Scheduling
Assignment 2 for the CMPS 240 - Operating Systems course at the American University of Beirut. It demonstrates the use of multithreading, pipes, synchronization and scheduling.

# Exercise 1: Concurrent Multi-Account Banking System

A multithreaded banking system simulating multiple users performing simultaneous transactions on shared bank accounts. Uses POSIX semaphores to prevent race conditions on all shared data.

**Build and run:**
```bash
gcc -o Exercise1 Exercise1.c -pthread
./Exercise1
```

It features `N = 5` bank accounts initialized with a random balance between $500 and $1500, and `M = 10` user threads performing 10 random transactions:

```C
#define NUMBER_ITERATIONS 10
#define NUMBER_BANK_ACCOUNTS 5
#define NUMBER_USER_THREADS 10
#define LOG_SIZE NUMBER_USER_THREADS*NUMBER_ITERATIONS

#define BALANCE_LOWER_BOUND 500
#define BALANCE_UPPER_BOUND 1500
#define TRANSCATION_LOWER_BOUND 50
#define TRANSCATION_UPPER_BOUND 300
```

Each account owns its own semaphore to allow parallel operations on different accounts:

```C
struct SharedBankAccount {
    int balance;
    sem_t semaphore;
};
```

A global transaction log protected by its own (binary) semaphore records all activity:

```C
sem_t logSemaphore;

// And later in main():

sem_init(&(logSemaphore), 0, 1); // Initialized to 1.
```

**For synchronization:**

Per-account semaphores guard reads and writes inside `deposit()` and `withdraw()`:

```C
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
```

Log semaphore: guards the shared log array and its index counter. Before writing, the thread must call `sem_wait(semaphorePointer)` and call `sem_post(semaphorePointer)` upon quitting. For example:

```C
void logDeposit(int accountIndex, int depositAmount){

    sem_t * semaphorePointer = &logSemaphore;
    sem_wait(semaphorePointer);

    snprintf(transactionLog[logIndex], BUFFER_SIZE, "Account %d: Deposit of amount $%d", accountIndex, depositAmount);
    logIndex = logIndex + 1;

    sem_post(semaphorePointer);
}
```

**Sample output:**

```
Account 0: Initial balance = $1286
Account 1: Initial balance = $1447
Account 2: Initial balance = $520
Account 3: Initial balance = $547
Account 4: Initial balance = $562
Account 4: Deposit of amount $130
Account 0: Withdraw succeeded of amount $271
Account 2: Withdraw succeeded of amount $298
Account 1: Withdraw succeeded of amount $295
Account 1: Withdraw succeeded of amount $126
Account 0: Withdraw succeeded of amount $230
Account 0: Deposit of amount $159
Account 1: Withdraw succeeded of amount $110
Account 0: Deposit of amount $105
Account 0: Deposit of amount $82
Account 4: Deposit of amount $259
Account 1: Deposit of amount $209
Account 3: Withdraw succeeded of amount $206
Account 3: Withdraw succeeded of amount $291
Account 0: Withdraw succeeded of amount $251
Account 4: Withdraw succeeded of amount $252
Account 3: Deposit of amount $99
Account 1: Deposit of amount $229
Account 2: Withdraw succeeded of amount $194
Account 3: Withdraw failed (amount $204)
Account 1: Deposit of amount $234
Account 3: Deposit of amount $129
Account 4: Withdraw succeeded of amount $170
Account 4: Deposit of amount $147
Account 0: Deposit of amount $139
Account 0: Withdraw succeeded of amount $156
Account 4: Deposit of amount $259
Account 0: Deposit of amount $114
Account 0: Withdraw succeeded of amount $271
Account 0: Deposit of amount $98
Account 3: Deposit of amount $210
Account 0: Withdraw succeeded of amount $185
Account 1: Withdraw succeeded of amount $57
Account 4: Withdraw succeeded of amount $139
Account 1: Withdraw succeeded of amount $109
Account 2: Withdraw failed (amount $109)
Account 2: Deposit of amount $67
Account 0: Withdraw succeeded of amount $93
Account 2: Withdraw succeeded of amount $60
Account 4: Withdraw succeeded of amount $88
Account 4: Withdraw succeeded of amount $209
Account 1: Withdraw succeeded of amount $193
Account 0: Withdraw succeeded of amount $201
Account 0: Deposit of amount $265
Account 1: Deposit of amount $60
Account 1: Deposit of amount $291
Account 2: Deposit of amount $282
Account 0: Deposit of amount $84
Account 4: Deposit of amount $127
Account 3: Deposit of amount $118
Account 3: Deposit of amount $210
Account 1: Deposit of amount $150
Account 3: Deposit of amount $268
Account 4: Deposit of amount $248
Account 4: Withdraw succeeded of amount $185
Account 2: Withdraw succeeded of amount $234
Account 2: Deposit of amount $62
Account 4: Deposit of amount $129
Account 4: Deposit of amount $250
Account 3: Withdraw succeeded of amount $206
Account 2: Deposit of amount $298
Account 4: Deposit of amount $184
Account 1: Deposit of amount $64
Account 1: Withdraw succeeded of amount $259
Account 0: Withdraw succeeded of amount $171
Account 0: Withdraw succeeded of amount $109
Account 2: Withdraw succeeded of amount $260
Account 4: Deposit of amount $76
Account 4: Withdraw succeeded of amount $230
Account 0: Deposit of amount $229
Account 4: Deposit of amount $177
Account 1: Withdraw succeeded of amount $169
Account 0: Deposit of amount $224
Account 2: Withdraw failed (amount $244)
Account 0: Deposit of amount $191
Account 2: Deposit of amount $141
Account 0: Withdraw succeeded of amount $148
Account 2: Deposit of amount $256
Account 4: Deposit of amount $250
Account 2: Withdraw succeeded of amount $271
Account 4: Withdraw succeeded of amount $136
Account 1: Withdraw succeeded of amount $69
Account 2: Withdraw succeeded of amount $170
Account 0: Withdraw succeeded of amount $220
Account 2: Deposit of amount $101
Account 1: Withdraw succeeded of amount $77
Account 3: Withdraw succeeded of amount $268
Account 1: Withdraw succeeded of amount $248
Account 0: Withdraw succeeded of amount $160
Account 4: Withdraw succeeded of amount $269
Account 1: Deposit of amount $229
Account 0: Deposit of amount $102
Account 1: Withdraw succeeded of amount $149
Account 2: Withdraw succeeded of amount $71
Account 2: Deposit of amount $268
Account 3: Deposit of amount $295
Account 4: Deposit of amount $212
Account 1: Withdraw succeeded of amount $106
Account 1: Withdraw succeeded of amount $255
Account 0: Withdraw succeeded of amount $149
Account 0: Final balance = $463
Account 1: Final balance = $691
Account 2: Final balance = $437
Account 3: Final balance = $905
Account 4: Final balance = $1332
```

- Initial balance of each account
- Transaction log (deposits, successful and failed withdrawals)
- Final balance of each account

---

# Exercise 2: Word Count Using Pipes and Threads

Counts occurrences of a word in a file by distributing work across multiple threads using POSIX pipes for communication.

**Build & run:**
```bash
gcc -o Exercise2 Exercise2.c -pthread
./Exercise2 <path_to_file> <word> <number_of_threads>
```

Two pipes coordinate communication:

- **Lines pipe:** main thread writes file lines into it; worker threads race to read from it atomically

```C
// Main process: write file to lines pipe

while (fgets(lineBuffer, sizeof(lineBuffer), filePointer)) { // Read the file
	ssize_t bytesWritten = write(linesPipe[1], lineBuffer, sizeof(lineBuffer)); // Write fixed-size messages to preserve line boundaries

	if (bytesWritten != (ssize_t)sizeof(lineBuffer)) {
		perror("write to lines pipe failed");
		close(linesPipe[1]);
		fclose(filePointer);
		return EXIT_ERROR;
	}
}

// Worker threads: atomically read from lines pipe

while (1) {
	
	ssize_t bytesRead = read(threadArguments->linesReadFd, lineBuffer, sizeof(lineBuffer)); // Read from own's read end duplicate of the lines pipe
	
	if (bytesRead == 0) {
	break;
	}
	
	if (bytesRead < 0) {
		if (errno == EINTR) {
		continue;
		}
	break;
	}
	
	if (bytesRead < (ssize_t)sizeof(lineBuffer)) {
		lineBuffer[bytesRead] = '\0';
	} else {
		lineBuffer[sizeof(lineBuffer) - 1] = '\0';
	}	
	
	localWordCount += countOccurrencesInLine(lineBuffer, threadArguments->wordQuery);

}
```

- **Counts pipe:** each worker thread writes its local tally into it when done; main thread sums the results

```C
// Main thread: after joining all worker threads
int wordCount = 0;
int totalWordCount = 0;
while (read(countPipe[0], &wordCount, sizeof(wordCount)) > 0) {
	totalWordCount = totalWordCount + wordCount; // Read word counts as they come from all the threads, and sum them up as you go.
}
close(countPipe[0]);

// Worker thread: after the while loop

write(threadArguments->countsWriteFd, &localWordCount, sizeof(localWordCount));
close(threadArguments->linesReadFd);
close(threadArguments->countsWriteFd);
```

Each worker thread receives via `dup()`:

- A duplicate of the read end of the lines pipe
- A duplicate of the write end of the counts pipe

```C
for (int i = 0; i < numberOfThreads; i++) {
	// Prepare arguments for thread
	// Duplicate pipes
	
	int dupLinesRead = dup(linesPipe[0]);
	if (dupLinesRead < 0) {
		return EXIT_ERROR;
	}
	
	int dupCountWrite = dup(countPipe[1]);
	if (dupCountWrite < 0) {
		return EXIT_ERROR;
	}
	
	initThreadArguments(&threadArguments[i], query, dupLinesRead, dupCountWrite);
	
	if (pthread_create(&threads[i], NULL, threadFunction, &threadArguments[i]) != 0) {
		perror("pthread_create failed");
		return EXIT_ERROR;
	}
}
```

This ensures each thread has its own file descriptor, so closing is handled independently per thread without affecting others.

**Summary of how the pipes work:**

| Endpoint                       | Closed by | When                      |
| ------------------------------ | --------- | ------------------------- |
| `linesPipe[0]`                 | Main      | After spawning threads    |
| `linesPipe[1]`                 | Main      | After writing all lines   |
| `countPipe[1]`                 | Main      | After spawning threads    |
| Thread's dup of `linesPipe[0]` | Thread    | After `read()` returns 0  |
| Thread's dup of `countPipe[1]` | Thread    | After writing local count |

**Sample output:**

```C
$ ./e2 example.txt coil 10
[Main] File: example.txt | Query: "coil" | Threads: 10
[Thread 0] Started. Waiting for lines...
[Main] Created worker thread 0 (linesReadFd=8, countsWriteFd=9).
[Main] Created worker thread 1 (linesReadFd=10, countsWriteFd=11).
[Main] Created worker thread 2 (linesReadFd=12, countsWriteFd=13).
[Thread 2] Started. Waiting for lines...
[Main] Created worker thread 3 (linesReadFd=14, countsWriteFd=15).
[Thread 1] Started. Waiting for lines...
[Thread 4] Started. Waiting for lines...
[Main] Created worker thread 4 (linesReadFd=16, countsWriteFd=17).
[Main] Created worker thread 5 (linesReadFd=18, countsWriteFd=19).
[Thread 6] Started. Waiting for lines...
[Thread 5] Started. Waiting for lines...
[Thread 3] Started. Waiting for lines...
[Main] Created worker thread 6 (linesReadFd=20, countsWriteFd=21).
[Main] Created worker thread 7 (linesReadFd=22, countsWriteFd=23).
[Main] Created worker thread 8 (linesReadFd=24, countsWriteFd=25).
[Thread 8] Started. Waiting for lines...
[Thread 7] Started. Waiting for lines...
[Main] Created worker thread 9 (linesReadFd=26, countsWriteFd=27).
[Main] Dispatched line 1 to worker pool (256 bytes).
[Thread 9] Started. Waiting for lines...
[Main] Dispatched line 2 to worker pool (256 bytes).
[Main] Dispatched line 3 to worker pool (256 bytes).
[Main] Dispatched line 4 to worker pool (256 bytes).
[Main] Dispatched line 5 to worker pool (256 bytes).
[Main] Dispatched line 6 to worker pool (256 bytes).
[Main] Finished dispatching 6 lines. Closing write end of lines pipe.
[Thread 2] Processed line chunk (256 bytes), matches in chunk: 0, local total: 0
[Thread 1] Processed line chunk (256 bytes), matches in chunk: 0, local total: 0
[Thread 1] Processed line chunk (256 bytes), matches in chunk: 0, local total: 0
[Thread 1] Reached EOF on lines pipe.
[Thread 5] Reached EOF on lines pipe.
[Thread 4] Processed line chunk (256 bytes), matches in chunk: 1, local total: 1
[Thread 6] Reached EOF on lines pipe.
[Thread 7] Reached EOF on lines pipe.
[Thread 8] Reached EOF on lines pipe.
[Thread 9] Reached EOF on lines pipe.
[Thread 9] Sending local count 0 to main thread.
[Thread 9] Finished.
[Thread 8] Sending local count 0 to main thread.
[Thread 8] Finished.
[Thread 3] Reached EOF on lines pipe.
[Thread 3] Sending local count 0 to main thread.
[Thread 3] Finished.
[Thread 2] Processed line chunk (256 bytes), matches in chunk: 0, local total: 0
[Thread 2] Reached EOF on lines pipe.
[Thread 2] Sending local count 0 to main thread.
[Thread 4] Reached EOF on lines pipe.
[Thread 4] Sending local count 1 to main thread.
[Thread 6] Sending local count 0 to main thread.
[Thread 0] Processed line chunk (256 bytes), matches in chunk: 0, local total: 0
[Thread 0] Reached EOF on lines pipe.
[Thread 0] Sending local count 0 to main thread.
[Thread 0] Finished.
[Thread 1] Sending local count 0 to main thread.
[Thread 1] Finished.
[Thread 2] Finished.
[Thread 7] Sending local count 0 to main thread.
[Thread 7] Finished.
[Thread 4] Finished.
[Thread 6] Finished.
[Main] Joined worker thread 0.
[Thread 5] Sending local count 0 to main thread.
[Main] Joined worker thread 1.
[Main] Joined worker thread 2.
[Main] Joined worker thread 3.
[Main] Joined worker thread 4.
[Thread 5] Finished.
[Main] Joined worker thread 5.
[Main] Joined worker thread 6.
[Main] Joined worker thread 7.
[Main] Joined worker thread 8.
[Main] Joined worker thread 9.
[Main] Received partial count 0 (contributors so far: 1, running total: 0).
[Main] Received partial count 0 (contributors so far: 2, running total: 0).
[Main] Received partial count 0 (contributors so far: 3, running total: 0).
[Main] Received partial count 0 (contributors so far: 4, running total: 0).
[Main] Received partial count 1 (contributors so far: 5, running total: 1).
[Main] Received partial count 0 (contributors so far: 6, running total: 1).
[Main] Received partial count 0 (contributors so far: 7, running total: 1).
[Main] Received partial count 0 (contributors so far: 8, running total: 1).
[Main] Received partial count 0 (contributors so far: 9, running total: 1).
[Main] Received partial count 0 (contributors so far: 10, running total: 1).
[Main] Final word count = 1
```

- Logs describing the main program creating threads and initializing them.
- Threads processing their lines, and writing their count into the count pipe.
- Total number of occurrences of the search word across the entire file.
---
# Exercise 3

In the GitHub repository, navigate to Exercise3.c for this part's implementation. The output is pasted below, but do not read deeply into it. I will provide a detailed explanation.

```
$ ./exercise3 
[Reader 0] Skipped, buffer empty
[Reader 1] Skipped, buffer empty
[Reader 2] Skipped, buffer empty
[Reader 3] Skipped, buffer empty
[Reader 4] Skipped, buffer empty
[Reader 5] Skipped, buffer empty
[Reader 6] Skipped, buffer empty
[Producer 0] Added 56 (items in buffer: 1)
[Producer 1] Added 2 (items in buffer: 2)
[Consumer 1] Removed 2 (items in buffer: 1)
[Consumer 0] Removed 56 (items in buffer: 0)
[Reader 11] Skipped, buffer empty
[Reader 12] Skipped, buffer empty
[Reader 10] Skipped, buffer empty
[Reader 8] Skipped, buffer empty
[Reader 7] Skipped, buffer empty
[Reader 9] Skipped, buffer empty
[Producer 2] Added 96 (items in buffer: 1)
[Consumer 2] Removed 96 (items in buffer: 0)
[Reader 15] Skipped, buffer empty
[Reader 14] Skipped, buffer empty
[Reader 16] Skipped, buffer empty
[Reader 17] Skipped, buffer empty
[Reader 3] Skipped, buffer empty
[Reader 5] Skipped, buffer empty
[Reader 6] Skipped, buffer empty
[Reader 19] Skipped, buffer empty
[Reader 18] Skipped, buffer empty
[Reader 0] Skipped, buffer empty
[Reader 1] Skipped, buffer empty
[Reader 13] Skipped, buffer empty
[Reader 4] Skipped, buffer empty
[Producer 1] Added 20 (items in buffer: 1)
[Consumer 0] Removed 20 (items in buffer: 0)
[Producer 0] Added 69 (items in buffer: 1)
[Consumer 1] Removed 69 (items in buffer: 0)
[Reader 2] Skipped, buffer empty
[Reader 9] Skipped, buffer empty
[Reader 7] Skipped, buffer empty
[Reader 5] Skipped, buffer empty
[Reader 11] Skipped, buffer empty
[Reader 12] Skipped, buffer empty
[Reader 15] Skipped, buffer empty
[Reader 1] Skipped, buffer empty
[Reader 13] Skipped, buffer empty
[Reader 10] Skipped, buffer empty
[Reader 8] Skipped, buffer empty
[Reader 14] Skipped, buffer empty
[Reader 0] Skipped, buffer empty
[Reader 19] Skipped, buffer empty
[Producer 2] Added 95 (items in buffer: 1)
[Consumer 0] Removed 95 (items in buffer: 0)
[Producer 0] Added 92 (items in buffer: 1)
[Reader 2] Read 92 (items in buffer: 1)
[Reader 18] Read 92 (items in buffer: 1)
[Reader 3] Read 92 (items in buffer: 1)
[Reader 5] Read 92 (items in buffer: 1)
[Reader 12] Read 92 (items in buffer: 1)
[Reader 13] Read 92 (items in buffer: 1)
[Reader 6] Read 92 (items in buffer: 1)
[Reader 7] Read 92 (items in buffer: 1)
[Producer 1] Added 24 (items in buffer: 2)
[Consumer 1] Removed 24 (items in buffer: 1)
[Reader 1] Read 92 (items in buffer: 1)
[Reader 3] Read 92 (items in buffer: 1)
[Reader 17] Read 92 (items in buffer: 1)
[Reader 12] Read 92 (items in buffer: 1)
[Consumer 2] Removed 92 (items in buffer: 0)
[Reader 5] Skipped, buffer empty
[Reader 4] Skipped, buffer empty
[Reader 9] Skipped, buffer empty
[Reader 11] Skipped, buffer empty
[Reader 15] Skipped, buffer empty
[Reader 16] Skipped, buffer empty
[Reader 8] Skipped, buffer empty
[Reader 10] Skipped, buffer empty
[Reader 19] Skipped, buffer empty
[Producer 2] Added 32 (items in buffer: 1)
[Producer 0] Added 86 (items in buffer: 2)
[Consumer 0] Removed 86 (items in buffer: 1)
[Reader 12] Read 32 (items in buffer: 1)
[Reader 14] Read 32 (items in buffer: 1)
[Reader 0] Read 32 (items in buffer: 1)
[Consumer 1] Removed 32 (items in buffer: 0)
[Reader 17] Skipped, buffer empty
[Reader 7] Skipped, buffer empty
[Reader 6] Skipped, buffer empty
[Reader 18] Skipped, buffer empty
[Reader 2] Skipped, buffer empty
[Reader 1] Skipped, buffer empty
[Reader 3] Skipped, buffer empty
[Reader 13] Skipped, buffer empty
[Producer 1] Added 28 (items in buffer: 1)
[Reader 14] Read 28 (items in buffer: 1)
[Producer 2] Added 29 (items in buffer: 2)
[Consumer 2] Removed 29 (items in buffer: 1)
[Producer 0] Added 7 (items in buffer: 2)
[Reader 9] Read 7 (items in buffer: 2)
[Reader 0] Read 7 (items in buffer: 2)
[Consumer 0] Removed 7 (items in buffer: 1)
[Reader 15] Read 28 (items in buffer: 1)
[Reader 7] Read 28 (items in buffer: 1)
[Reader 8] Read 28 (items in buffer: 1)
[Reader 2] Read 28 (items in buffer: 1)
[Reader 10] Read 28 (items in buffer: 1)
[Reader 18] Read 28 (items in buffer: 1)
[Reader 4] Read 28 (items in buffer: 1)
[Reader 11] Read 28 (items in buffer: 1)
[Consumer 1] Removed 28 (items in buffer: 0)
[Producer 1] Added 15 (items in buffer: 1)
[Consumer 2] Removed 15 (items in buffer: 0)
[Producer 2] Added 42 (items in buffer: 1)
[Producer 0] Added 72 (items in buffer: 2)
[Consumer 0] Removed 72 (items in buffer: 1)
[Reader 19] Read 42 (items in buffer: 1)
[Reader 13] Read 42 (items in buffer: 1)
[Reader 14] Read 42 (items in buffer: 1)
[Reader 4] Read 42 (items in buffer: 1)
[Reader 18] Read 42 (items in buffer: 1)
[Reader 6] Read 42 (items in buffer: 1)
[Reader 16] Read 42 (items in buffer: 1)
[Reader 8] Read 42 (items in buffer: 1)
[Reader 11] Read 42 (items in buffer: 1)
[Reader 15] Read 42 (items in buffer: 1)
[Reader 10] Read 42 (items in buffer: 1)
[Reader 17] Read 42 (items in buffer: 1)
[Reader 9] Read 42 (items in buffer: 1)
[Producer 1] Added 79 (items in buffer: 2)
[Consumer 1] Removed 79 (items in buffer: 1)
[Consumer 2] Removed 42 (items in buffer: 0)
[Producer 2] Added 46 (items in buffer: 1)
[Producer 0] Added 11 (items in buffer: 2)
[Consumer 0] Removed 11 (items in buffer: 1)
[Reader 19] Read 46 (items in buffer: 1)
[Reader 16] Read 46 (items in buffer: 1)
[Reader 17] Read 46 (items in buffer: 1)
[Producer 1] Added 54 (items in buffer: 2)
[Consumer 1] Removed 54 (items in buffer: 1)
[Reader 16] Read 46 (items in buffer: 1)
[Producer 2] Added 52 (items in buffer: 2)
[Producer 0] Added 14 (items in buffer: 3)
[Consumer 2] Removed 14 (items in buffer: 2)
[Consumer 0] Removed 52 (items in buffer: 1)
[Producer 1] Added 56 (items in buffer: 2)
[Producer 2] Added 31 (items in buffer: 3)
[Producer 0] Added 92 (items in buffer: 4)
[Consumer 1] Removed 92 (items in buffer: 3)
[Consumer 2] Removed 31 (items in buffer: 2)
[Consumer 0] Removed 56 (items in buffer: 1)
[Producer 1] Added 7 (items in buffer: 2)
[Producer 2] Added 29 (items in buffer: 3)
[Producer 0] Added 37 (items in buffer: 4)
[Consumer 1] Removed 37 (items in buffer: 3)
[Consumer 2] Removed 29 (items in buffer: 2)
[Consumer 0] Removed 7 (items in buffer: 1)
[Producer 1] Added 80 (items in buffer: 2)
[Producer 2] Added 86 (items in buffer: 3)
[Consumer 1] Removed 86 (items in buffer: 2)
[Consumer 2] Removed 80 (items in buffer: 1)
[Consumer 2] Removed 46 (items in buffer: 0)
```

This here is a trace of notable points from the output. What to note: After C0 and C1 drain the buffer, `empty` returns to 0 and `full` to 64. All three producers are sleeping via `usleep`. No new producer has been scheduled yet. Readers are not blocked by `emptyPriority` (that semaphore only blocks readers when a consumer is actively waiting). So readers pass through all semaphores freely but find `itemsInBuffer == 0` and skip. This continues until P2 is eventually scheduled, adds an item, and C2 immediately consumes it, so returning to the same state. This demonstrates that readers never starve producers or consumers.

| Arrival                  | readersInBuffer | mutex | full | empty | fullPriority | emptyPriority | wrt   | Note                                                                                                                                                              |
| ------------------------ | --------------- | ----- | ---- | ----- | ------------ | ------------- | ----- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| R0–R6                    | 0→1→0           | 1→0→1 | 64   | 0     | 1→0→1        | 1→0→1         | 1→0→1 | 7 readers skip, buffer empty                                                                                                                                      |
| P0                       | 0               | 1     | 63   | 1     | 1→0→1        | 1             | 1→0→1 | Added 56, buffer=1                                                                                                                                                |
| P1                       | 0               | 1     | 62   | 2     | 1→0→1        | 1             | 1→0→1 | Added 2, buffer=2                                                                                                                                                 |
| C1                       | 0               | 1     | 63   | 1     | 1            | 1→0→1         | 1→0→1 | Removed 2, buffer=1                                                                                                                                               |
| C0                       | 0               | 1     | 64   | 0     | 1            | 1→0→1         | 1→0→1 | Removed 56, buffer=0                                                                                                                                              |
| R11–R12, R10, R8, R7, R9 | 0→1→0           | 1→0→1 | 64   | 0     | 1→0→1        | 1→0→1         | 1→0→1 | 6 readers skip, buffer empty again                                                                                                                                |
| P2                       | 0               | 1     | 63   | 1     | 1→0→1        | 1             | 1→0→1 | Added 96, buffer=1                                                                                                                                                |
| C2                       | 0               | 1     | 64   | 0     | 1            | 1→0→1         | 1→0→1 | Removed 96, buffer=0                                                                                                                                              |
| R15–R19, R0, R1, R13, R4 | 0→1→0           | 1→0→1 | 64   | 0     | 1→0→1        | 1→0→1         | 1→0→1 | **Key case:** 9 readers find empty buffer, emptyPriority does not block them but buffer has nothing to read. No producer is awake at this point, so readers skip. |

---