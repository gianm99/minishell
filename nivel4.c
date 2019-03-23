/* nivel4.c - Parte 4 de Aventura 2 - Gian Lucas Martín Chamorro, 
Tomás Bordoy García-Carpintero y Jordi Antoni Sastre Moll*/
#include "nivel4.h"
static struct info_process jobs_list[N_JOBS];
const char *nombreshell;

int main(int argc, char **argv)
{
  signal(SIGCHLD, reaper);
  signal(SIGINT, ctrlc);
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

char *read_line(char *line)
{
  char *ptr;
  imprimir_prompt();
  ptr = fgets(line, COMMAND_LINE_SIZE, stdin);
  if (!ptr && !feof(stdin))
  {
    ptr = line;
    ptr[0] = 0;
  }
  return ptr;
}

int imprimir_prompt()
{
  printf(BOLD RED "%s", getenv("USER"));
  printf(RESET ":");
  printf(BOLD BLU "~%s", getenv("PWD"));
  printf(RESET "$ ");
  return 0;
}

int execute_line(char *line)
{
  char *args[ARGS_SIZE];
  pid_t pid;
  strtok(line, "\n");
  strcpy(jobs_list[0].command_line, line);
  parse_args(args, line);
  if (args[0] && !check_internal(args))
  {
    pid = fork();
    if (pid == 0)
    { //hijo
      signal(SIGCHLD, SIG_DFL); //acción por defecto
      signal(SIGINT, SIG_IGN);  //ignora la señal
      fprintf(stderr, "[execute_line()→ PID hijo: %d(%s)]\n", getpid(), jobs_list[0].command_line);
      execvp(args[0], args);
      fprintf(stderr, "%s: no se encontró la orden\n", line);
      exit(-1);
    }
    else if (pid > 0)
    { //padre
      fprintf(stderr, "[execute_line()→ PID padre: %d(%s)]\n", getpid(), nombreshell);
      jobs_list[0].pid = pid;
      jobs_list[0].status = 'E';
      while (jobs_list[0].pid != 0)
      {
        pause();
      }
    }
    else
    {
      perror("fork");
    }
  }
  return 0;
}

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
  n++;
  return (n - 1);
}

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

int internal_export(char **args)
{
  const char delim[2] = "=";
  char *evar;  //Environment variable
  char *value; //New environment variable value
  if (!args[1] || !(evar = strtok(args[1], delim)) ||
      !(value = strtok(NULL, "\n")))
  {
    fprintf(stderr, "Error de sintaxis. Uso: export Nombre=Valor\n");
    return -1;
  }
  setenv(evar, value, 1);
  return 0;
}

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
    fprintf(stderr, "[internal_source()→ LINE: %s]", str);
    execute_line(str);
    printf("\n");
  }
  fclose(fichero);
  return 0;
}

int internal_jobs(char **args)
{
  printf("El comando jobs lista el estatus de todos los trabajos activos.\n");
  return 0;
}

void reaper(int signum)
{
  signal(SIGCHLD, reaper);
  int status;
  pid_t pid;
  while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
  {
    if (WIFEXITED(status))
    {
      fprintf(stderr, "\n[reaper()→ Proceso hijo %d(%s) finalizado con exit code %d]\n", pid, jobs_list[0].command_line, WEXITSTATUS(status));
    }
    else if (WIFSIGNALED(status))
    {
      fprintf(stderr, "[reaper()→ Proceso hijo %d(%s) finalizado por señal %d]\n", pid, jobs_list[0].command_line, WTERMSIG(status));
    }
    jobs_list[0].pid = 0;
    jobs_list[0].status = 'F';
    strcpy(jobs_list[0].command_line, "");
  }
}

void ctrlc(int signum)
{
  signal(SIGINT, ctrlc);
  printf("\n");
  fflush(stdout);
  if (jobs_list[0].pid > 0)
  {
    if (strcmp(jobs_list[0].command_line, nombreshell))
    {
      fprintf(stderr, "[ctrlc()→ Soy el proceso con PID %d(%s), el proceso en "
                      "foreground es %d (%s)]\n",
              getpid(), nombreshell, jobs_list[0].pid, jobs_list[0].command_line);
      printf("[ctrlc()→ Señal %d enviada a %d(%s) por %d(%s)]", SIGTERM, jobs_list[0].pid, jobs_list[0].command_line, getpid(), nombreshell);
      kill(jobs_list[0].pid, SIGTERM);
    }
    else
    {
      fprintf(stderr, "[ctrlc()→ Soy el proceso con PID %d(%s), el "
                      "proceso en foreground es %d(%s)]\n",
              getpid(), nombreshell, jobs_list[0].pid, jobs_list[0].command_line);
      fprintf(stderr, "[ctrlc()→ Señal %d no enviada por %d(%s) debido a que"
                      " su proceso en foreground es el shell]",
              SIGTERM, getpid(), nombreshell);
    }
  }
  else
  {
    fprintf(stderr, "\n[ctrlc()→ Soy el proceso con PID %d(%s), el "
                    "proceso en foreground es %d(%s)]\n",
            getpid(), nombreshell, jobs_list[0].pid, jobs_list[0].command_line);
    fprintf(stderr, "[ctrlc()→ Señal %d no enviada por %d(%s)] debido a que no"
                    " hay proceso en foreground]",
            SIGTERM, getpid(), nombreshell);
  }
  printf("\n");
  fflush(stdout);
}