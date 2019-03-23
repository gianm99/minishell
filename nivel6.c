/* nivel6.c - Parte 6 de Aventura 2 - Gian Lucas Martín Chamorro, 
Tomás Bordoy García-Carpintero y Jordi Antoni Sastre Moll*/
#include "nivel6.h"
static struct info_process jobs_list[N_JOBS];
const char *nombreshell;
int n_pids = 1;

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
  fflush(stdout);
  return 0;
}

int execute_line(char *line)
{
  char *args[ARGS_SIZE];
  char command[COMMAND_LINE_SIZE];
  pid_t pid;
  int bkg;
  strtok(line, "\n");
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
    // fprintf(stderr, "[internal_source()→ LINE: %s]", str);
    execute_line(str);
  }
  fclose(fichero);
  return 0;
}

int internal_jobs(char **args)
{
  for (int i = 1; i < n_pids; i++)
  {
    printf("[%d] %d\t%c\t%s\n", i, jobs_list[i].pid, jobs_list[i].status,
           jobs_list[i].command_line);
  }
  return 0;
}

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
        fprintf(stderr, "\nTerminado PID %d (%s) en jobs_list[%d] con estatus %d\n",
                jobs_list[jobnum].pid,
                jobs_list[jobnum].command_line,
                jobnum, status);
        fflush(stderr);
        jobs_list_remove(jobnum);
      }
    }
  }
}
void ctrlc(int signum)
{
  signal(SIGINT, ctrlc);
  if (jobs_list[0].pid > 0)
  {
    if (strcmp(jobs_list[0].command_line, nombreshell))
    {
      fprintf(stderr, "\n[ctrlc()→ Soy el proceso con PID %d(%s), el proceso en "
                      "foreground es %d (%s)]\n",
              getpid(), nombreshell, jobs_list[0].pid, jobs_list[0].command_line);
      fprintf(stderr, "[ctrlc()→ Señal %d enviada a %d(%s) por %d(%s)]\n",
              SIGTERM, jobs_list[0].pid, jobs_list[0].command_line, getpid(), nombreshell);
      kill(jobs_list[0].pid, SIGTERM);
    }
    else
    {
      fprintf(stderr, "\n[ctrlc()→ Soy el proceso con PID %d(%s), el "
                      "proceso en foreground es %d(%s)]\n",
              getpid(), nombreshell, jobs_list[0].pid, jobs_list[0].command_line);
      fprintf(stderr, "[ctrlc()→ Señal %d no enviada por %d(%s) debido a que"
                      " su proceso en foreground es el shell]\n",
              SIGTERM, getpid(), nombreshell);
    }
  }
  else
  {
    fprintf(stderr, "\n[ctrlc()→ Soy el proceso con PID %d(%s), el "
                    "proceso en foreground es %d(%s)]\n",
            getpid(), nombreshell, jobs_list[0].pid, jobs_list[0].command_line);
    fprintf(stderr, "[ctrlc()→ Señal %d no enviada por %d(%s) debido a que no"
                    " hay proceso en foreground]\n",
            SIGTERM, getpid(), nombreshell);
  }
}

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
    else
    {
      fprintf(stderr, "\n[ctrlz()→ Soy el proceso con PID %d(%s), el "
                      "proceso en foreground es %d(%s)]\n",
              getpid(), nombreshell, jobs_list[0].pid, jobs_list[0].command_line);
      fprintf(stderr, "[ctrlz()→ Señal %d no enviada por %d(%s) debido a que"
                      " su proceso en foreground es el shell]\n",
              signum, getpid(), nombreshell);
    }
  }
  else
  {
    fprintf(stderr, "\n[ctrlz()→ Soy el proceso con PID %d(%s), el "
                    "proceso en foreground es %d(%s)]\n",
            getpid(), nombreshell, jobs_list[0].pid, jobs_list[0].command_line);
    fprintf(stderr, "[ctrlz()→ Señal %d no enviada por %d(%s)] debido a que no"
                    " hay proceso en foreground]\n",
            signum, getpid(), nombreshell);
  }
}

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

int jobs_list_remove(int pos)
{
  if (pos < n_pids)
  {
    n_pids--;
    jobs_list[pos].pid = jobs_list[n_pids].pid;
    jobs_list[pos].status = jobs_list[n_pids].status;
    strcpy(jobs_list[pos].command_line, jobs_list[n_pids].command_line);
  }
  else
  {
    return -1;
  }

  return 0;
}