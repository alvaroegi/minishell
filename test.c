#include <stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "parser.h"

pid_t pid;
int p_par[2];
int p_impar[2];
int status;

int
main(void) {
	char buf[1024];
	tline * line;
	int i,j;

	printf("msh> ");
	while (fgets(buf, 1024, stdin)) {
		pipe(p_par);
		pipe(p_impar);
		line = tokenize(buf);
		if(strcmp(line->commands[0].argv[0], "exit") == 0) exit(0);
		if (line==NULL) {
			continue;
		}
		if (line->redirect_input != NULL) {
			printf("redirección de entrada: %s\n", line->redirect_input);
		}
		if (line->redirect_output != NULL) {
			printf("redirección de salida: %s\n", line->redirect_output);
			mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR;
			int s = creat(line->redirect_output,mode);
			if(s < 0){
				printf("Error en la creación del archivo");
			}
		}
		if (line->redirect_error != NULL) {
			printf("redirección de error: %s\n", line->redirect_error);
		}
		if (line->background) {
			printf("comando a ejecutarse en background\n");
		}
		for (i=0; i<line->ncommands; i++) {
			printf("orden %d (%s):\n", i, line->commands[i].filename);
			for (j=0; j<line->commands[i].argc; j++) {
				printf("  argumento %d: %s\n", j, line->commands[i].argv[j]);
			}
			pid = fork();
			if(pid == 0){ //HIJO
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
				if (line->redirect_output != NULL){
					int fd = open(line->redirect_output, mode);
					if(fd < 0) {
						printf("Error al abrir el archivo llamado %s\n",line->redirect_output);
					} else{
						dup2(fd,1);
					}
				}
				if (line->redirect_input != NULL){
					int fd = open(line->redirect_input, mode);
					if(fd < 0) {
						printf("Error al abrir el archivo llamado %s\n",line->redirect_input);
					} else {
						dup2(fd,0);
					}

				}
				execvp(line->commands[i].filename, line->commands[i].argv);
				fprintf(stderr, "Se ha producido un error\n");
				exit(1);
			
			} else { //Padre
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
		close(p_par[0]);
		close(p_par[1]);
		close(p_impar[0]);
		close(p_impar[1]);
		
		for(i = 0; i < line->ncommands; i++){
			wait(&status);
			if(WIFEXITED(status) != 0){
				if(WEXITSTATUS(status) != 0){
					printf("El contenido no se ha ejecutado correctamente\n");
				}
			}
		}
		printf("msh> ");
		}
	}


	return 0;
}

