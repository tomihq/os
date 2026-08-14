/* An orphan process is a child process whose parent terminates before the child finishes executing. As a result, the child becomes an orphan.
    When this happens, the orphan process is immediately adopted by another process, typically systemd (or PID 1) on modern Linux systems, which becomes its new parent.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(){
    pid_t pid = getpid();
    printf("Hallo, meine pid ist: %d \n", pid);
    
    pid_t child = fork();
    if(child < 0){
        printf("Nein! Etwas ist schiefgelaufen \n");
        exit(EXIT_FAILURE);
    }

    // Note that after the fork(), both processes start from the same IP. We don't have a guarantee about the execution order. Consider these problems as independent.
    if(child == 0){
        printf("Hallo! Dies ist ein Kindprozess, und ich habe die PID: %d und meine Parent ist PID: %d \n", getpid(), getppid());
        sleep(20);
    }else{
        printf("Hallo! Das ist Parent.\n");
        exit(EXIT_SUCCESS);
    }

    printf("Hallo! Meine Parent ist PID: %d \n", getppid());
    return 0; 

}