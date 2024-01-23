/*
 * player.c
 *
 *  Created on: Jan 23, 2024
 *      Author: lcastedo
 */

#include <player.h>

#include <CS43L22_I2C.h>

#define PLAYER_BUF_SIZE (4096)

extern void playerFinishedCallback(void);  /* Called when stop playing & file closed. To write in app code */

static I2S_HandleTypeDef *PLAYER_hi2s;

FIL PLAYER_songFile;
static int PLAYER_nxtbuf_idx;
static uint16_t PLAYER_buf[2][PLAYER_BUF_SIZE];
static uint16_t PLAYER_buf_len[2];
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
  playerFinishedCallback();        /* Notify app */

  CS43L22_OFF();
}

void PLAYER_setVolume(unsigned vol)
{
  /* TODO: codificar función */
}

/*
 *  Refill next buffer before current run out of data
 */
void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
  if (hi2s != PLAYER_hi2s)
    Error_Handler();

  UINT rxBytes;
  FRESULT res = f_read(&PLAYER_songFile, PLAYER_buf[PLAYER_nxtbuf_idx], PLAYER_BUF_SIZE << 1, &rxBytes);
  if (res != FR_OK)
  {
    PLAYER_buf_len[PLAYER_nxtbuf_idx] = 0; /* Signal error to completion callback */
    return;
  }

  PLAYER_buf_len[PLAYER_nxtbuf_idx] = (uint16_t)rxBytes >> 1;
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

  HAL_StatusTypeDef st = HAL_I2S_Transmit_DMA(PLAYER_hi2s, PLAYER_buf[PLAYER_nxtbuf_idx], PLAYER_buf_len[PLAYER_nxtbuf_idx]);
  if (st != HAL_OK)
    Error_Handler();

  PLAYER_nxtbuf_idx = 1 - PLAYER_nxtbuf_idx;  /* Update next buffer index */
}
