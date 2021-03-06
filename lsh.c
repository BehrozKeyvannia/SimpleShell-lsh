  /* 
   * Main source code file for lsh shell program
   *
   * You are free to add functions to this file.
   * If you want to add functions in a separate file 
   * you will need to modify Makefile to compile
   * your additional functions.
   *
   * Add appropriate comments in your code to make it
   * easier for us while grading your assignment.
   *
   * Submit the entire lab1 folder as a tar archive (.tgz).
   * Command to create submission archive: 
        $> tar cvf lab1.tgz lab1/
   *
   * All the best 
   */


  #include <stdio.h>
  #include <stdlib.h>
  #include <readline/readline.h>
  #include <readline/history.h>
  #include "parse.h"
  #include <fcntl.h>
  #include <string.h>
  #include <errno.h>
  #include <sys/types.h>
  #include <unistd.h>
  #include <ctype.h>
  #include <sys/wait.h>

   
  /*
   * Function declarations
   */

  void PrintCommand(int, Command *);
  void PrintPgm(Pgm *);
  void stripwhite(char *);
  // void piping(char *args[]);
  void ifChangeDirectoryCommand(Command *cmd);
  void ifExitCommand(char **pl);
  // void execute(Command *cmd);
  void execute2(Command *cmd);
  int execute3(Pgm *pgm, int in, int out);

  /* When non-zero, this global means the user is done using this program. */
  int done = 0;


  /*
   * Name: main
   *
   * Description: Gets the ball rolling...
   *
   */



  int main(void)
  {
    Command cmd;
    int n;

    while (!done) {
      wait(NULL);
      char *line;
      line = readline("My lsh > ");

      if (!line) {
        /* Encountered EOF at top level */
        done = 1;
      }
      else {
        /*
         * Remove leading and trailing whitespace from the line
         * Then, if there is anything left, add it to the history list
         * and execute it.
         */
        stripwhite(line);

        if(*line) {
          add_history(line);
          /* execute it */
          n = parse(line, &cmd);
          
          ifChangeDirectoryCommand(&cmd);
          ifExitCommand(cmd.pgm->pgmlist);
          // execute(&cmd);
          execute2(&cmd);
          PrintCommand(n, &cmd);
        }
      }
      
      if(line) {
        free(line);
      }
    }
    return 0;
  }

  /*
   * Name: PrintCommand
   *
   * Description: Prints a Command structure as returned by parse on stdout.
   *
   */
  void
  PrintCommand (int n, Command *cmd)
  {
    printf("\nParse returned %d:\n", n);
    printf("   stdin : %s\n", cmd->rstdin  ? cmd->rstdin  : "<none>" );
    printf("   stdout: %s\n", cmd->rstdout ? cmd->rstdout : "<none>" );
    printf("   bg    : %s\n", cmd->bakground ? "yes" : "no");
    PrintPgm(cmd->pgm);
  }

  /*
   * Name: PrintPgm
   *
   * Description: Prints a list of Pgm:s
   *
   */
  void
  PrintPgm (Pgm *p)
  {
    int i = 0;
    if (p == NULL) {
      return;
    }
    else {
      char **pl = p->pgmlist;

      /* The list is in reversed order so print
       * it reversed to get right
       */
      PrintPgm(p->next);
      printf("    [");
      while (*pl) {
        printf("%s ", *pl++);
        i++;
      }

      printf("]\n");
    }
    // printf("Number of arguments: %d\n", i);
  }

  /*
   * Name: stripwhite
   *
   * Description: Strip whitespace from the start and end of STRING.
   */
  void
  stripwhite (char *string)
  {
    register int i = 0;

    while (isspace( string[i] )) {
      i++;
    }
    
    if (i) {
      strcpy (string, string + i);
    }

    i = strlen( string ) - 1;
    while (i> 0 && isspace (string[i])) {
      i--;
    }

    string [++i] = '\0';
  }

  // void execute(Command *cmd){
     
  //     pid_t pid; 
  //     int status; 

  //     if((pid = fork()) < 0){       //If the fork returns -1, the fork failed somehow
  //         printf("Forking child process failed\n");
  //         kill(getpid(),SIGTERM);
  //         exit(1);
  //     }

  //     else if (pid == 0){          //If forks() returns 0, the fork was successfull
  //         if((execvp(*cmd->pgm->pgmlist, cmd->pgm->pgmlist)) < 0){       //If the execvp returns -1, the command failed to be executed
  //             printf("Executing the command failed\n");
  //             kill(getpid(),SIGTERM);
  //             exit(1);
  //         }
  //     }
  //     else{
  //         while((wait(&status)) != pid);
  //     }
  // }



  /*
Frågor för TA:
--> När jag trycker sudo cat < tmp.1, får jag Error på open funktionen
--> när jag kör piping så hinner output komma innan lsh kommer tillbaka. T
    Testa med att köra ls -al | wc. Output My lsh > 13  110   780
--> När man trycker på ctrl+C ska child process termineras, inte själva
    lsh.




  */

   void execute2(Command *cmd){
      pid_t pid; 
      int status; 
      

     // wait(NULL);

      if((pid = fork()) < 0){       //If the fork returns -1, the fork failed somehow
          printf("Forking child process failed\n");
          kill(getpid(),SIGTERM);
          exit(1);
      }

      if (pid == 0){          
          if(cmd->rstdin != NULL){      
              int in = 0;
              if((in = open(cmd->rstdin, O_RDONLY | O_CREAT)) < 0 ){
                printf("Could not open cmd->rstdin\n");
                printf("%s\n", strerror(errno));
                return; 

              }
              dup2(in, fileno(stdin)); //CHANGE?
              close(in);
          }
          if(cmd->rstdout != NULL){
            int out = 0;
            //Could use creat(cmd->rstdout, mode) too???
            if((out = open(cmd->rstdout, O_WRONLY | O_CREAT)) < 0){
              printf("Could not open cmd->rstdout\n");
              return;
            }
            dup2(out, fileno(stdout));
            close(out);
          }
          if(cmd->pgm->next != NULL){
            execute3(cmd->pgm, -1, -1);
            exit(0);
          }
          


          int exec = 0;
          if((exec = execvp(*cmd->pgm->pgmlist, cmd->pgm->pgmlist)) < 0){
            printf("Could not execute command in pgmlist\n");
            printf("%s\n", strerror(errno));
            exit(0);
          }
         
      }
      if(cmd->bakground){
            //Dont wait for childs termination?
            // printf("Är i background nu \n");
            // signal(SIGINT, SIGTERM);
            signal(SIGCHLD, SIG_IGN); //do not interrupt for childen in B
            return;
      }
      waitpid(pid, NULL, 0);
      wait(NULL);
  }



  // void piping(char *args[]){

  //     int fd[2];
  //     pid_t pid[2];
  //     int status;


  //     if(pipe(fd) < 0 ){
  //       printf("Piping failed for some damn reason. \n");
  //       kill(getpid(),SIGTERM);
  //       exit(1);
  //     }
  //     fork(pid[0]);

  //     if( pid[0] < 0 ){
  //       printf("Forking in piping() failed\n");
  //       kill(getpid(),SIGTERM);
  //       exit(1);
  //     }

  //     // if(pid[0] == 0){
  //     //   dup2(fd[1], STDOUT_FILENO);
  //     // 	close(fd[0]);  //close the "write" end that is unused
  //     //   close(fd[1]);	

  //     //   execute()

  //     if(!pid[0]){

  //       //Set standard output to the pipe 
  //       close(fd[0]);
  //       dup2(fd[1], 1);
  //       close(fd[1]);

  //       execvp(args[0], args);
  //     }
      
  //     if(pid[0]){

  //       //we are in the parent 

  //         if(pid[1] = fork() < 0){

  //           //In the child

  //           close(fd[1]);
  //           dup2(fd[0], 0);
  //           close(fd[0]);

  //           execvp(args[0], args);

  //         }

  //         close(fd[0]);
  //         close(fd[1]);

  //         waitpid(pid[1], &status, 0);


  //     }
    

  // }

  void ifChangeDirectoryCommand(Command *cmd){
      if(strcmp(cmd->pgm->pgmlist[0], "cd")  == 0){
        if(cmd->pgm->pgmlist[1] == NULL){
          char *changeToHome;
          changeToHome = getenv("HOME");
          if(chdir(changeToHome) < 0) { printf("Could not change to home\n"); }

        }else if(strcmp(cmd->pgm->pgmlist[1], ".." ) == 0){
        char currentPath[1024];
        getcwd(currentPath, 1024);
        int lenghtOfPath = strlen(currentPath);
        while(currentPath[lenghtOfPath] != '/'){
            lenghtOfPath--;
        }
        currentPath[lenghtOfPath] = '\0';
        if(chdir(currentPath) < 0){
          printf("Could not change to directory --> cd .. \n");
          }
      }
      else{
        if(chdir(cmd->pgm->pgmlist[1]) < 0){
          printf("Failed to change directory\n");
        }
      }
      
    }
    //return false?
  }

  void ifExitCommand(char **pl){

      if(strcmp(pl[0], "exit") == 0){
        printf("Bye bye\n");
        exit(EXIT_SUCCESS);
      }

  }


int execute3(Pgm *pgm, int in, int out){

  

  int piping[2];
  
  if(pgm->next == NULL){
         // printf("Gick in i NULL\n");
                         //Probably last program to execute (so keep input)
      int child;
      if((child = fork()) < 0){
        printf("Could not fork child process\n");
        wait(NULL);     //Wait for any child process to return
        return child;

      }else if(child == 0){
        // printf("Snart kommer resultatet\n");
        
        
        close(fileno(stdout));
        
        if(dup(in) == -1){
          printf("ERROR DUP()\n");
        }

         if(close(in) == -1){
          printf("ERROR close(in)\n");
        }
        
        
        if(close(out) == -1 ){
          printf("ERROR close(out)\n");
        }
        

        if(execvp(*pgm->pgmlist, pgm->pgmlist) < 0 ){
          printf("Could not execute program from pgmlist, somewhere in the last pgm\n");
          printf("%s\n", strerror(errno));
          exit(0);
        }
      }
  }
  else if(in == -1){ //Probably first pgm to execute (so keep output)
      // printf("Piping\n");

      
    if(pipe(piping) < 0){
      printf("Could not pipe?????\n");
    }
      execute3(pgm->next, piping[1], piping[0]);
       //in and out
      int child;
      if((child = fork()) < 0 ){
        printf("Could not fork\n");
        close(piping[0]);
        close(piping[1]);
        wait(NULL); //Wait for any child to return
        return child;

      }else if (child == 0){
        // printf("Lyckades forka på in == -1\n");
        
        
        close(fileno(stdin));

        dup(piping[0]);
      
        close(piping[0]);
        close(piping[1]);

        if(execvp(*pgm->pgmlist, pgm->pgmlist) < 0){
          printf("Could not execute fork program, somewhere in first pgm\n");
          printf("%s\n", strerror(errno));
          exit(0);
        }
      }
  }
  else{   //Probably in the middle somewhere (so stdin and stdout)
      
      if(pipe(piping) < 0){
      printf("Could not pipe?????\n");
    }
      execute3(pgm->next, piping[1], piping[0]);
       int child;
      if((child = fork()) < 0 ){
        printf("Could not fork program somewhere in the middle\n");
        close(piping[0]);
        close(piping[1]);
        wait(NULL);
      }else if (child == 0){

        dup2(in, fileno(stdout));
        dup2(piping[0], fileno(stdin));
        close(in);
        close(piping[0]);
        close(piping[1]);

        if(execvp(*pgm->pgmlist, pgm->pgmlist) < 0){
          printf("Could not execute execvp in the middle somewhere\n");
          printf("%s\n", strerror(errno));
          exit(0);
        }

        }

      }

  }









