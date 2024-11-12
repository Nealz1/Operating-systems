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

//Semafory
struct sembuf buf;
key_t key1, key2, key3;
int semid, shmid;
char *buf_ptr;
int shmid_wstrzymanie;
int* wstrzymanie_ptr;

//Potrzeba projektu
pid_t pid0, pid1, pid2, pid3;
char pidy[256] = "./pidy";
char pidstring[10];
char temp[100] = "./temp";
int petla;

void podnies(int semid, int semnum){
    buf.sem_num = semnum;
    buf.sem_op = 1;
    buf.sem_flg = 0;
    int ret;
    do {
        ret = semop(semid, &buf, 1);
    } while (ret == -1 && errno == EINTR);
    if (ret == -1){
        perror("Podnoszenie semafora");
        exit(1);
    }
}

void opusc(int semid, int semnum){
    buf.sem_num = semnum;
    buf.sem_op = -1;
    buf.sem_flg = 0;
    int ret;
    do {
        ret = semop(semid, &buf, 1);
    } while (ret == -1 && errno == EINTR);
    if (ret == -1){
        perror("Opuszczenie semafora");
        exit(1);
    }
}

void Czyszczenie(){
	kill(pid3, SIGKILL);
    kill(pid2, SIGKILL);
    kill(pid1, SIGKILL);
    remove(temp);
}

void Wyslij(int sig){
	pid_t pidt = getpid();
    if(pid1 != 0 && pid1 != pidt) kill(pid1, sig);
    if(pid2 != 0 && pid2 != pidt) kill(pid2, sig);
    if(pid3 != 0 && pid3 != pidt) kill(pid3, sig);
}

void Quit(int sig){
	if(petla == 1){
		petla = 0;
		*wstrzymanie_ptr = 0;
		Wyslij(sig);
		Czyszczenie();
		exit(0);
	}   
}

void Stop(int sig){
	if(*wstrzymanie_ptr == 0){
		Wyslij(sig);
		*wstrzymanie_ptr = 1;
	}
}

void Start(int sig){
    if(*wstrzymanie_ptr == 1){
		Wyslij(sig);
		*wstrzymanie_ptr = 0;
	}
}

void Sygnaly(){
	signal(SIGINT, Quit);
	signal(SIGUSR1, Stop);
	signal(SIGCONT, Start);
}

int main()
{
    //SEMAFORY
    //Utworzenie klucza dla semaforów
    if ((key1 = ftok(".", 'A')) == -1)
    {
        errx(1, "Blad tworzenia klucza!");
    }

    semid = semget(key1, 3, IPC_CREAT | 0600);

    //Nadanie wartości semaforom
    if (semctl(semid, 0, SETVAL, (int)1) == -1)
    {
        perror("Nadanie wartosci semaforowi 0");
        exit(1);
    }
    if (semctl(semid, 1, SETVAL, (int)0) == -1)
    {
        perror("Nadanie wartosci semaforowi 1");
        exit(1);
    }
     if (semctl(semid, 1, SETVAL, (int)0) == -1)
    {
        perror("Nadanie wartosci semaforowi 2");
        exit(1);
    }

    //Utworzenie klucza dla pamieci wspoldzielonej
    if ((key2 = ftok(".", 'B')) == -1){
        errx(1, "Blad tworzenia klucza!");
	}
	if ((key3 = ftok(".", 'C')) == -1){
		errx(1, "Blad tworzenia klucza!");
	}
    //Deklaracja pamieci wspoldzielonej
    shmid = shmget(key2, 256 * sizeof(char), IPC_CREAT | 0600);
	shmid_wstrzymanie = shmget(key3, sizeof(int), IPC_CREAT | 0600);
	
    if (shmid == -1)
    {
        perror("Utworzenie segmentu pamieci wspoldzielonej");
        exit(1);
    }
    if (shmid_wstrzymanie == -1){
		perror("Utworzenie segmentu pamieci wspoldzielonej");
		exit(1);
	}

    //Pobranie wskaznika na adres pamieci wspoldzielonej
    buf_ptr = shmat(shmid, NULL, 0);
	wstrzymanie_ptr = shmat(shmid_wstrzymanie, NULL, 0);
    if (buf_ptr == NULL)
    {
        perror("Przylaczenie segmentu pamieci wspoldzielonej");
        exit(1);
    }
    if (wstrzymanie_ptr == (void*)-1)
	{
		perror("Przylaczenie segmentu pamieci wspoldzielonej");
		exit(1);
	}
    //KONIEC SEMAFOROW

    //PROJEKT
	Sygnaly();
    char nazwaPliku[256];
    char tekst[256];
    char hex[3];
    int opcja;
    
    int d;
    d = open(temp, O_CREAT | O_TRUNC, 0666);
    close(d);

    pid0 = getpid();
    
    int pids;
	pids = open(pidy, O_CREAT | O_TRUNC | O_WRONLY, 0666);
	if(pids < 0){
		printf("Nie znaleziono pliku o nazwie: %s\n", pidy);
		exit(1);
	}
    sprintf(pidstring, "%d\n", pid0);  
	write(pids, pidstring, strlen(pidstring));

    petla = 0;
    *wstrzymanie_ptr = 0;
    while (petla == 0)
    {
        printf("Wybierz tryb:\n1) - Recznie z klawiatury\n2) - Z pliku\n3) - /dev/urandom\nOpcja: ");
        scanf("%d", &opcja);
        switch (opcja)
        {
        case 1:
				printf("Podaj ciąg znaków\n");
				scanf("%s", tekst);
				petla = 1;
				break;
        case 2:
            printf("Wprowadz nazwe pliku do odczytu:\n");
            scanf("%s", nazwaPliku);
            petla = 1;
            break;
        case 3:
            strcpy(nazwaPliku, "/dev/urandom");
            petla = 1;
            break;
        default:
            petla = 0;
            printf("Zła opcja!\n");
            break;
        }
    }

    // Proces1 - komunikacja: wysyla - semafory. Konwersja na hex
    pid1 = fork();
    if (pid1 > 0) {
		sprintf(pidstring, "%d\n", pid1);  
		write(pids, pidstring, strlen(pidstring));
	}
	if(pid1 == 0){
		int i;
		if(opcja == 1){
			int dlugosc = strlen(tekst);
			for(i = 0; i < dlugosc; ++i){
				sprintf(hex, "%02x", tekst[i]);
				opusc(semid, 0);
				strcpy(buf_ptr, hex);
				podnies(semid, 1);
				while (*wstrzymanie_ptr == 1){
					usleep(100000);
				}
			}
		}
		else{
			int des;
			des = open(nazwaPliku, O_RDONLY);
			if(des < 0){
				printf("Nie znaleziono pliku o nazwie: %s\n", nazwaPliku);
				exit(1);
			}
			char c;
			while(read(des, &c, 1)){
				while (*wstrzymanie_ptr == 1){
					usleep(100000);
				}
				sprintf(hex, "%02x", c);
				opusc(semid,0);
				strcpy(buf_ptr, hex);
				podnies(semid, 1);	
			}
		}
	}
	//Proces2 - komunikacja: odbior - semafory, wysyla - plik
	//ODCZYT Z SEMAFOROW I wyslij do pliku
	pid2 = fork();
	if (pid2 > 0) {
		sprintf(pidstring, "%d\n", pid2);  
		write(pids, pidstring, strlen(pidstring));
	}
	if(pid2 == 0){
		int fd;
		fd = open(temp, O_CREAT | O_WRONLY, 0666); //tworzy, zapis, jesli juz istnieje
		if(fd < 0){
			perror("Nie można otworzyć pliku");
			exit(1);
		}
		while(1){
			while (*wstrzymanie_ptr == 1){
                usleep(100000);
            }
			opusc(semid, 1);
			write(fd, buf_ptr, strlen(buf_ptr));
            write(fd, "\n", 1);
			podnies(semid, 0);
		}
		close(fd);
	}

    // Proces3 - komunikacja: odbior - plik
	pid3 = fork();
	if (pid3 > 0) {
		sprintf(pidstring, "%d\n", pid3);  
		write(pids, pidstring, strlen(pidstring));
	}
	if(pid3 == 0){
        int fd;
        int licznik = 0;
        fd = open(temp, O_RDONLY);
        if(fd < 0){
            perror("Nie można otworzyć pliku");
            exit(1);
        }
		while(1){
			while (*wstrzymanie_ptr == 1){
                usleep(100000);
            }
			if(read(fd, hex, 2) == 2){
				hex[2] = '\0';
				printf("0x%s ", hex);
				licznik++;
				read(fd, &hex[0], 1);
				if(licznik == 15){
					printf("\n");
					licznik = 0;
				}
			} else if(opcja == 1 || opcja == 2){
				close(fd);
				exit(0);
			}
		}
	}
    while(petla == 1){
		usleep(100000);
	}
	Czyszczenie();
    return 0;
}
