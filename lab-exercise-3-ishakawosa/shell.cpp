/****************
LE2: Basic Shell
****************/
#include <unistd.h> // pipe, fork, dup2, execvp, close
#include <sys/wait.h> // wait
#include "Tokenizer.h"
#include <iostream>
using namespace std;


int main () {
     //save originial stdin and stout
    int originalstdin = dup(STDIN_FILENO);
    int originalstdout = dup(STDOUT_FILENO);

    string input;
    while(true){
        cout << "Provide commands: ";
        getline(cin, input);

        if(input=="exit" || input=="Exit"|| input=="EXIT"){
            break;
        }

        Tokenizer token(input);

        int prevPipeRead = -1;

        //loop for each command in token.commands:
        for(size_t i=0; i<token.commands.size(); i++){
            Command* current = token.commands.at(i);
            // Create pipe
            int pipefd[2];

            if(pipe(pipefd) ==-1){
                perror("pipe");
                exit(EXIT_FAILURE);
            }

            // Create child to run first command
            pid_t childPID = fork();
            if (childPID == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            if(childPID == 0){
                // child- redirect from the previous pipe
                if(prevPipeRead != 1){
                    dup2(prevPipeRead, STDIN_FILENO);
                    close(prevPipeRead);
                }

                // redirect output to the current pipe if not last command
                if(i< token.commands.size()-1){
                    dup2(pipefd[1],STDOUT_FILENO);
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

            else{
                //redirect the shell (parent) input to the read end of the pipe
                //close the write end of current pipe
                close(pipefd[1]);

                //if prev pipe- close
                if(prevPipeRead != 1){
                    close(prevPipeRead);
                }

                //read end of current pipe into prev pipe read for next iteration
                prevPipeRead = pipefd[0];
                
                //wait for child
                wait(NULL);
            }
        }
        //restore the stdin and stdout
        dup2(originalstdin, STDIN_FILENO);
        dup2(originalstdout, STDOUT_FILENO);       
    }

}

