/* my_shell.h - Gian Lucas Martín Chamorro, Tomás Bordoy García-Carpintero y
Jordi Antoni Sastre Moll*/

#define _POSIX_C_SOURCE 200112L

//Librerías
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>

#define COMMAND_LINE_SIZE 1024
#define ARGS_SIZE 64
#define N_JOBS 64
#define USE_READLINE

//Colores y estilos
#define RED "\x1B[31m"
#define BLU "\x1B[34m"
#define WHT "\x1B[37m"
#define RESET "\x1B[0m"
#define BOLD "\e[1m"

char *read_line(char *line);
char *obtener_prompt();
int execute_line(char *line);
int parse_args(char **args, char *line);
int check_internal(char **args);
int is_background(char **args);
//Funciones de comandos internos
int internal_cd(char **args);
int internal_export(char **args);
int internal_source(char **args);
int internal_jobs(char **args);
int is_output_redirection(char **args);
//Manejadores
void reaper(int signum);
void ctrlc(int signum);
void ctrlz(int signum);
//Funciones jobs_list
int jobs_list_add(pid_t pid, char status, char *command_line);
int jobs_list_find(pid_t pid);
int jobs_list_remove(int pos);
struct info_process
{
    pid_t pid;
    char status;
    char command_line[COMMAND_LINE_SIZE];
};
