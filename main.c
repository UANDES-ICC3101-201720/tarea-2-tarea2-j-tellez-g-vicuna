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
int nframes;

//Creamos las linked list con raiz iniciada en null:
struct nodo{
	int frame;
	int page;
	struct nodo *next;
};
struct nodo *raiz = NULL;

//Funciones de linked list:
//Eliminar raiz:
void eliminar_raiz(struct nodo **raiz){
	struct nodo * next_node = NULL;
	next_node = (*raiz)->next;
	free(*raiz);
	*raiz = next_node;
}
//Eliminar nodo especifico:
void eliminar_nodo(struct nodo ** raiz, int index){
	struct nodo * current = *raiz;
	struct nodo * temp = NULL;
	if (index ==0)eliminar_raiz(raiz);
	else{
		for(int i = 0; i < index-1; i++){
			current = current->next;
		}
		temp = current->next;
		current->next = temp->next;
		free(temp);
	}
}
//Eliminar ultimo nodo:
void eliminar_ultimo_nodo(struct nodo * raiz, int page, int frame){
	struct nodo * current = raiz;
	while(current->next != NULL){
		current = current->next;
	}
	current->next = malloc(sizeof(struct nodo));
	current->next->page = page;
	current->next->frame = frame;
	current->next->next = NULL;
}

//Handlers:



void custom_page_fault_handler( struct page_table *pt, int page ){
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
	nframes = atoi(argv[2]);
	const char *program = argv[4];
	const char *handler = argv[3];

	for (int i=0; i<nframes; i++){
		iframe[i]=-1;
	}

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
	disk_read(disk,99,&data);
	printf("%c",data);

	page_table_delete(pt);
	disk_close(disk);

	return 0;
}
