// This code belongs to Saksham Mittal
#include <bits/stdc++.h>
#include <thread>
#include <ctime>
#include <chrono>
#include <fstream>
#include <semaphore.h>

using namespace std;

int nw, nr, kw, kr, csSeed, remSeed, randCSTime, randRemTime;

double *waitingTimeR, *waitingTimeW, averageWaitingTime = 0;

FILE * fp;

// Semaphore declarations
sem_t rw_mtx;
sem_t mtx;
int read_count = 0;

void writer(int n) {
    int id = n + 1;

    for(int i=0; i<kw; i++) {
        // Each writer will perform this kw times

        time_t now = time(0);
        tm *ltm = localtime(&now);
        fprintf(fp, "%dth CS request by Writer %d at %d:%d\n", i + 1, id, ltm->tm_min, ltm->tm_sec);
        waitingTimeW[id - 1] += (-now);

        // Making the rw_mtx wait
        sem_wait(&rw_mtx);

        // Writing is performed
        now = time(0);
        ltm = localtime(&now);
        fprintf(fp, "%dth CS entry by Writer %d at %d:%d\n", i + 1, id, ltm->tm_min, ltm->tm_sec);
        waitingTimeW[id - 1] += (now);

        srand(csSeed);
        randCSTime = rand()%10 + 1;    // get random CS Time
        this_thread::sleep_for(chrono::milliseconds(randCSTime));      // simulate a thread executing in CS

        now = time(0);
        ltm = localtime(&now);
        fprintf(fp, "%dth CS exit by Writer %d at %d:%d\n", i + 1, id, ltm->tm_min, ltm->tm_sec);

        // Signaling the rw_mtx
        sem_post(&rw_mtx);

        srand(remSeed);
        randRemTime = rand()%10 + 1;    // get random Remainder Section Time
        this_thread::sleep_for(chrono::milliseconds(randRemTime));     // simulate a thread executing in Remainder section

    }
}

void reader(int n) {
    int id = n + 1;
    for(int i=0; i<kr; i++) {
        // Each writer will perform this kw times

        time_t now = time(0);
        tm *ltm = localtime(&now);
        fprintf(fp, "%dth CS request by Reader %d at %d:%d\n", i + 1, id, ltm->tm_min, ltm->tm_sec);
        waitingTimeR[id - 1] += (-now);

        // Making the mtx wait
        sem_wait(&mtx);
        read_count++;
        if(read_count == 1) {
            sem_wait(&rw_mtx);
        }
        sem_post(&mtx);

        // Reading is performed
        now = time(0);
        ltm = localtime(&now);
        fprintf(fp, "%dth CS entry by Reader %d at %d:%d\n", i + 1, id, ltm->tm_min, ltm->tm_sec);
        waitingTimeR[id - 1] += (now);

        srand(csSeed);
        randCSTime = rand()%10 + 1;    // get random CS Time
        this_thread::sleep_for(chrono::milliseconds(randCSTime));      // simulate a thread executing in CS

        now = time(0);
        ltm = localtime(&now);
        fprintf(fp, "%dth CS exit by Reader %d at %d:%d\n", i + 1, id, ltm->tm_min, ltm->tm_sec);


        sem_wait(&mtx);
        read_count--;
        if(read_count == 0) {
            // Signaling the rw_mtx if there are no readers waiting
            sem_post(&rw_mtx);
        }
        sem_post(&mtx);

        srand(remSeed);
        randRemTime = rand()%10 + 1;    // get random Remainder Section Time
        this_thread::sleep_for(chrono::milliseconds(randRemTime));     // simulate a thread executing in Remainder section

    }
}

int main(int argc, char const *argv[]) {
    ifstream infile("inp-params.txt");

    // nw is number of writer threads
    // nr is number of reader threads
    infile >> nw >> nr >> kw >> kr >> csSeed >> remSeed;
    thread writerThreads[nw], readerThreads[nr];

    waitingTimeR = new double[nr];
    waitingTimeW = new double[nw];

    fp = fopen ("RW-log.txt", "w+");

    // Initializing the semaphores
    sem_init(&rw_mtx, 0, 1);
    sem_init(&mtx, 0, 1);

    // Calling writer function for each writer thread
    for(int i=0; i<nw; i++) {
      writerThreads[i] = thread(writer, i);
    }
    // Calling reader function for each reader thread
    for(int i=0; i<nr; i++) {
      readerThreads[i] = thread(reader, i);
    }

    // Joining the reader threads
    for (int i=0; i<nr; i++) {
        readerThreads[i].join();
    }
    // Joining the writer threads
    for (int i=0; i<nw; i++) {
        writerThreads[i].join();
    }


    fclose(fp);
    for(int i=0; i<nw; i++)      averageWaitingTime += waitingTimeW[i];
    for(int i=0; i<nr; i++)      averageWaitingTime += waitingTimeR[i];
    averageWaitingTime /= (double)(nw * kw + nr * kr);

    ofstream avgTimes;
    avgTimes.open("Average_time.txt", ios::app);
    avgTimes << "Average Time for RW is: " << endl;
    avgTimes << averageWaitingTime << " seconds" << endl;
    avgTimes.close();

    return 0;
}
