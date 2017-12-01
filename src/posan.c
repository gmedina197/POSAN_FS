#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "data_structs.h"
#include "make_file.h"

#define clear() printf("\033[H\033[J")
int cont = 258;

int get_cluster(FILE* fat){
	unsigned short int entry;
    fseek(fat,1539,SEEK_SET);
    for(int i=0;i < 65278;i++){
        fread(&entry, sizeof(entry), 1, fat);
        if(entry == 0){
            return i;
        }
    }
    return -1;
}

int size(FILE* fp){
	fseek(fp, 0L, SEEK_END);
	int s = ftell(fp);
	rewind(fp);
	return s;
}

const char* get_nome(char* path, char rmv){
	char barra ='/';
	char *ret;
    ret = strrchr(path, rmv);          //procura a ultima ocorrencia do caracter barra
    ret++; //"pula" a barra
    strncpy(path, ret,25);
    return path;
}

int get_ncluster(FILE* save){
	double n = size(save);
	n = n/512;
	return (int)ceil(n);
}

void copy_file(FILE *fat, FILE *save, int cluster, char *name){
	//marcando a FAT
	cluster += cont++;
	fseek(fat, (cluster*2)+512, SEEK_SET);
	unsigned short int mark = 0xFFFF, fillc = 0x01;

	fwrite(&mark, sizeof(mark), 1, fat); 

	int nclusters = get_ncluster(save);
	if (nclusters > 1){
		printf("entrou\n");
		for (int i = 0; i < nclusters; i++){
			fwrite(&fillc, sizeof(fillc), 1, fat);
		}
	}	

	//criando diretorio------------------------------------------------------------------------------------
	directory rd;
	for (int i = 0; i < 25; i++){
		if (i < strlen(name)){
			rd.filename[i] = name[i];
		}else
			rd.filename[i] = 0x00;
	}
	rd.attribute = 1;
	rd.initial_cluster = cluster;
	rd.size_file = size(save);

	fseek(fat, 131584, SEEK_SET);
	directory dir;
	while(1){
		fread(&dir, sizeof(dir),1,fat);
		if (strcmp(dir.filename, "") == 0){
			fseek(fat, ftell(fat)-32, SEEK_SET);
			break;
		}
	}

	fwrite(&rd, sizeof(rd), 1, fat);
	
	//copiando o conteudo---------------------------------------------------------------------------------
	fseek(fat, (cluster) * 512, SEEK_SET);
	char reader, fill = 0;
	int cont = 0;
	while (!feof(save)){
		fread(&reader, sizeof(reader), 1, save);
		fwrite(&reader, sizeof(reader),1, fat);
		cont++;
		if (cont >= 512) cont = 0;
	}
	for (int i = 0; i < 512 - cont; i++)
		fwrite(&fill, sizeof(fill),1, fat);
}

void formatar(FILE *fat){
	boot_sec(fat);		
	make_fat(fat);		
	make_dir(fat);		
}

void listar(FILE *fat){
	fseek(fat, 131584, SEEK_SET);
	directory list;
	while(1){
		fread(&list, sizeof(list),1,fat);
		if (strcmp(list.filename, "") == 0) break;
		printf("%s\n", list.filename);
	}
}

int main(int argc, char **argv){
	FILE *posan, *save;
	int flag = 0;
    char choose[100];
	//if (access("posan.img", F_OK) != -1) flag = 1;

	posan = fopen("posan.img", "wb");
	if (posan == NULL){
		printf ("Erro!");
		exit(1);
	}

	if (flag == 0){
		boot_sec(posan);		//gerando boot sector
		make_fat(posan);		//gerando FAT
		make_dir(posan);		//gerando root dir
	}
	fclose(posan);
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
			strcpy(nome, get_nome(choose, ' '));
			save = fopen(choose, "rb");
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
			printf("quit             - close the program and generate the .img file\n");
			printf("format           - format the FAT table and the directories in the file\n");
			printf("add PATH_TO_FILE - insert a file into the filesystem\n");
			printf("clear            - \n");
			printf("ls               - show all the files in the file system\n");
		}	

		else {
			printf("%s: command not found\n", &choose);	
		}
	}
	fclose (posan);
	return 0;
}
