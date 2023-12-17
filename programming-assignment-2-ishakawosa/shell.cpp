#include <iostream>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <vector>
#include <string>

#include "Tokenizer.h"
#include <cstring>
#include <fcntl.h>
#include <ctime>
#include <pwd.h>
#include <cstdlib>
#include <limits.h> //for PATH_MAX
#include <unistd.h> //for HOST_NAME_MAX
#include <chrono>

// all the basic colours for a shell prompt
#define RED     "\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE	"\033[1;34m"
#define WHITE	"\033[1;37m"
#define NC      "\033[0m"

using namespace std;

int main () {
    //create copies of stdin, stdout using dup
    int originalstdin = dup(STDIN_FILENO);
    int originalstdout = dup(STDOUT_FILENO);

    vector<pid_t> backgroundProcesses;
    char previousDirectory[PATH_MAX]; 

    for (;;) {
        auto now = chrono::system_clock::now();
        auto currentTime = chrono::system_clock::to_time_t(now);

        char dateTime[80];
        strftime(dateTime, sizeof(dateTime), "%b %d %T", localtime(&currentTime));

        char hostName[HOST_NAME_MAX];
        gethostname(hostName, sizeof(hostName));

        uid_t uid = geteuid();
        struct passwd *pw = getpwuid(uid);
        const char *userName = pw->pw_name;

        char currentDirectory[PATH_MAX];
        getcwd(currentDirectory, sizeof(currentDirectory));

        cout << RED << dateTime << " " << userName << ":" << hostName << currentDirectory << "$ " << NC;

        //implement iteration over vectoir of bg pid (declared outside loop)
        //waitpid()- using flag for non-blocking

        // need date/time, username, and absolute path to current dir
        cout << YELLOW << "Shell$" << NC << " ";
        
        // get user inputted command
        string input;
        getline(cin, input);

        if (input == "exit" || input=="Exit"|| input=="EXIT") {  // print exit message and break out of infinite loop
            cout << WHITE << "Now exiting shell..." << endl << "Goodbye" << NC << endl;
            break;
        }

        //implement cd with chdir()
        //if the dir (cd <dir>) is "-", then go to previous working dir
        // else call change dir
        //need a variable storing previous working dir (outside loop)
        // get tokenized commands from user input
        Tokenizer tknr(input);

        if (tknr.hasError()) {  // continue to next prompt if input had an error
            continue;
        }

        int prevPipeRead = -1;

        if (tknr.commands.size() == 1 && tknr.commands[0]->args[0] == "cd") {
            if (tknr.commands[0]->args.size() == 2 && tknr.commands[0]->args[1] == "-") {
                if (chdir(previousDirectory) != 0) {
                    perror("chdir");
                }
            } 
            else{
                if (getcwd(previousDirectory, sizeof(previousDirectory)) == nullptr){
                    perror("getcwd");
                }
                if (chdir(tknr.commands[0]->args[1].c_str()) != 0) {
                    perror("chdir");
                }
            }
            continue; 
        }

        // print out every command token-by-token on individual lines
        // prints to cerr to avoid influencing autograder
        for (auto cmd : tknr.commands) {
            for (auto str : cmd->args) {
                cerr << "|" << str << "| ";
            }
            if (cmd->hasInput()) {
                cerr << "in< " << cmd->in_file << " ";
            }
            if (cmd->hasOutput()) {
                cerr << "out> " << cmd->out_file << " ";
            }
            cerr << endl;
        }

        //for piping
        //for cmd in commands-- for (auto cmd : tknr.commands)
        //      call pipe() to make pipe
        //      fork() -in child, redirect stdout close read side of pipe;; in parent redirect stdin
        //       ^ already written, just need to use
        //add checks for first/last command- for first cmd dont redirect stdin in parent; last cmd dont direct stdout of the child


        for(size_t i=0; i<tknr.commands.size(); i++){
            Command* current = tknr.commands.at(i);
            int pipefd[2];

            if(pipe(pipefd) ==-1){
                perror("pipe");
                exit(EXIT_FAILURE);
            }
            // fork to create child
            pid_t pid = fork(); //child
            if (pid < 0) {  // error check
                perror("fork");
                exit(2);
            }

            //add check for bg process- add pid to vector if bg and dont waitpid in parent
            // if (pid == 0) {  // if child, exec to run command
            //     // run single commands with no arguments
            //     //implement multiple args- iterates over args of current command to make char * array
                
            //     char* args[] = {(char*) tknr.commands.at(0)->args.at(0).c_str(), nullptr};

            //     if (execvp(args[0], args) < 0) {  // error check
            //         perror("execvp");
            //         exit(2);
            //     }
            // }
            if(pid == 0){
                // child- redirect from the previous pipe
                if(prevPipeRead != 1){
                    dup2(prevPipeRead, STDIN_FILENO);
                    close(prevPipeRead);
                }

                // redirect output to the current pipe if not last command
                if(i< tknr.commands.size()-1){
                    dup2(pipefd[1],STDOUT_FILENO);
                }

                //input redirection
                if (current->hasInput()) {
                    int inputFile = open(current->in_file.c_str(), O_RDONLY);
                    if (inputFile < 0) {
                        perror("input file");
                        exit(EXIT_FAILURE);
                    }
                    dup2(inputFile, STDIN_FILENO);
                    close(inputFile);
                }

                //output redirection
                if (current->hasOutput()) {
                    int outputFile = open(current->out_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
                    if (outputFile < 0) {
                        perror("output file");
                        exit(EXIT_FAILURE);
                    }
                    dup2(outputFile, STDOUT_FILENO);
                    close(outputFile);
                }
                
                // Close the read end of the pipe on the child side.
                close(pipefd[0]);

                //convert
                vector<const char*> argsArray;
                argsArray.reserve(current->args.size()+1);

                for (const string& arg : current->args) {
                    argsArray.push_back(arg.c_str());
                }

                argsArray.push_back(nullptr);
                
                // In child, execute the command
                execvp(argsArray[0], const_cast<char* const*>(argsArray.data()));
                //error message if execution fails
                cerr << "Error while executing this command: "<< argsArray[0] <<endl;
                exit(EXIT_FAILURE);
            }
            
            else {  // if parent, wait for child to finish
                int status = 0;
                if(current->isBackground()){
                    backgroundProcesses.push_back(pid);
                }
                else{
                    waitpid(pid, &status, 0);
                    if (status > 1) {  // exit if child didn't exec properly
                        exit(status);
                    }
                }
                close(pipefd[1]);
                prevPipeRead = pipefd[0];
            }
        }

        //restore original stdin/stdout (outside the loop)
        dup2(originalstdin, STDIN_FILENO);
        dup2(originalstdout, STDOUT_FILENO); 

        for (auto it = backgroundProcesses.begin(); it != backgroundProcesses.end();) {
            int status = 0;
            pid_t result = waitpid(*it, &status, WNOHANG);

            if (result == -1 || result == 0) {
                ++it;
            } 
            else {
                it = backgroundProcesses.erase(it);
            }
        }
        
    }
}
