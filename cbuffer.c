#ifdef __KERNEL__ 
# include <linux/slab.h>
#else
# define kmalloc(a,b) malloc(a)
# define kfree(a) free(a)
# define spin_lock(l)
# define spin_unlock(l)
# define spin_lock_irqsave(l, f)
# define spin_unlock_irqrestore(l, f)
# define ENOMEM 1
#endif

#include "cbuffer.h"

//IFT320 :   'cbuffer'  signifie  "Circular Buffer"

//IFT320 : Codez le corps de toutes les fonctions du tampon circulaire.

/*
Initialisation de la structure.
	Paramètres
		buf : la structure à initialiser
		size: la quantité maximale de caractères que le tampon peut contenir.
*/	         
void cb_init(struct cbuffer* buf,int size){
	
	
	
}

/*
Libération de la structure.
	Paramètres
		buf: la structure à libérer
*/
void cb_clean(struct cbuffer* buf){
	
}

/*
Enfiler un caractère.
Paramètres
	buf: la structure cbuffer utilisée
	val: la valeur du caractère à enfiler.
*/

void cb_push(struct cbuffer* buf,char val){
	
	
}
/*
Défiler un caractère.
Paramètres
	buf: la structure cbuffer utilisée
	
Retour
	la valeur en tête du tampon.
*/
char cb_pop(struct cbuffer* buf){
	
	
	return 0;
}

/*
Vérifier si le tampon est vide.
Paramètres
	buf: la structure cbuffer utilisée
	
Retour
	 0 (faux): le tampon n'est pas vide
	 1 (vrai): le tampon est vide
*/	 
int cb_empty(struct cbuffer* buf){
	
	return 0;
}

/*
Vérifier si le tampon est plein.
Paramètres
	buf: la structure cbuffer utilisée
	
Retour
	 0 (faux): le tampon n'est pas plein
	 1 (vrai): le tampon est plein
*/	
int cb_full(struct cbuffer* buf){
	
	return 0;
}
