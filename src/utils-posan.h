#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

int get_cluster(FILE* fat){
	unsigned short int entry;
    fseek(fat,1024,SEEK_SET);
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

int exists(const char *fname){
    FILE *file;
    if ((file = fopen(fname, "r"))){
        fclose(file);
        return 1;
    }
    return 0;
}

const char* get_nome(char* path, int rmv){
	char *ret;
    ret = strrchr(path, rmv);
    ret++;
    return ret;
}

int occurrences(char *path){
	char *pch;
	pch = strchr(path, ' ');
	int i = 0;
	while (pch != NULL){
		i++;
		pch = strchr(pch+1, ' ');
	}
	return i;
}

char* first_name(char* path){

    const char *PATTERN = " ";

    char *target = NULL;
    char *start, *end;

    if (start = strstr(path, PATTERN)){
        start += strlen( PATTERN );
        if (end = strstr(start, PATTERN)){
            target = ( char * )malloc( end - start + 1 );
            memcpy( target, start, end - start );
            target[end - start] = '\0';
        }
    }

	return target;
}

int get_ncluster(FILE* save){
	double n = size(save);
	n = n/512;
	return (int)ceil(n);
}

void copy_file(FILE *fat, FILE *save, int cluster, char *name){
	//marcando a FAT
	cluster = get_cluster(fat) + 256;
	fseek(fat, (cluster*2)+512, SEEK_SET);
	unsigned short int mark = 0xFFFF, fillc = 0x01;

	fwrite(&mark, sizeof(mark), 1, fat); 

	int nclusters = get_ncluster(save);
	if (nclusters > 1){
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

void export_file(FILE* posan, char* name, FILE* save){
	fseek(posan, 131616, SEEK_SET); //pula o root dir
	directory dir;
	unsigned char entry = 0;
	while (1){
		fread(&dir, sizeof(dir), 1, posan);
		if (strcmp(dir.filename, name) == 0) break;
	}

	double n = dir.size_file/512;
	int clusters = (int)ceil(n)+1;

	fseek(posan, 512 * dir.initial_cluster, SEEK_SET);
	unsigned char info;
	for (int i = 0; i < dir.size_file; i++){
		fread(&info, sizeof(info),1,posan);
		fwrite(&info, sizeof(info), 1, save);
	}
}

void formatar(FILE *fat){
	boot_sec(fat);		
	make_fat(fat);		
	make_dir(fat);		
}

void hard_format(FILE *fat){
	boot_sec(fat);		
	make_fat(fat);		
	make_dir(fat);
	unsigned char zero = 0;
	printf("%d\n", size(fat));
	for (int i = 0; i < size(fat); i++){
		fwrite(&zero, sizeof(zero), 1, fat);
	}		
}

void listar(FILE *fat){
	fseek(fat, 131584, SEEK_SET);
	directory list;
	while(1){
		fread(&list, sizeof(list),1,fat);
		if (strcmp(list.filename, "") == 0) break;
		if (list.attribute == 1)
			printf("%s\n", list.filename);
		else
			printf(ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET "\n", list.filename);
	}
}

void listar_dir(FILE *fat, char* nome_dir){
	fseek(fat, 131584, SEEK_SET);
	directory subdir, list;
	while(1){
		fread(&subdir, sizeof(subdir), 1, fat);
		if(strcmp(subdir.filename, nome_dir) == 0){
			break;
		}
	}
	fseek(fat, subdir.initial_cluster * 512, SEEK_SET);
	
	while(1){
		fread(&list, sizeof(list),1,fat);
		if (strcmp(list.filename, "") == 0) break;
		if (list.attribute == 1)
			printf("%s\n", list.filename);
		else
			printf(ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET "\n", list.filename);
	}

}

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
	fseek(posan, 512 * cluster, SEEK_SET);
	unsigned char entry = 0;
	for (int i = 0; i < 512; i++)
		fwrite(&entry, sizeof(entry), 1 , posan);
}

void fill_subdir(FILE* posan, FILE* save, char *name, char *name2){
	fseek(posan, 131616, SEEK_SET); //pula o root dir
	directory dir, subdir;
	unsigned char entry = 0;
	int cont = 0;
	while (1){
		if (cont == 16) {
			printf("Subdirectory not found\nPlease type a name that is valid\n");
			break;
		}
		fread(&dir, sizeof(dir), 1, posan);
		if (strcmp(dir.filename, name) == 0 && dir.attribute == 2){
			break;
		} 
		cont++;
	}
	//marcando fat
	int cluster = get_cluster(posan) + 256;
	fseek(posan, (cluster*2)+512, SEEK_SET);
	unsigned short int mark = 0xFFFF, fillc = 0x01;

	fwrite(&mark, sizeof(mark), 1, posan); 

	int nclusters = get_ncluster(save);
	if (nclusters > 1){
		for (int i = 0; i < nclusters; i++){
			fwrite(&fillc, sizeof(fillc), 1, posan);
		}
	}	

	//marcando subdir
	fseek(posan, (512 * dir.initial_cluster), SEEK_SET);

	for (int i = 0; i < 25; i++){
		if (i < strlen(name2)){
			subdir.filename[i] = name2[i];
		}else
			subdir.filename[i] = 0x00;
	}
	subdir.attribute = 1;
	subdir.initial_cluster = cluster;
	subdir.size_file = size(save);
	
	directory sdir;
	while(1){
		fread(&sdir, sizeof(sdir),1,posan);
		if (strcmp(sdir.filename, "") == 0){
			fseek(posan, ftell(posan)-32, SEEK_SET);
			break;
		}
	}

	fwrite(&subdir, sizeof(subdir), 1, posan);
	//copiando conteudo

	fseek(posan, (subdir.initial_cluster) * 512, SEEK_SET);
	char reader, fill = 0;
	cont = 0;
	while (!feof(save)){
		fread(&reader, sizeof(reader), 1, save);
		fwrite(&reader, sizeof(reader),1, posan);
		cont++;
		if (cont >= 512) cont = 0;
	}
	for (int i = 0; i < 512 - cont; i++)
		fwrite(&fill, sizeof(fill),1, posan);

}