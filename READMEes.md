Flaseador para Arranque-dual por Ignacio Garcia Perez <iggarpe@gmail.com>


# AVISO IMPORTANTE #

Para escribir en una flash NAND primero debes borrar el bloque entero, y cada  bloque se desgasta con cada borrado. Para chip flash MLC (Multi-Level Cell) hay garantizados un mínimo de ciclos de borrado de alrededor de 1000. Lo que no es mucho, y normalmente no es un problema porque el firmware y los sistemas operativos hacen lo que se llama "nivelación de desgaste". Para hacer corta una larga historia, se pone una capa de traducción que disemina las escrituras por diferentes bloques. Si tu borras 1000 veces un bloque lógico, realmente estas borrando 1000 bloques físicos diferentes una vez.

Sin embargo, cuando estás escribiendo directamente a un bloque sin ninguna capa de traducción intermedia, debes tener cuidado. Flashear el arranque dual significa borrar y escribir los primeros bloques de tu flash NAND. Siempre los primeros bloques. Y los desgasta. Es absolutamente imposible que se lancen 1000 versiones de arranque dual,
así que estas en terreno seguro. Sin embargo, no flasees tu A320 por diversión cada día antes del desayuno.



# Introducción #

Este paquete te permitirá escribir el arranque-dual en tu A320 así que puedes escoger entre arrancar el firmware original desde la flash interna (por defecto) o linux desde la miniSD (manteniendo el botón **SELECT** apretado durante el arranque).

Este paquete **NO** instala linux. El código de arranque dual solo buscara por una zImage en la primera partición (FAT32) de la miniSD, lo carga y lo lanza. Es tu cometido preparar la miniSD con linux en ella.

Hay dos binarios de arranque dual, uno para cada tipo de LCD conocido hasta la fecha. Puedes ver tu tipo de LCD en la pantalla About en el menú System Setup con la siguiente combinación:

```
    arriba-derecha-abajo-arriba-derecha-abajo
```

Entonces veras una pantalla de diagnostico oculta. Mira por ella y deberías ver "ILI9325" o "ILI9331" en algún sitio. Si no lo ves, probablemente tengas un modelo de LCD desconocido y deberías contactar conmigo.

También, como puedes ver esta guía es un poco simple. Si quieras ayudar y reescribirla y añadir imágenes o lo que sea, hazlo y mándamela así puedo incluir tu trabajo en el siguiente lanzamiento.



# Instrucciones para Windows #

  1. Desconecta tu A320.
  1. Ponerla en modo USB boot: resetear la mientras mantienes el botón B presionado. El LCD permanecerá oscuro.
  1. Conecta tu A320. Si el mensaje de "Nuevo hardware encontrado" aparece, selecciona "No, no por el momento", y cuando se te pregunte por la localización del driver usa el directorio donde descomprimiste los ficheros del arranque dual. Ve al paso 5.
  1. Si el mensaje "nuevo hardware encontrado" no sale, significa que tienes instalados los drivers de ChinaChip. Tienes que desintalarlos. Abre el administrador de dispositivos, encuentra el dispositivo, haz clic con el boton derecho y selecciona "Desinstalar". Desconecta la A320 y ve al paso 3.
  1. Abre un interfaz de comandos (Inicio --> Ejecutar --> "cmd"), cambia al directorio donde descomprimiste los ficheros del arranque dual y ejecuta los siguientes comandos (sustituye ILI9325 por ILI9331 si tienes el ultimo modelo de LCD):
```
   usbtool-win 1 hwinit.bin 0x80000000
   usbtool-win 1 zImage_dual_boot_installer_ILI9325 0x80600000
```
  1. Deberías ver tu A320 arrancando linux y lanzando el script de flasheo. Sigue las instrucciones que aparecen en la pantalla de la A320.


# Instrucciones para Linux #

Ve directamente al paso 5 de las instrucciones para Windows, pero
sustituye _usbtool-win_ por _sudo ./usbtool-linux_.



# Instrucciones para MAC OS X 10.4 Y SUPERIORES #

Ve directamente al paso 5 de las instrucciones para Windows, pero
sustituye _usbtool-win_ por _./usbtool-osx_.

(sin probar, por favor hágame saber si ha funcionado o no)