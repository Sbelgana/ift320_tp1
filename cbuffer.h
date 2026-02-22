#ifndef __CBUFFER_H
# define __CBUFFER_H
#ifdef __KERNEL__
# include <linux/types.h>
#else
# define u8 unsigned char
#include <stdlib.h>
#endif


//IFT320 :   'cbuffer'  signifie Circular Buffer (ou tampon circulaire).
/*
	Définition de la structure cbuffer. 
*/	
    struct cbuffer {
        u8 *data;		//Pointeur vers les données conservées dans le tampon
		int max;		//Quantité maximale de données qu'on peut conserver à la fois
		int contains;	//Quantité de données actuellement conservées dans le tampon
		int head;		//Tête: position où se trouve la prochaine donnée à défiler.
		int tail;		//Queue: position où on place la prochaine donnée à enfiler.
};
	
//Initialisation: réserve l'espace demandé et prépare les membres de la structure	
void cb_init(struct cbuffer* buf,int size);
//Nettoyage: Libère l'espace réservé.
void cb_clean(struct cbuffer* buf);
//Enfiler: place un caractère dans le tampon à la position Queue (tail).
void cb_push(struct cbuffer* buf,char val);
//Défiler: retourne le caractère en position Tête (head).
char cb_pop(struct cbuffer* buf);
//EstVide: vérifie si le tampon est vide.
int cb_empty(struct cbuffer* buf);
//EstPlein: vérifie si le tampon est plein.
int cb_full(struct cbuffer* buf);


#endif /* __CBUFFER_H */
