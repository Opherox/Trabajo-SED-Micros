/*
 * WavPlayer.c
 *
 *  Created on: Jan 30, 2024
 *      Author: Opherox
 */

#include <WavPlayer.h>
#include <CS43L22_I2C.h>

#define PLAYER_BUF_CHANNELS         (2)

static I2S_HandleTypeDef *PLAYER_hi2s;

static uint16_t *wavBuffer[2];

static int PLAYER_nxtbuf_idx;

static bool PLAYER_playing = false;

static FIL PLAYER_songFile;

static int memoryAllocated = 0;

static unsigned int BytesRead = 0;

extern WavHeader header;

static uint16_t PLAYER_buf_len[2];

void WavPLAYER_init(I2C_HandleTypeDef *hi2c, I2S_HandleTypeDef *hi2s)
{
  PLAYER_hi2s = hi2s;
  PLAYER_nxtbuf_idx = 0;

  CS43L22_Init(hi2c);
}

bool WavPLAYER_isPlaying()
{
  return PLAYER_playing;
}

FRESULT ReadWavHeader(const char* filePath, WavHeader* header)
{
    FIL file;
    FRESULT result;
    // Open the file
    result = f_open(&file, filePath, FA_READ);
    if (result != FR_OK)
    {
        //error
        return result;
    }
    // Read the WAV header
    result = f_read(&file, header, sizeof(WavHeader), NULL);
    // Close the file
    f_close(&file);
    return result;
}

int WavAllocateMemory()
{
	memoryAllocated = 1;
	for(int i=0; i<2;i++)
	{
		wavBuffer[i] = (uint16_t*)malloc(header.subchunk2Size);
		if(wavBuffer[i] == NULL)
		{
			for(int j=0; j<2;j++)
			{
				free(wavBuffer[j]);
			}
			memoryAllocated = 0;
			return 0;
		}
	}
	return 1;
}

unsigned int WavReadCallback(void *PCMBuffer, unsigned int bufferSize, void *token)
{
    UINT rxBytes;
    FRESULT res = f_read((FIL *)token, PCMBuffer, bufferSize, &rxBytes);
    BytesRead += rxBytes;
    // Assuming 16-bit stereo data, adjust accordingly
    unsigned int samplesRead = rxBytes / sizeof(short) / 2;

    return res == FR_OK ? samplesRead : 0;
}

bool WavPLAYER_play(const char *songFPath)
{
    WavPLAYER_stop(); // Preventive play termination

    FRESULT res = f_open(&PLAYER_songFile, songFPath, FA_READ);
    if (res != FR_OK)
    {
        // Handle file open error (e.g., log or return false)
        return false;
    }

    // Assuming 16-bit stereo data, adjust accordingly
    CS43L22_ON();

    PLAYER_playing = true;
    PLAYER_nxtbuf_idx = 0;

    WavReadCallback(wavBuffer[PLAYER_nxtbuf_idx], header.subchunk2Size, &PLAYER_songFile);

    // Assuming 16-bit stereo data, adjust accordingly
    HAL_I2S_TxHalfCpltCallback(PLAYER_hi2s);
    HAL_I2S_TxCpltCallback(PLAYER_hi2s);
    return true;
}

void WavPLAYER_stop()
{
    if (PLAYER_playing)
    {
        PLAYER_playing = false;
        CS43L22_OFF();
        f_close(&PLAYER_songFile);
        BytesRead = 0;
    }
}

void WavPLAYER_setVolume(uint16_t vol)
{
	CS43L22_Volume(vol);
}

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
  if (hi2s != PLAYER_hi2s)
    Error_Handler();

  PLAYER_buf_len[PLAYER_nxtbuf_idx] = (uint16_t)(header.subchunk2Size * PLAYER_BUF_CHANNELS);
}

/*
 *  Change to next buffer & schedule transfer to CODEC
 */
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
  if (hi2s != PLAYER_hi2s)
    Error_Handler();

  if (PLAYER_buf_len[PLAYER_nxtbuf_idx] == 0) /* Error or no more data */
  {
    PLAYER_stop();
    return;
  }

  HAL_StatusTypeDef st = HAL_I2S_Transmit_DMA(
    PLAYER_hi2s,
    (uint16_t *)wavBuffer[PLAYER_nxtbuf_idx],
    PLAYER_buf_len[PLAYER_nxtbuf_idx]
  );
  if (st != HAL_OK)
    Error_Handler();

  PLAYER_nxtbuf_idx = 1 - PLAYER_nxtbuf_idx;  /* Update next buffer index */
}
