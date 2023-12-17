#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#define MAX_MESSAGE 256

long long unsigned int hash(int seed, char* buf, int nbytes) {
    long long unsigned int H = seed; 
    for (int i = 0; i < nbytes; ++i) 
        H = H * 2 * buf[i] + 1;
    return H;
}

int main(int argc, char** argv) {

    if (argc != 2) {
        std::cerr << "wrong number of arguements" << std::endl;
        exit(1);
    }

	
    int p1_to_p2[2], p2_to_p1[2];
	// TODO: create pipe
    
    if (pipe(p1_to_p2) == -1 || pipe(p2_to_p1) == -1) {
        perror("pipe");
        exit(1);
    }

    int pid = fork();
    if (pid == 0) { //child process
        // TODO: read from parent
        close(p1_to_p2[1]);
        close(p2_to_p1[0]);
        
        char buf[MAX_MESSAGE];
        int bytes = read(p1_to_p2[0], buf, MAX_MESSAGE);
        // TODO: compute hash 
        long long unsigned int h = hash(getpid(), buf, bytes);

        // TODO: send hash to parent 
        write(p2_to_p1[1], &h, sizeof(h));
    }
    else {
        // TODO: write to child 
        close(p1_to_p2[0]);
        close(p2_to_p1[1]);

        write(p1_to_p2[1], argv[1], strlen(argv[1]));

        // TODO: get hash from child 
        long long unsigned int hrecv;
        read(p2_to_p1[0], &hrecv, sizeof(hrecv));
        
        // TODO: calculate hash on parent side
		long long unsigned int h = hash(pid, argv[1], strlen(argv[1]));
		// print hashes; DO NOT change
        printf("%llX\n", h);
        printf("%llX\n", hrecv);
    }

    return 0;
}