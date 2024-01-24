/*
 * player.c
 *
 *  Created on: Jan 23, 2024
 *      Author: lcastedo
 */

#include <player.h>

#include <CS43L22_I2C.h>
#include <spiritMP3Dec.h>

#define PLAYER_MP3_GRANULE_SZ       (576)
#define PLAYER_BUF_SIZE_IN_SAMPLES  (4 * PLAYER_MP3_GRANULE_SZ)
#define PLAYER_BUF_CHANNELS         (2)

extern void playerFinishedCallback(void);  /* Called when stop playing & file closed. To write in app code */

unsigned int MP3ReadCallback(void *pMP3CompressedData, unsigned int nMP3DataSizeInChars, void *token);

static I2S_HandleTypeDef *PLAYER_hi2s;

static FIL PLAYER_songFile;

static int PLAYER_nxtbuf_idx;
static int16_t PLAYER_buf[2][PLAYER_BUF_CHANNELS * PLAYER_BUF_SIZE_IN_SAMPLES];
static uint16_t PLAYER_buf_len[2];

static TSpiritMP3Decoder PLAYER_mp3Decoder;

static bool PLAYER_playing = false;

void PLAYER_init(I2C_HandleTypeDef *hi2c, I2S_HandleTypeDef *hi2s)
{
  PLAYER_hi2s = hi2s;
  PLAYER_nxtbuf_idx = 0;

  CS43L22_Init(hi2c);
}

bool PLAYER_isPlaying()
{
  return PLAYER_playing;
}

bool PLAYER_play(const char *songFPath)
{
  PLAYER_stop(); /* Preventive play termination */

  FRESULT res = f_open(&PLAYER_songFile, songFPath, FA_READ);
  if (res != FR_OK)
    return false;

  CS43L22_ON();

  PLAYER_playing = true;
  PLAYER_nxtbuf_idx = 0;


  SpiritMP3DecoderInit(&PLAYER_mp3Decoder, MP3ReadCallback, NULL, &PLAYER_songFile);

  HAL_I2S_TxHalfCpltCallback(PLAYER_hi2s);  /* Force first data load */
  HAL_I2S_TxCpltCallback(PLAYER_hi2s);      /* Force first transfer to CODEC */

  return true;
}

void PLAYER_stop()
{
  if (!PLAYER_playing)
    return;

  HAL_I2S_DMAStop(PLAYER_hi2s);  /* Terminate pending transfer (if any) */
  f_close(&PLAYER_songFile);     /* Close open file */
  PLAYER_playing = false;
  playerFinishedCallback();      /* Notify app */

  CS43L22_OFF();
}

void PLAYER_setVolume(unsigned vol)
{
  /* TODO: codificar función */
}

/*
 *  MP3 decoder file reading callback
 */
unsigned int MP3ReadCallback(void *pMP3CompressedData, unsigned int nMP3DataSizeInChars, void *token)
{
  UINT rxBytes;
  FRESULT res = f_read((FIL *)token, pMP3CompressedData, nMP3DataSizeInChars, &rxBytes);

  return res == FR_OK ? rxBytes : 0;
}

/*
 *  Refill next buffer before current run out of data
 */
void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
  if (hi2s != PLAYER_hi2s)
    Error_Handler();

  unsigned rxSamples = SpiritMP3Decode(
      &PLAYER_mp3Decoder,
      PLAYER_buf[PLAYER_nxtbuf_idx],
      PLAYER_BUF_SIZE_IN_SAMPLES,
      NULL
  );
  PLAYER_buf_len[PLAYER_nxtbuf_idx] = (uint16_t)(rxSamples * PLAYER_BUF_CHANNELS);
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
    (uint16_t *)PLAYER_buf[PLAYER_nxtbuf_idx],
    PLAYER_buf_len[PLAYER_nxtbuf_idx]
  );
  if (st != HAL_OK)
    Error_Handler();

  PLAYER_nxtbuf_idx = 1 - PLAYER_nxtbuf_idx;  /* Update next buffer index */
}
