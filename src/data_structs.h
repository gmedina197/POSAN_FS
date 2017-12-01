typedef struct boot_sector{
	unsigned short 	bytes_per_sector;
	unsigned char 	sector_per_clusters;
	unsigned char 	num_fats;
	unsigned int 	total_sectors;
	unsigned short 	sector_per_fat;
	unsigned short 	free_clusters;
	unsigned short	reserved_sectors;
}__attribute__((packed)) boot_sector;

typedef struct directory{
	unsigned char 	filename[25];
	unsigned char 	attribute;
	unsigned short 	initial_cluster;
	unsigned int 	size_file;
}__attribute__((packed)) directory;

typedef struct reader{
	unsigned char r;
}__attribute__((packed)) reader_t;