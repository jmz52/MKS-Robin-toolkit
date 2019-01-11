#include "Arduino.h"
#include "sdio.h"

#include <HardwareSerial.h>

SDIO_CardInfoTypeDef      SdCard;           /* SD Card information */

void sdio(void) {
  if (!SDIO_Init()) {
    Serial3.println("SDIO_Init()");
    Serial3.flush();
    return;
  }

// delay(1);

 uint8_t sector[1024];
 uint32_t i;
 for (i = 0; i < sizeof(sector); i++) { sector[i] = 0xFF; }

 uint8_t mbr[512];
 for (i = 0; i < sizeof(mbr); i++) { mbr[i] = (uint8_t)((i + 0x42) & 0xFF); }

  if (!SDIO_WriteBlocks(mbr, 1, 1)) {
    Serial3.println("HAL_SD_WriteBlocks()");
    Serial3.flush();
  }

  if (!SDIO_ReadBlocks(sector, 1, sizeof(sector) / 512)) {
    Serial3.println("HAL_SD_ReadBlocks()");
    Serial3.flush();
    return;
  }

  for (i = 0; i < sizeof(sector); i++) {
    if (sector[i] < 16) {
      Serial3.print("0");
    }
    Serial3.print(sector[i], HEX);
    if ((i & 0x0F) == 0x0F) {
      Serial3.println("");
      Serial3.flush();
    } else {
      Serial3.print(" ");
      if ((i & 0x03) == 0x03) {
        Serial3.print(" ");
      }
    }
  }

  return;
}
/****************************************************************************************************************************************************************************************/
/****************************************************************************************************************************************************************************************/

bool SDIO_CmdGoIdleState(void) { SDIO_SendCommand(CMD0_GO_IDLE_STATE, 0); return SDIO_GetCmdError(); }
bool SDIO_CmdSendCID(void) { SDIO_SendCommand(CMD2_ALL_SEND_CID, 0); return SDIO_GetCmdResp2(); }
bool SDIO_CmdSetRelAdd(uint32_t *rca) { SDIO_SendCommand(CMD3_SET_REL_ADDR, 0); return SDIO_GetCmdResp6(SDMMC_CMD_SET_REL_ADDR, rca); }
bool SDIO_CmdSelDesel(uint32_t address) { SDIO_SendCommand(CMD7_SEL_DESEL_CARD, address); return SDIO_GetCmdResp1(SDMMC_CMD_SEL_DESEL_CARD); }
bool SDIO_CmdOperCond(void) { SDIO_SendCommand(CMD8_HS_SEND_EXT_CSD, SDMMC_CHECK_PATTERN); return SDIO_GetCmdResp7(); }
bool SDIO_CmdSendCSD(uint32_t argument) { SDIO_SendCommand(CMD9_SEND_CSD, argument); return SDIO_GetCmdResp2(); }
bool SDIO_CmdStopTransfer(void) { SDIO_SendCommand(CMD12_STOP_TRANSMISSION, 0); return SDIO_GetCmdResp1(SDMMC_CMD_STOP_TRANSMISSION); }
bool SDIO_CmdSendStatus(uint32_t argument) { SDIO_SendCommand(CMD13_SEND_STATUS, argument); return SDIO_GetCmdResp1(SDMMC_CMD_SEND_STATUS); }
bool SDIO_CmdBlockLength(uint32_t blockSize) { SDIO_SendCommand(CMD16_SET_BLOCKLEN, blockSize); return SDIO_GetCmdResp1(SDMMC_CMD_SET_BLOCKLEN); }
bool SDIO_CmdReadSingleBlock(uint32_t address) { SDIO_SendCommand(CMD17_READ_SINGLE_BLOCK, address); return SDIO_GetCmdResp1(SDMMC_CMD_READ_SINGLE_BLOCK); }
bool SDIO_CmdReadMultiBlock(uint32_t address) { SDIO_SendCommand(CMD18_READ_MULT_BLOCK, address); return SDIO_GetCmdResp1(SDMMC_CMD_READ_MULT_BLOCK); }
bool SDIO_CmdWriteSingleBlock(uint32_t address) { SDIO_SendCommand(CMD24_WRITE_SINGLE_BLOCK, address); return SDIO_GetCmdResp1(SDMMC_CMD_WRITE_SINGLE_BLOCK); }
bool SDIO_CmdWriteMultiBlock(uint32_t address) { SDIO_SendCommand(CMD25_WRITE_MULT_BLOCK, address); return SDIO_GetCmdResp1(SDMMC_CMD_WRITE_MULT_BLOCK); }
bool SDIO_CmdAppCommand(uint32_t rsa) { SDIO_SendCommand(CMD55_APP_CMD, rsa); return SDIO_GetCmdResp1(SDMMC_CMD_APP_CMD); }


bool SDIO_CmdAppSetBusWidth(uint32_t rsa, uint32_t argument) {
  if (!SDIO_CmdAppCommand(rsa)) { return false; }
  SDIO_SendCommand(ACMD6_APP_SD_SET_BUSWIDTH, argument);
  return SDIO_GetCmdResp2();
}

bool SDIO_CmdAppOperCommand(uint32_t sdType) {
  if (!SDIO_CmdAppCommand(0)) { return false; }
  SDIO_SendCommand(ACMD41_SD_APP_OP_COND , SDMMC_VOLTAGE_WINDOW_SD | sdType);
  return SDIO_GetCmdResp3();
}

bool SDIO_CmdAppSetClearCardDetect(uint32_t rsa) {
  if (!SDIO_CmdAppCommand(rsa)) { return false; }
  SDIO_SendCommand(ACMD42_SD_APP_SET_CLR_CARD_DETECT, 0);
  return SDIO_GetCmdResp2();
}

void SDIO_SendCommand(uint16_t command, uint32_t argument) { SDIO->ARG = argument; SDIO->CMD = (uint32_t)(SDIO_CMD_CPSMEN | command); }
uint8_t SDIO_GetCommandResponse(void) { return (uint8_t)(SDIO->RESPCMD); }
uint32_t SDIO_GetResponse(uint32_t response) { return SDIO->RESP[response]; }

bool SDIO_GetCmdError(void) {
  register uint32_t count = SDIO_CMDTIMEOUT * (F_CPU / 8U / 1000U);
  do {
    if (count-- == 0U) {
      return false;
    }
  } while (!SDIO_GET_FLAG(SDIO_STA_CMDSENT));

  SDIO_CLEAR_FLAG(SDIO_ICR_CMD_FLAGS);
  return true;
}

bool SDIO_GetCmdResp1(uint8_t command) {
  register uint32_t count = SDIO_CMDTIMEOUT * (F_CPU / 8U / 1000U);
  do {
    if (count-- == 0U) {
      return false;
    }
  } while (!SDIO_GET_FLAG(SDIO_STA_CCRCFAIL | SDIO_STA_CMDREND | SDIO_STA_CTIMEOUT));

  if (SDIO_GET_FLAG(SDIO_STA_CCRCFAIL | SDIO_STA_CTIMEOUT)) {
    SDIO_CLEAR_FLAG(SDIO_STA_CCRCFAIL | SDIO_STA_CTIMEOUT);
    return false;
  }
  if (SDIO_GetCommandResponse() != command) {
    return false;
  }
  SDIO_CLEAR_FLAG(SDIO_ICR_CMD_FLAGS);
  return (SDIO_GetResponse(SDIO_RESP1) & SDMMC_OCR_ERRORBITS) == SDMMC_ALLZERO;
}

bool SDIO_GetCmdResp2(void) {
  register uint32_t count = SDIO_CMDTIMEOUT * (F_CPU / 8U / 1000U);
  do {
    if (count-- == 0U) { return false; }
  } while (!SDIO_GET_FLAG(SDIO_STA_CCRCFAIL | SDIO_STA_CMDREND | SDIO_STA_CTIMEOUT));

  if (SDIO_GET_FLAG(SDIO_STA_CCRCFAIL | SDIO_STA_CTIMEOUT)) {
    SDIO_CLEAR_FLAG(SDIO_STA_CCRCFAIL | SDIO_STA_CTIMEOUT);
    return false;
  }
  SDIO_CLEAR_FLAG(SDIO_ICR_CMD_FLAGS);
  return true;
}

bool SDIO_GetCmdResp3(void) {
  register uint32_t count = SDIO_CMDTIMEOUT * (F_CPU / 8U / 1000U);
  do {
    if (count-- == 0U) { return false; }
  } while (!SDIO_GET_FLAG(SDIO_STA_CCRCFAIL | SDIO_STA_CMDREND | SDIO_STA_CTIMEOUT));

  if (SDIO_GET_FLAG(SDIO_STA_CTIMEOUT)) {
    SDIO_CLEAR_FLAG(SDIO_STA_CTIMEOUT);
    return false;
  }

  SDIO_CLEAR_FLAG(SDIO_ICR_CMD_FLAGS);
  return true;
}

bool SDIO_GetCmdResp6(uint8_t command, uint32_t *rca) {
  register uint32_t count = SDIO_CMDTIMEOUT * (F_CPU / 8U / 1000U);
  do {
    if (count-- == 0U) {
      return false;
    }
  } while (!SDIO_GET_FLAG(SDIO_STA_CCRCFAIL | SDIO_STA_CMDREND | SDIO_STA_CTIMEOUT));

  if (SDIO_GET_FLAG(SDIO_STA_CCRCFAIL | SDIO_STA_CTIMEOUT)) {
    SDIO_CLEAR_FLAG(SDIO_STA_CCRCFAIL | SDIO_STA_CTIMEOUT);
    return false;
  }
  if (SDIO_GetCommandResponse() != command) {
    return false;
  }

  SDIO_CLEAR_FLAG(SDIO_ICR_CMD_FLAGS);
  if (SDIO_GetResponse(SDIO_RESP1) & (SDMMC_R6_GENERAL_UNKNOWN_ERROR | SDMMC_R6_ILLEGAL_CMD | SDMMC_R6_COM_CRC_FAILED)) {
    return false;
  }
  *rca = SDIO_GetResponse(SDIO_RESP1) >> 16;
  return true;
}

bool SDIO_GetCmdResp7(void) {
  register uint32_t count = SDIO_CMDTIMEOUT * (F_CPU / 8U / 1000U);
  do {
    if (count-- == 0U) { return false; }
  } while (!SDIO_GET_FLAG(SDIO_STA_CCRCFAIL | SDIO_STA_CMDREND | SDIO_STA_CTIMEOUT));

  if (SDIO_GET_FLAG(SDIO_STA_CTIMEOUT)) {
    SDIO_CLEAR_FLAG(SDIO_STA_CTIMEOUT);
    return false;
  }
  if (SDIO_GET_FLAG(SDIO_STA_CMDREND)) {
    SDIO_CLEAR_FLAG(SDIO_STA_CMDREND);
  }
  return true;
}

/****************************************************************************************************************************************************************************************/
/****************************************************************************************************************************************************************************************/

bool SDIO_Init(void) {
  sdio_begin();

  dma_init(SDIO_DMA_DEV);
	dma_disable(SDIO_DMA_DEV, SDIO_DMA_CHANNEL);
	dma_set_priority(SDIO_DMA_DEV, SDIO_DMA_CHANNEL, DMA_PRIORITY_VERY_HIGH);

  __IO uint32_t count = 0U;

  if (!SDIO_CmdGoIdleState()) {
    Serial3.println("CMD0_GO_IDLE_STATE");
    Serial3.flush();
    return false;
  }

  SdCard.CardVersion = SDIO_CmdOperCond() ? CARD_V2_X : CARD_V1_X;

  do {
    if (count++ == SDMMC_MAX_VOLT_TRIAL) {
      Serial3.println("SDMMC_ERROR_INVALID_VOLTRANGE");
      Serial3.flush();
      return false;
    }

    /* Send ACMD41 SD_APP_OP_COND */
    if (!SDIO_CmdAppOperCommand(SdCard.CardVersion == CARD_V2_X ? SDMMC_HIGH_CAPACITY : SDMMC_STD_CAPACITY)) {
      Serial3.println("SDMMC_ERROR_UNSUPPORTED_FEATURE");
      Serial3.flush();
      return false;
    }
  } while ((SDIO_GetResponse(SDIO_RESP1) & 0x80000000) == 0);

  SdCard.CardType = (SDIO_GetResponse(SDIO_RESP1) & SDMMC_HIGH_CAPACITY) ? CARD_SDHC_SDXC : CARD_SDSC;

   /* Send CMD2 ALL_SEND_CID */
  if (!SDIO_CmdSendCID()) {
    Serial3.println("CMD2_ALL_SEND_CID");
    Serial3.flush();
    return false;
  }

  /* Send CMD3 SET_REL_ADDR with argument 0 */
  /* SD Card publishes its RCA. */
  if (!SDIO_CmdSetRelAdd(&SdCard.RelCardAdd)) {
    Serial3.println("CMD3_SET_REL_ADDR");
    Serial3.flush();
    return false;
  }

  /* Send CMD9 SEND_CSD with argument as card's RCA */
  if (!SDIO_CmdSendCSD((uint32_t)(SdCard.RelCardAdd << 16U))) {
    Serial3.println("CMD9_SEND_CSD");
    Serial3.flush();
    return false;
  }

  SdCard.Class = (SDIO_GetResponse(SDIO_RESP2) >> 20U);

  if (SdCard.CardType == CARD_SDHC_SDXC) {
    SdCard.LogBlockNbr = SdCard.BlockNbr = (((SDIO_GetResponse(SDIO_RESP2) & 0x0000003FU) << 26U) | ((SDIO_GetResponse(SDIO_RESP3) & 0xFFFF0000U) >> 6U)) + 1024;
    SdCard.LogBlockSize = SdCard.BlockSize = 512U;
  } else {
    SdCard.BlockNbr  = ((((SDIO_GetResponse(SDIO_RESP2) & 0x000003FFU) << 2U ) | ((SDIO_GetResponse(SDIO_RESP3) & 0xC0000000U) >> 30U)) + 1U) * (4U << ((SDIO_GetResponse(SDIO_RESP3) & 0x00038000U) >> 15U));
    SdCard.BlockSize = 1U << ((SDIO_GetResponse(SDIO_RESP2) >> 16) & 0x0FU);
    SdCard.LogBlockNbr =  (SdCard.BlockNbr) * ((SdCard.BlockSize) / 512U);
    SdCard.LogBlockSize = 512U;
  }


if (SdCard.CardVersion == CARD_V2_X) Serial3.println("CARD_V2_X"); else Serial3.println("CARD_V1_X");
if (SdCard.CardType == CARD_SDHC_SDXC) Serial3.println("CARD_SDHC_SDXC"); else Serial3.println("CARD_SDSC");
Serial3.print("Class : ");
Serial3.println(SdCard.Class);
Serial3.print("BlockNbr : ");
Serial3.println(SdCard.BlockNbr);
Serial3.print("BlockSize : ");
Serial3.println(SdCard.BlockSize);
Serial3.print("LogBlockNbr : ");
Serial3.println(SdCard.LogBlockNbr);
Serial3.print("LogBlockSize : ");
Serial3.println(SdCard.LogBlockSize);
Serial3.flush();

  if (!SDIO_CmdSelDesel((uint32_t)(SdCard.RelCardAdd << 16U))) {
    Serial3.println("CMD7_SEL_DESEL_CARD");
    Serial3.flush();
    return false;
  }

  if (!SDIO_CmdAppSetClearCardDetect(SdCard.RelCardAdd << 16U)) {
    Serial3.println("ACMD42 to disconnect D3 pullup failed");
    Serial3.flush();
    return false;
  }

  if (!SDIO_CmdAppSetBusWidth(SdCard.RelCardAdd << 16U, 2)) {
    Serial3.println("ACMD6_APP_SD_SET_BUSWIDTH");
    Serial3.flush();
    return false;
  }

  sdio_set_dbus_width(SDIO_CLKCR_WIDBUS_4BIT);
  sdio_set_clock(18000000);
  return true;
}

bool SDIO_ReadBlocks(uint8_t *data, uint32_t blockAddress, uint32_t numberOfBlocks) {
  if (SDIO_GetCardState() != SDIO_CARD_TRANSFER) { return false; }
  if ((blockAddress + numberOfBlocks) > SdCard.LogBlockNbr) { return false; }
  if ((0x03 & (uint32_t)data) || numberOfBlocks == 0) { return false; }

  bool errorstate;

  if (SdCard.CardType != CARD_SDHC_SDXC) { blockAddress *= 512U; }

  dma_setup_transfer(SDIO_DMA_DEV, SDIO_DMA_CHANNEL, &SDIO->FIFO, DMA_SIZE_32BITS, data, DMA_SIZE_32BITS, DMA_MINC_MODE);
  dma_set_num_transfers(SDIO_DMA_DEV, SDIO_DMA_CHANNEL, 128 * numberOfBlocks);
  dma_clear_isr_bits(SDIO_DMA_DEV, SDIO_DMA_CHANNEL);
  dma_enable(SDIO_DMA_DEV, SDIO_DMA_CHANNEL);

//  sdio_setup_transfer(0x00FFFFFFU, 512 * numberOfBlocks, SDIO_BLOCKSIZE_512 | SDIO_DCTRL_DMAEN | SDIO_DCTRL_DTEN | SDIO_DIR_RX);
  sdio_setup_transfer(SDIO_DATA_TIMEOUT * (F_CPU / 1000U), 512 * numberOfBlocks, SDIO_BLOCKSIZE_512 | SDIO_DCTRL_DMAEN | SDIO_DCTRL_DTEN | SDIO_DIR_RX);

  if (numberOfBlocks > 1U) {
    errorstate = SDIO_CmdReadMultiBlock(blockAddress);
  } else {
    errorstate = SDIO_CmdReadSingleBlock(blockAddress);
  }

  if (!errorstate) {
    SDIO_CLEAR_FLAG(SDIO_ICR_CMD_FLAGS);
    dma_disable(SDIO_DMA_DEV, SDIO_DMA_CHANNEL);
    return false;
  }

  while (!SDIO_GET_FLAG(SDIO_STA_DATAEND | SDIO_STA_TRX_ERROR_FLAGS)) {}

  dma_disable(SDIO_DMA_DEV, SDIO_DMA_CHANNEL);

  if (SDIO_GET_FLAG(SDIO_STA_DATAEND) && (numberOfBlocks > 1U)) {
    if (!SDIO_CmdStopTransfer()) {
      SDIO_CLEAR_FLAG(SDIO_ICR_CMD_FLAGS | SDIO_ICR_DATA_FLAGS);
      Serial3.println("SDIO_CmdStopTransfer()");
      Serial3.flush();
      return false;
    }
  }

  if (SDIO_GET_FLAG(SDIO_STA_TRX_ERROR_FLAGS)) {
    SDIO_CLEAR_FLAG(SDIO_ICR_CMD_FLAGS | SDIO_ICR_DATA_FLAGS);
    return false;
  }
  SDIO_CLEAR_FLAG(SDIO_ICR_CMD_FLAGS | SDIO_ICR_DATA_FLAGS);
  return true;
}

bool SDIO_WriteBlocks(uint8_t *data, uint32_t blockAddress, uint32_t numberOfBlocks) {
  if (SDIO_GetCardState() != SDIO_CARD_TRANSFER) { return false; }
  if ((blockAddress + numberOfBlocks) > SdCard.LogBlockNbr) { return false; }
  if ((0x03 & (uint32_t)data) || numberOfBlocks == 0) { return false; }

  if (SdCard.CardType != CARD_SDHC_SDXC) { blockAddress *= 512U; }

  dma_setup_transfer(SDIO_DMA_DEV, SDIO_DMA_CHANNEL, &SDIO->FIFO, DMA_SIZE_32BITS, data, DMA_SIZE_32BITS, DMA_MINC_MODE | DMA_FROM_MEM);
  dma_set_num_transfers(SDIO_DMA_DEV, SDIO_DMA_CHANNEL, 128);
  dma_clear_isr_bits(SDIO_DMA_DEV, SDIO_DMA_CHANNEL);
  dma_enable(SDIO_DMA_DEV, SDIO_DMA_CHANNEL);

  if (!SDIO_CmdWriteSingleBlock(blockAddress)) {
    dma_disable(SDIO_DMA_DEV, SDIO_DMA_CHANNEL);
    return false;
  }

  //sdio_setup_transfer(0x00FFFFFFU, 512U, SDIO_BLOCKSIZE_512 | SDIO_DCTRL_DMAEN | SDIO_DCTRL_DTEN);
  sdio_setup_transfer(SDIO_DATA_TIMEOUT * (F_CPU / 1000U), 512U, SDIO_BLOCKSIZE_512 | SDIO_DCTRL_DMAEN | SDIO_DCTRL_DTEN);

  while (!SDIO_GET_FLAG(SDIO_STA_DATAEND | SDIO_STA_TRX_ERROR_FLAGS)) {}

  dma_disable(SDIO_DMA_DEV, SDIO_DMA_CHANNEL);

  if (SDIO_GET_FLAG(SDIO_STA_TRX_ERROR_FLAGS)) {
    SDIO_CLEAR_FLAG(SDIO_ICR_CMD_FLAGS | SDIO_ICR_DATA_FLAGS);
    return false;
  }

  SDIO_CLEAR_FLAG(SDIO_ICR_CMD_FLAGS | SDIO_ICR_DATA_FLAGS);

  uint32 timeout = millis() + SDIO_WRITE_TIMEOUT;
  while (timeout > millis()) {
    if (SDIO_GetCardState() == SDIO_CARD_TRANSFER) {
      return true;
    }
  }
  return false;
}

inline uint32_t SDIO_GetCardState(void) { return SDIO_CmdSendStatus((uint32_t)(SdCard.RelCardAdd << 16U)) ? (SDIO_GetResponse(SDIO_RESP1) >> 9U) & 0x0FU : SDIO_CARD_ERROR; }
