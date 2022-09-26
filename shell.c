#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include<sys/wait.h>

/**
 * A basic UNIX shell that supports input/output redirection.
 *
 * @return the exit status of the program.
 */
int main() {
    char c[1024]; //The main buffer for each user input
    char direct[1024]; //Direct holds the string of the directory displayed to the user
    char homedir[1024]; //Homedir holds the string of the full directory
    char homealias[64]; //Homealias holds the string of the home directory of the user
    int n; //Used for read/write
    char * argv[100]; //Holds command-line arguments for each command
    getcwd(direct, sizeof(direct)); //Gets the current working directory
    memcpy(homedir, &direct[0], 19); //Direct only holds what needs to be shown to the user (omits home dir)
    memcpy(homealias, &direct[0], 19); //Copies the home directory to homealias
    chdir(homedir); //Sets the current directory to the homedirectory

    while(1) { //Runs until the program is exited through exit() or a return statement
        memset(c, 0, 1024); //Clears the buffer each iteration
        getcwd(homedir, sizeof(homedir)); //Gets the cwd each iteration
        if (strstr(homedir, homealias)) { //If we're in the home directory
            memcpy(direct, &homedir[19], 1024);
            printf("1730sh:~%s$ ", direct);
            fflush(stdout); //Because I use a printf with no \n I flush stdout to get it to print
        } else { //If we're not in the home directory
            printf("1730sh:%s$ ", homedir);
            fflush(stdout);
        }
        if ((n = read(STDIN_FILENO, c, 1024)) > 0) { //Reads a full line of input
            char *token;
            if (c[strlen(c) - 1] == '\n') { //Gets rid of the newline char from read()
                c[strlen(c) - 1] = '\0';
            }
            token = strtok(c, " "); //Parses out the first command-line argument
            int x = 0; //x holds the spot for argv
            while (token != NULL) { //Gets each command-line argument and puts it in argv
                argv[x] = token;
                token = strtok(NULL, " ");
                x++;
            }
            argv[x] = NULL; //The final one needs to be a NULLPTR for execvp

            if (strcmp(argv[0], "exit") == 0) { //Exit is handled differently
                return 0;
            } else if (strcmp(argv[0], "cd") == 0) { //As is cd
                if (argv[1] != NULL) {
                    chdir(argv[1]);
                }
            } else { //This handles all over commands
                int pid = fork();
                if (pid == 0) { //Means the fork was successful
                    int fd0, fd1;
                    int z = 0;
                    char * buf[100]; //This is the buffer that will be used for execv
                    for(int i = 0; i < x; i++) { //Loops through the command-line arguments
                        if(strcmp(argv[i], ">") == 0) { //Output redirection
                            fd1 = open(argv[i + 1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR); //Files can be made, written to, etc.
                            if(fd1 < 0) {
                                perror("cant open file");
                                return 0;
                            }
                            if(dup2(fd1, 1) != 1) perror("dup2"); //dup2() is used to redirect STDOUT
                            close(fd1);
                            i++; //Another i++ gets rid of the next command-line argument (likely a file)
                        } else if(strcmp(argv[i], "<") == 0) { //Input redirection
                            fd0 = open(argv[i + 1], O_RDONLY); //Opens for read only
                            if(fd0 < 0) {
                                perror("cant open file");
                                return 0;
                            }
                            if(dup2(fd0, 0) != 0) perror("dup2"); //dup2() is used to redirect STDIN
                            close(fd0);
                            i++;
                        } else if(strcmp(argv[i], ">>") == 0) { //Same as output redirection except we append this time
                            fd1 = open(argv[i + 1], O_WRONLY | O_APPEND); //Opened for APPEND
                            if(fd1 < 0) {
                                perror("cant open file");
                                return 0;
                            }
                            if(dup2(fd1, 1) != 1) perror("dup2"); //Again we redirect STDOUT
                            close(fd1);
                            i++;
                        } else { //If no < > or >> is found we add whatever the argument is to the new buffer
                            buf[z] = argv[i];
                            z++; //z holds the location inside of buf[]
                        }
                    }
                    buf[z] = NULL; //Final spot is NULL for execvp
                    execvp(buf[0], buf); //We exec the command
                    printf("FAILED\n"); //This will only run of execvp fails
                } else { //This likely means the fork was unsuccessful
                    wait(NULL); //We use wait()
                }
            }
        }
    }
    //No return statement here needed since the only ways to exit are to use the exit command (which has a return) or to CTRL+Z to force quit
}
