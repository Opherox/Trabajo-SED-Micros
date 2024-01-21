/*
 * FileSystem.h
 *
 *  Created on: Jan 20, 2024
 *      Author: Opherox
 */

#ifndef INC_FILESYSTEM_H_
#define INC_FILESYSTEM_H_

static FATFS fs;
static FILINFO finfo;
#define MAX_MUSIC 30 //maximo de 30 canciones
#define BUFFER_SIZE 51200 //50KB
#define DELAY 500 //0,5s
#define NUM_BUFFERS 2
static FRESULT result;
static const uint8_t oktext[] = "USB mounted !";
static const uint8_t etext[] = "USB not mounted !";
static uint8_t isMounted = 0;
static uint8_t isIndexed = 0;
static uint8_t isConf = 0;
static uint8_t buf1CanRead = 0;
static uint8_t buf2CanRead = 0;
static uint8_t buf1Playing = 0;
static uint8_t buf2Playing = 0;
static uint8_t foundSongs = 0;
static int playing = -1;
static uint8_t bytesLeidos = 0;
static DIR dir;
static char *mp3Files[MAX_MUSIC]; //matriz de chars para almacenar nombres de ficheros que son canciones
static char *buffer[NUM_BUFFERS]; //bloque 1 y 2 para musica
static char *songPlaying = NULL;

void initFileSystem(void);
void mountFileSystem(void);
int scanMp3Music(void);
void indexFiles(void);
int configMusicRegisters(char *b[], int i, int size); //vector de buffers, el numero de buffers que queremos hacer como argumento y su tamaño
int readFile(const char *filename, char *buffer, int size, uint8_t bytesRead);
void updatePingPongBuffers();
int sendMusicBuffer(char* buff);
void selectMusicBuffer(void);
void changeSong(uint8_t bCS);
void mainFileSystem(uint8_t bCS);

#endif /* INC_FILESYSTEM_H_ */
