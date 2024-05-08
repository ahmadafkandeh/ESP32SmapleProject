#include "FileSystemHandler.hpp"
#include <spi.h>
#include <SD.h>
#include <driver/sdmmc_host.h>

#define SPI_MOSI 2 //1
#define SPI_MISO 41
#define SPI_SCK 42
#define SD_CS 1 //2

FileSystemHandler &FileSystemHandler::getInstance()
{
    // TODO: insert return statement here
    static FileSystemHandler mInstance;
    return mInstance;
}

bool FileSystemHandler::InitSDCard()
{
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    if (!SD.begin(SD_CS)) {
    Serial.println("Card Mount Failed");
    return false;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    SD.end();
    return false;
  }
  return true;
}
