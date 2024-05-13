#include "FileSystemHandler.hpp"
#include <spi.h>
#include <driver/sdmmc_host.h>
#include "Utility.hpp"

#define SPI_MOSI 2 //1
#define SPI_MISO 41
#define SPI_SCK 42
#define SD_CS 1 //2

FileSystemHandler &FileSystemHandler::getInstance()
{
    static FileSystemHandler mInstance = FileSystemHandler();
    return mInstance;
}

FileSystemHandler::FileSystemHandler()
{
  _mutex = xSemaphoreCreateMutex();
  assert( _mutex != NULL );
}

FileSystemHandler::~FileSystemHandler()
{
  vSemaphoreDelete(_mutex);
}

bool FileSystemHandler::InitSDCard()
{
    Utility::safe_lock_mutex lock(_mutex);
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

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  return true;
}

 fs::File FileSystemHandler::openFile(const char *filename, char* mode)
{
  Utility::safe_lock_mutex lock(_mutex);
  return SD.open(filename, mode);
}

void FileSystemHandler::closeFile(fs::File fileHndl)
{
  Utility::safe_lock_mutex lock(_mutex);
  fileHndl.close();
}

size_t FileSystemHandler::read(fs::File fileHndl, uint8_t *buf, size_t size)
{
  Utility::safe_lock_mutex lock(_mutex);
  return fileHndl.read(buf,size);
}

size_t FileSystemHandler::write(fs::File fileHndl, const uint8_t *buf, size_t size)
{
  Utility::safe_lock_mutex lock(_mutex);
  return fileHndl.write(buf,size);
}

void FileSystemHandler::flush(fs::File fileHndl)
{
  Utility::safe_lock_mutex lock(_mutex);
  fileHndl.flush();
}

bool FileSystemHandler::seek(fs::File fileHndl, uint32_t pos)
{
  Utility::safe_lock_mutex lock(_mutex);
  return fileHndl.seek(pos);
}

size_t FileSystemHandler::position(fs::File fileHndl) const
{
  Utility::safe_lock_mutex lock(_mutex);
  return fileHndl.position();
}

size_t FileSystemHandler::size(fs::File fileHndl) const
{
  Utility::safe_lock_mutex lock(_mutex);
  return fileHndl.size();
}

bool FileSystemHandler::setBufferSize(fs::File fileHndl, size_t size)
{
  Utility::safe_lock_mutex lock(_mutex);
  return fileHndl.setBufferSize(size);
}

bool FileSystemHandler::exists(const char *path)
{
  Utility::safe_lock_mutex lock(_mutex);
  return SD.exists(path);
}

bool FileSystemHandler::remove(const char *path)
{
  Utility::safe_lock_mutex lock(_mutex);
  return SD.remove(path);
}

bool FileSystemHandler::mkdir(const char *path)
{
  Utility::safe_lock_mutex lock(_mutex);
  return SD.mkdir(path);
}

bool FileSystemHandler::rmdir(const char *path)
{
  Utility::safe_lock_mutex lock(_mutex);
  return SD.rmdir(path);
}

bool FileSystemHandler::isDirectory(const char * path)
{
  Utility::safe_lock_mutex lock(_mutex);
  bool isDir = false;
  auto f = SD.open(path);
  if(f) {
    isDir = f.isDirectory();
    f.close();
  }
  
  return isDir;
}

bool FileSystemHandler::rename(const char *pathFrom, const char *pathTo)
{
  Utility::safe_lock_mutex lock(_mutex);
  return SD.rename(pathFrom, pathTo);
}

uint64_t FileSystemHandler::getTotalDiskSize()
{
  Utility::safe_lock_mutex lock(_mutex);
  return SD.cardSize();
}

uint64_t FileSystemHandler::getAvailableSize()
{
  Utility::safe_lock_mutex lock(_mutex);
  return SD.totalBytes() - SD.usedBytes();
}
