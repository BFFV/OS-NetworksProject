# Old School File System

## Equipo

| nombre          | mail              | número de alumno
| --------------- | ----------------- | -----------------
| Vicente Castro  | [vvcastro@uc.cl]  | 1763668J
| Benjamín Farías | [bffarias@uc.cl]  | 17642531
| Mauro Mendoza   | [msmendoza@uc.cl] | 17639743
| Juan Romero     | [jaromero6@uc.cl] | 17639298

[vvcastro@uc.cl]:   mailto:vvcastro@uc.cl
[bffarias@uc.cl]:   mailto:bffarias@uc.cl
[msmendoza@uc.cl]:  mailto:msmendoza@uc.cl
[jaromero6@uc.cl]:  mailto:jaromero6@uc.cl

## Decisiones de Diseño

* Se decidió crear un archivo ```main.c``` a modo de demo interactivo sobre todas las funcionalidades de la API. Esto permite realizar una demostración en vivo que valida todo lo pedido para el proyecto, incluyendo el manejo de errores (bonus).
* Dentro del archivo ```os_API.c``` y su header ```os_API.h``` se definieron todas las funciones mínimas de la API, además de otras funciones auxiliares de utilidad.
* Se trabajó en un computador que lee en Little Endian, por lo que fue necesario implementar funciones para convertir a Big Endian y viceversa.
* La función ```os_close``` simplemente se encarga de liberar memoria, ya que los cambios en el disco se ven reflejados inmediatamente dentro de la función ```os_write```.
* Los errores fueron definidos mediante un enum en el header  ```os_API.h```, y al momento de imprimirlos se agrupan en ciertas categorías genéricas para dar mayor entendimiento del error al usuario.
* Aparte de errores, se incluyeron alertas que sirven para indicar cuando se alcanza el tamaño máximo o no se logra leer/escribir los ```nbytes``` solicitados por el usuario, sin necesariamente significar que ocurrió un error.

# Bonus

## Errores Manejados

| Categoría               | Error                 | Descripción
|-------------------------|---------------------- | -----------------------------------------------------------
| `IOError`               | `DiskNotFound`        | Ruta del disco virtual inválida
| `IOError`               | `NoSpaceAvailable`    | No hay espacio en el disco para crear la partición
| `IOError`               | `NoBlocksAvailable`   | No quedan bloques disponibles para usar en la partición actual
| `IOError`               | `NoDirectoryEntry`    | No quedan entradas de directorio disponibles en la partición
| `InvalidPartitionError` | `PartitionNotFound`   | Partición no está en el disco
| `InvalidPartitionError` | `PartitionExists`     | ID de Partición ya existe en el disco
| `IndexError`            | `PartitionOutOfRange` | ID/Tamaño de la partición fuera del rango válido
| `IndexError`            | `InvalidBitmapIndex`  | Índice del bitmap fuera de rango
| `InvalidInputError`     | `InvalidFileName`     | Nombre de archivo tiene más de 28 caracteres
| `InvalidInputError`     | `InvalidBytesNumber`  | Nº de bytes no válido para lectura/escritura
| `FileIOError`           | `FileNotFound`        | Archivo no encontrado
| `FileIOError`           | `FileExists`          | Archivo ya existe
| `FileIOError`           | `InvalidFileMode`     | Modo de IO incorrecto (por ejemplo abrir con 'r' e intentar escribir)


## Alertas Manejadas

| Alerta                | Descripción
| --------------------- | ------------------------------------------------------
| `MaxFileSizeReached`  | Tamaño máximo de archivo fue alcanzado
| `BytesExceeded`       | No se pudo leer/escribir la cantidad total de bytes deseada
