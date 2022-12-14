#include <stdio.h>
#include <string.h>
#include <stdlib.h>		//Libreria estandar
#include <sys/types.h>		//Libreria estandar
#include <fcntl.h>		//Libreria ficheros
#include <errno.h>		//Libreria errno
#include <pthread.h>
#include <unistd.h>


#define SIZE 1024

struct paciente {
	int numPac;
	int centroP;
};

int hab;
int vacI;
int vacMin;
int vacMax;
int fabMin;
int fabMax;
int tReparto;
int tCita;
int tDesp;
int dato;

int target1, target2, target3; //numero de vacunas que van a fabricar las farmacias

int vacunados;
pthread_mutex_t v;

void *crearVacunas1(void* arg);
void *crearVacunas2(void* arg);
void *crearVacunas3(void* arg);
void *vacunarse(void* arg);

int main(int argc, char *argv[]) {

	char in[SIZE];
	char out[SIZE];
	int i;
	pthread_mutex_init(&v,NULL);
	
	
	srand(time(NULL));
	
	FILE *f;

	if(argc == 1) { //No nos meten ni fichero entrada ni salida
		strcpy(in,"entrada_vacunacion.txt");
		strcpy(out,"salida_vacunacion.txt");
	} else if(argc == 2) { //Nos meten fichero entrada
		strcpy(in,argv[1]);
		strcpy(out,"salida_vacunacion.txt");
	} else { //Nos meten fichero entrada y salida
		strcpy(in,argv[1]);
		strcpy(out,argv[2]);
	}

	f = fopen(in,"r");
	if(f == NULL) {
		 printf("Error al abrir el archivo llamado %s\n",in);
		return 1;
	}
	else {
		int dato;
		for(i=0;i<9;i++) {
			fscanf(f,"%d",&dato);
			if(i == 0) hab = dato;
			else if(i == 1) vacI = dato;
			else if(i == 2) vacMin = dato;
			else if(i == 3) vacMax = dato;
			else if(i == 4) fabMin = dato;
			else if(i == 5) fabMax = dato;
			else if(i == 6) tReparto = dato;
			else if(i == 7) tCita = dato;
			else if(i == 8) tDesp = dato;
			//printf("%d\n",dato);
		}
		fclose(f);
	}
	
	target1 = hab/3 + hab%3;
	target2 = hab/3;
	target3 = hab/3;
	
	pthread_t farm1, farm2, farm3;
	pthread_create(&farm1, NULL, crearVacunas1, NULL);
	pthread_create(&farm2, NULL, crearVacunas2, NULL);
	pthread_create(&farm3, NULL, crearVacunas3, NULL);
	
	
	for(int j = 0; j < 10 && vacunados != hab/10; j++){
		vacunados = 0;
		for(int h = 0; h < hab/10; h++){
			pthread_mutex_lock(&v);
			struct paciente p;
			pthread_t paciente;
			p.centroP = rand()%5+1;
			p.numPac = j*120 + h + 1;
			
			pthread_create(&paciente, NULL, vacunarse, &p);
		}
		
	
	}
	
	
	
	while(1);
	
	return 0;
}



void *crearVacunas1(void* arg){
	int numVacCreadas = 0;
	while(numVacCreadas < 400){
		int numVac = rand() % (vacMax-vacMin+1) + vacMin; //numero aleatorio de vacunas entre el vacMin y el vacMax
		if(numVac + numVacCreadas > target1) numVac = target1 - numVacCreadas;
		int tiempoFab = rand() % (fabMax-fabMin+1) + fabMin; //numero aleatorio de tiempo de fabricacion de vacunas (entre fabMin y fabMax)
		sleep(tiempoFab);
		
		printf("Fabrica 1 prepara %d vacunas\n",numVac);
		numVacCreadas = numVacCreadas + numVac;
		
		int tiempoRep = rand() % (tReparto) + 1; //numero aleatrio de tiempo de reparto (entre 1 y tReparto)
		sleep(tiempoRep);
		
		
		//Gestionar entregas
		//Añadir print con el numero de vacunas enviadas al centro x
	}
	
	pthread_exit(0);
	
}

void *crearVacunas2(void* arg){
	int numVacCreadas = 0;
	while(numVacCreadas < 400){
		int numVac = rand() % (vacMax-vacMin+1) + vacMin; //numero aleatorio de vacunas entre el vacMin y el vacMax
		if(numVac + numVacCreadas > target2) numVac = target2 - numVacCreadas;
		int tiempoFab = rand() % (fabMax-fabMin+1) + fabMin; //numero aleatorio de tiempo de fabricacion de vacunas (entre fabMin y fabMax)
		sleep(tiempoFab);
		
		printf("Fabrica 2 prepara %d vacunas\n",numVac);
		numVacCreadas = numVacCreadas + numVac;
		
		int tiempoRep = rand() % (tReparto) + 1; //numero aleatrio de tiempo de reparto (entre 1 y tReparto)
		sleep(tiempoRep);
		
		printf("numVacunas farmacia 2= %d\n",numVacCreadas);
		
		//Gestionar entregas
		//Añadir print con el numero de vacunas enviadas al centro x
	}
	
	pthread_exit(0);	
	
}

void *crearVacunas3(void* arg){
	int numVacCreadas = 0;
	while(numVacCreadas < 400){
		int numVac = rand() % (vacMax-vacMin+1) + vacMin; //numero aleatorio de vacunas entre el vacMin y el vacMax
		if(numVac + numVacCreadas > target3) numVac = target3 - numVacCreadas;
		int tiempoFab = rand() % (fabMax-fabMin+1) + fabMin; //numero aleatorio de tiempo de fabricacion de vacunas (entre fabMin y fabMax)
		sleep(tiempoFab);
		
		printf("Fabrica 3 prepara %d vacunas\n",numVac);
		numVacCreadas = numVacCreadas + numVac;
		
		int tiempoRep = rand() % (tReparto) + 1; //numero aleatrio de tiempo de reparto (entre 1 y tReparto)
		sleep(tiempoRep);
		
		printf("numVacunas farmacia 3= %d\n",numVacCreadas);
		
		//Gestionar entregas
		//Añadir print con el numero de vacunas enviadas al centro x
	}
	pthread_exit(0);
}

void *vacunarse(void* arg){
	struct paciente paci = *(struct paciente *) arg;
	pthread_mutex_unlock(&v);
	printf("Paciente = %d se quiere vacunar en centro %d\n",paci.numPac, paci.centroP);
	
	sleep(5);
	
	if(paci.centroP == 1){
		printf("Paciente %d está en el centro 1\n",paci.numPac);
	} else if(paci.centroP == 2){
		printf("Paciente %d está en el centro 2\n",paci.numPac);
	} else if(paci.centroP == 3){
		printf("Paciente %d está en el centro 3\n",paci.numPac);
	} else if(paci.centroP == 4){
		printf("Paciente %d está en el centro 4\n",paci.numPac);
	} else if(paci.centroP == 5){
		printf("Paciente %d está en el centro 5\n",paci.numPac);
	}

	
	pthread_exit(0);

}


