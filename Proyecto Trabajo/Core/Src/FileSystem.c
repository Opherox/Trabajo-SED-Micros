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
	if(isConf == 0)
	{
		isConf = 1;
		for(int j = 0; j < i; j++)
		{
			b[j] = (char*)malloc(size);
			if (b[j] == NULL)
			{
				for(int z = 0; z < j; z++)
				{
					free(b[z]); //liberamos la memoria de los buffers por el error al reservar memoria
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
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET); //si se han configurado los dos buffers bien encender luz roja
  	return 1;
}

int readFile(const char *filename, char *buffer, int size, uint32_t *bytesRead)
{
	if(filename != NULL)
	{
		FIL songFile;
		result = f_open(&songFile, filename, FA_READ); //abrimos el archivo en lectura
		UINT realBytesRead = 0;
		if(result != FR_OK)
		{
			printf("Error al abrir el archivo");
			return 1;
		}
		f_lseek(&songFile, *bytesRead); //nos ponemos al principio del archivo + los bytes leidos anteriormente
		if((result = f_read(&songFile, buffer, size, &realBytesRead)) == FR_OK) //si la lectura se ha podido realizar
		{
			printf("Lectura de fichero realizada con exito");
			f_close(&songFile);  // Cerramos el archivo después de leerlo
			*bytesRead += realBytesRead; //los bytes leidos son los que ya se habian leido mas los que se ha conseguido leer en esta ocasion
			return 1;
		}
		else
		{
			printf("Error al leer el archivo");
			f_close(&songFile);  // Cerramos el archivo en caso de error
			return 0;
		}
	}
}

void updatePingPongBuffers()
{
	if(isConf == 1)
	{
		if(buf1CanRead != 0)
		{
			if(readFile(mp3Files[playing], buffer[0], BUFFER_SIZE, &bytesLeidos) != 1)
			{
				printf("Buffer 1 actualizado con exito");
				buf1CanRead = 0;	//una vez actualizado no puede leer hasta terminar de reproducirse
				buf1Playing = 1;
			}
			else
			{
				printf("Error al actualizar el buffer 1");
			}
		}
		else if(buf2CanRead !=0)
		{
			if(readFile(mp3Files[playing], buffer[1], BUFFER_SIZE, &bytesLeidos) != 1)
					{
						printf("Buffer 2 actualizado con exito");
						buf2CanRead = 0;	//una vez actualizado no puede leer hasta terminar de reproducirse
						buf2Playing = 1;
					}
					else
					{
						printf("Error al actualizar el buffer 2");
					}
		}
	}
}

int sendMusicBuffer(char* buff)
{
	// TODO Esto no funca, apañarlo
	return 1;
	/*if((res = HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t*)buff, BUFFER_SIZE)) == HAL_OK)
	{
		return 1;	//si la transmision se ha efectuado con exito
	}
	else
	{
		return 0; 	//si ha habido error en la transmision o esta ocupado
	}*/
}

void selectMusicBuffer()
{
	if(isConf == 1 && songPlaying != NULL)
	{
		int flag = 1;
		if(buf1Playing == 1)
		{
			if(sendMusicBuffer(buffer[0]) == 1)
			{
				buf1CanRead = 1;	//una vez termina de mandarlo, pasa a leer
				buf1Playing = 0;	//y ya no transmite
				buf2CanRead = 0;	//el buffer contrario ya no lee
				buf2Playing = 1;	//el buffer contrario pasa a enviar sonido
				flag = 0;			//flag para no verificar el if siguiente sin pasar un ciclo primero
			}
		}
		if(buf2Playing == 1 && flag == 1)
		{
				if(sendMusicBuffer(buffer[1]) == 1)
				{
					buf2CanRead = 1;	//una vez termina de mandarlo, pasa a leer
					buf2Playing = 0;	//y ya no transmite
					buf1CanRead = 0;	//el buffer contrario ya no lee
					buf1Playing = 1;	//el buffer contrario pasa a enviar sonido
				}
		}
	}
}

void changeSong(uint8_t signal)
{
	if(isConf == 1)
	{
		if(signal == 1)
		{
			if(playing < foundSongs)
			{
				playing++;
			}
			else
			{
				playing = 0;
			}
			bytesLeidos = 0;
			songPlaying = (char*)malloc(sizeof(mp3Files[playing]));
			strcpy(songPlaying, mp3Files[playing]);
			buf1CanRead = 1;
		}
	}
}
void mainFileSystem(uint8_t buttonSong)
{
	initFileSystem();
	mountFileSystem();
	indexFiles();
	configMusicRegisters(buffer, NUM_BUFFERS, BUFFER_SIZE);
	changeSong(buttonSong);
	updatePingPongBuffers();
	selectMusicBuffer();
}
#endif /* APPLICATION_USER_FILESYSTEM_C_ */
