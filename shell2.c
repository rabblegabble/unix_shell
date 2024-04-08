#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
#define BUFFER_SIZE 50 /*50 char for the max buffer size */
#define MAX_COMMANDS 35
#define HISTORY_FILENAME "Hall1553.history"
void save_history(void);
static char buffer[BUFFER_SIZE];
char command_history[MAX_COMMANDS][MAX_LINE];
int command_count = 0;
bool sig_flag = 0;

char originalInput[MAX_LINE];
bool r_flag = 0;
bool rx_flag = 0;
int chooseInt;



void handle_SIGINT() {
  
	
    if (command_count > 0) {
      printf("\nLast commands (up to 10 and most recent first):\n");
    }
    else {
      printf("\nNo commands to display");
    }
    int i, j;
    for (i = 0; i < command_count && i < 10; i++) {
    j = (command_count - 1 - i + MAX_COMMANDS) % MAX_COMMANDS;
    
    printf("%i. %s", i + 1, command_history[j]);
    fflush(NULL);
        
    }
     fflush(NULL);
     printf("COMMAND->");
     sig_flag = 1;
  
        
}

/**
 * setup() reads in the next command line, separating it into distinct tokens
 * using whitespace as delimiters. setup() sets the args parameter as a 
 * null-terminated string.
 */

void setup(char inputBuffer[], char *args[],int *background)
{
    int length, /* # of characters in the command line */
        i,      /* loop index for accessing inputBuffer array */
        start,  /* index where beginning of next command parameter is */
        ct;     /* index of where to place the next parameter into args[] */
    
    ct = 0;
    char originalInput[MAX_LINE];

   
    /* read what the user enters on the command line */
    if (r_flag == 0 && rx_flag == 0) {
    length = read(STDIN_FILENO, inputBuffer, MAX_LINE); 
      fflush(NULL);
    }
    else if (r_flag == 1 && rx_flag == 0) {
      fflush(NULL);
      strcpy(inputBuffer, command_history[command_count - 2]);
      length = strlen(inputBuffer);
      fflush(NULL);
    }
    else if (rx_flag == 1 && r_flag == 0) {
      strcpy(inputBuffer, command_history[chooseInt]);
      length = strlen(inputBuffer);
    }

     if (sig_flag) {
        sig_flag = 0;
        fflush(NULL);
        length = read(STDIN_FILENO, inputBuffer, MAX_LINE); 
    }
    start = -1;
    if (length == 0) {
      
        exit(0);      
    }

    if (length < 0){

        perror("error reading the command");

        exit(-1);           /* terminate with error code of -1 */
    }

    /* make a copy of the input before modifying it */
    strcpy(originalInput, inputBuffer);
    
    /* examine every character in the inputBuffer */
    for (i = 0; i < length; i++) { 
        switch (inputBuffer[i]){
        case ' ':
        case '\t' :               /* argument separators */
            if(start != -1){
                args[ct] = &inputBuffer[start];    /* set up pointer */
                ct++;
            }
            inputBuffer[i] = '\0'; /* add a null char; make a C string */
            start = -1;
            break;
            
        case '\n':                 /* should be the final char examined */
            if (start != -1){
                args[ct] = &inputBuffer[start];     
                ct++;
            }
            inputBuffer[i] = '\0';
            args[ct] = NULL; /* no more arguments to this command */
            break;

        case '&':
            *background = 1;
            inputBuffer[i] = '\0';
            break;
            
        default :             /* some other character */
            if (start == -1)
                start = i;
        } 
    }    
    
    /* copy the entire original input to command history */
    
    fflush(NULL);
    strcpy(command_history[command_count], originalInput);
    fflush(NULL);
    command_count++; // increment the history counter
    save_history();
    r_flag = 0;
    rx_flag = 0;
}

void save_history() {
  //opens file
	int history_file = open(HISTORY_FILENAME, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	//printf("IN SAVE HISTORY\n");
	int i;
    if (history_file == -1) {
        perror("Failed to open history file for writing");
        return;
    }
    //writes commands to file
    for (i = 0; i < command_count; i++) {
        printf("Writing Command to buffer\n");
        write(history_file, command_history[i], strlen(command_history[i]));
        write(history_file, "\n", 1); // Adds a newline after each command
    }
    
    close(history_file);
}

// Function to load the command history from the file
void load_history() {
    int history_file = open(HISTORY_FILENAME, O_RDONLY);
    //printf("IN LOAD HISTORY");
    if (history_file == -1) {
      printf("No history to display, enter commands\n");
        return; // returns if file doesnt exist
    }

    char line[MAX_LINE];
    int bytes_read;

    while ((bytes_read = read(history_file, line, MAX_LINE)) > 0) {
        line[bytes_read - 1] = '\0'; // Removes the newline character
        strcpy(command_history[command_count], line);
        command_count++;
	printf("Loading from buffer\n");
    }

    close(history_file);
}


int main(void)
{
    
    load_history();
    
    // save_history(); // this line is for testing purposes, save history seems to work
    char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */
    int background;  /* equals 1 if a command is followed by '&' */ 
    char *args[MAX_LINE/2+1];/* command line (of 80) has max of 40 arguments */
    pid_t pid;
    int status;
    struct sigaction handler;
    handler.sa_handler = handle_SIGINT;
    sigaction(SIGINT, &handler, NULL);
    strcpy(buffer, "\nCaught <ctrl><c>\n");
    pid_t waitpid(pid_t pid, int *status_ptr, int options);
	
    while (1){ /* Program terminates normally inside setup */

      
      
	background = 0;
	printf("COMMAND->");
        fflush(NULL);
        setup(inputBuffer, args, &background);       /* get next command */
        
        if (strcmp(args[0], "r") == 0) {
          fflush(NULL);
          r_flag = 1;
          setup(inputBuffer, args, &background);
	  save_history();
        } else if (args[0][0] == 'r' && args[0][1] != '\0') {
          printf("were in rx loop");
          rx_flag = 1;
          fflush(NULL);
          char* str = &args[0][2];
          chooseInt = atoi(str);
          setup(inputBuffer, args, &background);
	  save_history();
        }
        
	pid = fork(); /* creates fork and assigns process id to pid */

        if (pid < 0) {              /* runs if an error has occurred */
	  fprintf(stderr, "For initiation failed");
          save_history();
	  return 1;
	}

	else if (pid == 0) {     /* child process */

    
    if(execvp(args[0], args) < 0) {    /* executes the command */
	    printf("execution failed\n");
            save_history();
	    return 1;
	  }
	}
        else {     /* the parent process!! */
	  if (background == 0) {  /* command waits for the child process */
	      waitpid(pid, &status, 0);
	    }
	}
     }

    save_history();	
    return 0;
 
}
