/****************
LE2: Introduction to Unnamed Pipes
****************/
#include <unistd.h> // pipe, fork, dup2, execvp, close
using namespace std;

int main () {
    // lists all the files in the root directory in the long format
    char* cmd1[] = {(char*) "ls", (char*) "-al", (char*) "/", nullptr};
    // translates all input from lowercase to uppercase
    char* cmd2[] = {(char*) "tr", (char*) "a-z", (char*) "A-Z", nullptr};

    // TODO: add functionality
    // Create pipe
    int fds[2]; // [0] is read and [1] is write
    if (pipe(fds) == -1){
        perror("pipe");
        return 1;
    }

    // Create child to run first command
    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("fork");
        return 1;
    }
    // In child, redirect output to write end of pipe
    dup2(fds[1], STDOUT_FILENO);
    // Close the read end of the pipe on the child side.
    close(fds[0]);
    close(fds[1]);
    // In child, execute the command
    execvp(cmd[0], cmd1);
    perror("execvp");


    // Create another child to run second command
    pid_t pid2 = fork();
    if (pid2 == -1){
        perror("fork")
        return 1;
    }
    // In child, redirect input to the read end of the pipe
    dup2(fds[0], STDOUT_FILENO);
    // Close the write end of the pipe on the child side.
    close(fds[0]);
    close(fds[1]);
    // Execute the second command.
    execvp(cmd2[0], cmd2);
    perror("execvp");
    return 1; 

    // Reset the input and output file descriptors of the parent.
}
