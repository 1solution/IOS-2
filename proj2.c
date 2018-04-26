/* 
Michal Plsek, xplsek03
IOS proj2, 26.4.2018
proj2.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <signal.h>
#define XPMUTEX "/xpmutex"
#define XPBUS "/xpbus"
#define XPALLABOARD "/xpallaboard"
#define XPRIDEFINISHED "/xpridefinished"
#define W "/xpw"
	
	void clean(); // cleaning fction
	
	/* SEMAPHORES */
	sem_t *xpallaboard = NULL;
	sem_t *xpbus = NULL;
	sem_t *xpmutex = NULL;
	sem_t *xpridefinished = NULL;
	sem_t *w = NULL; // sem. for writing one line at time
	
	/* SHARED MEMORY variables */
	int *SET = NULL;
	int id_SET = 0;
	int *A = NULL;
	int id_A = 0;
	int *CR = NULL;
	int id_CR = 0;
	int *TOT = NULL;
	int id_TOT = 0;
	int *FIN = NULL;
	int id_FIN = 0;
	int *E = NULL;
	int id_E = 0;

int main(int argc, char **argv) {
	
	/* ARGS processing */
	if(argc != 5) 
		goto arg_error;	
	char *ptr;
	long ABT = strtol(argv[4],&ptr,10); // ABT processing
	long ART = strtol(argv[3],&ptr,10); // ART processing
	long C = strtol(argv[2],&ptr,10); // C processing
	long R = strtol(argv[1],&ptr,10); // R processing
	if(R<=0 || C<=0)
		goto arg_error;
	if(ABT < 0 || ABT > 1000 || ART < 0 || ART > 1000)
		goto arg_error;	

	/* FILE creating and buffer setting */
	FILE *fp = fopen("proj2.out","w+");
	if(fp == NULL) {
		fprintf(stderr,"Cannot create file.\n");
		return 1;
	}
	setbuf(fp, NULL);
	
	/* SHARED MEMORY allocation */
    if ((id_A = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
		goto shm_error;
    if ((A = shmat(id_A, NULL, 0)) == NULL)
		goto shm_error;
    if ((id_CR = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
		goto shm_error;       
    if ((CR = shmat(id_CR, NULL, 0)) == NULL)
		goto shm_error;
    if ((id_TOT = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
		goto shm_error;       
    if ((TOT = shmat(id_TOT, NULL, 0)) == NULL)
		goto shm_error;
    if ((id_FIN = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
		goto shm_error;       
    if ((FIN = shmat(id_FIN, NULL, 0)) == NULL)
		goto shm_error;
    if ((id_E = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
		goto shm_error;       
    if ((E = shmat(id_E, NULL, 0)) == NULL)
		goto shm_error;
    if ((id_SET = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666)) == -1)
		goto shm_error;       
    if ((SET = shmat(id_SET, NULL, 0)) == NULL)
		goto shm_error;
	
	/* SHARED MEMORY variables */
	*A = 1; // total operations counter
	*CR = 0; // rider son bus stop
	*TOT = 0; // how many got on bus in total
	*FIN = 0; // all riders were created flag
	*E = 0; // how many got in bus
	*SET = 0; // intern counter of one interval of riders who made it to bus stop in time
	
	if((xpmutex = sem_open(XPMUTEX, O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED)
		goto sem_error;
	if((xpbus = sem_open(XPBUS, O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
		goto sem_error;
	if((xpallaboard = sem_open(XPALLABOARD, O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
		goto sem_error;
	if((xpridefinished = sem_open(XPRIDEFINISHED, O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
		goto sem_error;	
	if((w = sem_open(W, O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED)
		goto sem_error;	
	
	/* PROCCESS GEN. starts */	
	pid_t xp_bus = fork();
	
	if(xp_bus == 0) { // CHILD: BUS	
	sem_wait(w); fprintf(fp,"%d\t\t: BUS\t\t: start\n",*A); sem_post(w);
	
	while(*TOT <= R && *FIN != 1) {	
			sem_wait(xpmutex);	
			sem_wait(w); (*A)++;fprintf(fp,"%d\t\t: BUS\t\t: arrival\n",*A); sem_post(w);
			
			if(*CR > 0) {				
					sem_wait(w); (*A)++;fprintf(fp,"%d\t\t: BUS\t\t: start boarding: %lu\n",*A,(*CR > C ? C : *CR)); sem_post(w);
					*E = 0; // null E counter for rider capacity counting loop
					sem_post(xpbus); // signal to riders to start loading - LOADING STARTS
					sem_wait(xpallaboard); // wait untill last rider signals that everybodys on board - LOADING FINISHES
					*TOT+=*E;
					sem_wait(w); (*A)++;fprintf(fp,"%d\t\t: BUS \t\t: end boarding: 0\n",*A); sem_post(w);
			}		
		sem_wait(w); (*A)++;fprintf(fp,"%d\t\t: BUS\t\t: depart\n",*A); sem_post(w);
		sem_post(xpmutex);
		
		// bus departing		
		usleep(rand()%(1000*ABT+1));
		sem_wait(w); (*A)++;fprintf(fp,"%d\t\t: BUS\t\t: end\n",*A);	sem_post(w);	
		// bus arriving
		
		for(int i = 0; i < *E; i++)	
			sem_post(xpridefinished); // signals that it is ending ride, needed *E times (*E riders are waiting)	
	}
		
	sem_wait(w); (*A)++;fprintf(fp,"%d\t\t: BUS\t\t: finish\n",*A); sem_post(w);		
	exit(0);
	}
	
	if(xp_bus > 0) { // PARENT PT 1	
		pid_t xp_genrider = fork();		
		if(xp_genrider > 0) { // PARENT PT 2	
			waitpid(xp_genrider, NULL, 0);
			waitpid(xp_bus, NULL, 0);
		}		
		else if(xp_genrider == 0) {  // CHILD: RIDERMANAGER	
		pid_t waitpid; // waiting
		pid_t xp_rider[R];
		
		for(unsigned int i = 1; i < R+1; i++) { // generating child RIDERS in loop
			usleep(rand()%(1000*ART+1));
			xp_rider[i] = fork();
			if(xp_rider[i] == 0) {
				sem_wait(w); (*A)++;fprintf(fp,"%d\t\t: RID %d\t\t: start\n",*A,i); sem_post(w);
				
				sem_wait(xpmutex);
				(*SET)++;(*A)++;
				sem_wait(w); (*CR)++;fprintf(fp,"%d\t\t: RID %d\t\t: enter: %d\n",*A,i,*SET); sem_post(w);
				sem_post(xpmutex);
				
				sem_wait(xpbus); // wait until bus is ready to start loading
				*SET = 0;
				// LOADING START
					(*E)++;
					sem_wait(w); (*A)++;fprintf(fp,"%d\t\t: RID %d\t\t: boarding\n",*A,i); sem_post(w);
					(*CR)--;
					if(*CR == 0 || *E == C) {
						sem_post(xpallaboard); // last rider signals that everybody is onboard
						// LOADING FINISHED	
					}
					else
						sem_post(xpbus);					
				
				sem_wait(xpridefinished);
				sem_wait(w); (*A)++;fprintf(fp,"%d\t\t: RID %d\t\t: finish\n",*A,i); sem_post(w);
				exit(0);				
			}
			if(xp_rider[i] < 0) { // one of proc xp_rider[i] failed
				kill(-1*getpid(),SIGTERM); // kill all riders - childs of xp_genrider	
			}
		}
		while ((waitpid = wait(0)) > 0); // waiting for all riders to end
		*FIN = 1; // all riders are finished
		exit(0);
		}		
		else { // fork xp_genrider failed
			kill(-1*getpid(),SIGTERM);	
			goto proc_error;
		}
	}
	if(xp_bus < 0) { // proc xp_bus failed
		goto proc_error;
	}
	
	clean();
	fclose(fp);
	return 0;
	
	shm_error: // shared memory error
	clean();
	fprintf(stderr,"Shared memory error.\n");
	return 1;	
	
	sem_error: // semaphore error
	clean();
	fprintf(stderr,"Error while creating semaphores.\n");
	return 1;	
		
	arg_error: // bad arguments
	clean();
	fprintf(stderr,"Bad arguments.\n");
	return 1;
	
	proc_error: // fork returned -1
	clean();
	fprintf(stderr,"Process failure.\n");
	return 1;
}

void clean() {
	sem_close(w);
	sem_close(xpridefinished);
	sem_close(xpmutex);
	sem_close(xpbus);
	sem_close(xpallaboard);
	sem_unlink(XPRIDEFINISHED);
	sem_unlink(XPMUTEX);
	sem_unlink(XPBUS);
	sem_unlink(XPALLABOARD);
	sem_unlink(W);
	shmctl(id_A, IPC_RMID, NULL);
	shmctl(id_CR, IPC_RMID, NULL);
	shmctl(id_TOT, IPC_RMID, NULL);
	shmctl(id_FIN, IPC_RMID, NULL);
	shmctl(id_E, IPC_RMID, NULL);
	shmctl(id_SET, IPC_RMID, NULL);
}
