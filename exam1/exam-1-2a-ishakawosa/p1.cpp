#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <algorithm>

int main(int argc, char** argv) {
    int n = 1, opt;
    while ((opt = getopt(argc, argv, "n:")) != -1) {
        switch (opt) {
        case 'n':
            n = atoi(optarg);
            break;
        }
    }

    /*
    1. TODO: fork n child processes and run p1-helper on each using execvp
        > note: we need to synchronize the child processes to print in the desired order
        > note: sleep(..) does not guarantee desired synchronization
        > note: check "raise" system call
        > note: check "WUNTRACED" flag for "waitpid"
    */

    std::vector<pid_t> childPIDs;

    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        
        if(pid==0){ //child process
            raise(SIGSTOP);
            char* child_args[] = {(char*)"p1-helper", NULL};
            execvp("./p1-helper", child_args);
            perror("execvp");
            return 1;
        }


        else if (pid > 0){ //parent process
            childPIDs.push_back(pid);
        } 
        
        else{ 
            std::cerr << "Error: fork failed." << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    /* 
    2. TODO: print children pids 
    */

    //sorting
    std::sort(childPIDs.begin(), childPIDs.end());

    for (size_t i = 0; i < childPIDs.size(); i++) {
        printf("%d", childPIDs[i]);
        if(i < childPIDs.size() - 1) { //if its not the last element add space
            printf(" ");
        }
    }

    printf("\n");

    fflush(stdout);             // DO NOT REMOVE: ensures the first line prints all pids

    /* 
    3. TODO: signal children with the reverse order of pids 
        > note: take a look at "kill" system call 
    */
    for (int i = childPIDs.size() - 1; i >= 0; i--) {
        kill(childPIDs[i], SIGCONT);
        waitpid(childPIDs[i], NULL, WUNTRACED); //wait for child
    }

    printf("Parent: exiting\n");

    return 0;
}