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

struct farmacia {
	int objetivo;
	int num;
};

int hab;
//int vacI;
int vacMin;
int vacMax;
int fabMin;
int fabMax;
int tReparto;
int tCita;
int tDesp;
int dato;

int vacunadosTanda;

int vacunasCentro[5];
int demanda[5];
int terminado[3];

struct farmacia f1,f2,f3;

int vacunados;
pthread_mutex_t v, mutexDema, mutexVacTanda; //mutexVac
pthread_cond_t hayVac[5];
pthread_mutex_t arrayMutex[5];



void *crearVacunas(void* arg);
void *vacunarse(void* arg);
void *repartirFinal();

int main(int argc, char *argv[]) {

	char in[SIZE];
	char out[SIZE];
	int i;
	pthread_mutex_init(&v,NULL);
	//pthread_mutex_init(&mutexVac,NULL);
	pthread_mutex_init(&arrayMutex[0],NULL);
	pthread_mutex_init(&arrayMutex[1],NULL);
	pthread_mutex_init(&arrayMutex[2],NULL);
	pthread_mutex_init(&arrayMutex[3],NULL);
	pthread_mutex_init(&arrayMutex[4],NULL);
	
	pthread_mutex_init(&mutexDema,NULL);
	pthread_mutex_init(&mutexVacTanda,NULL);
	
	pthread_cond_init(&hayVac[0],NULL);
	pthread_cond_init(&hayVac[1],NULL);
	pthread_cond_init(&hayVac[2],NULL);
	pthread_cond_init(&hayVac[3],NULL);
	pthread_cond_init(&hayVac[4],NULL);
	
	
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
		printf("VACUNACION EN PANDEMIA: CONFIGURACION INICIAL\n");
		int dato;
		for(i=0;i<9;i++) {
			fscanf(f,"%d",&dato);
			if(i == 0) hab = dato;
			else if(i == 1) {
				for(int j=0;j<5;j++) vacunasCentro[j]=dato;
			}
			else if(i == 2) vacMin = dato;
			else if(i == 3) vacMax = dato;
			else if(i == 4) fabMin = dato;
			else if(i == 5) fabMax = dato;
			else if(i == 6) tReparto = dato;
			else if(i == 7) tCita = dato;
			else if(i == 8) tDesp = dato;
			//printf("%d\n",dato);
		}
		printf("Habitantes: %d\n",hab);
		printf("Centros de vacunacion: 5\n");
		printf("Fabricas: 3\n");
		printf("Vacunados por tanda: %d\n",(hab/10));
		printf("Vacunas iniciales en cada centro: %d\n",vacunasCentro[0]);
		printf("Vacunas totales en fabrica: %d\n",(hab/3));
		printf("Minimo numero de vacunas fabricadas en cada tanda: %d\n",vacMin);
		printf("Maximo numero de vacunas fabricadas en cada tanda: %d\n",vacMax);
		printf("Tiempo minimo de fabricacion de una tanda de vacunas: %d\n",fabMin);
		printf("Tiempo maximo de fabricacion de una tanda de vacunas: %d\n",fabMax);
		printf("Tiempo maximo que un habitante tarda en ver que esta citado para vacunarse: %d\n",tCita);
		printf("Tiempo maximo de desplazamiento del habitante al centro de vacunacion: %d\n",tDesp);
		printf("\n");
		printf("PROCESO DE VACUNACION\n");
		fclose(f);
	}
	
	f1.objetivo = hab/3 + hab%3;
	f1.num = 1;
	f2.objetivo = hab/3;
	f2.num = 2;
	f3.objetivo = hab/3;
	f3.num = 3;
	
	pthread_t farm1, farm2, farm3;
	pthread_create(&farm1, NULL, crearVacunas,&f1);
	pthread_create(&farm2, NULL, crearVacunas,&f2);
	pthread_create(&farm3, NULL, crearVacunas,&f3);
	
	for(int m = 0; m<3; m++) terminado[m] = 0;
	
	pthread_t vacunasFinal;
	pthread_create(&vacunasFinal,NULL,repartirFinal,NULL);
	
	
	for(int j = 0; j < 10; j++){
		vacunados = 0;
		for(int h = 0; h < hab/10; h++){
			pthread_mutex_lock(&v);
			struct paciente p;
			pthread_t paciente;
			p.centroP = rand()%5+1;
			p.numPac = j*120 + h + 1;
			
			pthread_create(&paciente, NULL, vacunarse, &p);
		}
		while(vacunadosTanda<(hab/10));
		pthread_mutex_lock(&mutexVacTanda);
		vacunadosTanda=0;
		pthread_mutex_unlock(&mutexVacTanda);
	}
	printf("VACUNACION TERMINADA\n");
	exit(0);
	
	
	while(1);
}


//EL MINIMO NO NOS FUNCIONA LA PUTA MADRE QUE NOS PARIO


void *crearVacunas(void* arg){
	struct farmacia farma = *(struct farmacia *)arg;
	int numVacCreadas = 0;
	int aumentado[5];
	int mostrar[5];
	
	while(numVacCreadas < farma.objetivo){
		int numVac = rand() % (vacMax-vacMin+1) + vacMin; //numero aleatorio de vacunas entre el vacMin y el vacMax
		if(numVac + numVacCreadas > farma.objetivo) numVac = farma.objetivo - numVacCreadas;
		int tiempoFab = rand() % (fabMax-fabMin+1) + fabMin; //numero aleatorio de tiempo de fabricacion de vacunas (entre fabMin y fabMax)
		sleep(tiempoFab);
		
		printf("Fabrica %d prepara %d vacunas\n",farma.num,numVac);
		numVacCreadas = numVacCreadas + numVac;
		
		int tiempoRep = rand() % (tReparto) + 1; //numero aleatrio de tiempo de reparto (entre 1 y tReparto)
		sleep(tiempoRep);


		for(int i=0;i<5;i++) {
			mostrar[i]=0;
			aumentado[i]=0;
		}
		
		for(int i=0;i<5;i++) {
			if(demanda[i]>0) {
				if(demanda[i]<=numVac) {
					pthread_mutex_lock(&mutexDema);
					aumentado[i]= demanda[i];
					numVac = numVac-demanda[i];
					if(numVacCreadas!=farma.objetivo) demanda[i]=0;
					pthread_mutex_unlock(&mutexDema);
					mostrar[i] = aumentado[i];
					pthread_mutex_lock(&arrayMutex[i]);
					vacunasCentro[i] = vacunasCentro[i] + aumentado[i];
					aumentado[i]=0;
					pthread_mutex_unlock(&arrayMutex[i]);
				} else {
					aumentado[i] = numVac;
					pthread_mutex_lock(&mutexDema);
					demanda[i]=demanda[i] - numVac;
					numVac=0;
					pthread_mutex_unlock(&mutexDema);
					mostrar[i] = aumentado[i];
					pthread_mutex_lock(&arrayMutex[i]);
					vacunasCentro[i] = vacunasCentro[i] + aumentado[i];
					aumentado[i]=0;
					pthread_mutex_unlock(&arrayMutex[i]);
					
				}
				//pthread_cond_broadcast(&hayVac[i]);
			}
		}
		
		int vacuRepaIgua = numVac/5;
		for(int i=0;i<5;i++) {
			aumentado[i] =  vacuRepaIgua;
			mostrar[i] = mostrar[i] + aumentado[i];
			if(i==0) { 
				if (vacuRepaIgua != 0 || (numVac%5 != 0)) {
					aumentado[i] = aumentado[i] + numVac%5;
					mostrar[i] = mostrar[i] + aumentado[i];
					pthread_mutex_lock(&arrayMutex[i]);
					vacunasCentro[i] = vacunasCentro[i] + aumentado[i];
					pthread_mutex_unlock(&arrayMutex[i]);
				}
			}
			else if (vacuRepaIgua != 0 && i!=0) {
				pthread_mutex_lock(&arrayMutex[i]);
				vacunasCentro[i] = vacunasCentro[i] + aumentado[i];
				pthread_mutex_unlock(&arrayMutex[i]);
			}
			printf("Fabrica %d entrega %d vacunas en el centro %d\n",farma.num,mostrar[i],i+1);
			pthread_cond_broadcast(&hayVac[i]);
		}
		printf("NumVacCreadas: %d\n",numVacCreadas);
		//REPARTIR EQUITATIVAMENTE
		
	}
	printf("Fabrica %d ha fabricado todas sus vacunas\n",farma.num);
	terminado[farma.num-1] = 1;
	pthread_mutex_lock(&mutexDema);
	for(int j = 0; j<5; j++) printf("20Demanda centro fin fabrica %d: %d\n",j+1,demanda[j]);
	pthread_mutex_unlock(&mutexDema);
	printf("vacunadosTanda: %d\n",vacunadosTanda);
	for(int p=0;p<5;p++) printf("Vacunas en centro %d son %d\n",(p+1),vacunasCentro[p]);
	pthread_exit(0);
	
}

void *vacunarse(void* arg){
	int dentro=0;
	struct paciente paci = *(struct paciente *) arg;
	pthread_mutex_unlock(&v);
	sleep(rand()%tCita+1);
	
	int k = paci.centroP-1;
	printf("Habitante %d elige el centro %d para vacunarse\n",paci.numPac, paci.centroP);
	sleep(rand()%tDesp+1);
	pthread_mutex_lock(&arrayMutex[k]);
	while(vacunasCentro[k]==0) {
		pthread_mutex_lock(&mutexDema);
		if(dentro==0) {
			demanda[k]++;
			dentro=1;
		}
		printf("demanda[%d]: %d\n",k+1,demanda[k]);
		pthread_mutex_unlock(&mutexDema);
		pthread_cond_wait(&hayVac[k],&arrayMutex[k]);
	}
	vacunasCentro[k]--;
	pthread_mutex_unlock(&arrayMutex[k]);
	pthread_mutex_lock(&mutexVacTanda);
	vacunadosTanda++;
	printf("vacunadosTanda: %d\n",vacunadosTanda);
	pthread_mutex_unlock(&mutexVacTanda);
	printf("Habitante %d vacunado en el centro %d\n",paci.numPac, paci.centroP);
	
	pthread_exit(0);

}

void* repartirFinal(){
	while((terminado[0] == 0) || (terminado[1] == 0) || (terminado[2] == 0));
	printf("Inicio repartir final\n");
	int vacunasRestantes = 0;
	for(int i = 0; i < 5; i++){
		vacunasRestantes = vacunasRestantes + vacunasCentro[i];
	}
	printf("Vacunas Restantes %d\n",vacunasRestantes);
	for(int j = 0; j<5; j++) printf("Demanda centro %d: %d\n",j+1,demanda[j]);
	
	for(int h = 0; h < 5; h++){
		pthread_mutex_lock(&mutexDema);
		vacunasRestantes = vacunasRestantes - demanda[h];
		if(vacunasRestantes > 0){
			pthread_mutex_lock(&arrayMutex[h]);
			vacunasCentro[h] = demanda[h];
			pthread_mutex_unlock(&arrayMutex[h]);
			demanda[h] = 0;
		}
		pthread_mutex_unlock(&mutexDema);
	}
	
	pthread_exit(0);


}
