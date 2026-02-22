//  Note:
//  L'exemple de code de base moderne provient de:
//  Derek Molloy's excellent tutorial on creating Linux Kernel Modules:
//    http://derekmolloy.ie/writing-a-linux-kernel-module-part-2-a-character-device/
//  Le reste du pilote est adapté depuis le code construit par Kristian Benoit et Nicolas Dufresne pour le cours IFT320.



/* IFT320: MODIFIEZ UNIQUEMENT LES SECTIONS IDENTIFIÉES */

#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include "serialdriver.h"
#include "cbuffer.h"



//  Définition des métadonnées du module
#define MODULE_NAME "serialdriver"
#define  DEVICE_NAME "serialdriver"
#define  CLASS_NAME  "serialdriver"
MODULE_AUTHOR("IFT320");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("RS232 driver");
MODULE_VERSION("1.0");

//Définition du nom du module noyau, utile pour les journaux noyau.
static char *name = "IFT320";
module_param(name, charp, S_IRUGO);
MODULE_PARM_DESC(name, "The name to display in /var/log/kern.log");


static int    majorNumber; //  Numéro de périphérique, obtenu à l'initialisaton.
static struct class*  serialdriverClass  = NULL; //Structure représentant la classe de périphérique.
static struct device* serialdriverDevice = NULL; //Structure représentant le pilote de ce périphérique.
static DEFINE_MUTEX(ioMutex); //Sémaphore. Protège le pilote contre plusieurs ouvertures concurrentes.

//  Déclarations des fonctions à implanter pour le devoir (services du pilote)
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);


//  Création du lien entre les fonctions à implanter pour le devoir et la structure standard de Linux.
//  Cela indique à Linux quelle fonction appeler pour les chacun des services (open, read, write, release).
static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};


//File d'attente qui sera utilisée lorsque le tampon d'écriture est plein, ou que le tampon de lecture est vide.
//Cette file sera passée en paramètre aux fonctions wait_event_interruptible et wake_up_interruptible.
static DECLARE_WAIT_QUEUE_HEAD(wq);

//Pointeur sur le tampon d'écriture. Il doit être initialisé dans la fonction dev_open.
struct cbuffer* buf_out;
//Pointeur sur le tampon de lecture. Il doit être initialisé dans la fonction dev_open.
struct cbuffer* buf_in;

//Drapeau utilisé pour gérer le cas limite où il y a interruption d'écriture avec un tampon d'écriture vide.
int ready_flag=0;

/*
	Routine d'interruption pour le pilote de ligne série.
	
	IFT320: À compléter. Ici, vous devez diagnostiquer l'interruption (tranmsission, réception, erreur?).
	Plusieurs interruptions peuvent avoir eu lieu en même temps, elles doivent toutes être traitées.
	
	Le diagnostic se fait avec le contenu du registre IIR (Interrupt Identification Register).
	Voir la documentation pour savoir comment correctement déterminer la cause de l'interruption (déjà fait).
	
	Ensuite, faites le traitement approprié pour chacun des cas: transmission, réception.
	
	Si vous devez réveiller des processus en attente, utilisez:
		wake_up_interruptible(&wq);	

	ATTENTION!
	Ceci est une routine d'interruption. Ce n'est pas une fonction normale. Vous ne devez jamais appeler cette fonction.
	Elle sera appelée lorsqu'une interruption surviendra.	
*/
irqreturn_t serialdriver_isr(int irq_no,void* dev_id){

    	
    char type; //Contenu obtenu du IIR. Permettra de déterminer le type d'interruption.
	char data; //Variable pour le caractère reçu ou le caractère à transférer.
	
	do{
		
		//Obtention du contenu du IIR pour diagnostic
		type=inb(COM1_IIR);
		pr_info("Interruption: 0x%x\n",type); //Pour déboguer. Retirez ce message s'il pollue votre journal.
		
		
		
		if(type & 0x02){
			
			/*===============================Section IFT320==========================*/
			/*
			  Cas où le registre THR est vide (transmission). 
			  À surveiller, un cas limite survient lorsque votre tampon d'écriture est également vide.
			*/
			pr_info("THR vide\n");  //Pour déboguer. Retirez ce message s'il pollue votre journal.
			
			//IFT320: à compléter
			
			
			/*===============================Fin Section IFT320=======================*/
			
		}
		else if (type & 0x04){
			
			/*===============================Section IFT320==========================*/
			/*
			  Cas où le registre RBR est plein (réception). 
			  À surveiller, un cas limite survient lorsque votre tampon de lecture est également plein.
			*/	
			
			//Lecture du caractère reçu. ATTENTION: doit absolument être fait lorsqu'on détecte ce type d'interruption.
			data=inb(COM1_RBR); 
			
			pr_info("RBR plein: %c (0x%x)\n",data,(int)data); //Pour déboguer. Retirez ce message s'il pollue votre journal.
			
			//IFT320: à compléter
			
			
			/*===============================Fin Section IFT320=======================*/
			
		}
		else if (type&0x06){
			/*
				Cas où une erreur de transmission s'est produite.
				Le registre LSR doit absolument être lu lorsque ça se produit.
			*/
			pr_info("Erreur de transmission. État de la ligne série:0x%x\n",inb(COM1_LSR));			
		}	
			
	}
	while((type & 0x01) != 0x01); 

    return IRQ_HANDLED;
}



/** 
Service open.
Cette fonction est appelée à chaque fois que le périphérique est ouvert par une application.
Le rôle de cette fonction est d'initialiser correctement la ligne série (vitesse et conditions de transfert)
en plus d'initialiser les tampons de lecture et d'écriture.

 *  @param inodep: pointeur vers l'entrée du système de fichier correspondant au pilote.
 *  @param filep: pointeur vers la structure de fichier représentant le pilote.
*/
static int dev_open(struct inode *inodep, struct file *filep){
    
	
	pr_info("[IFT320] Ouverture du périphérique par l'application...\n");
	
	//  Vérification pour les ouvertures concurrentes. L'ouverture sera refusée si une application utilise déjà le pilote.
    if(!mutex_trylock(&ioMutex)) {
        pr_alert("[IFT320] %s: Périphérique déjà utilisé par un autre processus\n", MODULE_NAME);
        return -EBUSY;
    }
	
	
	/*===============================Section IFT320==========================*/
	
	pr_info("[IFT320] Initialisation des tampons\n");
	/*
	  IFT320: à compléter.
	  Initialisation de vos tampons buf_in et buf_out.
	  Les pointeurs sont NULL à l'origine, vous devez les initialiser d'abord avec la fonction kmalloc.
	  Ensuite, vous pouvez utiliser votre fonction d'initialisation de cbuffer.
	*/
	
	
	
		
	/*
		Le RBR doit être lu à l'initialisation.
		Si un caractère a été reçu avant l'initialisation, alors une interruption 
		sera signalée lorsqu'elles seront activées.
	*/	
    pr_info("[IFT320] Vidange du RBR\n...");
	inb(COM1_RBR); 
	
	
	/*
		IFT320: à compléter.
		La séquence d'initialisation comprend plusieurs étapes. 
		Regardez dans la documentation, ainsi que dans serialdriver.h
		pour déterminer quelles valeurs mettre ici. 
		Utilisez outb pour donner des valeurs de configuration aux divers registres.
		Exemple: outb(valeur, COM1_MCR) écrit une valeur dans le registre de contrôle du modem.
	*/	
	pr_info("[IFT320] Configuration de la vitessse...");
	
	
	pr_info("[IFT320] Configuration de la transmission (8 bits, sans parité)...");	
    
	
	
	pr_info("[IFT320] Activation des interruptions...");	
	ready_flag=0;  //On s'assure ici que le drapeau d'interruption manquée est baissé avant d'activer les interruptions.	
    
    
	pr_info("[IFT320] Désactivation du tampon matériel");
	
	
	pr_info("[IFT320] Déblocage des interruption dans le MCR");
     
	
	pr_info("[IFT320] Fin de l'ouverture du périphérique.\n");
	
	/*===============================Fin Section IFT320=======================*/
	
    return 0;
}

/**
	Service read.
	Cette fonction est appelée lorsque l'application demande une lecture sur la ligne série.
	IMPORTANT: 
	Pour transférer	les caractères reçus vers l'application, il faut utiliser copy_to_user.
	Ne déréférencez jamais le pointeur user_buffer directement!
 
 *  @param filep: Pointeur sur la structure représentant le pilote.
 *  @param user_buffer: Pointeur vers l'espace prévu dans l'application pour recevoir les caractères lus.
 *  @param max_size: La taille de l'espace prévu dans l'application pour recevoir les caractères lus.
 *  @param offset: Position dans user_buffer, l'appplication peut la changer pour avancer dans son espace prévu.
 */
static ssize_t dev_read(struct file *filep, char *user_buffer, size_t max_size, loff_t *offset){
	
    //Quantité de caractères lus lors de cette demande.
	int taille_lue=0;
	
	pr_info("[IFT320] Demande de lecture...\n"); //Pour déboguer. Retirez ce message s'il pollue votre journal.
	
	
	/*===============================Section IFT320==========================*/
	/*
		IFT320: à compléter
		Ici, vous devez vérifier si de nouveaux caractères sont arrivés.
		Pour mettre cette demande de lecture en attente, vous allez devoir utiliser:
		
			wait_event_interruptible(wq, CONDITION);
		
		où CONDITION est une condition qui doit être vraie avant que l'on puisse continuer.
		La condition est revérifiée lorsque vous appelez wake_up_interruptible (à faire uniquement dans la routine d'interruption).
	*/
			
   
	
	
	pr_info("[IFT320] Fin de lecture, caractères reçus: %d\n",taille_lue); //Pour déboguer. Retirez ce message s'il pollue votre journal.
	
	//La quantité de caractères lus doit être retournée. Assurez-vous que ce n'est pas 0 quand vous en avez reçu.
	return taille_lue;  
	
	/*===============================Fin Section IFT320=======================*/
   
}

/** 
	Service write.
	Cette fonction est appelée lorsque l'application demande une écriture sur la ligne série.
	IMPORTANT: 
	Pour récupérer les caractères à écrire depuis l'application, il faut utiliser copy_from_user.
	Ne déréférencez jamais le pointeur user_buffer directement!
 *  @param filep: Pointeur sur la structure représentant le pilote.
 *  @param user_buffer: Pointeur vers l'espace prévu dans l'application pour recevoir les caractères lus.
 *  @param max_size: La taille de l'espace prévu dans l'application pour recevoir les caractères lus.
 *  @param offset: Position dans user_buffer, l'appplication peut la changer pour avancer dans son espace prévu.
 */
static ssize_t dev_write(struct file *filep, const char *user_buffer, size_t max_len, loff_t *offset){
    
    
	char toSend; //Variable pour contenir un caractère à transférer.
	
	pr_info("[IFT320] Demande d'écriture...\n"); //Pour déboguer. Retirez ce message s'il pollue votre journal.
	
	/*===============================Section IFT320==========================*/
	
	/*
		IFT320: à compléter
		Ici, vous devez mettre en tampon les caractères à écrire, si possible.
		Pour mettre cette demande d'écriture en attente, vous allez devoir utiliser:
		
			wait_event_interruptible(wq, CONDITION);
		
		où CONDITION est une condition qui doit être vraie avant que l'on puisse continuer.
		La condition est revérifiée lorsque vous appelez wake_up_interruptible (à faire uniquement dans la routine d'interruption).
	*/
	
		
	/*===============================Fin Section IFT320=======================*/
		
	pr_info("[IFT320] Fin d'écriture, caractères envoyés: %d\n",(int)max_len); //Pour déboguer. Retirez ce message s'il pollue votre journal.
	return max_len;
}

/** Service release
	Cette fonction est appelée lorsque l'application termine ou ferme le périphérique.
	Elle est responsable de libérer les ressources utilisées.
	
 *  @param inodep: pointeur vers l'entrée du système de fichier correspondant au pilote.
 *  @param filep: pointeur vers la structure de fichier représentant le pilote.
 */
static int dev_release(struct inode *inodep, struct file *filep){
     	 
	 pr_info("[IFT320] Arrêt des interruptions...");
	 //Les interruptions doivent être désactivées avant de quitter.
	 outb(DEACTIVATE_INTERRUPTS,COM1_IER);
	 
	 
	 /*===============================Section IFT320==========================*/
	 /*
		IFT320: à compléter
		Libérez ici la mémoire occuppée par vos tampons buf_in et buf_out en utilisant kfree.
		Attention, on doit d'abord appeler la fonction de désinitialisation des tampons, pour 
		ensuite libérer les pointeurs buf_in et buf_out avec kfree.
	*/
	
	
	
	 /*===============================Fin Section IFT320=======================*/
	 
	 //Libération du sémaphore. Le périphérique n'est plus ouvert et réservé par l'application.
	 mutex_unlock(&ioMutex);
     pr_info("[IFT320] %s: Périphérique correctement fermé.", MODULE_NAME);
     return 0;
}

/*
	Initialisation du pilote. 
	Appelée lors de l'installation du pilote dans le noyau par la commande insdriver (insmod).
	NE PAS MODIFIER.
*/
static int __init mod_init(void)
{
    
    pr_info("[IFT320] %s: module chargé à l'adresse 0x%p\n", MODULE_NAME, mod_init);

    //  Création d'un sémaphore pour empêcher les ouvertures concurrentes.
    mutex_init(&ioMutex);

    //  Enregistrement du périphérique et demande d'un numéro majeur.
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber < 0) {
        pr_alert("[IFT320] %s: Échec d'inscription du numéro majeur.\n", MODULE_NAME);
        return majorNumber;
    }
    pr_info("%s: Module inscrit avec le numéro majeur %d.\n", MODULE_NAME, majorNumber);

    //  Création et inscription de la classe de périphérique.
    serialdriverClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(serialdriverClass)) {
        //  Nettoyage en cas d'erreur
        unregister_chrdev(majorNumber, DEVICE_NAME);
        pr_alert("[IFT320] %s: échec d'inscription de la classe de périphérique.\n", MODULE_NAME);        
        return PTR_ERR(serialdriverClass);
    }
    pr_info("[IFT320] %s: Classe de périphérique inscrite.\n", MODULE_NAME);

    //  Création du périphérique.
    serialdriverDevice = device_create(serialdriverClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(serialdriverDevice)) {
        class_destroy(serialdriverClass);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        pr_alert("[IFT320] %s: échec de création du périphérique.\n", DEVICE_NAME);
        return PTR_ERR(serialdriverDevice);
    }
    pr_info("[IFT320] %s: Périphérique correctement créé.\n", DEVICE_NAME);

   
	//	Réservation de la ligne IRQ
    int err=request_irq(4,serialdriver_isr,0,MODULE_NAME,NULL);
    if(err<0){
        pr_info("[IFT320] Échec de réservation de la ligne IRQ 4\n");
    }
	else{
		pr_info("[IFT320] Ligne IRQ 4 correctement réservée\n");
	}

    pr_info("[IFT320] Pilote correctement chargé.\n");
    return 0;
}
/*
	Désinitialisation du pilote. 
	Appelé lors de la désinstallation du pilote dans le noyau par la commande rmdriver (rmmod).
	NE PAS MODIFIER.
*/
static void __exit mod_exit(void)
{
    pr_info("[IFT320] %s: Destruction du périphérique...\n", MODULE_NAME);
    device_destroy(serialdriverClass, MKDEV(majorNumber, 0));
	pr_info("[IFT320] %s: Désinscription du périphérique...\n", MODULE_NAME);
    class_unregister(serialdriverClass);
	pr_info("[IFT320] %s: Destruction de la classe de périphérique...\n", MODULE_NAME);
    class_destroy(serialdriverClass);
	pr_info("[IFT320] %s: Désinscription de la classe de périphérique...\n", MODULE_NAME);
    unregister_chrdev(majorNumber, DEVICE_NAME);
	pr_info("[IFT320] %s: Périphérique désinscrit\n", MODULE_NAME);
	pr_info("[IFT320] %s: Destruction du sémaphore...\n", MODULE_NAME);    
    mutex_destroy(&ioMutex);
    
    pr_info("[IFT320] %s: Libération de la ligne IRQ...\n",MODULE_NAME);    
    free_irq (4, NULL);
	pr_info("[IFT320] %s: Pilote correctement déchargé.\n",MODULE_NAME);  
}

module_init(mod_init);
module_exit(mod_exit);

