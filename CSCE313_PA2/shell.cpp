/*
    Programming Assignment 2: Aggie Shell

    ==================================================
    Original assignment code written and provided by:
        Professor Kumar
        Department of Computer Science and Engineering
        Texas A&M University
        Date: 10/10/2024
    ==================================================

    ==================================================
    Pseudo-code and commented edits authored by:
        Kyle Lang
        Department of Computer Science and Engineering
        Texas A&M University
        Date: 10/10/2024
    ==================================================
*/


#include <iostream>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <termios.h>


#include <vector>
#include <string>
#include <ctime>

#include "Tokenizer.h"


// all the basic colours for a shell prompt
#define RED     "\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE	"\033[1;34m"
#define WHITE	"\033[1;37m"
#define NC      "\033[0m"
#define PATH_MAX 500


using namespace std;


/**
 * Return Shell prompt to display before every command as a string
 * 
 * Prompt format is: `{MMM DD hh:mm:ss} {user}:{current_working_directory}$`
 */

void function_wait(int signo) {
    (void)signo;  // avoid warnings
    while (waitpid(-1, nullptr, WNOHANG) > 0) {
    }
}

void print_prompt() {
    // Use time() to obtain the current system time, and localtime() to parse the raw time data
    time_t cur_time = time(0);
    struct tm timeinfo;
    localtime_r(&cur_time, &timeinfo);

    // Parse date time string from raw time data using strftime()
    // Date-Time format will be in the format (MMM DD hh:mm:ss), with a constant size of 15 characters requiring 16 bytes to output
    char buf1[100];
    strftime(buf1, sizeof(buf1), "%b %d %H:%M:%S", &timeinfo);

    // Obtain current user and current working directory by calling getenv() and obtaining the "USER" and "PWD" environment variables
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));

    std::string username = getenv("USER");

    // Can use colored #define's to change output color by inserting into output stream
    // MODIFY to output a prompt format that has at least the USER and CURRENT WORKING DIRECTORY
    std::cout << YELLOW << buf1 << " " << username << ":" << cwd << "$ " << NC;
}


/**
 * Process shell command line
 * 
 * Each command line from the shell is parsed by '|' pipe symbols, so command line must be iterated to execute each command
 * Example: `ls -l / | grep etc`    ::  This command will list (in detailed list form `-l`) the root directory and then pipe into a filter for `etc`
 * When parsed into the Tokenizer, this will split into two separate commands, `ls -l /` and `grep etc`.
 */
void process_commands(Tokenizer& tknr) {
    // Declare file descriptor variables for storing unnamed pipe fd's
    // Maintain both a FORWARD and BACKWARDS pipe in the parent for command redirection
    int backwards_fds[2] = {-1, -1};
    int forwards_fds[2] = {-1, -1};

    // LOOP THROUGH COMMANDS FROM SHELL INPUT
    for (size_t i = 0; i < tknr.commands.size(); i++) {
        auto cmd = tknr.commands[i];

        // Check if the command is 'cd' first
        // This will not have any pipe logic and needs to be done manually since 'cd' is not an executable command
        // Use getenv(), setenv(), and chdir() here to implement this command
        if (cmd->args[0] == "cd") {
            // There are two tested inputs for 'cd':
            // (1) User provides "cd -", which directs to the previous directory that was opened
            // (2) User provides a directory to open
            if (cmd->args.size() > 1) {
                setenv("OLDPWD", getenv("PWD"), 1); 
                if (chdir(cmd->args[1].c_str()) == 0) {
                    setenv("PWD", cmd->args[1].c_str(), 1); 
                } else {
                    perror("cd error");
                }
            } else {
                setenv("OLDPWD", getenv("PWD"), 1);
                chdir(getenv("HOME"));
                setenv("PWD", getenv("HOME"), 1); 
            }
            continue;
        }
        
        // Initialize backwards pipe to forward pipe
        // read is 0 and write is 1
        // backwards_fds = forwards_fds;
        // If any other command, set up forward pipe IF the current command is not the last command to be executed
        // handle piped between commands
        if (i < tknr.commands.size() -1 ) {
            if (pipe(forwards_fds) < 0){
                perror("pipe failed");
                exit(1);
            }

        }

        // fork to create child
        pid_t pid = fork();
        if (pid < 0) {  // error check
            perror("fork error");
            exit(2);
        }
        if (pid == 0) {  // if child, exec to run command
            if (i < tknr.commands.size() - 1) {  // i.e. {current command} | {next command} ...
                dup2(forwards_fds[1], STDOUT_FILENO);     // Reidrect STDOUT to forward pipe
                close(forwards_fds[1]);    // Close respective pipe end
            }

            if (i > 0) {    // i.e. {first command} | {current command} ...
                dup2(backwards_fds[0], STDIN_FILENO);     // Redirect STDIN to backward pipe
                close(backwards_fds[0]);    // Close respective pipe end
            }
            
            if (cmd->hasInput()) {    // i.e. {command} < {input file}
                int input = open(cmd->in_file.c_str(), O_RDONLY); // Open input file
                if (input < 0){
                    perror("redirection failed");
                    exit(1);
                }
                dup2(input, STDIN_FILENO); // Redirect STDIN from file
                close(input);
            }

            if (cmd->hasOutput()) {   // i.e. {command} > {output file}
                int output = open(cmd->out_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644); // Open output file
                if (output < 0){
                    perror("redirection failed");
                    exit(1);
                }
                dup2(output, STDOUT_FILENO); // Redirect STDOUT to file
                close(output);
            }

            // Execute command after any redirection
            vector<char*> args;
            for (auto& arg: cmd->args){
                args.push_back(const_cast<char*>(arg.c_str()));
            }
            args.push_back(nullptr);

            execvp(args[0], args.data());
            perror("execvp failed");
            exit(2);
        }
        else {  // if parent, wait for child to finish    
            // Close pipes from parent so pipes receive `EOF`
            // Pipes will otherwise get stuck indefinitely waiting for parent input/output that will never occur
            if (tknr.commands.size() > 1) {
                close(forwards_fds[1]);
            }

            if (backwards_fds[0] != -1) {
                close(backwards_fds[0]);
            }

            if (i < tknr.commands.size() - 1) {
                close(forwards_fds[1]); 
                backwards_fds[0] = forwards_fds[0]; 
                backwards_fds[1] = -1; 
            }

            // If command is indicated as a background process, set up to ignore child signal to prevent zombie processes
            if (cmd->args.back() == "&") {
                cmd->args.pop_back();
                signal(SIGCHLD, function_wait);
            }
            else {
                int status;
                waitpid(pid, &status, 0);
                
                if (status > 1) {  // exit if child didn't exec properly
                    exit(status);
                }
            }
        }
    }
}


int main () {
    // Use setenv() and getenv() to set an environment variable for the previous PWD from 'PWD'
    const char* cur_dir = getenv("PWD");
    setenv("OLDPWD", cur_dir, 1);

    
    // This is your process loop; Will run infinitely until given defined break case
    for (;;) {
        // need to print date/time, username, and absolute path to current dir into 'shell'
        // Might want to use a helper function like this here for code clarity
        print_prompt();
                
        // get user inputted command
        string input;
        getline(cin, input);
        
        if (input == "exit") {  // print exit message and break out of infinite loop
            cout << RED << "Now exiting shell..." << endl << "Goodbye" << NC << endl;
            break;
        }
        // Might want to catch empty input; Tokenizer will error and not recover on empty input
        if (input.empty()){
            continue; // skip empty input
        }

        // get tokenized commands from user input
        // This will parse your user input for you; Please refer the header files to understand how to use this class
        Tokenizer tknr(input);
        if (tknr.hasError()) {  // continue to next prompt if input had an error
            continue;
        }


        /*
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
        */
       
        // Might want to use a helper function like this for processing the shell's command line 
        process_commands(tknr);
    }
}
