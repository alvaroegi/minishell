//LIBRERIAS

#include <stdio.h>		//libreria estandar
#include <string.h>		//Libreria strings
#include <stdlib.h>		//Libreria estandar
#include <sys/types.h>		//Libreria estandar
#include <fcntl.h>		//Libreria ficheros
#include <errno.h>		//Libreria errno
#include <pthread.h>		//Libreria hilos y mutex
#include <unistd.h>		//libreria estandar


//CONSTANTES

#define SIZE 1024		//Tamaño por defecto de los buffers

//STRUCTS

struct paciente {
	int numPac;
	int centroP;
};

struct farmacia {
	int objetivo;
	int num;
};

//VARIABLES GLOBALES

FILE *fout;	//Descriptor de fichero que necesitamos para imprimir en fichero todos los mensajes

int hab;	// Variables en las que almacenamos las variables de entrada
int vacI;	//
int vacMin;	//
int vacMax;	//
int fabMin;	//
int fabMax;	//
int tReparto;	//
int tCita;	//
int tDesp;	// Hasta aqui.

int tanda;	//Variable en la que almacenamos porque tanda vamos

int vacunadosTanda;	//Habitantes vacunados por tanda

int vacunasCentro[5];	//Array donde almacenamos las vacunas que tiene cada centro
int demanda[5];		//Array donde almacenamos la gente que tenemos esperando

int terminado[3];	//Array donde almacenamos si una fabrica ha terminado de fabricar

int estadisticaFarma[15];	//Array donde almacenamos las vacunas que manda cada fabrica a cada centro: 1-[0,4] 2-[5,9] 3-[10,14]
int estadisticaPaci[5];		//Array donde almacenamos el numero de pacientes que se ha vacunado en cada centro

struct farmacia f1,f2,f3;	//Structs donde almacenamos el numero de la farmacia y el numero de vacunas que debe fabricar cada una


pthread_mutex_t v, mutexDema, mutexVacTanda, mutexEstaPaci, mutexOut;	//Mutexs para: Structs pacientes - Array Demanda - vacunadosTanda - Array Estadistica Pacientes - Puntero fichero de salida
pthread_cond_t hayVac[5];
pthread_mutex_t arrayMutex[5];


//FUNCIONES AUXILIARES

void *crearVacunas(void* arg);
void *vacunarse(void* arg);
void *repartirFinal();


int main(int argc, char *argv[]) {

	//VARIABLES
	
	char in[SIZE];		//buffer para almacenar el nombre del fichero de entrada
	char out[SIZE];		//buffer para almacenar el nombre del fichero de salida
	int dato;		//Variable para almacenar los datos leidos del fichero de entrada
	int i,j;		//indices fors
	int remo;		//variable auxiliar para imprimir las vacunas que removemos a posteriori
	
	pthread_t farm1, farm2, farm3;
	pthread_t vacunasFinal;
	pthread_t paciente;
	
	struct paciente p;
	
	//INICIALIZAMOS TODOS LOS MUTEXS
	pthread_mutex_init(&v,NULL);
	
	pthread_mutex_init(&arrayMutex[0],NULL);
	pthread_mutex_init(&arrayMutex[1],NULL);
	pthread_mutex_init(&arrayMutex[2],NULL);
	pthread_mutex_init(&arrayMutex[3],NULL);
	pthread_mutex_init(&arrayMutex[4],NULL);
	
	pthread_mutex_init(&mutexDema,NULL);
	pthread_mutex_init(&mutexVacTanda,NULL);
	pthread_mutex_init(&mutexEstaPaci,NULL);
	pthread_mutex_init(&mutexOut,NULL);
	
	//INICIALIZAMOS TODAS LAS CONDICONES
	pthread_cond_init(&hayVac[0],NULL);
	pthread_cond_init(&hayVac[1],NULL);
	pthread_cond_init(&hayVac[2],NULL);
	pthread_cond_init(&hayVac[3],NULL);
	pthread_cond_init(&hayVac[4],NULL);
	
	//INICIALIZAMOS EL RANDOM
	srand(time(NULL));
	
	FILE *fin;	//Descriptor de fichero para leer del fichero de entrada

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

	fin = fopen(in,"r");		//Abrimos fichero de entrada con permiso de lectura
	fout = fopen(out,"w");		//Abrimos fichero de salida con permiso de escritura
	
	if(fin == NULL) {
		printf("Error al abrir el archivo llamado %s\n",in);
		return 1;
	} else if (fout == NULL) {
		printf("Error al abrir el archivo llamado %s\n",out);
		return 2;
	} else {	//Leemos los datos de entrada
		printf("VACUNACION EN PANDEMIA: CONFIGURACION INICIAL\n");
		fprintf(fout,"VACUNACION EN PANDEMIA: CONFIGURACION INICIAL\n");
		for(i=0;i<9;i++) {
			fscanf(fin,"%d",&dato);
			if(i == 0) hab = dato;
			else if(i == 1) {
				for(j=0;j<5;j++) vacunasCentro[j]=dato;
				vacI = vacunasCentro[0];
			}
			else if(i == 2) vacMin = dato;
			else if(i == 3) vacMax = dato;
			else if(i == 4) fabMin = dato;
			else if(i == 5) fabMax = dato;
			else if(i == 6) tReparto = dato;
			else if(i == 7) tCita = dato;
			else if(i == 8) tDesp = dato;
		}
		printf("Habitantes: %d\n",hab);
		fprintf(fout,"Habitantes: %d\n",hab);
		printf("Centros de vacunacion: 5\n");
		fprintf(fout,"Centros de vacunacion: 5\n");
		printf("Fabricas: 3\n");
		fprintf(fout,"Fabricas: 3\n");
		printf("Vacunados por tanda: %d\n",(hab/10));
		fprintf(fout,"Vacunados por tanda: %d\n",(hab/10));
		printf("Vacunas iniciales en cada centro: %d\n",vacunasCentro[0]);
		fprintf(fout,"Vacunas iniciales en cada centro: %d\n",vacunasCentro[0]);
		printf("Vacunas totales en fabrica: %d\n",(hab/3));
		fprintf(fout,"Vacunas totales en fabrica: %d\n",(hab/3));
		printf("Minimo numero de vacunas fabricadas en cada tanda: %d\n",vacMin);
		fprintf(fout,"Minimo numero de vacunas fabricadas en cada tanda: %d\n",vacMin);
		printf("Maximo numero de vacunas fabricadas en cada tanda: %d\n",vacMax);
		fprintf(fout,"Maximo numero de vacunas fabricadas en cada tanda: %d\n",vacMax);
		printf("Tiempo minimo de fabricacion de una tanda de vacunas: %d\n",fabMin);
		fprintf(fout,"Tiempo minimo de fabricacion de una tanda de vacunas: %d\n",fabMin);
		printf("Tiempo maximo de fabricacion de una tanda de vacunas: %d\n",fabMax);
		fprintf(fout,"Tiempo maximo de fabricacion de una tanda de vacunas: %d\n",fabMax);
		printf("Tiempo maximo que un habitante tarda en ver que esta citado para vacunarse: %d\n",tCita);
		fprintf(fout,"Tiempo maximo que un habitante tarda en ver que esta citado para vacunarse: %d\n",tCita);
		printf("Tiempo maximo de desplazamiento del habitante al centro de vacunacion: %d\n",tDesp);
		fprintf(fout,"Tiempo maximo de desplazamiento del habitante al centro de vacunacion: %d\n",tDesp);
		printf("\n");
		fprintf(fout,"\n");
		printf("PROCESO DE VACUNACION\n");
		fprintf(fout,"PROCESO DE VACUNACION\n");
		fclose(fin);
	}
	
	//Inicializamos los structs de las farmacias: Numero y numero de vacunas objetivo
	f1.objetivo = hab/3 + hab%3;
	f1.num = 1;
	f2.objetivo = hab/3;
	f2.num = 2;
	f3.objetivo = hab/3;
	f3.num = 3;
	
	//Creamos los tres hilos de las farmacias/fabricas
	pthread_create(&farm1, NULL, crearVacunas,&f1);
	pthread_create(&farm2, NULL, crearVacunas,&f2);
	pthread_create(&farm3, NULL, crearVacunas,&f3);
	
	//inicializamoms todos los arrays a 0 por seguridad
	for(i = 0; i < 3; i++) terminado[i] = 0;
	
	for(i = 0; i < 15; i++) estadisticaFarma[i]=0;
	
	for(i = 0; i < 5; i++) estadisticaPaci[i] = 0;		
	
	pthread_create(&vacunasFinal,NULL,repartirFinal,NULL);
	
	tanda=0;
	
	//Creamos los hilos de todos los pacientes que van en cada tanda
	for(i = 0; i < 10; i++){
		for(j = 0; j < hab/10; j++){
			pthread_mutex_lock(&v);
			p.centroP = rand()%5+1;
			p.numPac = i*120 + j + 1;
			pthread_create(&paciente, NULL, vacunarse, &p);
		}
		
		while(vacunadosTanda<(hab/10));			//Esperamos a que todos los pacientes de la tanda se vacunen
		
		pthread_mutex_lock(&mutexVacTanda);		//Cambiamos de tanda y reseteamos
		tanda++;
		vacunadosTanda=0;
		pthread_mutex_unlock(&mutexVacTanda);
	}
	printf("-----VACUNACION TERMINADA-----\n");
	fprintf(fout,"-----VACUNACION TERMINADA-----\n");
	printf("\n");
	fprintf(fout,"\n");
	
	pthread_join(vacunasFinal,NULL);			//Join para ejecutar el reparto de vacunas final
	
	
	//Imprimimos las estadisticas de la vacunacion
	printf("-----ESTADISTICAS FINALES-----\n");
	fprintf(fout,"-----ESTADISTICAS FINALES-----\n");
	printf("\n");
	fprintf(fout,"\n");
	
	for(i = 0; i < 3; i++) {
		printf("FABRICA %d\n",(i+1));
		fprintf(fout,"FABRICA %d\n",(i+1));
		printf("\t Vacunas Fabricadas: %d\n",(hab/3));
		fprintf(fout,"\t Vacunas Fabricadas: %d\n",(hab/3));
		for(j = 0; j < 5; j++) {
			printf("\t Centro %d: %d vacunas\n",(j+1),estadisticaFarma[j+(5*i)]);
			fprintf(fout,"\t Centro %d: %d vacunas\n",(j+1),estadisticaFarma[j+(5*i)]);
		}
		printf("\n");
		fprintf(fout,"\n");
	}
	
	for(i = 0; i < 5; i++) {
		printf("CENTRO %d\n",(i+1));
		fprintf(fout,"CENTRO %d\n",(i+1));
		printf("\t Vacunas recibidas: %d\n",estadisticaFarma[i]+ estadisticaFarma[i+5]+ estadisticaFarma[i+10]);
		fprintf(fout,"\t Vacunas recibidas: %d\n",estadisticaFarma[i]+ estadisticaFarma[i+5]+ estadisticaFarma[i+10]);
		printf("\t Habitantes vacunados: %d\n",estadisticaPaci[i]);
		fprintf(fout,"\t Habitantes vacunados: %d\n",estadisticaPaci[i]);
		if((estadisticaFarma[i]+ estadisticaFarma[i+5]+ estadisticaFarma[i+10]+vacI)-estadisticaPaci[i] < 0) {
			printf("\t Vacunas restantes: 0\n");
			fprintf(fout, "\t Vacunas restantes: 0\n");
		} else {
			printf("\t Vacunas restantes: %d\n",(estadisticaFarma[i]+ estadisticaFarma[i+5]+ estadisticaFarma[i+10]+vacI)-estadisticaPaci[i]);
			fprintf(fout,"\t Vacunas restantes: %d\n",(estadisticaFarma[i]+ estadisticaFarma[i+5]+ estadisticaFarma[i+10]+vacI)-estadisticaPaci[i]);
		}
		printf("\n");
		fprintf(fout,"\n");
	}
	
	printf("RE-UBICACION DE VACUNAS\n");
	fprintf(fout,"RE-UBICACION DE VACUNAS\n");
	for(i = 0; i < 5; i++) {
		remo = (estadisticaFarma[i]+ estadisticaFarma[i+5]+ estadisticaFarma[i+10]+vacI)-estadisticaPaci[i];
		if(remo < 0) remo = vacI -remo;
		else if(remo >= 0 && remo <= vacI) remo = vacI - remo;
		else remo = vacI - remo;
		printf("\t Centro %d: %d\n",(i+1),remo);
		fprintf(fout,"\t Centro %d: %d\n",(i+1),remo);
	}
	
	fclose(fout);
	return 0;
}




void *crearVacunas(void* arg){
	
	//VARIABLES LOCALES
	
	struct farmacia farma = *(struct farmacia *)arg;
	int numVacCreadas;
	int aumentado[5];
	int mostrar[5];
	int numVac;
	int tiempoFab;
	int tiempoRep;
	int i;
	int vacuRepaIgua;
	
	numVacCreadas = 0;
	
	while(numVacCreadas < farma.objetivo){
		numVac = rand() % (vacMax-vacMin+1) + vacMin; //numero aleatorio de vacunas entre el vacMin y el vacMax
		if(numVac + numVacCreadas > farma.objetivo) numVac = farma.objetivo - numVacCreadas;
		tiempoFab = rand() % (fabMax-fabMin+1) + fabMin; //numero aleatorio de tiempo de fabricacion de vacunas (entre fabMin y fabMax)
		sleep(tiempoFab);
		
		pthread_mutex_lock(&mutexOut);
		printf("Fabrica %d prepara %d vacunas\n",farma.num,numVac);
		fprintf(fout,"Fabrica %d prepara %d vacunas\n",farma.num,numVac);
		pthread_mutex_unlock(&mutexOut);
		
		numVacCreadas = numVacCreadas + numVac;
		
		tiempoRep = rand() % (tReparto) + 1; //numero aleatrio de tiempo de reparto (entre 1 y tReparto)
		sleep(tiempoRep);


		for(i = 0; i < 5; i++) {	//Inicializamos a cero por seguridad y por necesidad
			mostrar[i]=0;
			aumentado[i]=0;
		}
		
		for(i = 0; i < 5; i++) {	//Repartimos vacunas a los centros que necesiten, empezando por el primero
			if(demanda[i]>0) {
				if(demanda[i]<=numVac) {
					pthread_mutex_lock(&mutexDema);
					aumentado[i]= demanda[i];
					numVac = numVac-demanda[i];
					demanda[i]=0;
					pthread_mutex_unlock(&mutexDema);
					mostrar[i] = aumentado[i];
					pthread_mutex_lock(&arrayMutex[i]);
					vacunasCentro[i] = vacunasCentro[i] + aumentado[i];
					estadisticaFarma[((farma.num-1)*5)+i]=estadisticaFarma[((farma.num-1)*5)+i] + aumentado[i];
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
					estadisticaFarma[((farma.num-1)*5)+i]=estadisticaFarma[((farma.num-1)*5)+i] + aumentado[i];
					aumentado[i]=0;
					pthread_mutex_unlock(&arrayMutex[i]);
					
				}
			}
		}
		
		vacuRepaIgua = numVac/5;
		
		for(i = 0; i < 5; i++) {	//si ningun centro necesita vacunas o nos han sobrado, repartimos equitativamente
			aumentado[i] =  vacuRepaIgua;
			mostrar[i] = mostrar[i] + aumentado[i];
			if(i==0) { 
				if (vacuRepaIgua != 0 || (numVac%5 != 0)) {
					aumentado[i] = aumentado[i] + numVac%5;
					mostrar[i] = mostrar[i] + aumentado[i];
					pthread_mutex_lock(&arrayMutex[i]);
					vacunasCentro[i] = vacunasCentro[i] + aumentado[i];
					estadisticaFarma[((farma.num-1)*5)+i]=estadisticaFarma[((farma.num-1)*5)+i] + aumentado[i];
					pthread_mutex_unlock(&arrayMutex[i]);
				}
			}
			else if (vacuRepaIgua != 0 && i!=0) {
				pthread_mutex_lock(&arrayMutex[i]);
				vacunasCentro[i] = vacunasCentro[i] + aumentado[i];
				estadisticaFarma[((farma.num-1)*5)+i]=estadisticaFarma[((farma.num-1)*5)+i] + aumentado[i];
				pthread_mutex_unlock(&arrayMutex[i]);
			}
			pthread_mutex_lock(&mutexOut);
			printf("Fabrica %d entrega %d vacunas en el centro %d\n",farma.num,mostrar[i],i+1);
			fprintf(fout,"Fabrica %d entrega %d vacunas en el centro %d\n",farma.num,mostrar[i],i+1);
			pthread_mutex_unlock(&mutexOut);
			pthread_cond_broadcast(&hayVac[i]);
		}
		
	}
	pthread_mutex_lock(&mutexOut);
	printf("Fabrica %d ha fabricado todas sus vacunas\n",farma.num);
	fprintf(fout,"Fabrica %d ha fabricado todas sus vacunas\n",farma.num);
	pthread_mutex_unlock(&mutexOut);
	terminado[farma.num-1] = 1;
	pthread_exit(0);
	
}

void *vacunarse(void* arg){
	
	//VARIABLES LOCALES
	int dentro;
	struct paciente paci;
	int k;
	
	//INICIALIZACION DE VARIABLES
	paci = *(struct paciente *) arg;
	pthread_mutex_unlock(&v);
	dentro=0;
	k = paci.centroP-1;
	
	sleep(rand()%tCita+1);
	
	pthread_mutex_lock(&mutexOut);
	printf("Habitante %d elige el centro %d para vacunarse\n",paci.numPac, paci.centroP);
	fprintf(fout,"Habitante %d elige el centro %d para vacunarse\n",paci.numPac, paci.centroP);
	pthread_mutex_unlock(&mutexOut);
	
	sleep(rand()%tDesp+1);
	
	pthread_mutex_lock(&arrayMutex[k]);
	while(vacunasCentro[k]==0) {			//condicion del mutex, si no hay vacunas, esperar
		pthread_mutex_lock(&mutexDema);
		if(dentro==0) {
			demanda[k]++;
			dentro=1;
		}
		pthread_mutex_unlock(&mutexDema);
		pthread_cond_wait(&hayVac[k],&arrayMutex[k]);
	}
	vacunasCentro[k]--;
	
	pthread_mutex_lock(&mutexEstaPaci);
	estadisticaPaci[k]++;
	pthread_mutex_unlock(&mutexEstaPaci);
	
	pthread_mutex_unlock(&arrayMutex[k]);
	
	pthread_mutex_lock(&mutexVacTanda);
	vacunadosTanda++;
	pthread_mutex_unlock(&mutexVacTanda);
	
	pthread_mutex_lock(&mutexOut);
	printf("Habitante %d vacunado en el centro %d\n",paci.numPac, paci.centroP);
	fprintf(fout,"Habitante %d vacunado en el centro %d\n",paci.numPac, paci.centroP);
	pthread_mutex_unlock(&mutexOut);
	
	pthread_exit(0);

}

void* repartirFinal(){

	//VARIABLES
	int i, h;	//indices bucles for
	int tiempoRep;	//tiempo de reparto otra vez, ya que volvemos a juntar las vacunas que no sobran

	while((terminado[0] == 0) || (terminado[1] == 0) || (terminado[2] == 0));	//Esperamos a que las 3 farmacias terminen de fabricar
	
	while((tanda*(hab/10) + vacunadosTanda) < hab) {	//Mientras no se vacune todo el mundo seguimos comprobando
		tiempoRep = rand() % (tReparto) + 1; 		//numero aleatrio de tiempo de reparto (entre 1 y tReparto)
		sleep(tiempoRep);
		
		for(h = 0; h < 5; h++){
			pthread_mutex_lock(&mutexDema);
			pthread_mutex_lock(&arrayMutex[h]);
			vacunasCentro[h] = demanda[h];
			pthread_mutex_unlock(&arrayMutex[h]);
			if(demanda[h] != 0){
				pthread_cond_broadcast(&hayVac[h]);	//Mandamos señal si nos ha llegado un habitante
			}
			demanda[h] = 0;
			pthread_mutex_unlock(&mutexDema);
		}
	}
	pthread_exit(0);
}
