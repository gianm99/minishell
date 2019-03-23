/* nivel7.c - Parte 7 de Aventura 2 - Gian Lucas Martín Chamorro, 
Tomás Bordoy García-Carpintero y Jordi Antoni Sastre Moll*/
#include "my_shell.h"
static char prompt[COMMAND_LINE_SIZE];
static struct info_process jobs_list[N_JOBS];
const char *nombreshell;
int n_pids = 1;

/**
 * main es la función principal que asigna las señales a nuestro minishell,
 * y ejecuta el bucle de lectura y ejecución de comandos.
 */
int main(int argc, char **argv)
{
    signal(SIGCHLD, reaper);
    signal(SIGINT, ctrlc);
    signal(SIGTSTP, ctrlz);
    char line[COMMAND_LINE_SIZE];
    jobs_list[0].pid = 0;
    jobs_list[0].status = 'F';
    strcpy(jobs_list[0].command_line, "");
    nombreshell = argv[0];
    while (read_line(line))
    {
        execute_line(line);
    }
    return 0;
}

/**
 * read_line es una función que, dependiendo de si USE_READLINE está definido,
 * leerá con las funciones de la librería readline o con las funciones
 * básicas de C.
 */
char *read_line(char *line)
{
#ifdef USE_READLINE
    obtener_prompt(prompt);
    char *teclado = readline(prompt);
    if (!teclado)
    {
        return NULL;
    }
    strcpy(line, teclado);
    add_history(teclado);
    free(teclado);
    return line;
#else
    char *ptr;
    obtener_prompt(prompt);
    printf("%s", prompt);
    ptr = fgets(line, COMMAND_LINE_SIZE, stdin);
    if (!ptr)
    {
        printf("\r");
        if (feof(stdin))
        {
            exit(0);
        }
        else
        {
            ptr = line;
            ptr[0] = 0;
        }
    }
    else
    {
        strtok(line, "\n");
    }
    return ptr;
#endif
}

/**
 * obtener_prompt es una función que escribe en la consola el prompt y lo devuelve.
 */
char *obtener_prompt(char *prompt)
{
    sprintf(prompt, BOLD RED "%s" RESET ":" BOLD BLU "~%s" RESET "$ ",
            getenv("USER"),
            getenv("PWD"));
    return prompt;
}

/**
 * execute_line es la función encargada de ejecutar las líneas de comandos.
 * Como tal, se encarga de, mediante otras funciones auxiliares, de dividir
 * la línea de comandos en tokens, hacer el fork() para crear un proceso
 * hijo, mirar si es un comando que se tiene que ejecutar en background
 * y realizar el comando designado.
 */
int execute_line(char *line)
{
    char *args[ARGS_SIZE];
    char command[COMMAND_LINE_SIZE];
    pid_t pid;
    int bkg;
    memset(command, '\0', COMMAND_LINE_SIZE);
    strcpy(command, line);
    parse_args(args, line);
    if (args[0] && !check_internal(args)) /*Comando externo*/
    {
        bkg = is_background(args);
        pid = fork();
        if (pid == 0) /*Proceso hijo*/
        {
            signal(SIGCHLD, SIG_DFL);
            signal(SIGINT, SIG_IGN);
            if (bkg) /*BACKGROUND*/
            {
                signal(SIGTSTP, SIG_IGN);
            }
            else /*FOREGROUND*/
            {
                signal(SIGTSTP, SIG_DFL);
            }
            is_output_redirection(args);
            execvp(args[0], args);
            fprintf(stderr, "%s: no se encontró la orden\n", command);
            exit(EXIT_FAILURE);
        }
        else if (pid > 0) /*Proceso padre*/
        {
            if (bkg) /*BACKGROUND*/
            {
                jobs_list_add(pid, 'E', command);
            }
            else /*FOREGROUND*/
            {
                jobs_list[0].pid = pid;
                jobs_list[0].status = 'E';
                strcpy(jobs_list[0].command_line, command);
                while (jobs_list[0].pid != 0)
                {
                    pause();
                }
            }
        }
    }
    return 0;
}

/**
 * parse_args es una función que se encarga de dividir las líneas en tokens.
 * Devuelve el número de tokens.
 */
int parse_args(char **args, char *line)
{
    const char delim[5] = " \t\n\r";
    char *token;
    int n = 0;

    token = strtok(line, delim);
    while (token)
    {
        if (token[0] == '#')
            token = NULL;
        args[n] = token;
        n++;
        token = strtok(NULL, delim);
    }
    args[n] = token;
    return n;
}

/**
 * check_internal es una función que mira si el comando introducido es "cd, 
 * export, soruce, jobs o exit". Si lo es, entonces usaremos nuestras
 * funciones internas (menos en el exit que simplemente sale) para manejarlos.
 */
int check_internal(char **args)
{
    if (strcmp(args[0], "cd") == 0)
    {
        internal_cd(args);
        return 1;
    }
    else if (strcmp(args[0], "export") == 0)
    {
        internal_export(args);
        return 1;
    }
    else if (strcmp(args[0], "source") == 0)
    {
        internal_source(args);
        return 1;
    }
    else if (strcmp(args[0], "jobs") == 0)
    {
        internal_jobs(args);
        return 1;
    }
    else if (strcmp(args[0], "exit") == 0)
    {
        exit(EXIT_SUCCESS);
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * is_background es una función booleana para mirar si el comando acaba en "&",
 * lo cual indicará que es en background, y le quitará el carácter.
 */
int is_background(char **args)
{
    int i = 0;
    while (args[i] != NULL)
    {
        i++;
    }
    if (!strcmp(args[i - 1], "&"))
    {
        args[i - 1] = NULL;
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * is_output_redirection es una función booleana que mira si en el args hay un
 * token ">", seguido de otro que será el nombre de fichero. Si lo encuentra,
 * cambiará ">" por NULL y redireccionará el stdout a un fichero.
 */
int is_output_redirection(char **args)
{
    int i = 0, fd;
    while (args[i] != NULL)
    {
        if (!strcmp(args[i], ">"))
        {
            if (!args[i + 1])
            {
                fprintf(stderr, "Error de sintaxis. Uso: comando > fichero\n");
                return 0;
            }
            if ((fd = open(args[i + 1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR)) >= 0)
            {
                args[i] = NULL;
                dup2(fd, 1);
                close(fd);
                return 1;
            }
        }
        i++;
    }
    return 0;
}

/**
 * internal_cd es una función que permite al usuario moverse por los 
 * directorios de su equipo.
 */
int internal_cd(char **args)
{
    char cwd[256];
    if (!args[1])
    {
        chdir("/home");
    }
    else if (chdir(args[1]) < 0)
    {
        perror("chdir");
        return -1;
    }
    getcwd(cwd, sizeof(cwd));
    setenv("PWD", cwd, 1);
    return 0;
}

/**
 * internal_export es una función que permite al usuario cambiar el valor de 
 * variables de entorno.
 */
int internal_export(char **args)
{
    const char delim[2] = "=";
    char *evar;
    char *value;
    if (!args[1] || !(evar = strtok(args[1], delim)) ||
        !(value = strtok(NULL, "\n")))
    {
        fprintf(stderr, "Error de sintaxis. Uso: export Nombre=Valor\n");
        return -1;
    }
    setenv(evar, value, 1);
    return 0;
}

/**
 * internal_source es una función que lee y ejecuta los comandos escritos en el
 * fichero indicado.
 */
int internal_source(char **args)
{

    if (!args[1])
    {
        fprintf(stderr, "Error de sintaxis. Uso: source <nombre_fichero>\n");
        return -1;
    }
    FILE *fichero;
    fichero = fopen(args[1], "r");
    if (!fichero)
    {
        perror("fopen");
        return -1;
    }
    char str[COMMAND_LINE_SIZE];
    while (fgets(str, COMMAND_LINE_SIZE, fichero))
    {
        fflush(fichero);
        strtok(str, "\n");
        execute_line(str);
    }
    fclose(fichero);
    return 0;
}
/**
 * internal_jobs es una función que muestra los procesos guardados en jobs_list.
 */
int internal_jobs(char **args)
{
    for (int i = 1; i < n_pids; i++)
    {
        printf("[%d] %d\t%c\t%s\n", i, jobs_list[i].pid, jobs_list[i].status,
               jobs_list[i].command_line);
    }
    return 0;
}

/**
 * reaper es un manejador propio para la señal SIGCHLD. Si el proceso está en 
 * foreground lo actualiza en jobs_list. Si está en background notifica que ha 
 * terminado y lo elimina de jobs_list.
 */
void reaper(int signum)
{
    signal(SIGCHLD, reaper);
    int status;
    pid_t pid;
    int jobnum;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        if (pid == jobs_list[0].pid) /*FOREGROUND*/
        {
            jobs_list[0].pid = 0;
            jobs_list[0].status = 'F';
            strcpy(jobs_list[0].command_line, "");
        }
        else /*BACKGROUND*/
        {
            jobnum = jobs_list_find(pid);
            if (jobnum)
            {
                fprintf(stderr, "\nTerminado PID %d (%s) en jobs_list[%d] con estatus %d",
                        jobs_list[jobnum].pid,
                        jobs_list[jobnum].command_line,
                        jobnum, status);
#ifndef USE_READLINE
                fprintf(stderr, "\n");
#endif
                fflush(stderr);
                fflush(stdout);
                jobs_list_remove(jobnum);
            }
        }
    }
}

/**
 * ctrlc es un manejador propio para la señal SIGINT. Si hay proceso en 
 * foreground y no es el minishell le envía la señal SIGTERM.
 */
void ctrlc(int signum)
{
    signal(SIGINT, ctrlc);
    if (jobs_list[0].pid > 0)
    {
        if (strcmp(jobs_list[0].command_line, nombreshell))
        {
            kill(jobs_list[0].pid, SIGTERM);
        }
        printf("\n");
    }
    else
    {
#ifdef USE_READLINE
        printf("\n%s", prompt);
#else
        printf("\n");
#endif
    }
}

/**
 * ctrlz es un manejador propio para la señal SIGTSTP. Si hay proceso en 
 * foreground y no es el minishell le envía la señal SIGTSTP y lo añade como 
 * proceso detenido en jobs_list.
 */
void ctrlz(int signum)
{
    signal(SIGTSTP, ctrlz);
    if (jobs_list[0].pid > 0)
    {
        if (strcmp(jobs_list[0].command_line, nombreshell))
        {
            kill(jobs_list[0].pid, SIGTSTP);
            jobs_list[0].status = 'D';
            jobs_list_add(jobs_list[0].pid, jobs_list[0].status, jobs_list[0].command_line);

            jobs_list[0].pid = 0;
            jobs_list[0].status = 'F';
            strcpy(jobs_list[0].command_line, "");
        }
        printf("\n");
    }
    else
    {
#ifdef USE_READLINE
        printf("\n%s", prompt);
#else
        printf("\n");
#endif
    }
}
/**
 * jobs_list_add es una función que añade un proceso a jobs_list en la última 
 * posición. Le asigna los valores introducidos como parámetros.
 */
int jobs_list_add(pid_t pid, char status, char *command_line)
{
    if (n_pids < N_JOBS)
    {
        jobs_list[n_pids].pid = pid;
        jobs_list[n_pids].status = status;
        strcpy(jobs_list[n_pids].command_line, command_line);
        printf("[%d]\t%d\t%c\t%s\n",
               n_pids, jobs_list[n_pids].pid, jobs_list[n_pids].status,
               jobs_list[n_pids].command_line);
        n_pids++;
        return n_pids;
    }
    else
    {
        fprintf(stderr, "Límite de procesos en jobs_list alcanzado\n");
        return -1;
    }
}

/**
 * jobs_list_find es una función que busca un proceso en jobs_list por su pid 
 * y devuelve su posición en la misma.
 */
int jobs_list_find(pid_t pid)
{
    int i = 1;
    while (i < N_JOBS)
    {
        if (jobs_list[i].pid == pid)
        {
            return i;
        }
        i++;
    }
    return 0;
}

/**
 * jobs_list_remove es una función que "elimina" de jobs_list un proceso. 
 * Mueve el último de la lista a su posición y disminuye n_pids (el número de 
 * procesos en la lista). Solo elimina el proceso que ocupa la posición pos 
 * en la lista.
 */
int jobs_list_remove(int pos)
{
    if (pos < n_pids)
    {
        n_pids--;
        jobs_list[pos].pid = jobs_list[n_pids].pid;
        jobs_list[pos].status = jobs_list[n_pids].status;
        strcpy(jobs_list[pos].command_line, jobs_list[n_pids].command_line);
        return 0;
    }
    else
    {
        return -1;
    }
}