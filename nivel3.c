/* nivel3.c - Parte 3 de Aventura 2 - Gian Lucas Martín Chamorro, 
Tomás Bordoy García-Carpintero y Jordi Antoni Sastre Moll*/
#include "nivel3.h"

int main() {
  char line[COMMAND_LINE_SIZE];
  while(read_line(line)){
    execute_line(line);
  }
  return 0;
}

char * read_line(char * line) {
  imprimir_prompt();
  if(fgets(line,COMMAND_LINE_SIZE,stdin)){
    return line;
  }else{
    return NULL;
  }
}

int imprimir_prompt() {
  printf(BOLD RED "%s",getenv("USER"));
  printf(RESET ":");
  printf(BOLD BLU "~%s",getenv("PWD"));
  printf(RESET "$ ");
  return 0;
}

int execute_line(char *line){
  char *args[ARGS_SIZE];
  pid_t pid;
  int status;
  parse_args(args,line);
  if(!check_internal(args)){
    fprintf(stderr,"[execute_line()→ PID padre: %d]\n",getpid());
    pid=fork();
    if(pid == 0){//hijo
      fprintf(stderr,"[execute_line()→ PID hijo: %d]\n",getpid());
      execvp(args[0],args);
      fprintf(stderr,"%s: no se encontró la orden\n",line);
      exit(0);
    }else if(pid > 0){//padre
      wait(&status);
      if(WIFEXITED(status)){
        fprintf(stderr,"[execute_line()→ Proceso hijo %d finalizado con exit(), estado: %d]\n",pid,WEXITSTATUS(status));
      }else{
        if(WIFSIGNALED(status)){
          fprintf(stderr,"[execute_line()→ Proceso hijo %d finalizado por señal %d]\n",pid,WTERMSIG(status));
        }
      }
    }else{
      fprintf(stderr,"Error %d: %s\n",errno,strerror(errno));
    }
  }
  return 0;
}

int parse_args(char **args,char *line){
  const char delim[5] = " \t\n\r";
  char *token;
  int n=0;
  
  token=strtok(line,delim);
  while(token){
    if(token[0]!='#'){
      args[n]=token;
      n++;
    }
    token=strtok(NULL,delim);
  }
  args[n]=token;
  n++;
  return(n-1);
}

int check_internal(char **args){
  if (strcmp(args[0], "cd") == 0){
    internal_cd(args);
    return 1;                    
  } else if(strcmp(args[0], "export") == 0){
    internal_export(args);
    return 1;
  } else if (strcmp(args[0], "source") == 0){
    internal_source(args);
    return 1;
  } else if (strcmp(args[0], "jobs") == 0){
    internal_jobs(args);
    return 1;
  } else if (strcmp(args[0], "exit") == 0){
    exit(EXIT_SUCCESS);
    return 1;    
  } else {
    return 0;
  }
}         


int internal_cd(char **args){  
  char cwd[256];
  if(!args[1]){
    chdir("/home"); 
  }else if(chdir(args[1])<0){
    fprintf(stderr,"Error %d: %s\n",errno,strerror(errno));
    return -1;
  } 
  printf("%s\n",getcwd(cwd,sizeof(cwd)));
  setenv("PWD",cwd,1);
  return 0;
}

int internal_export(char **args){
  const char delim[2] = "=";
  char *evar;   //Environment variable
  char *value;  //New environment variable value
  if(!args[1]||!(evar=strtok(args[1],delim))||!(value=strtok(NULL,"\n"))){
    fprintf(stderr, "Error de sintaxis. Uso: export Nombre=Valor\n");
  }
  fprintf(stderr,"[%s=%s]\n",evar,getenv(evar));
  setenv(evar, value, 1);
  fprintf(stderr,"[%s=%s]\n",evar,getenv(evar));

  return 0;
}

int internal_source(char **args){

    if (!args[1]){
        fprintf(stderr,"Error de sintaxis. Uso: source <nombre_fichero>");
        return -1;
    }
    FILE *fichero;
    fichero =fopen(args[1], "r");
    if(!fichero){
        fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
        return -1;
    }
    char str[100];
    while (fgets(str, 100, fichero)){
        puts(str);
        fflush(fichero);
    }
    fclose(fichero);
  return 0;
}

int internal_jobs(char **args){
  printf("El comando jobs lista el estatus de todos los trabajos activos.\n");
  return 0;
}