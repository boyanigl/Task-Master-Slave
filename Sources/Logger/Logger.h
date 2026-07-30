#ifndef LOGGER_H
#define LOGGER_H

typedef enum {
    LOG_INFO = 0,
    LOG_WARNING,
    LOG_ERROR
} LogLevel;

/* Public functions declarations */
void Log_Init(void);
void Log_Event(LogLevel level, const char* source, const char* message);

#endif
