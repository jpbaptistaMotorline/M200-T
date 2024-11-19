#include <stdint.h>

#define CRC32_INITIAL_VALUE     0xFFFFFFFF
#define CRC32_FINAL_XOR_VAL     0x00000000


#define CRC32_POLY_BZIP2        0x04C11DB7
#define CRC32_POLY_C            0x1EDC6F41
#define CRC32_POLY_D            0xA833982B
#define CRC32_MPEG2             0x04C11DB7
#define CRC32_POSIX             0x04C11DB7
#define CRC32_Q                 0x814141AB
#define CRC32_JAMCRC            0x04C11DB7
#define CRC32_XFER              0x000000AF

/* choose one from above */
#define CRC32_POLY_DEFAULT      CRC32_POSIX//CRC32_POLY_BZIP2

/* calculate CRC32 value
 *
 * @param uint8_t *data - pointer to data
 * @param uint16_t len - size of the data in byte
 * 
 * @return calculated CRC32 value
 */


extern uint32_t crc32_Language_file;

uint32_t crc32(uint8_t *data, uint16_t len);


uint32_t esp_rom_crc32_le(uint32_t crc, uint8_t const *buf, uint32_t len);

/**
 * @brief CRC32 value in big endian.
 *
 * @param crc: Initial CRC value (result of last calculation or 0 for the first time)
 * @param buf: Data buffer that used to calculate the CRC value
 * @param len: Length of the data buffer
 * @return CRC32 value
 */
uint32_t esp_rom_crc32_be(uint32_t crc, uint8_t const *buf, uint32_t len);

/**
 * @brief CRC16 value in little endian.
 *
 * @param crc: Initial CRC value (result of last calculation or 0 for the first time)
 * @param buf: Data buffer that used to calculate the CRC value
 * @param len: Length of the data buffer
 * @return CRC16 value
 */
uint16_t esp_rom_crc16_le(uint16_t crc, uint8_t const *buf, uint32_t len);

/**
 * @brief CRC16 value in big endian.
 *
 * @param crc: Initial CRC value (result of last calculation or 0 for the first time)
 * @param buf: Data buffer that used to calculate the CRC value
 * @param len: Length of the data buffer
 * @return CRC16 value
 */
uint16_t esp_rom_crc16_be(uint16_t crc, uint8_t const *buf, uint32_t len);

/**
 * @brief CRC8 value in little endian.
 *
 * @param crc: Initial CRC value (result of last calculation or 0 for the first time)
 * @param buf: Data buffer that used to calculate the CRC value
 * @param len: Length of the data buffer
 * @return CRC8 value
 */
uint8_t esp_rom_crc8_le(uint8_t crc, uint8_t const *buf, uint32_t len);

/**
 * @brief CRC8 value in big endian.
 *
 * @param crc: Initial CRC value (result of last calculation or 0 for the first time)
 * @param buf: Data buffer that used to calculate the CRC value
 * @param len: Length of the data buffer
 * @return CRC8 value
 */
uint8_t esp_rom_crc8_be(uint8_t crc, uint8_t const *buf, uint32_t len);