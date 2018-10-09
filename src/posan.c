#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "data_structs.h"
#include "make_file.h"
#include "utils-posan.h"

#define clear() printf("\033[H\033[J")

/** 
 * 
 * ! Entrega 10/10
 * 
 * */

int main(int argc, char **argv)
{
	FILE *posan, *save;
	char choose[100];

	if (exists("posan.img") == 0)
	{
		posan = fopen("posan.img", "wb+");
		if (posan == NULL)
		{
			printf("Erro!");
			exit(1);
		}
		boot_sec(posan); //gerando boot sector
		make_fat(posan); //gerando FAT
		make_dir(posan); //gerando root dir
		fclose(posan);
	}
	posan = fopen("posan.img", "rb+");

	clear();
	while (1)
	{
		printf("posan_fs:~$ ");
		scanf("%[^\n]%*c", &choose);

		if (strcmp(choose, "quit") == 0 || strcmp(choose, "q") == 0)
		{
			printf("bye :(\n");
			exit(-1);
		}
		else if (strcmp(choose, "format") == 0)
		{
			printf("Formating...Please Wait \n");
			formatar(posan);
			printf("Done.\n");
		}
		else if (strcmp(choose, "hard format") == 0)
		{
			printf("Hard formating...Please wait\n");
			hard_format(posan);
			printf("Done.\n");
		}
		else if (strstr(choose, "add") != NULL)
		{
			char nome[100];
			strncpy(nome, get_nome(choose, ' '), sizeof(nome));

			save = fopen(nome, "rb");
			if (save == NULL)
			{
				printf("file cannot be open\n");
				exit(-1);
			}
			strcpy(nome, get_nome(choose, '/'));

			if (occurrences(choose) == 2)
				fill_subdir(posan, save, first_name(choose), nome);
			else
				copy_file(posan, save, nome);
		}
		else if (strstr(choose, "ls") != NULL)
		{
			char nome[100];
			if (occurrences(choose) == 1)
			{
				strcpy(nome, get_nome(choose, ' '));
				listar_dir(posan, nome);
			}
			else
				listar(posan);
		}
		else if (strstr(choose, "rm") != NULL)
		{
			char nome[100], backup[100];
			strncpy(nome, get_nome(choose, ' '), sizeof(nome));

			remove_file(posan, nome);
		}
		else if (strcmp(choose, "clear") == 0)
			clear();
		else if (strcmp(choose, "help") == 0)
		{
			printf("POSAN(Projeto Ordinario de um Sistema de Aquivo Novo) version 2.0\n");
			printf("quit               - close the program and generate the .img file\n");
			printf("format             - format the FAT table and the directories in the file\n");
			printf("add [PATH_TO_FILE] - insert a file into the filesystem\n");
			printf("export [FILENAME]  - exports a file of the filesystem to the designated location\n");
			printf("clear              - \n");
			printf("ls                 - show all the files in the file system\n");
		}
		else if (strstr(choose, "export") != NULL)
		{

			char nome[100];
			strncpy(nome, get_nome(choose, ' '), sizeof(nome));
			save = fopen(nome, "wb");
			if (save == NULL)
			{
				printf("file cannot be open\n");
				exit(-1);
			}
			strcpy(nome, get_nome(choose, '/'));

			if (occurrences(choose) == 2)
			{
				export_dir(posan, nome, first_name(choose), save);
			}
			else
			{
				export_file(posan, nome, save);
			}
			fclose(save);
		}
		else if (strstr(choose, "mkdir") != NULL)
		{
			char nome[25];
			strcpy(nome, get_nome(choose, ' '));
			subdir(posan, nome);
		}
		else
		{
			printf("%s: command not found\n", &choose);
		}
	}
	fclose(posan);
	return 0;
}
