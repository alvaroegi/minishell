#include <stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "parser.h"

struct hijoPosiJobs{
	int posi;
	int pidB;
	char man[1024];
	int bloque;
};


pid_t pid;
int p_par[2];
int p_impar[2];
int status;
int a,b,c; //contador arrays hijos y jobs
int* childB;
int* estado;
int* estado2;
int* estado3;
int auxEstado;

struct hijoPosiJobs arjobs[50];


int cd(tcommand * com);
int umaskC(tcommand * com);
void fg(int* hijoB, tcommand * com);
void jobs(tline * li);
char* copiarString(char* in);



int
main(void) {
	char buf[1024];
	tline * line;
	int i;
	auxEstado = 0;
	
	
	int* child;
	
	b = 0;
	childB = (int*)malloc(sizeof(int));
	estado = (int*)malloc(sizeof(int));
	estado2 = (int*)malloc(sizeof(int));
	estado3 = (int*)malloc(sizeof(int));
			
	c = 0;
	
	printf("msh> ");
	while (fgets(buf, 1024, stdin)) {
		//reiniciamos los valores que recorre el array para que se realoje bien la memoria con el malloc
		a = 0;
		child = (int*)malloc(sizeof(int));
		pipe(p_par);
		pipe(p_impar);
		line = tokenize(buf);
		
		if (line->ncommands > 0){ 			
			if(strcmp(line->commands[0].argv[0], "cd") == 0) cd(line->commands); 	
			else if(strcmp(line->commands[0].argv[0], "exit") == 0){
				free(childB);
				free(estado);
				exit(0);
			}
			else if(strcmp(line->commands[0].argv[0], "umask") == 0) umaskC(line->commands);
			else if(strcmp(line->commands[0].argv[0], "fg") == 0) fg(childB,line->commands);
			else if(strcmp(line->commands[0].argv[0], "jobs") == 0) jobs(line);
			else if(line->commands[0].filename == NULL) printf("%s: no se encuentra el mandato\n",line->commands[0].argv[0]);
			else {		 					
				if (line->redirect_input != NULL) {
					//printf("redirección de entrada: %s\n", line->redirect_input);
				}
				if (line->redirect_output != NULL) {
					//printf("redirección de salida: %s\n", line->redirect_output);
					mode_t mode = 000;
					int s = creat(line->redirect_output, mode);
					if(s < 0){
						printf("Error en la creación del archivo");
					}
				}
				if (line->redirect_error != NULL) {
					//printf("redirección de error: %s\n", line->redirect_error);
					mode_t mode = 000;
					int e = creat(line->redirect_error, mode);
					if(e < 0){
						printf("Error en la creacion del archivo");
					}
				}
				if (line->background) {
					//printf("comando a ejecutarse en background\n");
					buf[strlen(buf) -1] = '\0';
					strcpy(arjobs[c].man,buf);
					arjobs[c].posi = c;
					arjobs[c].bloque = line -> ncommands;
					c++;
				}
				for (i=0; i<line->ncommands; i++) {
					//printf("orden %d (%s):\n", i, line->commands[i].filename);
					/*for (j=0; j<line->commands[i].argc; j++) {
						printf("  argumento %d: %s\n", j, line->commands[i].argv[j]);
					}*/
					pid = fork();
					
					
					if(pid < 0) { //ERROR
						fprintf(stderr, "Fallo el fork().\n%s\n", strerror(errno));
						exit(1);
					}else if(pid == 0){ //HIJO
						if (i == 0){ //Primer hijo
							close(p_impar[0]);
							close(p_impar[1]);
							close(p_par[0]);
							if(line -> ncommands != 1) {
								dup2(p_par[1],1);
							}
							close(p_par[1]);
						} else { //Resto de hijos
							if(i%2 == 0) { //Hijos pares
								close(p_impar[1]);
								close(p_par[0]);
								dup2(p_impar[0],0);
								close(p_impar[0]);
								if(i != line->ncommands - 1){ //No estoy en el último hijo
									dup2(p_par[1],1);
								}
								close(p_par[1]);
							
							} else { //Hijos impares
								close(p_impar[0]);
								close(p_par[1]);
								dup2(p_par[0],0);
								close(p_par[0]);
								if(i != line->ncommands -1){ //No estoy en el último hijo
									dup2(p_impar[1],1);
								}
								close(p_impar[1]);
							
							}
						}

						mode_t mode = O_RDWR;
						if (line->redirect_output != NULL && i == line->ncommands -1){
							int fd = open(line->redirect_output, mode);
							if(fd < 0) {
								printf("Error al abrir el archivo llamado %s\n",line->redirect_output);
							} else{
								dup2(fd,1);
							}
						}
						if (line->redirect_input != NULL && i == 0){
							int fd = open(line->redirect_input, mode);
							if(fd < 0) {
								printf("Error al abrir el archivo llamado %s\n",line->redirect_input);
							} else {
								dup2(fd,0);
							}
						}
						if (line->redirect_error != NULL && i == line->ncommands -1){
							int fd = open(line->redirect_input, mode);
							if(fd < 0){
								printf("Error al abrir el archivo llamado %s\n",line->redirect_error);
							} else {
								dup2(fd,2);
							}
						}
						execvp(line->commands[i].filename, line->commands[i].argv);
						fprintf(stderr, "Se ha producido un error\n");
						exit(1);
					
					} else { //Padre
						if (line->background){	
							if(i == line->ncommands - 1){
								printf("[%d]: %d\n",c,pid);
							}						
							childB[b] = pid;
							arjobs[c].pidB = pid;
							//printf("Hijo en background %d: %d\n",b,childB[b]);
							b++;	
							childB = realloc(childB, (b+1)*sizeof(int));		
							estado = realloc(estado, (b+1)*sizeof(int));
							estado2 = realloc(estado2, (b+1)*sizeof(int));
							estado3 = realloc(estado2, (b+1)*sizeof(int));
							
						} else {
							child[a] = pid;
							a++;
							child = realloc(child, (a+1)*sizeof(int));					
						}
						
						if(i % 2 == 0) { //Pares (Reseteamos las pipes)
							close(p_impar[0]);
							close(p_impar[1]);
							pipe(p_impar);
						} else { //Impares, cambiamos las que no vamos a usar
							close(p_par[0]);
							close(p_par[1]);
							pipe(p_par);
						}		
					}
				}
				close(p_par[0]);
				close(p_par[1]);
				close(p_impar[0]);
				close(p_impar[1]);
				
				//HIJOS EN BACKGROUND
				int p;
				
				for(p = 0; p<b; p++){
					waitpid(childB[p],&status,WNOHANG);			
				} 
				
				//HIJOS EN FOREGROUND
				int o;
				for(o = 0; o<a; o++){
					waitpid(child[o],&status,0);			
				}
				
				/*for(p = 0; p < b; p++){
					if (ok != 0){
						for(int l = 0; l < b; l++){ 
							estado2[l] = 1;
							if(childB[p] == arjobs[l].pidB){
								if(arjobs[l].posi % 2 == 0){ 
									if(estado3[l] != 1){
										printf("[%d]+ Hecho 		%s\n",arjobs[l].posi+1, arjobs[l].man);
									}
									estado3[l] = 1;
								
								}
								else{
									if(estado3[l] != 1){
										printf("[%d]- Hecho 		%s\n",arjobs[l].posi+1, arjobs[l].man);
									}
									estado3[l] = 1;
								}
							}
						}
					}
				}*/
				
				
				
				/*for(i = 0; i < line->ncommands; i++){
					//printf("Valor de pid: %d\n",pid);
					waitpid(pid, &status, 0);
					if(WIFEXITED(status) != 0){
						if(WEXITSTATUS(status) != 0){
							printf("El contenido no se ha ejecutado correctamente\n");
						}
					}
				}*/
			}
		}
		free(child);
		printf("msh> ");
		
		
	}

	return 0;
}

int cd(tcommand * com){
	char buffer[100];
	if(com -> argc <= 1){
		//printf("%s\n", getcwd(buffer, 100));
		if(chdir(getenv("HOME")) != 0){
			fprintf(stderr, "No ha podido acceder a HOME\n");
			return 1;
		} else {
			printf("%s\n", getcwd(buffer, 100));
		}
	} else if(com -> argc == 2) {
		//printf("%s\n", getcwd(buffer, 100));
		if(chdir(com -> argv[1]) != 0){
			fprintf(stderr, "No ha podido acceder al directorio del argumento\n");
			return 1;
		} else {
			printf("%s\n", getcwd(buffer, 100));
		}
	} else {
		fprintf(stderr, "Mas de un argumento, fin del programa\n");
		return 1;
	}
	
	return 0;

}

int umaskC(tcommand * com){
	if(com -> argc == 2){
		int mask = (int) strtol(com -> argv[1], NULL, 8);
		umask(mask); //cambia a la máscara nueva, pero en mask se guarda la máscara antigua
		printf("La mascara es : %d\n",mask);
		return 0;
	}
	return 1;
}

void fg(int* hijoB, tcommand * com){
	int num;
	if(com -> argc == 1) {
		printf("msh: actual: no existe ese trabajo");
		//si hay en bg, pasar a fg el ultimo de la lista
	} else if(com -> argc == 2){
		num = strtol(com->argv[1],NULL,10);
		num--;
		if(num < b) waitpid(hijoB[num], &status, 0);
	} 
	
}

void jobs(tline * li){	
	int p;
	int ok;
	int contador = 0;
	for(p = 0; p<c; p++){
		printf("auxEstado: %d\n",auxEstado);
		printf("Num bloques: %d\n",arjobs[p].bloque);
		for(int k = auxEstado; k < arjobs[p].bloque + auxEstado; k++){
			printf("k: %d\n",k);
			ok = waitpid(childB[k],&status,WNOHANG);
			printf("ok: %d\n",ok);
			if (ok < 0) contador++;
		}
		if (contador == arjobs[p].bloque) { //el proceso en bg ha terminado
			printf("p: %d\n",p);
			estado[p] = 1;
			printf("estado de 0: %d\n",estado[0]);
		}			
	} 
	
	for(int i = 0; i < c; i++){
		if(i%2 == 0){
			if(estado[i] == 0){ 
				printf("[%d]+ Ejecutando 		%s\n",i+1,arjobs[i].man);
			}
                	else if(estado[i] == 1){
                      		if(estado2[i] != 1) printf("[%d]+ Hecho 			%s\n",i+1,arjobs[i].man);             	
                		estado2[i] = 1;
                	}
                		
		} else {
			if(estado[i] == 0){ 
				printf("[%d]- Ejecutando 		%s\n",i+1,arjobs[i].man);
			}
                	else if(estado[i] == 1){ 
                		if(estado2[i] != 1) printf("[%d]- Hecho 			%s\n",i+1,arjobs[i].man);
                		estado2[i] = 1;
                  	}
                	
		}
	
	}
	
	int z = 0;
	int fin = 0;
	
	while(estado[z] == 1){
		fin++;
		auxEstado = auxEstado + arjobs[z].bloque;
		z++;
	} 
	
	
	if(fin == c){
		for(int h = 0; h < b; h++) estado[h] = 0;
		for(int h2 = 0; h2 < c; h2++) estado2[h2] = 0;
		c = 0;	
	}	


}











