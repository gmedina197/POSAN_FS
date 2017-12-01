void boot_sec(FILE *fat){
	boot_sector bs;
	bs.bytes_per_sector = 	512;
	bs.sector_per_clusters =  1;
	bs.num_fats = 			  1;
	bs.total_sectors =    65536;
	bs.sector_per_fat =     256;
	bs.free_clusters =    65278;
	bs.reserved_sectors =   258;
	fwrite(&bs, sizeof(bs), 1, fat);
	char fill = 0;
	for (int i = 0; i < 498; i++)
		fwrite(&fill, sizeof(fill), 1, fat);

}

//2 para setor reservado, 3 para cluster livre e 1 para fim do arquivo 
void make_fat(FILE *fat){ //256 setores * 512 = 131072
	char fill = 0;
	unsigned short int directory = 0xFFFF; 
	unsigned char reserved  = 2;
	fseek (fat, 512, SEEK_SET);
	for (int i = 0; i < 65536; i++){
		if (i < 257)
			fwrite(&reserved, sizeof(reserved),1,fat);
		if (i == 257)
			fwrite(&directory, sizeof(directory),1,fat);
		else
			fwrite(&fill, sizeof(fill), 1, fat);	
		
	}
} 

void make_dir(FILE *fat){
	fseek(fat, 131584, SEEK_SET);
	directory root;
	for (int i = 0; i < 25; i++){
		if (i == 0)
			root.filename[i] = '.';
		else
			root.filename[i] = 0x00;
	}
	root.attribute = 1;
	root.initial_cluster = 258;
	root.size_file = 0;
	fwrite(&root, sizeof(root), 1, fat);
	char fill = 0;
	for (int i = 0; i < 480; i++)
		fwrite(&fill, sizeof(fill), 1, fat);
}