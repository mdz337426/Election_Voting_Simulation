#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <mutex>
#include <vector>

#define N 1000  // Number of voters
#define M 5   // Number of candidates
#define P 10    // Number of polling booths

std::mutex log_mutex;
void polling_booth(int boothid)
{
        std::string boothfile = "booth_" + std::to_string(boothid) + ".txt";
        std::ofstream boothstream(boothfile);

        if(!boothstream.is_open())
        {
            std::cerr<<"Error opening file"<<std::endl;
            exit(1);
        }
        std::ofstream logstream;
        logstream.open("log.txt", std::ios_base::app);
        srand(time(0)+boothid);
        int k = N/P;
        for(int i=0; i<k; ++i)
        {
            int voter_id = boothid*(k) + i+1;
            int vote = rand()%M+1;
            boothstream << "Voter " << voter_id << " voted for Candidate " << vote << std::endl;
            std::lock_guard<std::mutex> guard(log_mutex);
            logstream << "Booth " << boothid<< ": Voter " << voter_id << " voted for Candidate " << vote << std::endl;
        }
    boothstream.close();
    logstream.close();
}

int main() {
    std::vector<pid_t> pids;

    // Create P polling booth processes
    for (int i = 0; i < P; ++i) {
        pid_t pid = fork();
        if (pid < 0) {
            std::cerr << "Fork failed" << std::endl;
            exit(1);
        }
        if (pid == 0) {
            polling_booth(i);
            exit(0);
        } else {
            pids.push_back(pid);
        }
    }
    // Wait for all child processes to complete
    for (pid_t pid : pids) {
        waitpid(pid, nullptr, 0);
    }
    // Collect votes from all booths
    int votes[M + 1] = {0};
    for (int i = 0; i < P; ++i) {
        std::string boothFile = "booth_" + std::to_string(i) + ".txt";
        std::ifstream boothStream(boothFile);
        if (!boothStream.is_open()) {
            std::cerr << "Error opening file for booth " << i << std::endl;
            continue;
        }

        std::string line;
        while (std::getline(boothStream, line)) {
            int candidate = line.back() - '0';
            if (candidate >= 1 && candidate <= M) {
                votes[candidate]++;
            }
        }
        boothStream.close();
    }
    // Announce results
    int winner = 0;
    int maxVotes = 0;
    for (int i = 1; i <= M; ++i) {
        if (votes[i] > maxVotes) {
            maxVotes = votes[i];
            winner = i;
        }
        std::cout << "Candidate " << i << " received " << votes[i] << " votes." << std::endl;
    }
    std::cout << "The winner is Candidate " << winner << " with " << maxVotes << " votes." << std::endl;
    return 0;
}


