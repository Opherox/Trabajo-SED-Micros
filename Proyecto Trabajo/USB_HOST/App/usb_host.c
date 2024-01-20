/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file            : usb_host.c
  * @version         : v1.0_Cube
  * @brief           : This file implements the USB Host
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/

#include "usb_host.h"
#include "usbh_core.h"
#include "usbh_msc.h"
#include "ff.h"
#include "stm32f4xx_hal.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USB Host core handle declaration */
USBH_HandleTypeDef hUsbHostFS;
ApplicationTypeDef Appli_state = APPLICATION_IDLE;

/*
 * -- Insert your variables declaration here --
 */
/* USER CODE BEGIN 0 */
#define MAX_MUSIC 30
#define BUFFER_SIZE 51200 //50KB
#define DELAY 500 //0,5s
FATFS *fs;
FRESULT result;
const uint8_t oktext[] = "USB mounted !";
const uint8_t etext[] = "USB not mounted !";
static uint8_t isMounted = 0;
static uint8_t isIndexed = 0;
static uint8_t isConf = 0;
static uint8_t buf1CanRead = 0;
static uint8_t buf2CanRead = 0;
static uint8_t buf1Playing = 0;
static uint8_t buf2Playing = 0;
static uint8_t foundSongs = 0;
static uint8_t playing = 0;
static uint8_t bytesLeidos = 0;
int change = 0;
DIR dir;
FILINFO finfo;
char *mp3Files[MAX_MUSIC]; //matriz de chars para almacenar nombres de ficheros que son canciones
char *buffer[1]; //bloque 1 y 2 para musica
I2C_HandleTypeDef hi2c;

/* USER CODE END 0 */

/*
 * user callback declaration
 */
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id);

/*
 * -- Insert your external function declaration here --
 */
/* USER CODE BEGIN 1 */

int scanMp3Music(void)
{
	if (f_opendir(&dir, "/") == FR_OK) //abrimos el directorio raiz
	{
		int numf = 0; //para contar numero de archibos mp3
		while(f_readdir(&dir, &finfo) == FR_OK && finfo.fname[0] != 0 && numf < MAX_MUSIC) //mientras queden archivos por leer
		{
			if(!(finfo.fattrib & AM_DIR) && strstr(finfo.fname, ".mp3") != NULL) //si el archivo es un directorio y el nombre del archivo tiene una subcadena .mp3
			{
				mp3Files[numf] = finfo.fname;	//guardamos el nombre del archivo en un vector de chars, en el elemento de numero actual encontrado
				numf++;	//aumentamos el numero de archivos mp3 encontrados
				printf("Se han encontrado &d canciones.\n", numf);
			}
		}
		f_close(&dir);
		printf("Se ha podido abrir el directorio, hay &d canciones.\n",numf);
		return numf;
	}
	else
	{
		printf("Error al abrir el directorio.\n");
		return 0;
	}
}

int configMusicRegisters(char *b[], int i, int size) //vector de buffers, el numero de buffers que queremos hacer como argumento y su tamaño
{
	for(int j = 0; j < i; j++)
	{
		b[j] = (char*)malloc(size);
		if (b[j] == NULL)
		{
			for(int z = 0; z < j; z++)
			{
				free(buffer[z]); //liberamos la memoria de los buffers por el error al reservar memoria
			}
			printf("No se ha podido reservar memoria para el Buffer %d",j);
			return 0;
		}
		else
		{
			printf("Se ha podido reservar memoria para el Buffer %d", j);
		}
	}
	return 1;
}

int readMusicFile(char *song, char *buffer, int size, uint8_t bytesRead)
{
	FILE *songFile;
	songFile = fopen(song, "rb"); //abrimos el archivo en lectura binaria
	uint8_t realBytesRead;
	if(songFile == NULL)
	{
		printf("Error al abrir el archivo");
		return 1;
	}
	fseek(songFile, bytesRead, SEEK_SET); //nos ponemos al principio del archivo + los bytes leidos anteriormente
	if(fread(songFile, buffer, size, realBytesRead) == FR_OK) //si la lectura se ha podido realizar
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

/*TODO Creo que faltaria configurar y habilitar el periferico I2C, no estoy seguro, hacer mañana en un include aparte*/

int sendMusicBuffer(char* buffer)
{
	if(HAL_I2C_MasterTransmit(&hi2c, I2C_ADDRESS, buffer, sizeof(buffer), DELAY) == HAL_OK)
	{
		printf("Buffer enviado con exito");
		return 1;
	}
	else
	{
		printf("El buffer no se ha enviado correctamente");
		return 0;
	}
}

int changeSong(uint8_t playing, int change, uint8_t lastSong)
{
	if (change != 0) //si se pide cambiar de cancion
	{
		if(playing + change > 0) // si no intentamos ir a la anterior del 0
		{
			playing += change;
			change = 0;
		}
		if(playing + change > lastSong) //si sobrepasamos la capacidad maxima, vamos a la primera
		{
			playing = 0;
			change = 0;
		}
		else //si vamos de la primera a su anterior (-1) iriamos a la ultima de la lista
		{
			playing = lastSong;
			change = 0;
		}
		return 1;
	}
	else //si no se pide cambiar de cancion
	{
		return 0;
	}
	printf("Reproducciendo la cancion %d de la lista", playing);
}

/* USER CODE END 1 */

/**
  * Init USB host library, add supported class and start the library
  * @retval None
  */
void MX_USB_HOST_Init(void)
{
  /* USER CODE BEGIN USB_HOST_Init_PreTreatment */

  /* USER CODE END USB_HOST_Init_PreTreatment */

  /* Init host Library, add supported class and start the library. */
  if (USBH_Init(&hUsbHostFS, USBH_UserProcess, HOST_FS) != USBH_OK)
  {
    Error_Handler();
  }
  if (USBH_RegisterClass(&hUsbHostFS, USBH_MSC_CLASS) != USBH_OK)
  {
    Error_Handler();
  }
  if (USBH_Start(&hUsbHostFS) != USBH_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_HOST_Init_PostTreatment */

  /* USER CODE END USB_HOST_Init_PostTreatment */
}

/*
 * Background task
 */
void MX_USB_HOST_Process(void)
{
  /* USB Host Background task */
  USBH_Process(&hUsbHostFS);
}
/*
 * user callback definition
 */
static void USBH_UserProcess  (USBH_HandleTypeDef *phost, uint8_t id)
{
  /* USER CODE BEGIN CALL_BACK_1 */
  switch(id)
  {
  case HOST_USER_SELECT_CONFIGURATION:
  break;

  case HOST_USER_DISCONNECTION:
  Appli_state = APPLICATION_DISCONNECT;
  break;

  case HOST_USER_CLASS_ACTIVE:
  Appli_state = APPLICATION_READY;
  break;

  case HOST_USER_CONNECTION:
  Appli_state = APPLICATION_START;
  break;

  default:
  break;
  }

  /* TODO : Aqui va el codigo de buscar ficheros en el pen */

  //primero montamos el sistema de ficheros
  if (isMounted != 1) //solo se va a montar el sistema de ficheros una vez ya que no lo vamos a modificar
  {
	  result = f_mount(fs, "", 1); //puntero al objeto filesystem, direccion (default = raiz), 0 para no montar ahora (cuando se haga el primer acceso), 1 para forzarlo y ver si esta listo para trabajar.
	  if (result != FR_OK)
	    {
	  	  printf("Coudnt mount the file sistem for the USB.\n");
	  	  //HAL_UART_Transmit(&huart3, etext, sizeof(text), 100); //para deteccion de errores
	    }
	    else
	    {
	  	  printf("USB file system has been mounted.\n");
	  	  isMounted = 1;
	  	  //HAL_UART_Transmit(&huart3, oktext, sizeof(oktext), 100); //para deteccion de errores
	    }
  }
  //a continuacion filtramos en una lista los nombres de los archivos de musica
  if (isMounted == 1 && isIndexed == 0) //solo buscaremos una vez las canciones
  {
	  if((foundSongs = scanMp3Music()) != 0) //funcion buscar musica consigue encontrar las canciones, asignamos el numero de canciones encontradas
	  {
		  isIndexed = 1;
	  }

  }
  //procedemos a reservar memoria para mas tarde reproducir la musica
  if (isMounted == 1 && isIndexed == 1 && isConf == 0) //solo reservaremos memoria una vez para los registros de audio
  {
	  isConf = 1; //suponemos que lo va a conseguir configurar y modificamos el valor en caso de que no lo consiga
	  buf1CanRead = 1;
	  buf2CanRead = 1;
	  if(configMusicRegisters(buffer, 2, BUFFER_SIZE) == 1)
		  	  {
		  		  isConf = 0; //si no se consigue reservar memoria para alguno de los dos bloques, se vuelve a poner que no se ha configurado a 0
		  		  buf1CanRead = 0; //si no tenemos los buffers configurados no podemos empezar a leer los ficheros
		  		  buf2CanRead = 0;
		  	  }
  }
  //a continuacion toca leer el fichero para obtener el sonido en el formato en el que esta, alternando en la posicion de lectura
  if(isMounted == 1 && isIndexed == 1 && isConf == 1)
  {
	  if(changeSong(playing, change, foundSongs) == 1)
	  {
		  bytesLeidos = 0; //si cambias de cancion se empieza de 0 en el nuevo fichero
		  buf1Playing = 0, buf2Playing = 0; //se para de reproducir cualquier buffer
		  buf1CanRead = 1, buf2CanRead = 1; //se tiene que rellenar los buffers para luego mandarlos a reproducir
	  }
	  if(buf1CanRead == 1)
	  {
		  buf1CanRead = 0;
		  if(readMusicFile(mp3Files[playing], buffer[0], BUFFER_SIZE, bytesLeidos) == 1) //si no consigue leer nada
		  {
			  buf1CanRead = 1; //que siga intentandolo
		  }
		  else
		  {
		  buf1Playing = 1; //sino pasa a mandar a reproducir
		  }
	  }
	  if(buf2CanRead == 1)
	  {
		  buf2CanRead = 0;
		  if(readMusicFile(mp3Files[playing], buffer[1], BUFFER_SIZE, bytesLeidos) == 1) //si no consigue leer nada
		  {
			  buf2CanRead = 1; //que siga intentandolo
		  }
		  else
		  {
		  buf2Playing = 1; //sino pasa a mandar a reproducir
		  }
	  }
	  //finalmente, pasamos el buffer que se este reproduciendo en cada momento.
	  if(buf1Playing == 1)
	  {
		  if(sendMusicBuffer(buffer[0]) == 0)
		  {
			  buf1CanRead = 1;
			  buf1Playing = 0;	//cuando termine de reproducir todo lo de ese registro pasa a leer y ya no reproduce
		  }
	  }
	  if(buf2Playing == 1)
	  {
		  if(sendMusicBuffer(buffer[1]) == 0)
		  {
			  buf2CanRead = 1;
			  buf2Playing = 0;  //cuando termine de reproducir todo lo de ese registro pasa a leer y ya no reproduce
		  }
	  }
  }


  /* USER CODE END CALL_BACK_1 */
}

/**
  * @}
  */

/**
  * @}
  */

