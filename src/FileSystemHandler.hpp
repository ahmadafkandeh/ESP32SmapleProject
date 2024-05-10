#ifndef FILE_SYSTEM_HANDLER_HPP
#define FILE_SYSTEM_HANDLER_HPP
#include <Arduino.h>
#include <FreeRTOS.h>
#include <SD.h>

class FileSystemHandler
{
public:
    static FileSystemHandler & getInstance();
    bool InitSDCard();
    void deInitSDCard();
    fs::File openFile(const char * filename, char * mode);
    void closeFile(fs::File fileHndl);
    size_t read(fs::File fileHndl, uint8_t* buf, size_t size);
    size_t write(fs::File fileHndl, const uint8_t *buf, size_t size);
    void flush(fs::File fileHndl);
    bool seek(fs::File fileHndl, uint32_t pos);
    size_t position(fs::File fileHndl) const;
    size_t size(fs::File fileHndl) const;
    bool setBufferSize(fs::File fileHndl, size_t size);
    bool exists(const char* path);
    bool remove(const char* path);
    bool mkdir(const char *path);
    bool rmdir(const char *path);
    bool isDirectory(const char *path);
    bool rename(const char* pathFrom, const char* pathTo);
    
    uint64_t getTotalDiskSize();
    uint64_t getAvailableSize();

private:
    FileSystemHandler();
    ~FileSystemHandler();

protected:
    SemaphoreHandle_t _mutex = NULL;


};

#endif // !FILE_SYSTEM_HANDLER_HPP