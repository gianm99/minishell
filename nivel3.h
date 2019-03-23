/* nivel3.h - Parte 3 de Aventura 2 - Gian Lucas Martín Chamorro, 
Tomás Bordoy García-Carpintero y Jordi Antoni Sastre Moll*/
#define _POSIX_C_SOURCE 200112L

//Librerías
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

//Tamaño línea de comandos y nº máximo de tokens
#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64

//Colores y estilos
#define RED "\x1B[31m"
#define BLU "\x1B[34m"
#define WHT "\x1B[37m"
#define RESET "\x1B[0m"
#define BOLD "\e[1m"

char * read_line(char *line);
int imprimir_prompt();
int execute_line(char * line);
int parse_args(char ** args, char * line);
int check_internal(char ** args);
int internal_cd(char ** args);
int internal_export(char ** args);
int internal_source(char ** args);
int internal_jobs(char ** args);
