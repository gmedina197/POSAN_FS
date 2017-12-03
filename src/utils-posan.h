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

int get_ncluster(FILE* save){
	double n = size(save);
	n = n/512;
	return (int)ceil(n);
}

void copy_file(FILE *fat, FILE *save, int cluster, char *name){
	//marcando a FAT
	cluster = get_cluster(fat) + 256;
	printf("%d\n", cluster);
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