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
int * frame_table;

int frame = 0; //variable que indica el siguiente frame libre.
int iframe = 0; //variable que indica el frame victima para custom handler.

//Contadores de eventos para imprimir en consola:
int counterw=0;
int counterr=0;
int counterf=0;

//Creamos cola para fifo:
int *queue;
int head = -1; //cabeza de la cola (no necesariamente inicio de lista)
int iqueue = -1; //indice que recorre la cola

//Funciones de queue:
//Eliminar valor de queue (retorna el valor eliminado para luego modificar la pagina):
int pop_queue(){
	if (head != -1){
		head++;
		if (head > iqueue) head = iqueue = -1;
		return queue[head];
	}
	return 0;
}
//Agregar valor a queue:
int add_queue(int valor){
	if (iqueue - head != nframes - 1){
		if (head == -1) head = 0;
		iqueue++;
		queue[iqueue] = valor;
		return 1;
	}
	return 0;
}

//Handlers:
//FIFO:
void fifo_page_fault_handler(struct page_table *pt, int page){

	counterf++;

	//En caso de que si pudo agregarse al queue:
	if (add_queue(frame)){

		page_table_set_entry(pt, page, frame, PROT_READ|PROT_WRITE|PROT_EXEC);
		disk_read(disk, page, &physmem[frame*PAGE_SIZE]);
		counterr++;

		frame_table[frame] = page;

		frame++;
	}
	//Si el queue estaba lleno, hace swapping:
	else{

		int vframe = pop_queue(); //Frame victima es el ultimo en ser usado

		disk_write(disk, frame_table[vframe], &physmem[vframe*PAGE_SIZE]);
		disk_read(disk, page, &physmem[vframe*PAGE_SIZE]);
		counterw++;
		counterr++;

		page_table_set_entry(pt, page, vframe, PROT_READ|PROT_WRITE|PROT_EXEC);
		page_table_set_entry(pt, frame_table[vframe], vframe, 0);

		frame_table[vframe] = page;

		add_queue(vframe);
	}
}
//RANDOM:
void random_page_fault_handler(struct page_table *pt, int page){

	counterf++;

	//Si todavia no se completan los frames vamos agregando en orden:
	if (frame < nframes){

		page_table_set_entry(pt, page, frame, PROT_READ|PROT_WRITE|PROT_EXEC);
		disk_read(disk, page, &physmem[frame*PAGE_SIZE]);
		counterr++;

		frame_table[frame] = page;

		frame++;
	}
	//Si ya se lleno la tabla de frames, hace swapping:
	else{

		int vframe = lrand48()%nframes; //Frame victima es uno escogido al azar de todos los frames.

		disk_write(disk, frame_table[vframe], &physmem[vframe*PAGE_SIZE]);
		disk_read(disk, page, &physmem[vframe*PAGE_SIZE]);
		counterw++;
		counterr++;

		page_table_set_entry(pt, page, vframe, PROT_READ|PROT_WRITE|PROT_EXEC);
		page_table_set_entry(pt, frame_table[vframe], vframe, 0);
		
		frame_table[vframe] = page;
	}
	
}

void custom_page_fault_handler(struct page_table *pt, int page){

	counterf++;

	//Si todavia no se completan los frames vamos agregando en orden:
	if (frame < nframes){

		page_table_set_entry(pt, page, frame, PROT_READ|PROT_WRITE|PROT_EXEC);
		disk_read(disk, page, &physmem[frame*PAGE_SIZE]);
		counterr++;

		frame_table[frame] = page;

		frame++;
	}
	//Si ya se lleno la tabla de frames, hace swapping:
	else{

		int vframe = iframe; //Frame victima es el indicado por iframe.
		//iframe aumenta y vuelve al inicio si llega al final:
		iframe++;
		if (iframe==nframes)iframe=0;

		disk_write(disk, frame_table[vframe], &physmem[vframe*PAGE_SIZE]);
		disk_read(disk, page, &physmem[vframe*PAGE_SIZE]);
		counterw++;
		counterr++;

		page_table_set_entry(pt, page, vframe, PROT_READ|PROT_WRITE|PROT_EXEC);
		page_table_set_entry(pt, frame_table[vframe], vframe, 0);
		
		frame_table[vframe] = page;

	}
}


int main( int argc, char *argv[] )
{
	if(argc!=5) {
		/* Add 'random' replacement algorithm if the size of your group is 3 */
		printf("use: virtmem <npages> <nframes> <random|fifo|custom> <sort|scan|focus>\n");
		return 1;
	}

	int npages = atoi(argv[1]);
	nframes = atoi(argv[4]);
	const char *program = argv[3];
	const char *handler = argv[2];
	frame_table = malloc(sizeof(int) * nframes);

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

	//Inicio de la memoria virtual de la tabla de paginas pt
	char *virtmem = page_table_get_virtmem(pt); 
	//Inicio de la memoria fisica  de la tabla de paginas pt
	physmem = page_table_get_physmem(pt);

	queue = malloc(sizeof(int)*10000);

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

	//page_table_print(pt);//

	printf("page faults: %i\nwrites on disk: %i\nreads on disk: %i\nmarcos: %i\n\n", counterf,counterw,counterr,nframes);

	page_table_delete(pt);
	disk_close(disk);

	return 0;
}
