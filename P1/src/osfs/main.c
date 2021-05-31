#include "os_API.h"

// Function to manage partitions
void manage_partitions(char* disk_name) {

    // List available partitions
    os_mbt();

    // Ask for command
    printf("Seleciona uno de los siguientes comandos:\n");
    printf(">>> [select]    Monta la partición de id 'partition_id'\n");
    printf(">>> [create]    Crear una nueva partición\n");
    printf(">>> [delete]    Eliminar una de las particiones\n");
    printf(">>> [reset]     Elimina las particiones de la MBT\n");

    // Read command
    char command[12];
    scanf("%s", command);

    if (strcmp(command, "create")) {
        printf(">>> Ingresa un id [int] y size [int] para la partición a crear ...\n");
        int partition_id;
        int size;
        scanf("ID: %d", partition_id);
        scanf("Size: %d", size);
        os_create_partition(partition_id, size);
        manage_partitions(disk_name);

    } else if (strcmp(command, "delete")) {
        printf(">>> Ingresa el id [int] de la partición a eliminar ...\n");
        int partition_id;
        scanf("ID: %d", partition_id);
        os_delete_partition(partition_id);
        manage_partitions(disk_name);

    } else if (strcmp(command, "reset")) {
        printf(">>> Esto eliminará todos los datos en el disco, estás seguro??? ...\n");
        char answer[1];
        scanf("y/n: %s", answer);
        if (strcmp(answer, "y")){
            reset_mbt();
        }
        manage_partitions(disk_name);

    } else {
        bool is_number = true;
        for (int d; d < 3; d++) {
            if (!isdigit(command[d])){
                is_number = false;
            }
        }

        if (is_number){
            os_mount(disk_name, (int) command);
        }
    }
}

void manage_this(char* disk_name){

    // Ask for command
    printf("Seleciona uno de los siguientes comandos:\n");
    printf(">>> [filename]  Chequea si el archivo 'filename' existe en la partición!\n");
    printf(">>> [bitmap]    Muestra el BitMap de la partición\n");
    printf(">>> [ls]        Muestra todos los archivos en la partición'\n");
    printf(">>> [back]      Retroceder\n");

    // Read command
    char command[12];
    scanf("%[^\n]%s", command);

    if (strcmp(command, "ls")) {
        os_ls();
        manage_this(disk_name);

    } else if (strcmp(command, "bitmap")) {
        printf(">>> Ingresa el número [int] del bitmap que quieres mostrar ([all] para mostrarlos todos)\n");
        char bitmap[3];
        scanf("%[^\n]%s", bitmap);
        if (isdigit(bitmap[0])) {
            os_bitmap((int) bitmap);
        } else {
            os_bitmap(0);
        }
        manage_this(disk_name);

    } else if (strcmp(command, "back")) {
        manage_partitions(disk_name);

    } else {
        bool exists = os_exists(command);
        if (!exists) {
            printf("No existe archivo con ese nombre D: \n");
            manage_this(disk_name);
        }
        printf("El archivo existe, deseas seleccionarlo????\n");
        char answer[2];
        scanf("y/n: %[^\n]%s", answer);
        if (strcmp(answer, "y")) {// existing file options
            existing_file_opt(disk_name, command);
        } else if (strcmp(answer, "n")) {// non-existing file options
            nonexistent_file_opt(disk_name, command);
        }
    }
}

void existing_file_opt(char* disk_name, char* filename){
    
    // Archivo que ya existe
    printf("[%s] Seleciona uno de los siguientes comandos:\n");
    printf(">>> [download]  Descargar archivo\n"); // read, open, close
    printf(">>> [more]      Impreme en pantalla los detalles del archivo\n");
    printf(">>> [rm]        Borrar archivo\n");
    printf(">>> [back]      Retroceder\n");

    char command[8];
    scanf("%[^\n]%s", command);

    // Download file to local (PC)
    if (strcmp(command, "download")) {
        char str[6 + strlen(filename) + 1];
        sprintf(str, "files/%s", filename);
        download_file(str);

    // Show data inside disk file
    } else if (strcmp(command, "more")) {
        osFile* file = os_open(filename, 'r');
        char text[256];
        os_read(file, text, 256);
        os_close(file);
        existing_file_opt(disk_name, filename);

    // Remove file
    } else if (strcmp(command, "rm")) {
        os_rm(filename);
        manage_this(disk_name);

    // Return to Manage Partition
    } else {
        manage_this(disk_name);
    }
}

void nonexistent_file_opt(char* disk_name, char* filename){

    // Crear nuevo archivo
    printf("[%s] Seleciona uno de los siguientes comandos:\n", filename);
    printf(">>> [create]  Crea un archivo con el nombre y agregale cosas xD\n"); // open
    printf(">>> [load]    Carga una archivo al disco\n"); // write, open, close
    printf(">>> [back]    Retroceder\n");

    // Read command
    char command[6];
    scanf("%[^\n]%s", command);
    if (strcmp(command, "back")) {
        manage_this(disk_name);

    // Create new disk file and put some text
    } else if (strcmp(command, "create")) {
        osFile* file = os_open(filename, 'w');
        printf(">>> Ingresa el texto que quieres guardar...\n");
        char text[256];
        scanf("%[^\n]%s", text);
        os_write(file, text, strlen(text));
        os_close(file);
        existing_file_opt(disk_name, filename);

    // Load 
    } else if (strcmp(command, "load")) {
        printf(">>> Ingresa el nombre de archivo que quieres subir...\n");

        char pc_filename[256];
        scanf("// %[^\n]%s", pc_filename);
        upload_file(pc_filename, filename);
        printf(" >>> Archivo subido con éxito!!!\n");
        manage_this(disk_name);
    }

}

// Upload file to disk
void upload_file(char* filename, char* disk_filename) {
    FILE* input = fopen(filename, "rb");
    char* buffer = calloc(4096, sizeof(char));
    osFile* file = os_open(disk_filename, 'w');
    int bytes = 1;
    size_t bytes_read = 0;
    bool uploading = true;
    while (uploading) {
        bytes_read = fread(buffer, 1, 4096, input);
        bytes = os_write(file, buffer, (int) bytes_read);
        if ((bytes_read < 4096) || (!bytes)) {
            uploading = false;
        }
    }
    os_close(file);
    free(buffer);
    fclose(input);
}

// Download file from disk
void download_file(char* filename) {
    FILE* output = fopen(filename, "w");
    char* buffer = calloc(4096, sizeof(char));
    osFile* file = os_open(filename, 'r');
    int bytes = 1;
    while (bytes) {
        bytes = os_read(file, buffer, 4096);
        fwrite(buffer, 1, bytes, output);
    }
    os_close(file);
    free(buffer);
    fclose(output);
}

// OldSchool File System Demo
int main(int argc, char** argv) {

    // Check input
    if (argc < 2) {
        printf("[Error] Not enough arguments were provided!\n");
        return 1;
    }

    // mount disk
    printf("Leyendo disco ...");
    os_mount(argv[1], NULL);

    // First: all partitions
    manage_partitions(argv[1]);

    // Second: this partition [bitmap, os ls, os exists]
    manage_this(argv[1]);

    os_unmount();
    return 0;
}
