#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "data_structs.h"
#include "make_file.h"
#include "utils-posan.h"

#define clear() printf("\033[H\033[J")

void subdir(FILE* posan, char* name){
	//marcando na fat -----------------------------------------
	int cluster = get_cluster(posan) + 256;
	fseek(posan, (cluster*2)+512, SEEK_SET);
	unsigned short int mark = 0xFFFF;
	fwrite(&mark, sizeof(mark), 1, posan); 
	//marcando no subdiretorio---------------------------------
	directory rd;
	for (int i = 0; i < 25; i++){
		if (i < strlen(name)){
			rd.filename[i] = name[i];
		}else
			rd.filename[i] = 0x00;
	}
	rd.attribute = 2;
	rd.initial_cluster = cluster;
	rd.size_file = 0;

	fseek(posan, 131584, SEEK_SET);
	directory dir;
	while(1){
		fread(&dir, sizeof(dir),1,posan);
		if (strcmp(dir.filename, "") == 0){
			fseek(posan, ftell(posan)-32, SEEK_SET);
			break;
		}
	}

	fwrite(&rd, sizeof(rd), 1, posan);
}

void fill_subdir(FILE* posan, FILE* save, char *name){
	
}

int main(int argc, char **argv){
	FILE *posan, *save;
    char choose[100];

	if (exists("posan.img") == 0){
		posan = fopen("posan.img", "wb+");
		if (posan == NULL){
			printf ("Erro!");
			exit(1);
		}
		boot_sec(posan);		//gerando boot sector
		make_fat(posan);		//gerando FAT
		make_dir(posan);		//gerando root dir
		fclose(posan);
	}
	posan = fopen("posan.img", "rb+");

	clear();
	while(1){
		printf("posan_fs:~$ ");
		scanf("%[^\n]%*c", &choose);

		if (strcmp(choose, "quit") == 0){
			exit(-1);
		}else if(strcmp(choose, "format") == 0){
			printf("Formating...Please Wait \n");
			formatar(posan);
			printf("Done.\n");
		}else if(strstr(choose, "add") != NULL){
			char nome[100];
			
			strncpy(nome, get_nome(choose, ' '), sizeof(nome));
			
			printf("%s\n", nome);
			save = fopen(nome, "rb");
			if(save == NULL){
				printf("deu merda\n");
				exit(-1);
			}
			strcpy(nome, get_nome(choose, '/'));

			copy_file(posan, save, get_cluster(posan), nome);

		}
		else if(strcmp(choose, "ls") == 0)	listar(posan);
		else if (strcmp(choose, "clear") == 0) clear();
		else if (strcmp(choose, "help") == 0){
			printf("POSAN(Projeto Ordinario de um Sistema de Aquivo Novo) version 1.0\n");
			printf("quit               - close the program and generate the .img file\n");
			printf("format             - format the FAT table and the directories in the file\n");
			printf("add [PATH_TO_FILE] - insert a file into the filesystem\n");
			printf("export [FILENAME]  - exports a file of the filesystem to the designated location\n");
			printf("clear              - \n");
			printf("ls                 - show all the files in the file system\n");
		}else if(strstr(choose, "export") != NULL){
			char nome[100];
			strcpy(nome, get_nome(choose, ' '));
			save = fopen(nome, "wb");
			if(save == NULL){
				printf("deu merda\n");
				exit(-1);
			}
			strcpy(nome, get_nome(choose, '/'));
			export(posan, nome, save);
			fclose(save);
		}else if (strstr(choose, "mkdir") != NULL){
			char nome[25];
			strcpy(nome, get_nome(choose, ' '));
			printf("%s\n", nome);
			subdir(posan, nome);
		}
		else {
			printf("%s: command not found\n", &choose);	
		}
	}
	fclose (posan);
	return 0;
}
