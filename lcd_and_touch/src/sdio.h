#include "libmaple/sdio.h"
#include <libmaple/dma.h>
//#include <boards.h>

void sdio(void);

#define SDMMC_ALLZERO                        0x00000000U

/*
#define SDMMC_OCR_ADDR_OUT_OF_RANGE          0x80000000U
#define SDMMC_OCR_ADDR_MISALIGNED            0x40000000U
#define SDMMC_OCR_BLOCK_LEN_ERR              0x20000000U
#define SDMMC_OCR_ERASE_SEQ_ERR              0x10000000U
#define SDMMC_OCR_BAD_ERASE_PARAM            0x08000000U
#define SDMMC_OCR_WRITE_PROT_VIOLATION       0x04000000U
#define SDMMC_OCR_LOCK_UNLOCK_FAILED         0x01000000U
#define SDMMC_OCR_COM_CRC_FAILED             0x00800000U
#define SDMMC_OCR_ILLEGAL_CMD                0x00400000U
#define SDMMC_OCR_CARD_ECC_FAILED            0x00200000U
#define SDMMC_OCR_CC_ERROR                   0x00100000U
#define SDMMC_OCR_GENERAL_UNKNOWN_ERROR      0x00080000U
#define SDMMC_OCR_STREAM_READ_UNDERRUN       0x00040000U
#define SDMMC_OCR_STREAM_WRITE_OVERRUN       0x00020000U
#define SDMMC_OCR_CID_CSD_OVERWRITE          0x00010000U
#define SDMMC_OCR_WP_ERASE_SKIP              0x00008000U
#define SDMMC_OCR_CARD_ECC_DISABLED          0x00004000U
#define SDMMC_OCR_ERASE_RESET                0x00002000U
#define SDMMC_OCR_AKE_SEQ_ERROR              0x00000008U
*/
#define SDMMC_OCR_ERRORBITS                  0xFDFFE008U

#define SDMMC_R6_GENERAL_UNKNOWN_ERROR       0x00002000U
#define SDMMC_R6_ILLEGAL_CMD                 0x00004000U
#define SDMMC_R6_COM_CRC_FAILED              0x00008000U

#define SDMMC_VOLTAGE_WINDOW_SD              0x80100000U
#define SDMMC_HIGH_CAPACITY                  0x40000000U
#define SDMMC_STD_CAPACITY                   0x00000000U
#define SDMMC_CHECK_PATTERN                  0x000001AAU

#define SDIO_TRANSFER_MODE_BLOCK             0x00000000U
#define SDIO_DPSM_ENABLE                     0x00000001U
#define SDIO_TRANSFER_DIR_TO_CARD            0x00000000U
#define SDIO_DATABLOCK_SIZE_512B             0x00000090U
#define SDIO_TRANSFER_DIR_TO_SDIO            0x00000100U
#define SDIO_DMA_ENABLE                      0x00001000U

#define SDMMC_CMD_GO_IDLE_STATE                       ((uint8_t)0)   /* Resets the SD memory card. */
#define SDMMC_CMD_ALL_SEND_CID                        ((uint8_t)2)   /* Asks any card connected to the host to send the CID numbers on the CMD line. */
#define SDMMC_CMD_SET_REL_ADDR                        ((uint8_t)3)   /* Asks the card to publish a new relative address (RCA). */
#define SDMMC_CMD_SEL_DESEL_CARD                      ((uint8_t)7)   /* Selects the card by its own relative address and gets deselected by any other address */
#define SDMMC_CMD_HS_SEND_EXT_CSD                     ((uint8_t)8)   /* Sends SD Memory Card interface condition, which includes host supply voltage information and asks the card whether card supports voltage. */
#define SDMMC_CMD_SEND_CSD                            ((uint8_t)9)   /* Addressed card sends its card specific data (CSD) on the CMD line. */
#define SDMMC_CMD_STOP_TRANSMISSION                   ((uint8_t)12)  /*!< Forces the card to stop transmission. */
#define SDMMC_CMD_SEND_STATUS                         ((uint8_t)13)  /*!< Addressed card sends its status register. */
#define SDMMC_CMD_SET_BLOCKLEN                        ((uint8_t)16)  /* Sets the block length (in bytes for SDSC) for all following block commands (read, write, lock). Default block length is fixed to 512 Bytes. Not effective for SDHS and SDXC. */
#define SDMMC_CMD_READ_SINGLE_BLOCK                   ((uint8_t)17)  /* Reads single block of size selected by SET_BLOCKLEN in case of SDSC, and a block of fixed 512 bytes in case of SDHC and SDXC. */
#define SDMMC_CMD_READ_MULT_BLOCK                     ((uint8_t)18)  /* Continuously transfers data blocks from card to host until interrupted by STOP_TRANSMISSION command. */
#define SDMMC_CMD_WRITE_SINGLE_BLOCK                  ((uint8_t)24)  /* Writes single block of size selected by SET_BLOCKLEN in case of SDSC, and a block of fixed 512 bytes in case of SDHC and SDXC. */
#define SDMMC_CMD_WRITE_MULT_BLOCK                    ((uint8_t)25)  /* Continuously writes blocks of data until a STOP_TRANSMISSION follows. */
#define SDMMC_CMD_APP_CMD                             ((uint8_t)55)  /* Indicates to the card that the next command is an application specific command rather than a standard command. */

#define SDMMC_ACMD_APP_SD_SET_BUSWIDTH                ((uint8_t)6)   /* (ACMD6) Defines the data bus width to be used for data transfer. The allowed data bus widths are given in SCR register. */
#define SDMMC_ACMD_SD_APP_OP_COND                     ((uint8_t)41)  /* (ACMD41) Sends host capacity support information (HCS) and asks the accessed card to send its operating condition register (OCR) content in the response on the CMD line. */
#define SDMMC_ACMD_SD_APP_SET_CLR_CARD_DETECT         ((uint8_t)42)  /* (ACMD42) Connect/Disconnect the 50 KOhm pull-up resistor on CD/DAT3 (pin 1) of the card  */

#define CMD0_GO_IDLE_STATE                            (uint16_t)(SDMMC_CMD_GO_IDLE_STATE | SDIO_CMD_WAIT_NO_RESP)
#define CMD2_ALL_SEND_CID                             (uint16_t)(SDMMC_CMD_ALL_SEND_CID | SDIO_CMD_WAIT_LONG_RESP)
#define CMD3_SET_REL_ADDR                             (uint16_t)(SDMMC_CMD_SET_REL_ADDR | SDIO_CMD_WAIT_SHORT_RESP)
#define CMD7_SEL_DESEL_CARD                           (uint16_t)(SDMMC_CMD_SEL_DESEL_CARD | SDIO_CMD_WAIT_SHORT_RESP)
#define CMD8_HS_SEND_EXT_CSD                          (uint16_t)(SDMMC_CMD_HS_SEND_EXT_CSD | SDIO_CMD_WAIT_SHORT_RESP)
#define CMD9_SEND_CSD                                 (uint16_t)(SDMMC_CMD_SEND_CSD | SDIO_CMD_WAIT_LONG_RESP)
#define CMD12_STOP_TRANSMISSION                       (uint16_t)(SDMMC_CMD_STOP_TRANSMISSION | SDIO_CMD_WAIT_SHORT_RESP)
#define CMD13_SEND_STATUS                             (uint16_t)(SDMMC_CMD_SEND_STATUS | SDIO_CMD_WAIT_SHORT_RESP)
#define CMD16_SET_BLOCKLEN                            (uint16_t)(SDMMC_CMD_SET_BLOCKLEN | SDIO_CMD_WAIT_SHORT_RESP)
#define CMD17_READ_SINGLE_BLOCK                       (uint16_t)(SDMMC_CMD_READ_SINGLE_BLOCK | SDIO_CMD_WAIT_SHORT_RESP)
#define CMD18_READ_MULT_BLOCK                         (uint16_t)(SDMMC_CMD_READ_MULT_BLOCK | SDIO_CMD_WAIT_SHORT_RESP)
#define CMD24_WRITE_SINGLE_BLOCK                      (uint16_t)(SDMMC_CMD_WRITE_SINGLE_BLOCK | SDIO_CMD_WAIT_SHORT_RESP)
#define CMD25_WRITE_MULT_BLOCK                        (uint16_t)(SDMMC_CMD_WRITE_MULT_BLOCK | SDIO_CMD_WAIT_SHORT_RESP)
#define CMD55_APP_CMD                                 (uint16_t)(SDMMC_CMD_APP_CMD | SDIO_CMD_WAIT_SHORT_RESP)

#define ACMD6_APP_SD_SET_BUSWIDTH                     (uint16_t)(SDMMC_ACMD_APP_SD_SET_BUSWIDTH | SDIO_CMD_WAIT_SHORT_RESP)
#define ACMD41_SD_APP_OP_COND                         (uint16_t)(SDMMC_ACMD_SD_APP_OP_COND | SDIO_CMD_WAIT_SHORT_RESP)
#define ACMD42_SD_APP_SET_CLR_CARD_DETECT             (uint16_t)(SDMMC_ACMD_SD_APP_SET_CLR_CARD_DETECT | SDIO_CMD_WAIT_SHORT_RESP)

typedef struct {
  uint32_t CardType;                     /*!< Specifies the card Type                         */
  uint32_t CardVersion;                  /*!< Specifies the card version                      */
  uint32_t Class;                        /*!< Specifies the class of the card class           */
  uint32_t RelCardAdd;                   /*!< Specifies the Relative Card Address             */
  uint32_t BlockNbr;                     /*!< Specifies the Card Capacity in blocks           */
  uint32_t BlockSize;                    /*!< Specifies one block size in bytes               */
  uint32_t LogBlockNbr;                  /*!< Specifies the Card logical Capacity in blocks   */
  uint32_t LogBlockSize;                 /*!< Specifies logical block size in bytes           */
} SDIO_CardInfoTypeDef;

#define SDIO_GET_FLAG(__FLAG__)              (((SDIO->STA) & (__FLAG__)) != 0)
#define SDIO_CLEAR_FLAG(__FLAG__)            (SDIO->ICR = (__FLAG__))

#define CARD_V1_X                            0x00000000U
#define CARD_V2_X                            0x00000001U
#define CARD_SDSC                            0x00000000U
#define CARD_SDHC_SDXC                       0x00000001U

#define SDIO_RESP1                           0
#define SDIO_RESP2                           1
#define SDIO_RESP3                           2
#define SDIO_RESP4                           3

bool SDIO_CmdGoIdleState(void);
bool SDIO_CmdSendCID(void);
bool SDIO_CmdSetRelAdd(uint16_t *rca);
bool SDIO_CmdSelDesel(uint32_t address);
bool SDIO_CmdOperCond(void);
bool SDIO_CmdSendCSD(uint32_t argument);
bool SDIO_CmdStopTransfer(void);
bool SDIO_CmdSendStatus(uint32_t argument);
bool SDIO_CmdBlockLength(uint32_t blockSize);
bool SDIO_CmdReadSingleBlock(uint32_t address);
bool SDIO_CmdReadMultiBlock(uint32_t address);
bool SDIO_CmdWriteSingleBlock(uint32_t address);
bool SDIO_CmdWriteMultiBlock(uint32_t address);
bool SDIO_CmdAppCommand(uint32_t rsa);

bool SDIO_CmdAppSetBusWidth(uint32_t rsa, uint32_t argument);
bool SDIO_CmdAppOperCommand(uint32_t sdType);
bool SDIO_CmdAppSetClearCardDetect(uint32_t rsa);

void SDIO_SendCommand(uint16_t command, uint32_t argument);
uint8_t SDIO_GetCommandResponse(void);
uint32_t SDIO_GetResponse(uint32_t response);
bool SDIO_GetCmdError(void);
bool SDIO_GetCmdResp1(uint8_t command);
bool SDIO_GetCmdResp2(void);
bool SDIO_GetCmdResp3(void);
bool SDIO_GetCmdResp6(uint8_t command, uint32_t *rca);
bool SDIO_GetCmdResp7(void);


#define SDMMC_MAX_VOLT_TRIAL          0x0000FFFFU

#define SDIO_CARD_TRANSFER            0x00000004U  /* Card is in transfer state */
#define SDIO_CARD_ERROR               0x000000FFU   /* Card response Error */

#define SDIO_CMDTIMEOUT               200U        /* Command send and response timeout */
#define SDIO_DATA_TIMEOUT             100U
#define SDIO_WRITE_TIMEOUT            200U

bool SDIO_Init(void);
bool SDIO_ReadBlocks(uint8_t *data, uint32_t blockAddress, uint32_t numberOfBlocks);
bool SDIO_WriteBlocks(uint8_t *data, uint32_t blockAddress, uint32_t numberOfBlocks);
inline uint32_t SDIO_GetCardState(void);
