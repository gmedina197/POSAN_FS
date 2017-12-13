#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "data_structs.h"
#include "make_file.h"
#include "utils-posan.h"

#define clear() printf("\033[H\033[J")

void update_fat(FILE* posan, directory list){
	//marcando a fat -----------------------------------------------
	long int save = ftell(posan);
	int cluster = list.initial_cluster;
	fseek(posan, (cluster*2)+512, SEEK_SET);
	unsigned short int mark = 0x00;
	fwrite(&mark, sizeof(mark), 1, posan);

	int cl = get_c(list.size_file);

	if (cl > 1){
		for (int i = 0; i < cl; i++){
			fwrite(&mark, sizeof(mark), 1, posan);
		}
	}	
	fseek(posan, save, SEEK_SET);
	// -------------------------------------------------------------
}

void remove_file(FILE* posan, char* dir, int desloc){
	fseek(posan, desloc, SEEK_SET);
	directory list, subdir;
	int cont = 0;
	unsigned short rem = 0xE5;
	while(1){
		if (cont == 16) {
			printf("Archive does not found\nPlease type a name that is valid\n");
			break;
		}
		fread(&list, sizeof(list), 1, posan);
		if (strcmp(list.filename, dir) == 0){
			fseek(posan, ftell(posan) - 32, SEEK_SET);
			fwrite(&rem, sizeof(rem), 1, posan);
			break;
		}
		cont++;
	}
	update_fat(posan, list);
	if(list.attribute == 2){
		fseek(posan, list.initial_cluster * 512, SEEK_SET);
		while(1){
			fread(&subdir, sizeof(subdir), 1, posan);
			if (subdir.filename[0] == 0) break;
			else{
				fseek(posan, ftell(posan) - 32, SEEK_SET);
				fwrite(&rem, sizeof(rem), 1, posan);
				fseek(posan, ftell(posan) + 30, SEEK_SET);
				update_fat(posan, subdir);
			}
			printf("%s\n", subdir.filename);
		}
	}
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
		}else if (strcmp(choose, "hard format") == 0){
			printf("Hard formating...Please wait\n");
			hard_format(posan);
			printf("Done.\n");
		}else if(strstr(choose, "add") != NULL){
			char nome[100], backup[100];
			strncpy(nome, get_nome(choose, ' '), sizeof(nome));
			
			save = fopen(nome, "rb");
			if(save == NULL){
				printf("file cannot be open\n");
				exit(-1);
			}
			strcpy(nome, get_nome(choose, '/'));
			
			if (occurrences(choose) == 2 ){
				//;
				fill_subdir(posan, save, first_name(choose), nome);

			}else{
				copy_file(posan, save, get_cluster(posan), nome);
			}

		}
		else if(strstr(choose, "ls") != NULL){
			char nome[100];
			if (occurrences(choose) == 1){
				strcpy(nome, get_nome(choose, ' '));
				listar_dir(posan, nome);
			}
			else
				listar(posan);
		}
		else if(strstr(choose, "rm") != NULL){
			char nome[100], backup[100];
			strncpy(nome, get_nome(choose, ' '), sizeof(nome));
			
			if (occurrences(choose) == 2 ){
				printf("remove arquivo dentro de diretorio\n");
				
			}else{
				remove_file(posan, nome, 131584);
			}
		}
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
				printf("file cannot be open\n");
				exit(-1);
			}
			strcpy(nome, get_nome(choose, '/'));
			export_file(posan, nome, save);
			fclose(save);
		}else if (strstr(choose, "mkdir") != NULL){
			char nome[25];
			strcpy(nome, get_nome(choose, ' '));
			subdir(posan, nome);
		}
		else {
			printf("%s: command not found\n", &choose);	
		}
	}
	fclose (posan);
	return 0;
}
