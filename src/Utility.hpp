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
        safe_lock_mutex_recursive(SemaphoreHandle_t mtx):
        _obj(mtx){
            assert(_obj != NULL);
            xSemaphoreTakeRecursive(_obj, portMAX_DELAY);
        }
        ~safe_lock_mutex_recursive() {
            xSemaphoreGiveRecursive(_obj);
        }
        private:
        SemaphoreHandle_t _obj;
    };
} // namespace Utility


#endif //!_UTILITY_HPP_