#include <stdio.h>
#include <unistd.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

int main() {
    int i;
    FILE *file = fopen("./pidy", "r");
    if (file == NULL) {
        perror("Nie można otworzyć pliku");
        return 1;
    }
    pid_t pid[4];
    for (i = 0; i < 4; i++) {
        if (fscanf(file, "%d", &pid[i]) != 1) {
            fprintf(stderr, "Nie można odczytać PID numer %d\n", i+1);
            return 1;
        }
    }
    fclose(file); 

    int petla = 0;
    int opcja, opcja2;
    while (petla == 0)
    {
        do {
            printf("Wybierz sygnal jaki chcesz przeslac:\n1) S1 - Zakoncz dzialanie\n2) S2 - Wstrzymanie programu\n3) S3 - Wznowienie\nOpcja: ");
            scanf("%d", &opcja);
        } while(opcja < 1 || opcja > 3);
        
        do {
            printf("Wybierz do ktorego procesu chcesz wyslac sygnal:\n1) Proces1\n2) Proces2\n3) Proces3\nWybor: ");
            scanf("%d", &opcja2);
        } while(opcja2 < 1 || opcja2 > 3);
        switch (opcja)
        {
        case 1:
            kill(pid[opcja2 - 1], SIGINT);
            exit(1);
            break;
        case 2:
            kill(pid[opcja2 - 1], SIGUSR1);
            break;
        case 3:
            kill(pid[opcja2 - 1], SIGCONT);
            break;
        default:
            break;
        }
    }
    return 0;
}
