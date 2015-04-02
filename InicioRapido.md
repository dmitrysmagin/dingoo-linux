**IMPORTANTE: esta información está anticuada tras la publicación de FAT dingux. Será actualizada lo antes posible.**

Cómo arrancar y tener linux funcionando en tu A320 en unos pocos pasos.

# Requerimientos #

  * usbtool: te permitirá usar la consola cuando enciendas en modo USB boot. Descarga el binario de la sección Downloads o los fuentes de [aquí](http://jz-hacking.googlecode.com/files/jz_tools_20090224.tar.bz2) y compílalo.
  * hwinit: es una aplicación para la inicialización mínima del hardware
  * zImage: este es el kernel de linux
  * rootfs: es el sistema básico de ficheros.

# Prepara el sistema de ficheros #

Crea dos particiones en la tarjeta miniSD utilizando fdisk o la aplicación con la que te sientas más cómodo. Asegúrate de que la primera partición es 0x0b (FAT32).

El tamaño no es crítico. La intención es almacenar la imagen del kernel de Linux en la primera partición, así que con tan solo 2MB será suficiente (pero hazla de 64MB, la memoria flash es barata). La segunda partición se usara para guardar el sistema de ficheros. De momento es básico, así que bastara con, digamos, 128MB, pero puedes hacerla tan grande como quieras.

Formatea ambas particiones (pon tus propios dispositivos en vez de /dev/sdd1 y /dev/sdd2):

```
sudo mkfs.vfat -F 32 -n A320_VFAT /dev/sdd1
sudo mkfs.ext3 -L A320_EXT3 /dev/sdd2
```

Descomprime el contenido del archive en la segunda partición:

```
sudo mount /dev/sdd2 /mnt/flash
sudo tar -C /mnt/flash -jxvf rootfs.tar.bz2
sudo umount /dev/sdd2
```

Sí… hemos creado y formateado una partición como FAT32 que no vamos a usar. Esta partición es desde donde u-boot cargará la imagen de kernel… cuando se use u-boot. Estamos tomando un atajo y estaremos cargando el kernel a través de USB, así que no necesitamos tener la imagen del kernel lista para u-boot en la primera partición.

# Vamos allá #

Primero descargar el [usbtool de rockbox](http://www.rockbox.org/twiki/bin/viewfile/Main/OndaVX747?rev=2;filename=usbtool).

Pon la Dingoo A320 en modo USB boot manteniendo apretado el botón B mientras hacemos un reset. Esto le dice a la CPU que arranque desde el código en ROM que inicializa el interfaz USB y espere comandos desde el PC. Fíjate que en esta etapa la inicialización de la CPU es mínima y ni siquiera la SDRAM está disponible. Todo lo que tenemos es la caché de instrucciones para cargar y ejecutar código (y es muy pequeña, así que no podemos cargar el kernel allí).


Al arrancar en modo USB boot, la Dingoo parecerá muerta pero si hacéis un _dmesg_ deberíais ver algo así:

```
[ 2018.565047] usb 1-3: new high speed USB device using ehci_hcd and address 11
[ 2018.698513] usb 1-3: configuration #1 chosen from 1 choice
```

Ahora cargaremos el programa de inicialización de hardware que configurará la SDRAM de modo que podamos poner mas cosas allí.

```
sudo usbtool 1 hwinit.bin 0x80000000

[INFO] File size: 3144 bytes
[INFO] Searching for device...
[INFO] Found device, uploading application.
[INFO] GET_CPU_INFO: JZ4740V1
[INFO] SET_DATA_ADDRESS to 0x80000000... Done!
[INFO] Sending data... Done!
[INFO] Verifying data... Done!
[INFO] Booting device STAGE1... Done!
[INFO] Done!
```

Ahora carga el kernel de linux por sí misma:

```
sudo usbtool 1 zImage 0x80600000

[INFO] File size: 1220608 bytes
[INFO] Searching for device...
[INFO] Found device, uploading application.
[INFO] GET_CPU_INFO: JZ4740V1
[INFO] SET_DATA_ADDRESS to 0x80600000... Done!
[INFO] Sending data... Done!
[INFO] Verifying data... Done!
[INFO] Booting device STAGE1... Done!
[INFO] Done!
```

Y ya está. Verás el proceso de arranque de linux en la pantalla. En algún punto, el PC reconocerá un nuevo dispositivo USB (dispositivo de comunicación ACM) y creará /dev/ttyACM0. Puedes usar minicom (57600 8N1, sin control del flujo) para loguearte en la consola (usuario ‘root’ sin password). Para ello:

```
sudo apt-get install minicom
```

Después crear en $HOME un fichero _.minirc.configuration_ con el siguiente contenido:

```
pu port             /dev/ttyACM0
pu baudrate         57600
pu bits             8
pu parity           N
pu stopbits         1
pu rtscts           No
```

Luego basta ejecutar lo siguiente para acceder a tu Dingoo:

```
minicom configuration
```

IMPORTANTE: la consola del kernel se quejará (la pantalla parpadea) si desconectas el cable USB. Este es un problema conocido que se está investigando.

# ¿Cómo copio ficheros? #

Bien, tienes una consola serie, ¿pero cómo subes tus programas? Hay varias formas de hacerlo por el momento.

  * Haz un reset normal y arranca con el firmware original. El PC con Linux debería ver y montar las dos particiones de la miniSD. Haz lo mismo que haces con cualquier dispositivo de almacenamiento. Cuando hayas acabado, tendrás que realizar el proceso de arranque de Linux de nuevo. Este es el mejor camino, en tanto en cuanto no tienes que sacar la miniSD.
  * Saca la miniSD, conéctala al PC (se necesita un lector de tarjetas obviamente) y añade o quita contenidos como desees.
  * Usa zmodem a través de la consola serie, necesitaras compilar el programa rz e instalarlo en la Dingoo por medio de una de las dos opciones anteriores.

# ¿Cómo compilo programas? #

Consigue el toolchain del FTP de Ingenic o a través de google (mipseltools-gcc412-glibc261.tar.bz2). Descomprímelo en /opt y añade el directorio de binarios al path:

```
export PATH=/bin
```

Para la mayoría de programas compatibles con automake, haz algo lo siguiente:

```
./configure --host=mipsel-linux --prefix=/
make
```

El sistema de ficheros básico contiene libmad, libSDL, libSDL\_image and libSDL\_gfx, pero para el desarrollo también necesitarás instalarlos (y sus ficheros de cabeceras) en el toolchain mips de tu PC. Compila las librerías tu mismo o (mejor), cógelas del archivo de librerías de la sección Download y descomprímelas en /opt/mipseltools-gcc412-glibc261 (configurar correctamente las SDL para la compilación es como un grano en el culo).

IMPORTANTE: necesitas poner la variable de entorno SDL\_NOMOUSE o la inicialización de las SDL en tus programas fallará.

```
export SDL_NOMOUSE=1
```