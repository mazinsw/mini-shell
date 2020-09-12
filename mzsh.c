//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//      
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//      
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <sys/stat.h>
#include <pwd.h>
// quantidade máxima de argumentos nos comandos do usuário
#define MAX_ARGS 64

/**
 * imprime o nome de usuário, do computador e da pasta atual
 */
void type_prompt()
{
	char *user_name, comp_name[1024], cur_dir[1024];
	char no_name[]  = {'\0'};
	struct passwd *pw;
	uid_t uid;

	// obtem o nome do usuário
	uid = geteuid ();
	pw = getpwuid (uid);
	if (pw)
		user_name = pw->pw_name;
	else
		user_name = no_name;
	// obtem o nome do computador
	comp_name[0] = '\0';
	gethostname(comp_name, 1023);
	// obtem nome do diretório atual
	getcwd(cur_dir, 1024);
	printf("%s@%s:%s$ ", user_name, comp_name, cur_dir);
}

/**
 * lê do teclado um comando e seus argumentos
 * 
 * cmd_line: buffer para guardar tudo que o usuário digitar
 * argv: vetor de buffer para guardar os parâmetros separados
 */
void read_command(char* cmd_line, char **argv)
{
	int argc = 0;
	char * pstr;

	argv[argc] = NULL;
	scanf(" %[^\n]", cmd_line);
	pstr = strtok(cmd_line, " ");
	while (pstr && argc < MAX_ARGS - 1)
	{
		argv[argc] = pstr;
		argc++;
		argv[argc] = NULL;
		pstr = strtok(0, " ");
	}
}

/** 
 * verifica se um arquivo existe
 * 
 * filename: nome do arquivo para ser verificado
 * retorna
 * 	 1 caso exista, ou 0 caso não exista
 */
int file_exists(char * filename)
{
	struct stat sts;

	if (stat(filename, &sts) == -1)
		return 0;
	return 1;
}

int main(int argc, char** argv, char** env)
{
	pid_t cpid;
	int status = 0;
	char *commands[MAX_ARGS];
	char cmd_line[1024];
	char filename[1024];
	char* _path, *path, *pstr;

	while(1)
	{
		type_prompt();
		read_command(cmd_line, commands);
		// sai do programa
		if(strcmp("exit", commands[0]) == 0)
			exit(status);
		// limpa a tela
		if(strcmp("clear", commands[0]) == 0)
		{
			printf("\033[2J\033[1;1H");
			continue;
		}
		// muda o diretório atual
		if(strcmp("cd", commands[0]) == 0)
		{
			// verifica se digitou o diretório
			if(commands[1] != NULL)
			{
				if(chdir(commands[1]) != 0)
					fprintf(stderr, "diretório \"%s\" não encontrado\n", commands[1]);
			}
			else
				fprintf(stderr, "comando inválido esperado: cd diretório\n");
			continue;
		}
		// copia o nome do binário supondo que seja um caminho inteiro
		strcpy(filename, commands[0]);
		if (filename[0] != '/' && filename[0] != '.')
		{
			// obtendo a variável de ambiente PATH
			_path = getenv("PATH");
			// tornando acessível para modificação
			path = (char*)malloc((strlen(_path) + 1) * sizeof(char));
			strcpy(path, _path);
			// procurando o programa no PATH
			pstr = strtok(path, ":");
			
			while (pstr)
			{
				// copia o diretório base
				strcpy(filename, pstr);
				strcat(filename, "/");
				// concatena o nome do binário
				strcat(filename, commands[0]);
				if(file_exists(filename))
					break;
				pstr = strtok(0, ":");
			}
			free(path);
		}
		// cria um processo
		cpid = fork();
		// falha ao criar processo filho
		if (cpid == -1)
			exit(2);
		// código executado pelo processo filho
		if(cpid == 0)
		{
			execve(filename, commands, env);
			printf("mzsh: %s: comando não encontrado\n", commands[0]);
			exit(1);
		}
		else
		// código executado pelo processo pai
		{
			waitpid(-1, &status, 0);
		}
	}
	return status;
}
