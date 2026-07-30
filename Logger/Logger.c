/* Basic event logger - timestamps and serializes transition/error log lines
 * coming from multiple tasks so console output does not interleave. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>
#include "Logger.h"

static SemaphoreHandle_t xLogMutex = NULL;

static const char* const levelNames[] = {
    "INFO ",
    "WARN ",
    "ERROR"
};

void Log_Init(void) {
    xLogMutex = xSemaphoreCreateMutex();
    configASSERT(xLogMutex != NULL);
}

void Log_Event(LogLevel level, const char* source, const char* message) {
    if (xSemaphoreTake(xLogMutex, portMAX_DELAY) == pdTRUE) {
        printf("[%6lums][%s][%s] %s\n",
               (unsigned long)(xTaskGetTickCount() * 1000u / configTICK_RATE_HZ),
               levelNames[level], source, message);
        (void)xSemaphoreGive(xLogMutex);
    }
}
