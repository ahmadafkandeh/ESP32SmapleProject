#ifndef _UTILITY_HPP_
#define _UTILITY_HPP_

#include <Arduino.h>
#include <freeRTOS.h>
#include <assert.h>


namespace Utility
{
    struct safe_lock_mutex {
        public:
        safe_lock_mutex(SemaphoreHandle_t mtx):
        _obj(mtx){
            assert(_obj != NULL);
            xSemaphoreTake(_obj, portMAX_DELAY);
        }
        ~safe_lock_mutex() {
            xSemaphoreGive(_obj);
        }
        private:
        SemaphoreHandle_t _obj;
    };
    struct safe_lock_mutex_recursive {
        public:
        safe_lock_mutex_recursive(SemaphoreHandle_t mtx, char* name = "", int line = 0):
        _obj(mtx),
        _name(name),
        _line(line){
            assert(_obj != NULL);
            if (line)
                Serial.printf("before taking mutex for %s from line %d\r\n", _name.c_str(), line);
            xSemaphoreTakeRecursive(_obj, portMAX_DELAY);
            if (line)
        Serial.printf("mutex taken for %s from line %d\r\n", _name.c_str(), line);
        }
        ~safe_lock_mutex_recursive() {
            xSemaphoreGiveRecursive(_obj);
            if (_line)
        Serial.printf("mutex released for %s from line %d\r\n", _name.c_str(), _line);
        }
        private:
        SemaphoreHandle_t _obj;
        int _line;
    String _name;
    };
} // namespace Utility


#endif //!_UTILITY_HPP_