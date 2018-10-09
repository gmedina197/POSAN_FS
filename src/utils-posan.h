#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RESET "\x1b[0m"

int get_cluster(FILE *fat)
{
	unsigned short int entry;
	fseek(fat, 1024, SEEK_SET);
	for (int i = 0; i < 65278; i++)
	{
		fread(&entry, sizeof(entry), 1, fat);
		if (entry == 0)
		{
			return i;
		}
	}
	return -1;
}

int pos_in_fat(int cluster)
{
	return (cluster * 2) + 512;
}

int size(FILE *fp)
{
	fseek(fp, 0L, SEEK_END);
	int s = ftell(fp);
	rewind(fp);
	return s;
}

int exists(const char *fname)
{
	FILE *file;
	if ((file = fopen(fname, "r")))
	{
		fclose(file);
		return 1;
	}
	return 0;
}

const char *get_nome(char *path, int rmv)
{
	char *ret;
	ret = strrchr(path, rmv);
	ret++;
	return ret;
}

int occurrences(char *path)
{
	char *pch;
	pch = strchr(path, ' ');
	int i = 0;
	while (pch != NULL)
	{
		i++;
		pch = strchr(pch + 1, ' ');
	}
	return i;
}

char *first_name(char *path)
{

	const char *PATTERN = " ";

	char *target = NULL;
	char *start, *end;

	if (start = strstr(path, PATTERN))
	{
		start += strlen(PATTERN);
		if (end = strstr(start, PATTERN))
		{
			target = (char *)malloc(end - start + 1);
			memcpy(target, start, end - start);
			target[end - start] = '\0';
		}
	}

	return target;
}

int get_ncluster(FILE *save)
{
	double n = size(save);
	n = n / 512;
	return (int)ceil(n);
}

void array_of_clusters(FILE *fat, unsigned short int *free_clusters, int nclusters)
{
	cif check;
	int cluster = get_cluster(fat) + 256;
	fseek(fat, pos_in_fat(cluster), SEEK_SET);

	int cont = 0;
	unsigned short int free_cluster = cluster;

	while (cont < nclusters)
	{
		fread(&check, sizeof(check), 1, fat);
		if (check.cluster == 0)
		{
			free_clusters[cont] = free_cluster;
			cont++;
			free_cluster++;
		}
		else
		{
			free_cluster++;
		}
	}
}

void fill_fat(FILE *fat, FILE *save, int cluster, int nclusters, unsigned short int *arr)
{
	cif check;
	unsigned short int mark = 0xFFFF;

	fseek(fat, pos_in_fat(cluster), SEEK_SET);
	for (int i = 0; i < nclusters - 1; i++)
	{
		unsigned short int next_cluster = arr[i + 1];
		fwrite(&next_cluster, sizeof(next_cluster), 1, fat);
		if (arr[i] + 1 != next_cluster)
			fseek(fat, pos_in_fat(next_cluster), SEEK_SET);
	}
	fwrite(&mark, sizeof(mark), 1, fat);
}

void copy_content(FILE *fat, FILE *save, int cluster, int nclusters, unsigned short int *arr)
{
	char reader, fill = 0;
	int complete_cluster = 0;
	int cont = 0;
	fseek(fat, cluster * 512, SEEK_SET);
	while (!feof(save))
	{
		complete_cluster++;
		if (complete_cluster == 511)
		{
			cont++;
			fseek(fat, arr[cont] * 512, SEEK_SET);
			complete_cluster = 0;
		}
		fread(&reader, sizeof(reader), 1, save);
		fwrite(&reader, sizeof(reader), 1, fat);
	}
	for (int i = 0; i < 512 - complete_cluster; i++)
		fwrite(&fill, sizeof(fill), 1, fat);
}

void copy_file(FILE *fat, FILE *save, char *name)
{
	int cluster = get_cluster(fat) + 256;
	int nclusters = get_ncluster(save);
	unsigned short int arr[nclusters];
	array_of_clusters(fat, arr, nclusters);

	fill_fat(fat, save, cluster, nclusters, arr);

	//criando diretorio----------------------------------------------------------------------
	directory rd;
	for (int i = 0; i < 25; i++)
	{
		if (i < strlen(name))
		{
			rd.filename[i] = name[i];
		}
		else
			rd.filename[i] = 0x00;
	}
	rd.attribute = 1;
	rd.initial_cluster = cluster;
	rd.size_file = size(save);

	fseek(fat, 131584, SEEK_SET);
	directory dir;
	while (1)
	{
		fread(&dir, sizeof(dir), 1, fat);
		if (strcmp(dir.filename, "") == 0)
		{
			fseek(fat, ftell(fat) - 32, SEEK_SET);
			break;
		}
	}

	fwrite(&rd, sizeof(rd), 1, fat);

	copy_content(fat, save, cluster, nclusters, arr);
}

int get_c(int s)
{
	double n = s;
	n = n / 512;
	return (int)ceil(n);
}

directory get_dir_name(FILE *posan, char *name)
{
	struct directory dir;
	int cont = 0;
	while (1)
	{
		if (cont == 16)
			break;
		fread(&dir, sizeof(dir), 1, posan);
		if (strcmp(dir.filename, name) == 0)
		{
			break;
		}
		cont++;
	}
	return dir;
}

void get_content(FILE *posan, FILE *save, directory dir)
{
	unsigned char info;
	int nclusters = get_c(dir.size_file);
	unsigned short int clusters[nclusters];
	cif check;

	fseek(posan, pos_in_fat(dir.initial_cluster), SEEK_SET);

	for (int i = 0; i < nclusters; i++)
	{
		fread(&check, sizeof(check), 1, posan);
		clusters[i] = check.cluster;
		fseek(posan, pos_in_fat(check.cluster), SEEK_SET);
	}

	int cont = -1;

	while (cont < nclusters)
	{
		if (cont == -1)
			fseek(posan, dir.initial_cluster * 512, SEEK_SET);
		else
			fseek(posan, clusters[cont] * 512, SEEK_SET);

		for (int j = 0; j < 512; j++)
		{
			fread(&info, sizeof(info), 1, posan);
			if (info != 0)
				fwrite(&info, sizeof(info), 1, save);
		}
		cont++;
	}
}

void export_file(FILE *posan, char *name, FILE *save)
{
	fseek(posan, 131616, SEEK_SET); //pula o root dir
	directory dir = get_dir_name(posan, name);
	printf("%s\n", dir.filename);

	get_content(posan, save, dir);
}

void export_dir(FILE *posan, char *name, char *dir_name, FILE *save)
{
	fseek(posan, 131616, SEEK_SET); //pula o root dir
	directory dir = get_dir_name(posan, dir_name), file_name;
	cif check;
	unsigned char info;

	fseek(posan, dir.initial_cluster * 512, SEEK_SET);
	file_name = get_dir_name(posan, name);
	printf("%s", file_name.filename);

	get_content(posan, save, file_name);
}

void formatar(FILE *fat)
{
	boot_sec(fat);
	make_fat(fat);
	make_dir(fat);
}

void hard_format(FILE *fat)
{
	unsigned char zero = 0;
	printf("%d\n", size(fat));
	for (int i = 0; i < size(fat); i++)
	{
		fwrite(&zero, sizeof(zero), 1, fat);
	}
	boot_sec(fat);
	make_fat(fat);
	make_dir(fat);
}

void listar(FILE *fat)
{
	fseek(fat, 131584, SEEK_SET);
	directory list;
	while (1)
	{
		fread(&list, sizeof(list), 1, fat);
		if (strcmp(list.filename, "") == 0)
			break;
		if (list.attribute == 1 && list.filename[0] != 0xe5)
			printf("%s\n", list.filename);
		else if (list.filename[0] != 0xe5)
			printf(ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET "\n", list.filename);
	}
}

void listar_dir(FILE *fat, char *nome_dir)
{
	fseek(fat, 131584, SEEK_SET);
	directory subdir, list;
	int cont = 0;
	while (1)
	{
		if (cont == 16)
		{
			printf("Subdirectory does not found\nPlease type a valid name\n");
			break;
		}
		fread(&subdir, sizeof(subdir), 1, fat);
		if (strcmp(subdir.filename, nome_dir) == 0)
		{
			break;
		}
		cont++;
	}
	fseek(fat, subdir.initial_cluster * 512, SEEK_SET);

	while (1)
	{
		fread(&list, sizeof(list), 1, fat);
		if (list.filename[0] == 0)
			break;
		if (list.attribute == 1 && list.filename[0] != 0xe5)
			printf("%s\n", list.filename);
	}
}

void subdir(FILE *posan, char *name)
{
	//marcando na fat -----------------------------------------
	int cluster = get_cluster(posan) + 256;
	fseek(posan, pos_in_fat(cluster), SEEK_SET);
	unsigned short int mark = 0xFFFF;
	fwrite(&mark, sizeof(mark), 1, posan);
	//marcando no subdiretorio---------------------------------
	directory rd;
	for (int i = 0; i < 25; i++)
	{
		if (i < strlen(name))
		{
			rd.filename[i] = name[i];
		}
		else
			rd.filename[i] = 0x00;
	}
	rd.attribute = 2;
	rd.initial_cluster = cluster;
	rd.size_file = 512;

	fseek(posan, 131584, SEEK_SET);
	directory dir;
	while (1)
	{
		fread(&dir, sizeof(dir), 1, posan);
		if (strcmp(dir.filename, "") == 0)
		{
			fseek(posan, ftell(posan) - 32, SEEK_SET);
			break;
		}
	}

	fwrite(&rd, sizeof(rd), 1, posan);
	fseek(posan, 512 * cluster, SEEK_SET);
	unsigned char entry = 0;
	for (int i = 0; i < 512; i++)
		fwrite(&entry, sizeof(entry), 1, posan);
}

void fill_subdir(FILE *posan, FILE *save, char *name, char *name2)
{
	fseek(posan, 131616, SEEK_SET); //pula o root dir
	directory dir, subdir;
	unsigned char entry = 0;
	int cont = 0;
	while (1)
	{
		if (cont == 16)
		{
			printf("Subdirectory not found\nPlease type a name that is valid\n");
			break;
		}
		fread(&dir, sizeof(dir), 1, posan);
		if (strcmp(dir.filename, name) == 0 && dir.attribute == 2)
		{
			break;
		}
		cont++;
	}
	//marcando fat
	int cluster = get_cluster(posan) + 256;
	int nclusters = get_ncluster(save);
	unsigned short int arr[nclusters];
	array_of_clusters(posan, arr, nclusters);

	fill_fat(posan, save, cluster, nclusters, arr);

	//marcando subdir
	fseek(posan, dir.initial_cluster * 512, SEEK_SET);

	for (int i = 0; i < 25; i++)
	{
		if (i < strlen(name2))
		{
			subdir.filename[i] = name2[i];
		}
		else
			subdir.filename[i] = 0x00;
	}
	subdir.attribute = 1;
	subdir.initial_cluster = cluster;
	subdir.size_file = size(save);

	directory sdir;
	while (1)
	{
		fread(&sdir, sizeof(sdir), 1, posan);
		if (strcmp(sdir.filename, "") == 0)
		{
			fseek(posan, ftell(posan) - 32, SEEK_SET);
			break;
		}
	}

	fwrite(&subdir, sizeof(subdir), 1, posan);
	//copiando conteudo
	copy_content(posan, save, cluster, nclusters, arr);
}

void update_fat(FILE *posan, directory list)
{
	fseek(posan, pos_in_fat(list.initial_cluster), SEEK_SET);
	unsigned short int mark = 0x00;
	cif check;

	while (fread(&check, sizeof(check), 1, posan))
	{
		fseek(posan, ftell(posan) - sizeof(check), SEEK_SET);
		fwrite(&mark, sizeof(mark), 1, posan);
		fseek(posan, pos_in_fat(check.cluster), SEEK_SET);
		if (check.cluster == 0xFFFF || check.cluster == 0)
			break;
	}
}

void remove_file(FILE *posan, char *dir)
{
	int desloc = 131584, cont = 0;
	directory list;
	unsigned short rem = 0xE5;

	fseek(posan, desloc, SEEK_SET);

	while (1)
	{
		if (cont == 16)
		{
			printf("Archive does not found\nPlease type a name that is valid\n");
			break;
		}
		fread(&list, sizeof(list), 1, posan);
		if (strcmp(list.filename, dir) == 0)
		{
			fseek(posan, ftell(posan) - 32, SEEK_SET);
			fwrite(&rem, sizeof(rem), 1, posan);
			break;
		}
		cont++;
	}
	update_fat(posan, list);
}

void remove_from_subdir(FILE *posan, char *dir)
{
	directory list, subdir;
	unsigned short rem = 0xE5;

	if (list.attribute == 2)
	{
		fseek(posan, list.initial_cluster * 512, SEEK_SET);
		while (1)
		{
			fread(&subdir, sizeof(subdir), 1, posan);
			if (subdir.filename[0] == 0)
				break;
			else
			{
				fseek(posan, ftell(posan) - 32, SEEK_SET);
				fwrite(&rem, sizeof(rem), 1, posan);
				fseek(posan, ftell(posan) + 30, SEEK_SET);
				update_fat(posan, subdir);
			}
			printf("%s\n", subdir.filename);
		}
	}
}