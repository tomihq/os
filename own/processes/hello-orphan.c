/* An orphan process is a child process whose parent terminates before the child finishes executing. As a result, the child becomes an orphan.
    When this happens, the orphan process is immediately adopted by another process, typically systemd (or PID 1) on modern Linux systems, which becomes its new parent.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(){
    pid_t pid = getpid();
    printf("[PARENT] Hallo, meine pid ist: %d \n", pid);
    
    pid_t child = fork();
    if(child < 0){
        printf("[PARENT] Nein! Etwas ist schiefgelaufen \n");
        exit(EXIT_FAILURE);
    }

    if(child == 0){
        printf("[CHILD] Ich habe die PID: %d und meine Parent ist PID: %d \n", getpid(), getppid());
        printf("[CHILD] Ich gehe jetzt schlafen; bis in 20 Sekunden. \n");
        fflush(stdout);
        sleep(20);
    }else{
        printf("[PARENT] Auf Wiedersehen! \n");
        exit(EXIT_SUCCESS);
    }
    /* To see more information the parent, execute in bash:ps -p $PID -o pid,ppid,comm,args. Replace $PID with your child's PID. 
    E.g.:  PID: 2959  PPID:1 COMMAND: systemd COMMAND: /usr/lib systemd/systemd --user*/
    printf("[CHILD] Meine Parent ist PID: %d \n", getppid());
    exit(EXIT_SUCCESS);

}