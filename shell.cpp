#include <iostream>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <vector>
#include <string>

#include "Tokenizer.h"

// all the basic colours for a shell prompt
#define RED     "\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE	"\033[1;34m"
#define WHITE	"\033[1;37m"
#define NC      "\033[0m"

using namespace std;

int main () {
    
    int num1 = dup(0);
    int num2 = dup(1);
    string pwd, cwd;
    char buf[400];
    vector<int> backgroundPIDs;
    for (;;) {
        time_t t;
        time(&t);
        char* pTime = 4+ctime(&t);
        char* username;
        username = getenv("USER");
        cwd = getcwd(buf,sizeof(buf));
        
        // Before prompting user, iterate over vector to reap processes.
        // Make waitpid() non-blocking for background processes.
        int status = 0;
        for(unsigned int sig = 0; sig < backgroundPIDs.size(); sig++){

            int result = waitpid(backgroundPIDs.at(sig),&status,WNOHANG);
            // Condition for the pid 
            if(result > 0){
                backgroundPIDs.pop_back();
            }
        }
        
        // need date/time, username, and absolute path to current dir
        cout << YELLOW << pTime << " " << username << " " << buf << NC << "$ ";
        
        // get user inputted command
        string input;
        getline(cin, input);
        // for(unsigned int i=0; i<input.length(); i++){
        //     tolower(input.at(i));
        // }

        if (input == "exit") {  // print exit message and break out of infinite loop
            cout << RED << "Now exiting shell..." << endl << "Goodbye" << NC << endl;
            break;
        }

        // get tokenized commands from user input
        Tokenizer tknr(input);
        if (tknr.hasError()) {  // continue to next prompt if input had an error
            continue;
        }
        
        // Directory Handling
        if(tknr.commands.at(0)->args.at(0)=="cd"){
            if(tknr.commands.at(0)->args.at(1)=="-"){
                chdir(pwd.c_str());
                
                string temp = pwd;
                pwd = cwd;
                cwd = temp;
            }
            else{
                chdir((char *) (tknr.commands.at(0)->args.at(1)).c_str());
                pwd = cwd;
                memset(buf, 0, 400);
                getcwd(buf, sizeof(buf));
            }
            continue;
        }
    
        
        // // print out every command token-by-token on individual lines
        // // prints to cerr to avoid influencing autograder
        // for (auto cmd : tknr.commands) {
        //     for (auto str : cmd->args) {
        //         cerr << "|" << str << "| ";
        //     }
        //     if (cmd->hasInput()) {
        //         cerr << "in< " << cmd->in_file << " ";
        //     }
        //     if (cmd->hasOutput()) {
        //         cerr << "out> " << cmd->out_file << " ";
        //     }
        //     cerr << endl;
        // }

        // Foreach command in token.commands
        for(unsigned int i=0; i<tknr.commands.size(); i++){
            // TODO: add functionality
            // Create pipe
            int fds[2];
            pipe(fds);
            
            vector<string> v = tknr.commands.at(i)->args;
            unsigned int x = v.size();
            char** argsX = new char*[x+1];
            
            int f = fork();
            // Create child to run first command
            if (f == 0){
                // Multiple Redirection
                for(unsigned int j=0; j<x; j++){
                    argsX[j] = (char*) v.at(j).c_str();
                }
                argsX[x] = nullptr;
                
                if(i<tknr.commands.size()-1){
                    // In child, redirect output to write end of pipe
                    dup2(fds[1],1);
                }
                
                // Output Redirection
                if(tknr.commands.at(i)->hasOutput()){
                    int varWrite = open((char *)tknr.commands.at(i)->out_file.c_str(), O_CREAT|O_WRONLY|O_TRUNC, 0600);
                    dup2(varWrite, 1);
                }
                
                 // Input Redirection
                if(tknr.commands.at(i)->hasInput()){
                    int varRead = open((char *)tknr.commands.at(i)->in_file.c_str(), O_RDONLY);
                    dup2(varRead, 0);
                }
                
                // Close the read end of the pipe on the child side.
                close(fds[0]);
                // In child, execute the command - command.args
                if(execvp(argsX[0],argsX)==-1){
                    exit(0);
                }
                execvp(argsX[0],argsX);
                
            } 
            else{
                // REDIRECT the Shell (Parent)'s input to the read end of the pipe.
                dup2(fds[0],0);
                
                // Close the write end of the pipe.
                close(fds[1]);
                
                if(i==tknr.commands.size()-1){
                    // Handle commands that run background.
                    // Return the user prompt to user control until the command is done.
                    // Store pid from fork() in vector of background processes.
                    int status = 0;
                    if(tknr.commands.at(i)->isBackground()){
                        backgroundPIDs.push_back(f);
                    }else{
                        waitpid(f,&status,0); //WAITPID PLS
                    }
                }
            }
            delete [] argsX;
        }
        
        dup2(num1,0);
        dup2(num2,1);    
    }
}