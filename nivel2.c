/* nivel2.c - Parte 2 de Aventura 2 - Gian Lucas Martín Chamorro, 
Tomás Bordoy García-Carpintero y Jordi Antoni Sastre Moll*/
#include "nivel2.h"

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
  parse_args(args,line);
  check_internal(args);
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

  for(int i=0;i<n;i++){
    printf("%d, %s\n",i,args[i]);  
  }
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
    exit(0);
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
  printf("[%s=%s]\n",evar,getenv(evar));
  setenv(evar, value, 1);
  printf("[%s=%s]\n",evar,getenv(evar));

  return 0;
}

int internal_source(char **args){
  printf("El comando source sirve para cargar cualquier archivo de funciones"
  " en el shell o en un indicador de comandos.\n");
  return 0;
}

int internal_jobs(char **args){
  printf("El comando jobs lista el estatus de todos los trabajos activos.\n");
  return 0;
}