/*
Main program for the virtual memory project.
Make all of your modifications to this file.
You may add or rearrange any code or data as you need.
The header files page_table.h and disk.h explain
how to use the page table and disk interfaces.
*/

#include "page_table.h"
#include "disk.h"
#include "program.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

//Funciones utiles:
	//Chequeo de espacio libre en memoria fisica:
int check_first_free_space(struct page_table *pt){

}

//Handlers:
void fifo_page_fault_handler( struct page_table *pt, int page )
{
	int *a=1;
	int *b=0;	
	printf("page fault on page #%d\n",page);
	printf("using fifo handler for missing page...\n");
	/*page_table_set_entry(pt, page, page, 1); */
	exit(1);
}
void random_page_fault_handler( struct page_table *pt, int page )
{
	printf("page fault on page #%d\n",page);
	printf("using lru handler for missing page...\n");
	exit(1);
}
void custom_page_fault_handler( struct page_table *pt, int page )
{
	printf("page fault on page #%d\n",page);
	printf("using custom handler for missing page...\n");
	exit(1);
}

int main( int argc, char *argv[] )
{
	if(argc!=5) {
		/* Add 'random' replacement algorithm if the size of your group is 3 */
		printf("use: virtmem <npages> <nframes> <random|fifo|custom> <sort|scan|focus>\n");
		return 1;
	}

	int npages = atoi(argv[1]);
	int nframes = atoi(argv[2]);
	const char *program = argv[4];
	const char *handler = argv[3];

	struct disk *disk = disk_open("myvirtualdisk",npages);
	if(!disk) {
		fprintf(stderr,"couldn't create virtual disk: %s\n",strerror(errno));
		return 1;
	}

	/* Ahora separamos la creación de la tabla de paginas dependiendo del handler indicado */
	struct page_table *pt;
	if (!strcmp(handler,"fifo")){
		pt = page_table_create( npages, nframes, fifo_page_fault_handler );
	}
	if (!strcmp(handler,"random")){
		pt = page_table_create( npages, nframes, lru_page_fault_handler );
	}
	if (!strcmp(handler,"custom")){
		pt = page_table_create( npages, nframes, custom_page_fault_handler );
	}
	/* Hasta aca */

	if(!pt) {
		fprintf(stderr,"couldn't create page table: %s\n",strerror(errno));
		return 1;
	}

	char *virtmem = page_table_get_virtmem(pt);

	char *physmem = page_table_get_physmem(pt);

	if(!strcmp(program,"sort")) {
		sort_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"scan")) {
		scan_program(virtmem,npages*PAGE_SIZE);

	} else if(!strcmp(program,"focus")) {
		focus_program(virtmem,npages*PAGE_SIZE);

	} else {
		fprintf(stderr,"unknown program: %s\n",argv[3]);

	}

	page_table_print(pt);

	page_table_delete(pt);
	disk_close(disk);

	return 0;
}
