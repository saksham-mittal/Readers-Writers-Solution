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
sem_t in;
sem_t out;
sem_t wrt;
int no1 = 0;
int no2 = 0;
bool waitVar = false;

void writer(int n) {
    int id = n + 1;

    for(int i=0; i<kw; i++) {
        // Each writer will perform this kw times

        time_t now = time(0);
        tm *ltm = localtime(&now);
        fprintf(fp, "%dth CS request by Writer %d at %d:%d\n", i + 1, id, ltm->tm_min, ltm->tm_sec);
        waitingTimeW[id - 1] += (-now);

        // Wait on in so that only one writer can be in Cs
        sem_wait(&in);
        sem_wait(&out);

        if(no1 == no2) {
            sem_post(&out);
            // if no one is in the CS then signal count
        } else {
            waitVar = true;
            sem_post(&out);
            sem_wait(&wrt);
            // means writer has come out of wait
            waitVar = false;
        }

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

        // Signaling 'in'
        sem_post(&in);

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

        // Incrementing the number of readers count
        sem_wait(&in);
        no1++;
        sem_post(&in);

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

        sem_wait(&out);
        no2++;
        if(waitVar && no1 == no2) {
            // If a writer is waiting and there is no reader in CS, then signal writer.
            sem_post(&wrt);
        }
        sem_post(&out);

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

    fp = fopen ("FairRW-log.txt", "w+");

    // Initializing the semaphores
    sem_init(&in, 0, 1);
    sem_init(&out, 0, 1);
    sem_init(&wrt, 0, 0);

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
    avgTimes << "Average Time for RW-Fair is: " << endl;
    avgTimes << averageWaitingTime << " seconds" << endl;
    avgTimes.close();

    return 0;
}
