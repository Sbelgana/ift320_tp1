/**
 * chat.c
 * Ce fichier contient le code du programme chat. Le programme
 * chat est un simple programme qui part 2 threads, un pour lire et un pour
 * ecrire dans le fichier representant le peripherique.
 */

#define _GNU_SOURCE
#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 80

/* File Descriptor du device */
int file = 0;
char *w_line = NULL;

/* Thread du lecteur */
void *reader(void *ptr)
{
    printf("Starting reader...\n");
	char r_line[BUFFER_SIZE + 1];
    size_t line_size;
    while (true) {
       line_size = read(file,r_line,BUFFER_SIZE);
       if (line_size < 0)
           exit(1);
       r_line[line_size]='\0';	   
       printf("%s",r_line);
    }
    return NULL;
}

/* Thread de l'ecriveur */
void writer() 
{
    size_t buf_size = 0;
    size_t line_size;
    const char * quit_str = "quit\n";
    while (true) {
        line_size = getline(&w_line,&buf_size,stdin);
        write(file,w_line, line_size);
        if (!strcmp(quit_str,w_line)) 
            return;
    }
}

/* Fermeture des fichiers et liberation de la memoire. */
void clean(void)
{
    if (file)
        close(file);
    if (w_line)
        free(w_line);
    file = 0;
    w_line = NULL;
}

/* Fonction principale */
int main(int argc,char **argv)
{
    pthread_t reader_thread;
    atexit(clean);

    /* Check les parametres */
    if (argc < 2) {
        printf("Usage: chat <nodeName>\n");
        exit(1);
    }

    /* Ouverture du fichier passe en parametre. */
    file = open(argv[1],O_RDWR);
    if (!file) {
        printf ("Unable to open file '%s'.\n",argv[1]);
        exit(1);
    }

    /* Creation du thread de lecture. */
    pthread_create(&reader_thread,NULL,reader,NULL);
    /* Execution de la fonction d'ecriture dans ce thread. */
    writer();
    
    pthread_cancel(reader_thread);
    
    return 0;
}

