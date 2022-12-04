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
pid_t pidAux;
int p_par[2];
int p_impar[2];
int status;
int a,b,c; //contador arrays hijos y jobs
int* childB;
int estado[50];
int estado2[50];
int auxEstado;
int z=0;
int fin=0;

struct hijoPosiJobs arjobs[50];

int cd(tcommand * com);
int umaskC(tcommand * com);
void fg(tline * li);
void jobs(int g);
char* copiarString(char* in);
void manejador_hijo(int sig);


int
main(void) {
	umask(75); //establecer permisos por defecto en los ficheros
	char buf[1024];
	tline * line;
	int i;
	auxEstado = 0;
	
	signal(SIGINT, SIG_IGN);
	signal(SIGUSR2, manejador_hijo);
	
	int* child;
	
	b = 0;
	childB = (int*)malloc(sizeof(int));
			
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
				exit(0);
			}
			else if(strcmp(line->commands[0].argv[0], "umask") == 0) umaskC(line->commands);
			else if(strcmp(line->commands[0].argv[0], "fg") == 0) fg(line);
			else if(strcmp(line->commands[0].argv[0], "jobs") == 0) jobs(1);
			else if(line->commands[0].filename == NULL) printf("%s: no se encuentra el mandato\n",line->commands[0].argv[0]);
			else {						
				if (line->background) {
					buf[strlen(buf) -1] = '\0';
					strcpy(arjobs[c].man,buf);
					arjobs[c].bloque = line -> ncommands;
					c++;
				}
				for (i=0; i<line->ncommands; i++) {

					pid = fork();
					
					
					if(pid < 0) { //ERROR
						fprintf(stderr, "Fallo el fork().\n%s\n", strerror(errno));
						exit(1);
					}else if(pid == 0){ //HIJO
						
						if(line->background == 0) {
							signal(SIGINT,SIG_DFL);
						}	
					
					
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

						if (line->redirect_output != NULL && i == line->ncommands -1){
							int fd = creat(line->redirect_output, 0777);
							if(fd < 0) {
								printf("Error al abrir el archivo llamado %s\n",line->redirect_output);
							} else{
								dup2(fd,1);
							}
						}
						if (line->redirect_input != NULL && i == 0){
							int fd = open(line->redirect_input, O_RDWR);
							if(fd < 0) {
								printf("Error al abrir el archivo llamado %s\n",line->redirect_input);
							} else {
								dup2(fd,0);
							}
						}
						if (line->redirect_error != NULL){
							int fd = creat(line->redirect_error, 0777);
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
							arjobs[b].pidB = pid;
							arjobs[b].posi = b;
							b++;	
							childB = realloc(childB, (b+1)*sizeof(int));		
							
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
				jobs(0); //para que salga el Hecho en una instruccion en background al ejecutar otra 	
				
			
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

void fg(tline * li){
	int num;
	int auxFG = arjobs[auxEstado].posi;
	if(li -> commands -> argc == 1) {
		if(c == 0) printf("msh: actual: no existe ese trabajo\n");
		else{
			
			for(int p = 0; p<c; p++){
				auxFG = auxFG + arjobs[p].bloque;
			}
			
			//para imprimir cual es el ultimo mandato
			int t = c-1;
			while(estado[t]==1) t--;
			printf("%s\n",arjobs[t].man);

			for(int h = auxFG; h < auxFG+arjobs[c-1].bloque; h++){
				waitpid(childB[h], &status, 0);
			}
			//printf("[%d] Hecho 			%s\n",c,arjobs[c-1].man);
			estado2[t] = 1;
			estado[t] = 1;
		}
		//si hay en bg, pasar a fg el ultimo de la lista
		
	} else if(li -> commands -> argc == 2){
		num = strtol(li -> commands->argv[1],NULL,10);
		num--;
		if(num < c){
			for(int p = 0; p < num; p++){
				auxFG = auxFG + arjobs[p].bloque;
			}
			
			printf("%s\n",arjobs[num].man);
			for(int h = auxFG; h < auxFG+arjobs[num].bloque; h++){
				waitpid(childB[h], &status, 0);
			}
			//printf("[%d] Hecho 			%s\n",num+1,arjobs[num].man);
			estado2[num] = 1;
			estado[num] = 1;
			
			
		}
	} 
	jobs(0);
	
}

void jobs(int g){
	int p;
	int ok;
	int contador;
	int y = 0;
	
	
	for(p = 0; p<c; p++){
		contador = 0;
		for(int k = y + arjobs[auxEstado].posi; k < arjobs[p].bloque + y + arjobs[auxEstado].posi && k<50; k++){
			ok = waitpid(childB[k],&status,WNOHANG);
			if (ok != 0) contador++;
		}
		if (contador == arjobs[p].bloque) { //el proceso en bg ha terminado
			estado[p] = 1;
		}		
		y = y + arjobs[p].bloque;	
	} 
	
	for(int i = 0; i < c; i++){
		if(i%2 == 0){
			if(estado[i] == 0 && g == 1){ 
				printf("[%d]+ Ejecutando 		%s\n",i+1,arjobs[i].man);
			}
	        	else if(estado[i] == 1){
	              		if(estado2[i] != 1) printf("[%d]+ Hecho 			%s\n",i+1,arjobs[i].man);             	
	        		estado2[i] = 1;
	        	}
	        		
		} else {
			if(estado[i] == 0 && g == 1){ 
				printf("[%d]- Ejecutando 		%s\n",i+1,arjobs[i].man);
			}
	        	else if(estado[i] == 1){ 
	        		if(estado2[i] != 1) printf("[%d]- Hecho 			%s\n",i+1,arjobs[i].man);
	        		estado2[i] = 1;
	          	}
	        	
		}
	
	}
	

	while(estado[z] == 1){
		fin++;
		auxEstado = auxEstado + arjobs[z].bloque;
		z++;
	} 
	
	
	if(fin == c){
		for(int h=0;h<c;h++) {
			estado[h]=0;
			estado2[h]=0;
		}
		z=0;
		c = 0;
		fin=0;
	}
	

}



void manejador_hijo(int sig) {
	if(pid == 0) {
		printf("Hola entra aqui");
			
	} else {
		printf("HOla mierda");
		signal(SIGINT,SIG_DFL);
	}
}













