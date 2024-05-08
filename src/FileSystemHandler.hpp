#ifndef FILE_SYSTEM_HANDLER_HPP
#define FILE_SYSTEM_HANDLER_HPP
#include <Arduino.h>

class FileSystemHandler
{
public:
    static FileSystemHandler & getInstance();
    bool InitSDCard();
    void deInitSDCard();
    uint32_t openFile(const char * filename, uint32_t mode);
    void closeFile(uint32_t fileHndl);


};

#endif // !FILE_SYSTEM_HANDLER_HPP