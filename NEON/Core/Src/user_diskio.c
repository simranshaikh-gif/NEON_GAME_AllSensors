#include "user_diskio.h"
#include "main.h"

/* SD Card CS Pin - Using PA15 based on reference */
#define SD_CS_Pin GPIO_PIN_15
#define SD_CS_GPIO_Port GPIOA

/* SPI Handle - Using hspi3 as it is free on the board */
extern SPI_HandleTypeDef hspi3;
#define SPI_HANDLE hspi3

/* Definitions for MMC/SDC command */
#define CMD0 (0)           /* GO_IDLE_STATE */
#define CMD1 (1)           /* SEND_OP_COND (MMC) */
#define ACMD41 (0x80 + 41) /* SEND_OP_COND (SDC) */
#define CMD8 (8)           /* SEND_IF_COND */
#define CMD9 (9)           /* SEND_CSD */
#define CMD10 (10)         /* SEND_CID */
#define CMD12 (12)         /* STOP_TRANSMISSION */
#define ACMD23 (0x80 + 23) /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD16 (16)         /* SET_BLOCKLEN */
#define CMD17 (17)         /* READ_SINGLE_BLOCK */
#define CMD18 (18)         /* READ_MULTIPLE_BLOCK */
#define CMD23 (23)         /* SET_BLOCK_COUNT (MMC) */
#define CMD24 (24)         /* WRITE_BLOCK */
#define CMD25 (25)         /* WRITE_MULTIPLE_BLOCK */
#define CMD32 (32)         /* ERASE_ER_BLK_START */
#define CMD33 (33)         /* ERASE_ER_BLK_END */
#define CMD38 (38)         /* ERASE */
#define CMD55 (55)         /* APP_CMD */
#define CMD58 (58)         /* READ_OCR */

/* Card type flags (CardType) */
#define CT_MMC 0x01              /* MMC ver 3 */
#define CT_SD1 0x02              /* SD ver 1 */
#define CT_SD2 0x04              /* SD ver 2 */
#define CT_SDC (CT_SD1 | CT_SD2) /* SD */
#define CT_BLOCK 0x08            /* Block addressing */

static volatile DSTATUS Stat = STA_NOINIT;
static BYTE CardType;

/* Static functions for SPI and SD control */
static void SD_CS_LOW(void) {
  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);
}

static void SD_CS_HIGH(void) {
  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);
}

static void SD_SPI_Transmit(BYTE val) {
  HAL_SPI_Transmit(&SPI_HANDLE, &val, 1, 100);
}

static BYTE SD_SPI_Receive(void) {
  BYTE val = 0xFF;
  BYTE dummy = 0xFF;
  HAL_SPI_TransmitReceive(&SPI_HANDLE, &dummy, &val, 1, 100);
  return val;
}

static int wait_ready(UINT wt) {
  BYTE d;
  do {
    d = SD_SPI_Receive();
  } while (d != 0xFF && --wt);
  return (d == 0xFF) ? 1 : 0;
}

static void deselect(void) {
  SD_CS_HIGH();
  SD_SPI_Receive(); /* Dummy clock */
}

static int select(void) {
  SD_CS_LOW();
  if (wait_ready(500))
    return 1;
  deselect();
  return 0;
}

static int rcvr_datablock(BYTE *buff, UINT btr) {
  BYTE token;
  UINT tmr;

  for (tmr = 1000; tmr; tmr--) {
    token = SD_SPI_Receive();
    if (token != 0xFF)
      break;
    HAL_Delay(1);
  }
  if (token != 0xFE)
    return 0;

  do {
    *buff++ = SD_SPI_Receive();
    *buff++ = SD_SPI_Receive();
    *buff++ = SD_SPI_Receive();
    *buff++ = SD_SPI_Receive();
  } while (btr -= 4);

  SD_SPI_Receive(); /* Discard CRC */
  SD_SPI_Receive();
  return 1;
}

static int xmit_datablock(const BYTE *buff, BYTE token) {
  BYTE resp;
  UINT wc;

  if (wait_ready(500) != 1)
    return 0;

  SD_SPI_Transmit(token);
  if (token != 0xFD) {
    wc = 512;
    do {
      SD_SPI_Transmit(*buff++);
      SD_SPI_Transmit(*buff++);
      SD_SPI_Transmit(*buff++);
      SD_SPI_Transmit(*buff++);
    } while (wc -= 4);

    SD_SPI_Receive(); /* CRC (dummy) */
    SD_SPI_Receive();
    resp = SD_SPI_Receive();
    if ((resp & 0x1F) != 0x05)
      return 0;
  }
  return 1;
}

static BYTE send_cmd(BYTE cmd, DWORD arg) {
  BYTE n, res;

  if (cmd & 0x80) {
    cmd &= 0x7F;
    res = send_cmd(CMD55, 0);
    if (res > 1)
      return res;
  }

  deselect();
  if (!select())
    return 0xFF;

  SD_SPI_Transmit(0x40 | cmd);
  SD_SPI_Transmit((BYTE)(arg >> 24));
  SD_SPI_Transmit((BYTE)(arg >> 16));
  SD_SPI_Transmit((BYTE)(arg >> 8));
  SD_SPI_Transmit((BYTE)arg);
  n = 0x01;
  if (cmd == CMD0)
    n = 0x95;
  if (cmd == CMD8)
    n = 0x87;
  SD_SPI_Transmit(n);

  if (cmd == CMD12)
    SD_SPI_Receive();
  n = 10;
  do {
    res = SD_SPI_Receive();
  } while ((res & 0x80) && --n);

  return res;
}

/* Public Functions */
DSTATUS USER_initialize(BYTE pdrv) {
  BYTE n, ty, ocr[4];
  if (pdrv)
    return STA_NOINIT;

  SD_CS_HIGH();
  for (n = 0; n < 10; n++)
    SD_SPI_Receive();

  ty = 0;
  if (send_cmd(CMD0, 0) == 1) {
    if (send_cmd(CMD8, 0x1AA) == 1) {
      for (n = 0; n < 4; n++)
        ocr[n] = SD_SPI_Receive();
      if (ocr[2] == 0x01 && ocr[3] == 0xAA) {
        while (send_cmd(ACMD41, 1UL << 30) & 1)
          ;
        if (send_cmd(CMD58, 0) == 0) {
          for (n = 0; n < 4; n++)
            ocr[n] = SD_SPI_Receive();
          ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
        }
      }
    } else {
      if (send_cmd(ACMD41, 0) <= 1) {
        ty = CT_SD1;
      } else {
        ty = CT_MMC;
      }
      while (send_cmd((ty == CT_SD1) ? ACMD41 : CMD1, 0))
        ;
      if (send_cmd(CMD16, 512) != 0)
        ty = 0;
    }
  }
  CardType = ty;
  deselect();

  if (ty)
    Stat &= ~STA_NOINIT;
  else
    Stat = STA_NOINIT;

  return Stat;
}

DSTATUS USER_status(BYTE pdrv) {
  if (pdrv)
    return STA_NOINIT;
  return Stat;
}

DRESULT USER_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count) {
  if (pdrv || !count)
    return RES_PARERR;
  if (Stat & STA_NOINIT)
    return RES_NOTRDY;

  if (!(CardType & CT_BLOCK))
    sector *= 512;

  if (count == 1) {
    if ((send_cmd(CMD17, sector) == 0) && rcvr_datablock(buff, 512))
      count = 0;
  } else {
    if (send_cmd(CMD18, sector) == 0) {
      do {
        if (!rcvr_datablock(buff, 512))
          break;
        buff += 512;
      } while (--count);
      send_cmd(CMD12, 0);
    }
  }
  deselect();
  return count ? RES_ERROR : RES_OK;
}

DRESULT USER_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count) {
  if (pdrv || !count)
    return RES_PARERR;
  if (Stat & STA_NOINIT)
    return RES_NOTRDY;

  if (!(CardType & CT_BLOCK))
    sector *= 512;

  if (count == 1) {
    if ((send_cmd(CMD24, sector) == 0) && xmit_datablock(buff, 0xFE))
      count = 0;
  } else {
    if (CardType & CT_SDC)
      send_cmd(CMD55, 0);
    if (send_cmd(CMD25, sector) == 0) {
      do {
        if (!xmit_datablock(buff, 0xFC))
          break;
        buff += 512;
      } while (--count);
      if (!xmit_datablock(0, 0xFD))
        count = 1;
    }
  }
  deselect();
  return count ? RES_ERROR : RES_OK;
}

DRESULT USER_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
  DRESULT res;
  BYTE n, csd[16];
  DWORD csize;

  if (pdrv)
    return RES_PARERR;
  if (Stat & STA_NOINIT)
    return RES_NOTRDY;

  res = RES_ERROR;
  switch (cmd) {
  case CTRL_SYNC:
    if (select())
      res = RES_OK;
    break;
  case GET_SECTOR_COUNT:
    if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {
      if ((csd[0] >> 6) == 1) {
        csize = csd[9] + ((WORD)csd[8] << 8) + 1;
        *(DWORD *)buff = csize << 10;
      } else {
        n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
        csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) +
                ((WORD)(csd[6] & 3) << 10) + 1;
        *(DWORD *)buff = csize << (n - 9);
      }
      res = RES_OK;
    }
    break;
  case GET_BLOCK_SIZE:
    *(DWORD *)buff = 128;
    res = RES_OK;
    break;
  default:
    res = RES_PARERR;
  }
  deselect();
  return res;
}

Diskio_drvTypeDef USER_Driver = {
    USER_initialize, USER_status, USER_read, USER_write, USER_ioctl,
};
