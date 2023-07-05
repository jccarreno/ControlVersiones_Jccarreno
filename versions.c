#include <limits.h>
#include "versions.h"

/**
 * @brief Obtiene el hash de un archivo.
 * @param filename Nombre del archivo a obtener el hash
 * @param hash Buffer para almacenar el hash (HASH_SIZE)
 * @return Referencia al buffer, NULL si ocurre error
 */
char *get_file_hash(char * filename, char * hash);

/**
 * @brief Copia un archivo
 *
 * @param source Archivo fuente
 * @param destination Destino
 *
 * @return 1 en caso de exito, 0 si existe error.
 */
int copy(char * source, char * destination);
void adicionar_version(file_version *r);
int existe_version(char* hash);



return_code add(char * filename, char * comment) {
	struct stat s;
	//Verifica si el archivo existe
	if(stat(filename,&s)<0)
		return 0;
	//Verifica si es un archivo regular
	if(!S_ISREG(s.st_mode))
		return 0;
	//Obtener el has del archivo
	char* hash=get_file_hash(filename,hash);

	//Construir la cadena para el nombre del archivo
	size_t len1 = strlen(".versions/"), len2 = strlen(hash);
	char *nuevo = (char*) malloc(len1 + len2 + 1);
	memcpy(nuevo, ".versions/", len1);
	memcpy(nuevo+len1, hash, len2+1);

	//Copiar el archivo al directorio .versions y adicionar el registro a la db
	file_version r;
	if (copy(filename,nuevo))
	{
		strcpy(r.filename,filename);
		strcpy(r.comment,comment);
		strcpy(r.hash,hash);
		adicionar_version(&r);
		return 1;
	}
	return 0;
}

void adicionar_version(file_version *r){
	FILE * fp=fopen("versiones.db","a");
	if(fp==NULL)
		perror("No se pudo hacer el guardado");
	if(fwrite(r,sizeof(file_version),1,fp)!=1){
		fprintf(stderr,"No se pudo guardar la version");
	}
}

int existe_version(char* hash){
	FILE* fp=fopen(".versions/versions.db","r");
	if(fp==NULL)
		return;
	//Leer hasta el fin del archivo
	file_version r;
	while(!feof(fp))
	{
		//Realizar una lectura y validar
		if(fread(&r,sizeof(file_version),1,fp)!=1)
			break;
		//Si el registro corresponde al archivo buscado imprimir
		if(strcmp(r.hash,hash))
		{
			return 1;
		}
	}
	fclose(fp);
	return 0;
}

void list(char * filename) {
	FILE* fp=fopen(".versions/versions.db","r");
	if(fp==NULL)
		return;
	
	int cont=1;
	//Leer hasta el fin del archivo
	file_version r;
	while(!feof(fp))
	{
		//Realizar una lectura y validar
		if(fread(&r,sizeof(file_version),1,fp)!=1)
			break;
		//Si el registro corresponde al archivo buscado imprimir
		if(strcmp(r.filename,filename))
		{
			printf("%i. Hash:%s  Comentario:%s",cont,r.hash,r.comment);
			cont++;
		}
	}
	fclose(fp);
}

char *get_file_hash(char * filename, char * hash) {
	char *comando;
	FILE * fp;

	struct stat s;

	//Verificar que el archivo existe y que se puede obtener el hash
	if (stat(filename, &s) < 0 || !S_ISREG(s.st_mode)) {
		perror("stat");
		return NULL;
	}

	sha256_hash_file_hex(filename, hash);

	return hash;

}

int copy(char * source, char * destination) {
	FILE *f,*df; 
	char c;
 
	if(!(f=fopen(source,"rt")) || !(df=fopen(destination,"wt")))
	{
		perror("Error de apertura de ficheros");
		return 0;
	}
 
	while((c=fgetc(f))!=EOF && !ferror(f) && !ferror(df))
		fputc(c,df);
 
	if(ferror(f) || ferror(df))
		return 1;
 
	fclose(f);
	fclose(df);
}


return_code get(char * filename, int version) {
	//Abrir el archivo ".versions/versions.db"
	file_version r;

	FILE* fp = fopen(".versions/versions.db", "r");
	if(fp == NULL)
		return;

	int cont = 1;

	//Leer hasta fin de archivo
	while(!feof(fp)){
		//Realizar una lectura y validar
		if(fread(&r, sizeof(file_version), 1, fp) != 1)
			break;

		//Buscar el archivo y la version solicitada
		if(strcmp(r.filename,filename)){
			if(cont==version){
				//Copiar el archivo

				size_t len1 = strlen(".versions/"), len2 = strlen(r.hash);
				char *ruta_origen = (char*) malloc(len1 + len2 + 1);
				memcpy(ruta_origen, ".versions/", len1);
				memcpy(ruta_origen+len1, r.hash, len2+1);
				copy(ruta_origen, r.filename);
				break;
			}
		}
		cont = cont + 1;
	}
	fclose(fp);
}
