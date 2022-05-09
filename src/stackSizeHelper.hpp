#ifndef STACK_SIZE_HELPER_HPP
#define STACK_SIZE_HELPER_HPP

#include <Arduino.h>

extern char *StackPtrAtStart;
extern char *StackPtrEnd;

#define STACK_SIZE_HELPER_START() {{ \
    char* SpStart = NULL; \
    StackPtrAtStart = (char *)&SpStart; \
    UBaseType_t watermarkStart =  uxTaskGetStackHighWaterMark(NULL); \
    StackPtrEnd = StackPtrAtStart - watermarkStart; \
}}

#define STACK_SIZE_HELPER_PRINT() {{ \
    char* SpActual = NULL; \
    Serial.printf("Free Stack is: %d Bytes\n", (uint32_t)&SpActual - (uint32_t)StackPtrEnd); \
}}

#endif /* STACK_SIZE_HELPER_HPP */
