CONFIG_MODULE_SIG=n

#obj-m spécifie le nom du driver. Le module sera serial_dev.ko, une fois chargé, /dev/serial_dev
obj-m += serial_dev.o

#Spécifie la liste des sources. Les fichiers .c doivent avoir le même nom. 
#Avant -y, le nom doit être exactement celui spécifié dans obj-m. Le nom du device ne peut pas être le même que le nom des sources.
serial_dev-y := serialdriver.o cbuffer.o


# Chemin pour les Kernel build utils. 
KBUILD=/lib/modules/$(shell uname -r)/build/
 
# make sans aucun paramètre: construit le module et l'application
default:
	$(MAKE) -C $(KBUILD) M=$(PWD) modules
	cc chat.c -o chat

# make clean : efface les produits de compilation
clean:
	$(MAKE) -C $(KBUILD) M=$(PWD) clean
	rm -rf chat

menuconfig:
	$(MAKE) -C $(KBUILD) M=$(PWD) menuconfig


