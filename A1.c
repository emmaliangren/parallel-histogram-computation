#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/wait.h>

void sigHandler(int sigNum);

// pipe & pid arr declaration
int ** pipes = NULL;
pid_t * childPIDs = NULL; // for linking child pids to their pipes!
int args = 0; // global argc - 1 (excluding executable)
int nbytes;
int children;

// pipe / file read vars
int writebuffer[26];

int main(int argc, char **argv)
{
    signal(SIGCHLD, sigHandler);
    args = argc - 1;
    children = args;

    for (int i=0;i<26;i++) // initializing writebuffer
    {
        writebuffer[i] = 0;
    }

    if (argc==1) // just the executable, no files
    {
        fprintf(stderr, "No input files were provided via command line.\n");
        exit(-1);
    }

    // pipe alloc
    pipes = (int**) malloc(sizeof(int*) * (args));
    if (pipes==NULL)
    {
        fprintf(stderr, "Malloc failed");
        exit(1);
    }

    for (int i=0;i<args;i++)
    {
        pipes[i] = (int*) malloc(2 * sizeof(int));
        if (pipes[i]==NULL)
        {
            fprintf(stderr, "Malloc failed");
            exit(1);
        }
    }

    // making pipes for processes
    for (int i=0;i<args;i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            perror("Pipe Creation Error");
            exit(1);
        }
    }

    childPIDs = (pid_t *) malloc(sizeof(pid_t) * (args));
    // fork children & child processes!
    for (int i=0;i<args;i++)
    {
        if ((childPIDs[i] = fork()) == 0) // child process
        {
            if (strcmp(argv[i],"SIG")==0)
            {
                // frees
                for (int i=0;i<args;i++)
                {
                    free(pipes[i]);
                }
                free(pipes);
                free(childPIDs);
                
                kill(childPIDs[i],SIGINT);
            }
            
            else
            {
                FILE * fptr = fopen(argv[i+1],"r");
                if (fptr == NULL)
                {
                    close(pipes[i][1]); // close child process write pipe
                    exit(1);
                }

                // histogram calculations
                char buff;
                while(!feof(fptr))
                {
                    buff = fgetc(fptr);
                    if ((int)buff>=97 && (int)buff<=122) // lowercase letters
                    {
                        writebuffer[(int)buff-97] += 1;
                    }
                    else if ((int)buff>=65 && (int)buff<=90) // uppercase letters
                    {
                        writebuffer[(int)buff-65] += 1;
                    }
                }

                // writing histogram data to parent
                close(pipes[i][0]); // close read 
                nbytes = write(pipes[i][1], &writebuffer, sizeof(writebuffer));
                
                // exit protocol :)
                sleep(10+(3*i));
                close(pipes[i][1]); // close write

                // frees
                for (int i=0;i<args;i++)
                {
                    free(pipes[i]);
                }
                free(pipes);
                free(childPIDs);
                fclose(fptr);
                exit(0);
            }
        }
    }
    while (children > 0);
    for (int i=0;i<args;i++)
    {
        free(pipes[i]);
    }
    free(pipes);
    free(childPIDs);
    exit(0);
}

void sigHandler(int sigNum)
{
    // reinstate signal handler 
    if (signal(SIGCHLD, sigHandler) == SIG_ERR)
        fprintf(stderr, "Can't catch SIGCHILD");

    // process signal
    if (sigNum == SIGCHLD)
    {
        children-=1;

        int child_status;
        pid_t childPid = waitpid(-1,&child_status, WNOHANG);
        printf("caught SIGCHLD; child id = [%d]\n",childPid);
        if (WIFEXITED(child_status))
        {
            // getting index of correct pipes
            int idx = -1;
            for (int i=0;i<args;i++)
            {
                if (childPid == childPIDs[i])
                {
                    idx = i; 
                }
            }

            if (idx!=-1) // if not invalid idx
            {
                close(pipes[idx][1]); // close child process write pipe
                nbytes = read(pipes[idx][0], writebuffer, sizeof(writebuffer));

                // file name & file open
                char * filename = malloc(sizeof(char)*15);
                sprintf(filename,"file%d.hist",childPid);

                FILE *fptr = fopen(filename,"w");

                if (fptr == NULL)
                {
                    fprintf(stderr, "File open failed");
                    exit(1);
                }

                for (int i=0;i<26;i++)
                {
                    fprintf(fptr, "%c %d\n",(char)(97+i),writebuffer[i]);
                }

                fclose(fptr);
                free(filename);

            }
        }
    }  
}
