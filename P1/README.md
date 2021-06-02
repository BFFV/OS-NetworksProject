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

Se decidió crear un archivo `main.c` a modo de demo interactivo sobre todas las funciones de la API.

## Errores Manejados

| Error                 | Descripción
|---------------------- | -----------------------------------------------------------
| `DiskNotFound`        | Ruta del disco virtual inválida
| `NoSpaceAvailable`    | No hay espacio en el disco para crear la partición
| `NoBlocksAvailable`   | No quedan bloques disponibles para usar en la partición actual
| `FileNotFound`        | Archivo no encontrado
| `FileExists`          | Archivo ya existe
| `PartitionNotFound`   | Partición no está en el disco
| `PartitionOutOfRange` | ID/Tamaño de la partición fuera del rango válido
| `PartitionExists`     | ID de Partición ya existe en el disco
| `NoDirectoryEntry`    | No quedan entradas de directorio disponibles en la partición
| `InvalidBitmapIndex`  | Índice del bitmap fuera de rango
| `InvalidFileName`     | Nombre de archivo tiene más de 28 caracteres
| `InvalidFileMode`     | Modo de IO incorrecto (por ejemplo abrir con 'r' e intentar escribir)
| `InvalidBytesNumber`  | Nº de bytes no válido para lectura/escritura

## Alertas Manejadas

| Alerta                | Descripción
| --------------------- | ------------------------------------------------------
| `MaxFileSizeReached`  | Tamaño máximo de archivo fue alcanzado
| `BytesExceeded`       | No se pudo leer/escribir la cantidad total de bytes deseada
