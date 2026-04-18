#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

/*
 *  USAGE: Exercise2.o <Filename> <Search word> <Number of worker threads>
 *
 * This exercise demonstrates multithreading using pipes: the main program receives the name of a file to read from, a word to search for, and a number M of threads to create.
 * Communication between the main program and worker threads happens via two pipes: the line pipe and word count pipe.
 * The main program opens the file, then creates M threads and passes to each: a duplicate of the read end of the line pipe, and a duplicate of the write end of the word count pipe.
 * The main program writes each line of the file into the line pipe.
 * On the other end: each worker thread waits to read this line. Then the thread that is scheduled first (read/write in pipes is atomic), reads the line.
 * Once a thread reads, it fetches the query word and notes the number of occurences. Each thread has its own tally count.
 * This repeats as the main program writes the file line-by-line into the line pipe.
 * Once the main program closes the write end of the pipe, the worker threads stop hanging at read and know it is time to report.
 * Each thread writes its count to the write end of the word count pipe (each worker thread has a duplicate of this end of the pipe).
 * The main program sums the results to get the total occurences.
 * Compiling & running prints the total number of occurences of a word.
 */

#define PIPE_ERROR "Could not create pipe"
#define FILE_ERROR "Could not open file"
#define EXIT_ERROR 1

char* pathToFile;

typedef struct {
    int threadId;
    const char *wordQuery;
    int linesReadFd;
    int countsWriteFd; 
} ThreadArguments;

void initThreadArguments(ThreadArguments* threadArguments, int threadId, const char *wordQuery, int linesReadFd, int countsWriteFd){
    threadArguments->threadId = threadId;
    threadArguments->wordQuery = wordQuery;
    threadArguments->linesReadFd = linesReadFd;
    threadArguments->countsWriteFd = countsWriteFd;
}

static int countOccurrencesInLine(const char *line, const char *query) {
    int count = 0;
    size_t queryLen = strlen(query);
    const char *cursor = line;

    if (queryLen == 0) {
        return 0;
    }

    while ((cursor = strstr(cursor, query)) != NULL) {
        count++;
        cursor += queryLen;
    }

    return count;
}

void* threadFunction(void *arg) {
    ThreadArguments* threadArguments = (ThreadArguments*)arg;
    char lineBuffer[256]; // Buffer to store the current line's content
    int localWordCount = 0;

    printf("[Thread %d] Started. Waiting for lines...\n", threadArguments->threadId);

    while (1) {
        ssize_t bytesRead = read(threadArguments->linesReadFd, lineBuffer, sizeof(lineBuffer)); // Read from own's read end duplicate of the lines pipe
        if (bytesRead == 0) {
            printf("[Thread %d] Reached EOF on lines pipe.\n", threadArguments->threadId);
            break;
        }
        if (bytesRead < 0) {
            if (errno == EINTR) {
                printf("[Thread %d] Read interrupted by signal, retrying.\n", threadArguments->threadId);
                continue;
            }
            perror("[Thread] read failed");
            break;
        }

        if (bytesRead < (ssize_t)sizeof(lineBuffer)) {
            lineBuffer[bytesRead] = '\0';
        } else {
            lineBuffer[sizeof(lineBuffer) - 1] = '\0';
        }

        int matches = countOccurrencesInLine(lineBuffer, threadArguments->wordQuery);
        localWordCount += matches;
        printf("[Thread %d] Processed line chunk (%zd bytes), matches in chunk: %d, local total: %d\n",
               threadArguments->threadId, bytesRead, matches, localWordCount);
    }

    printf("[Thread %d] Sending local count %d to main thread.\n", threadArguments->threadId, localWordCount);
    write(threadArguments->countsWriteFd, &localWordCount, sizeof(localWordCount));
    close(threadArguments->linesReadFd);
    close(threadArguments->countsWriteFd);
    printf("[Thread %d] Finished.\n", threadArguments->threadId);
    return NULL;
}

int main(int argc, char *argv[]) {

    if (argc != 4) { // Ensure correct usage
        printf("Usage: Exercise2 <Path to file> <Word> <Number of threads>\n");
        return EXIT_ERROR;
    }

    // Initialize file to search
    pathToFile = argv[1];                 // Get file from argument
    FILE *filePointer = fopen(pathToFile, "r"); // Attempt to open it
    if (filePointer == NULL) {
        perror(FILE_ERROR);
        return EXIT_ERROR;
    }

    // Get word to search for
    char* query = argv[2];

    // Initialize pipes
    int linesPipe[2];
    if (pipe(linesPipe) == -1) {
        perror(PIPE_ERROR);
        return EXIT_ERROR;
    }

    int countPipe[2];
    if (pipe(countPipe) == -1) {
        perror(PIPE_ERROR);
        return EXIT_ERROR;
    }

    // Initialize threads
    int numberOfThreads = atoi(argv[3]); // Convert string to int using atoi(str)
    if (numberOfThreads <= 0) {
        fprintf(stderr, "Number of threads must be greater than 0\n");
        return EXIT_ERROR;
    }

    printf("[Main] File: %s | Query: \"%s\" | Threads: %d\n", pathToFile, query, numberOfThreads);

    pthread_t threads[numberOfThreads];
    ThreadArguments threadArguments[numberOfThreads];

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

        initThreadArguments(&threadArguments[i], i, query, dupLinesRead, dupCountWrite);
        if (pthread_create(&threads[i], NULL, threadFunction, &threadArguments[i]) != 0) {
            perror("pthread_create failed");
            return EXIT_ERROR;
        }

        printf("[Main] Created worker thread %d (linesReadFd=%d, countsWriteFd=%d).\n", i, dupLinesRead, dupCountWrite);
    }

    close(linesPipe[0]); // Main does not read from this end.
    close(countPipe[1]); // Main does not write to this end.

    // Write file to lines pipe
    char lineBuffer[256];                                           // Buffer to store the current line's content
    int lineNumber = 0;
    while (fgets(lineBuffer, sizeof(lineBuffer), filePointer)) {    // Read the file
        lineNumber++;
        ssize_t bytesWritten = write(linesPipe[1], lineBuffer, sizeof(lineBuffer)); // Write fixed-size messages to preserve line boundaries
        if (bytesWritten != (ssize_t)sizeof(lineBuffer)) {
            perror("write to lines pipe failed");
            close(linesPipe[1]);
            fclose(filePointer);
            return EXIT_ERROR;
        }
        printf("[Main] Dispatched line %d to worker pool (%zd bytes).\n", lineNumber, bytesWritten);
    }
    printf("[Main] Finished dispatching %d lines. Closing write end of lines pipe.\n", lineNumber);
    close(linesPipe[1]); // Close write end after finishing writing
    fclose(filePointer); // Close the file

    // Read results
    for (int i = 0; i < numberOfThreads; i++) {
        pthread_join(threads[i], NULL);
        printf("[Main] Joined worker thread %d.\n", i);
    }

    int wordCount = 0;
    int totalWordCount = 0;
    int contributors = 0;
    while (read(countPipe[0], &wordCount, sizeof(wordCount)) > 0) {
        contributors++;
        totalWordCount = totalWordCount + wordCount; // Read word counts as they come from all the threads, and sum them up as you go.
        printf("[Main] Received partial count %d (contributors so far: %d, running total: %d).\n",
               wordCount, contributors, totalWordCount);
    }
    close(countPipe[0]);

    printf("[Main] Final word count = %d\n", totalWordCount);
}