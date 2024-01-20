/*
 * FileSystem.c
 *
 *  Created on: Jan 20, 2024
 *      Author: Opherox
 */

#ifndef APPLICATION_USER_FILESYSTEM_C_
#define APPLICATION_USER_FILESYSTEM_C_

#include "ff.h"
#include "usbh_core.h"
#include "usbh_msc.h"
#include "FileSystem.h"

void initFileSystem(void)
{
	result = f_mount(&fs, "", 0); //montamos el sistema de archivos, 0:no forzar montaje
}

void mountFileSystem(void)
{
	if(isMounted == 0)
	{
		if((result == FR_OK) && isMounted == 0)
		{
			printf("Sistema de ficheros montado con exito");
			isMounted = 1;
		}
		else
		{
			Error_Handler();
		}
	}
}

int scanMp3Music(void)
{
  	if ((result = f_opendir(&dir, "/")) == FR_OK) //abrimos el directorio raiz
  	{
  		int numf = 0; //para contar numero de archibos mp3
  		while((result = f_readdir(&dir, &finfo)) == FR_OK && finfo.fname[0] != 0 && numf < MAX_MUSIC) //mientras queden archivos por leer, aqui actualiza el archivo que lee
  		{
  			if(!(finfo.fattrib & AM_DIR) && strcasecmp(strrchr(finfo.fname, '.'), ".MP3") == 0) //si el archivo es un directorio y el nombre del archivo tiene una subcadena .mp3
  			{
  				mp3Files[numf] = (char*)malloc(strlen(finfo.fname)+1);	//guardamos el nombre del archivo en un vector de chars, en el elemento de numero actual encontrado
  				strcpy(mp3Files[numf], finfo.fname);
  				numf++;	//aumentamos el numero de archivos mp3 encontrados
  				printf("Se han encontrado %d canciones.\n", numf);
  			}
  		}
  		f_closedir(&dir);
  		printf("Se ha podido abrir el directorio, hay %d canciones.\n",numf);
  		return numf;
  	}
  	else
  	{
  		printf("Error al abrir el directorio.\n");
  		return 0;
  	}
}

void indexFiles(void)
{
	if (isMounted == 1 && isIndexed == 0) //solo buscaremos una vez las canciones
	    {
	  	  if((foundSongs = scanMp3Music()) != 0) //funcion buscar musica consigue encontrar las canciones, asignamos el numero de canciones encontradas
	  	  {
	  		  printf("Se ha indexado con exito, existen %d canciones", foundSongs);
	  		  isIndexed = 1;
	  	  }
	  	  else
	  	  {
	  		  printf("Error al indexar o no hay canciones");
	  	  }
	    }
}

int configMusicRegisters(char *b[], int i, int size) //vector de buffers, el numero de buffers que queremos hacer como argumento y su tamaño
{
	/* TODO posibles dudas de funcionamiento correcto */
	if(isConf == 0)
	{
		isConf = 1;
		for(int j = 0; j < i-1; j++)
		{
			b[j] = (char*)malloc(size);
			if (b[j] == NULL)
			{
				for(int z = 0; z < j; z++)
				{
					free(buffer[z]); //liberamos la memoria de los buffers por el error al reservar memoria
				}
				printf("No se ha podido reservar memoria para el Buffer %d",j);
				isConf = 0;
				return 0;
			}
			else
			{
				printf("Se ha podido reservar memoria para el Buffer %d", j);
			}
		}
  	}
  	return 1;
}

int readFile(const char *filename, char *buffer, int size, uint8_t bytesRead)
{
	FIL songFile;
	result = f_open(&songFile, filename, FA_READ); //abrimos el archivo en lectura
	UINT realBytesRead = 0;
	if(result != FR_OK)
	{
		printf("Error al abrir el archivo");
		return 1;
	}
	f_lseek(&songFile, bytesRead); //nos ponemos al principio del archivo + los bytes leidos anteriormente
	if((result = f_read(&songFile, buffer, size, &realBytesRead)) == FR_OK) //si la lectura se ha podido realizar
	{
		printf("Lectura de fichero realizada con exito");
		bytesRead += realBytesRead; //los bytes leidos son los que ya se habian leido mas los que se ha conseguido leer en esta ocasion
		return 0;
	}
	else
	{
		printf("Error al leer el archivo");
		return 1;
	}
}

void updatePingPongBuffers()
{
	if(buf1CanRead != 0)
	{
		if(readFile(mp3Files[playing], buffer[0], BUFFER_SIZE, bytesLeidos) != 1)
		{
			printf("Buffer 1 actualizado con exito");
			buf1CanRead = 0;	//una vez actualizado no puede leer hasta terminar de reproducirse
		}
		else
		{
			printf("Error al actualizar el buffer 1");
		}
	}
	if(buf2CanRead !=0)
	{
		if(readFile(mp3Files[playing], buffer[1], BUFFER_SIZE, bytesLeidos) != 1)
				{
					printf("Buffer 2 actualizado con exito");
					buf2CanRead = 0;	//una vez actualizado no puede leer hasta terminar de reproducirse
				}
				else
				{
					printf("Error al actualizar el buffer 2");
				}
	}
}

int sendMusicBuffer(char* buff)
{
	/* TODO ver la comunicacion por donde mandarlo */
	return 1;
}

void selectMusicBuffer()
{

	if(buf1Playing == 1)
	{
		if(sendMusicBuffer(buffer[0]) == 0)
		{
			buf1CanRead = 1;	//una vez termina de mandarlo, pasa a leer
			buf1Playing = 0;	//y ya no transmite
		}
	}
	if(buf2Playing == 1)
		{
			if(sendMusicBuffer(buffer[1]) == 0)
			{
				buf2CanRead = 1;	//una vez termina de mandarlo, pasa a leer
				buf2Playing = 0;	//y ya no transmite
			}
		}
}

void mainFileSystem(void)
{
	initFileSystem();
	mountFileSystem();
	indexFiles();
	configMusicRegisters(buffer, 2, BUFFER_SIZE);
	updatePingPongBuffers();
	selectMusicBuffer();
}
#endif /* APPLICATION_USER_FILESYSTEM_C_ */
