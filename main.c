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
// Variables globales para usar en handlers:
struct disk *disk;
char *physmem;
char *virtmem;

//Funciones utiles:
//Handlers:
//Funcion base que todo handler ejecuta:
int main_page_fault_handler( struct page_table *pt, int page){
	//En caso de que la pagina sea menor a la cantidad de frames simplemente apunta a su numero de frame:
	if (page<=page_table_get_nframes(pt)-1){
		page_table_set_entry(pt, page, page, PROT_READ | PROT_WRITE | PROT_EXEC);
		disk_read(disk,page,&physmem[page*PAGE_SIZE]);
		printf("page saved at frame #%d\n",page);
		return 1;
	}
	//En caso contrario se hace swapping con el algoritmo seleccionado:
	return 0;
}

void fifo_page_fault_handler( struct page_table *pt, int page )
{	
	printf("\npage fault on page #%d\n",page);
	printf("using fifo handler for missing page...\n");

	if (!main_page_fault_handler(pt, page)){
		exit(1);
	}
	disk_write(disk,page,&virtmem[page*PAGE_SIZE]);
}
void random_page_fault_handler( struct page_table *pt, int page )
{
	printf("\npage fault on page #%d\n",page);
	printf("using random handler for missing page...\n");

	if (!main_page_fault_handler(pt,page)){
		exit(1);
	}
	disk_write(disk,page,&virtmem[page*PAGE_SIZE]);
}
void custom_page_fault_handler( struct page_table *pt, int page )
{
	printf("\npage fault on page #%d\n",page);
	printf("using custom handler for missing page...\n");

	if (!main_page_fault_handler(pt,page)){
		exit(1);
	}
	disk_write(disk,page,&virtmem[page*PAGE_SIZE]);
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

	disk = disk_open("myvirtualdisk",npages);
	if(!disk) {
		fprintf(stderr,"couldn't create virtual disk: %s\n",strerror(errno));
		return 1;
	}

	/* Ahora separamos la creación de la tabla de paginas dependiendo del handler indicado */
	struct page_table *pt;
	if (!strcmp(handler,"fifo")){
		pt = page_table_create( npages, nframes, fifo_page_fault_handler );
	}
	else if (!strcmp(handler,"random")){
		pt = page_table_create( npages, nframes, random_page_fault_handler );
	}
	else if (!strcmp(handler,"custom")){
		pt = page_table_create( npages, nframes, custom_page_fault_handler );
	}
	else{
		printf("unknown handler: %s\n",handler);
		exit(1);
	}
	/* Hasta aca */

	if(!pt) {
		fprintf(stderr,"couldn't create page table: %s\n",strerror(errno));
		return 1;
	}

	virtmem = page_table_get_virtmem(pt);

	physmem = page_table_get_physmem(pt);

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
	
	char data;
	disk_read(disk,1,&data);
	printf("%c",data);
	page_table_delete(pt);
	disk_close(disk);

	return 0;
}
