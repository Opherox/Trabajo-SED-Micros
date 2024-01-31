/*
 * WavPlayer.h
 *
 *  Created on: Jan 30, 2024
 *      Author: Opherox
 */

#ifndef INC_WAVPLAYER_H_
#define INC_WAVPLAYER_H_

#include <stm32f4xx_hal.h>
#include <ff.h>

#include <stdbool.h>

typedef struct {
    char     chunkID[4];
    uint32_t chunkSize;
    char     format[4];
    char     subchunk1ID[4];
    uint32_t subchunk1Size;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    char     subchunk2ID[4];
    uint32_t subchunk2Size;
} WavHeader;

void WavPLAYER_init(I2C_HandleTypeDef *hi2c, I2S_HandleTypeDef *hi2s);
bool WavPLAYER_isPlaying();
FRESULT ReadWavHeader(const char* filePath, WavHeader* header);
int WavAllocateMemory();
int WavRead(const char filename[], uint16_t buffer[], uint32_t *bytesRead);
bool WavPlaterStop();
void WavPLAYER_setVolume(uint16_t vol);
void TransmitAudio(uint16_t buff[]);

#endif /* INC_WAVPLAYER_H_ */
