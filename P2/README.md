# Monster Hunter: Ruz :dragon:

## Equipo

| name            | mail              | student id
| --------------- | ----------------- | -----------
| Vicente Castro  | [vvcastro@uc.cl]  | 1763668J
| Benjamín Farías | [bffarias@uc.cl]  | 17642531
| Mauro Mendoza   | [msmendoza@uc.cl] | 17639743
| Richard Morales | [rgmorales1@uc.cl]| 17627621
| Juan Romero     | [jaromero6@uc.cl] | 17639298

[vvcastro@uc.cl]:    mailto:vvcastro@uc.cl
[bffarias@uc.cl]:    mailto:bffarias@uc.cl
[msmendoza@uc.cl]:   mailto:msmendoza@uc.cl
[rgmorales1@uc.cl]:  mailto:rgmorales1@uc.cl
[jaromero6@uc.cl]:   mailto:jaromero6@uc.cl

## Modo de Uso

* El servidor se debe compilar utilizando el `makefile` incluído, asegurándose de tener la carpeta `loot` en ese mismo directorio (`server`) para que funcione el bonus correctamente.
* Los clientes se deben compilar utilizando el `makefile` incluído, asegurándose de que estén separados entre sí (en vez de correr el mismo binario múltiples veces), para que de esta forma tengan sus carpetas de `loot_conseguido` separadas y se eviten colisiones de estas imágenes para distintos clientes.
* El servidor se corre con el comando `./server -i 0.0.0.0 -p 8080`, mientras que cada cliente se corre con `./client -i 0.0.0.0 -p 8080`.
* Lo único que deben hacer los clientes es entregar inputs cuando el juego les pida, los que pueden ser un nombre para el jugador, o bien, un número que representa la opción seleccionada de entre un listado que aparece en la interfaz. De todas formas el servidor valida los input, y en caso de que no sean correctos se le pide a dicho jugador que vuelva a intentarlo.
* Los clientes pueden elegir dejar de jugar tras cada combate, lo que los desconecta del servidor y termina su ejecución. Cuando se rinden o pierden en medio de un combate, quedan como **espectadores** hasta que termine dicho combate.
* Si **todos** los clientes deciden dejar de jugar tras un combate, el servidor libera todos sus recursos y termina su ejecución. En caso contrario, sigue funcionando para los jugadores que desean continuar.

## Decisiones de Diseño y Consideraciones

* Se utilizaron diversos módulos para el desarrollo del programa del **servidor**. Los que más destacan son:
  - `game.c`: Maneja toda la lógica del programa, desde el lobby hasta el combate.
  - `characters.c`: Modela las entidades que participan en el combate (jugadores y monstruo), todos como una struct de tipo _Character_.
  - `communication.c`: Maneja toda la comunicación entre el cliente y el servidor.
* Los puntos de daño que aparecerán en el juego corresponden al mínimo entre la vida actual del personaje y lo que le quitará el ataque, es decir, si un ataque quita 2000 pero el jugador tiene sólo 1000 puntos de vida, el juego mostrará que se quitaron 1000 puntos (lo mismo aplica cuando se sana a un jugador).
* Las rondas no se consideran completas hasta que juega el monstruo, por lo que la habilidad `sudo rm -rf` hace 0 de daño durante la **primera** ronda.
* La habilidad `Inyección SQL` puede ser aplicada sobre uno mismo.
* Se recomienda fuertemente probar el cliente con fondo de color negro en la terminal para percibir mejor los colores de ciertos `printfs`.

## Supuestos

### 1. Sobre los Buffs/Debuffs:
Para el buff `Inyección SQL` decidimos hacer un sistema que permita el "stackeo" de los buffs, es decir, se pueden acumular unos sobre otros. De esta forma, un usuario puede terminar con un multiplicador 2<sup>n</sup> con `n` el número de buffs activos que tiene. Además, este multiplicador no afecta al sangrado de `Estocada` ni la intoxicación de `Espina Venenosa`, es decir, su daño es siempre el mismo daño base señalado en el enunciado (a menos que el jugador se encuentre reprobado).

### 2. Sobre Caso Copia:
El flujo típico de la habilidad `Caso Copia` es que se selecciona, al azar, un jugador de los que siguen en la partida y se copia una habilidad al azar de este. Luego, dicha habilidad puede tener dos caminos: (1) si es una habilidad dañina, se elige un jugador al azar como objetivo (si el monstruo estaba distraído, se elige al jugador que lo distrajo). (2) Si la habilidad tiene algún beneficio (como `Curar`, `Destello Regenerador` o `Inyección SQL`), el beneficio se lanza directamente sobre el monstruo.

**Caso Borde**: Cuando el monstruo roba la habiliad `Distraer` del cazador y la ejecuta sobre un personaje con una habilidad beneficiosa, esta habilidad, en caso de ser ejecutada, será redirigida hacia el monstruo. Es decir, en el caso de que se distraiga al `Médico` y luego este use `Curar`, el monstruo terminaría siendo sanado.

**Caso Borde**: Cuando el monstruo roba la habiliad `Estocada` del cazador y la ejecuta sobre un personaje, este queda con sangrado.

## Paquetes (Protocolo de Comunicación)

| ID | Tipo               | Descripción
| -- | ------------------ | ----------------------------------------------------------------
| 0  | `DISCONNECT`       | Desconectar cliente del servidor.
| 1  | `SET_USERNAME`     | Solicitar o enviar username.
| 2  | `SELECT_CLASS`     | Solicitar o enviar el tipo de personaje que se quiere usar.
| 3  | `START_GAME`       | Solicitar al líder que dé comienzo al combate.
| 4  | `SELECT_MONSTER`   | Solicitar al líder el tipo de monstruo al que se enfrentarán.
| 5  | `SELECT_ACTION`    | Solicitar o enviar la acción a realizar en un turno (Luchar o Rendirse).
| 6  | `SELECT_SKILL`     | Solicitar o enviar la habilidad a usar en un turno.
| 7  | `SELECT_OBJECTIVE` | Solicitar o enviar el personaje aliado al que irá dirigida la habilidad (si es que aplica).
| 8  | `CONTINUE`         | Preguntarle al cliente si seguirá en el juego o se desconectará.
| 9  | `MESSAGE`          | Enviar mensajes al cliente.
| 10 | `IMAGE`            | Enviar las imágenes del loot que ganó el cliente.

# Bonus

## Recompensas (Loot)

* Se realizó este bonus **completo**. Las imágenes de loot son guardadas con el nombre `lootX.PNG`, donde `X` es un ID que comienza en 0 y aumenta por cada recompensa obtenida.
* Se asume que los clientes estarán **separados entre sí**, es decir, que no se ejecutarán en la misma carpeta raíz, debido a que sino el loot conseguido colisionaría al compartir el mismo directorio de `loot_conseguido`.
* En el caso de que un cliente ya tuviera loot anteriormente, y en una nueva conexión al servidor gana loot, el antiguo **se comenzará a sobreescribir**, partiendo desde el 0 hasta el loot que gane en esta nueva partida. Esto ocurre debido a que se asume que el loot que se obtiene al estar conectado al servidor no es válido una vez que se desconecta.
