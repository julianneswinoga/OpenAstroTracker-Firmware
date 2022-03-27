#include <math.h>

#include "../Configuration.hpp"
#include "Utility.hpp"

#if DEBUG_LEVEL > 0
    #include <stdarg.h>
#endif

#if DEBUG_LEVEL > 0
unsigned long RealTime::_pausedTime   = 0;
unsigned long RealTime::_startTime    = micros();
unsigned long RealTime::_suspendStart = 0;
int RealTime::_suspended              = 0;
#endif

#if BUFFER_LOGS == true
    #define LOG_BUFFER_SIZE 512
char logBuffer[LOG_BUFFER_SIZE];
int bufferWritePos = 0;
int bufferStartPos = 0;

int scanForNextNewLine(int bufPos)
{
    for (int i = bufPos; i < LOG_BUFFER_SIZE; i++)
    {
        if (logBuffer[i] == '\n')
        {
            return i;
        }
    }

    for (int i = 0; i < LOG_BUFFER_SIZE; i++)
    {
        if (logBuffer[i] == '\n')
        {
            return i;
        }
    }
    // WTF?
    return 0;
}

void addToLogBuffer(String s)
{
    s += '\n';
    int charsToWrite = s.length();
    if (bufferWritePos + charsToWrite > LOG_BUFFER_SIZE)
    {
        int charsToWriteAtEndOfBuffer = LOG_BUFFER_SIZE - bufferWritePos;
        memcpy(logBuffer + bufferWritePos, s.c_str(), charsToWriteAtEndOfBuffer);
        charsToWrite -= charsToWriteAtEndOfBuffer;
        memcpy(logBuffer, s.c_str() + charsToWriteAtEndOfBuffer, charsToWrite);
        bufferWritePos            = charsToWrite;
        logBuffer[bufferWritePos] = '\0';
        bufferStartPos            = scanForNextNewLine(bufferWritePos);
    }
    else
    {
        memcpy(logBuffer + bufferWritePos, s.c_str(), charsToWrite);
        if (bufferStartPos > bufferWritePos)
        {
            bufferWritePos += charsToWrite;
            bufferStartPos = scanForNextNewLine(bufferWritePos);
        }
        else
        {
            bufferWritePos += charsToWrite;
        }
    }
}

class MyString : public String
{
  public:
    void setLen(unsigned int newLen)
    {
        len = newLen;
    }
};

String getLogBuffer()
{
    MyString result;
    unsigned int len = (bufferStartPos > bufferWritePos) ? LOG_BUFFER_SIZE - bufferStartPos : 0;
    len += bufferWritePos;
    result.reserve(len + 2);
    char *buffer = result.begin();

    if (bufferStartPos > bufferWritePos)
    {
        for (int i = bufferStartPos; i < LOG_BUFFER_SIZE; i++)
        {
            *buffer++ = (logBuffer[i] == '#') ? '%' : logBuffer[i];
        }
    }

    for (int i = 0; i < bufferWritePos; i++)
    {
        *buffer++ = (logBuffer[i] == '#') ? '%' : logBuffer[i];
    }

    *buffer++ = '#';
    *buffer   = '\0';
    result.setLen(buffer - result.begin());
    return result;
}
#else
String getLogBuffer()
{
    return "Debugging disabled.#";
}
#endif

// Adjust the given number by the given adjustment, wrap around the limits.
// Limits are inclusive, so they represent the lowest and highest valid number.
int adjustWrap(int current, int adjustBy, int minVal, int maxVal)
{
    current += adjustBy;
    if (current > maxVal)
    {
        current -= (maxVal + 1 - minVal);
    }
    if (current < minVal)
    {
        current += (maxVal + 1 - minVal);
    }
    return current;
}

// Adjust the given number by the given adjustment, clamping to the limits.
// Limits are inclusive, so they represent the lowest and highest valid number.
int adjustClamp(int current, int adjustBy, int minVal, int maxVal)
{
    current += adjustBy;
    if (current > maxVal)
    {
        current = maxVal;
    }
    if (current < minVal)
    {
        current = minVal;
    }
    return current;
}

// Clamp the given number to the limits.
// Limits are inclusive, so they represent the lowest and highest valid number.
long clamp(long current, long minVal, long maxVal)
{
    if (current > maxVal)
    {
        current = maxVal;
    }
    if (current < minVal)
    {
        current = minVal;
    }
    return current;
}

// Clamp the given number to the limits.
// Limits are inclusive, so they represent the lowest and highest valid number.
int clamp(int current, int minVal, int maxVal)
{
    if (current > maxVal)
    {
        current = maxVal;
    }
    if (current < minVal)
    {
        current = minVal;
    }
    return current;
}

// Clamp the given number to the limits.
// Limits are inclusive, so they represent the lowest and highest valid number.
float clamp(float current, float minVal, float maxVal)
{
    if (current > maxVal)
    {
        current = maxVal;
    }
    if (current < minVal)
    {
        current = minVal;
    }
    return current;
}

// Return -1 if the given number is less than zero, 1 if not.
int sign(long num)
{
    if (num < 0)
    {
        return -1;
    }
    return 1;
}

// Return -1 if the given number is less than zero, 1 if not.
int fsign(float num)
{
    if (num < 0)
    {
        return -1;
    }
    return 1;
}

// float-type point implementation of fabs
float fabsf(float x)
{
    return static_cast<float>(fabs(static_cast<double>(x)));
}

// float-type point implementation of round
float roundf(float x)
{
    return static_cast<float>(round(static_cast<double>(x)));
}

// float-type point implementation of atan
float atanf(float x)
{
    return static_cast<float>(atan(static_cast<double>(x)));
}

#if defined(ESP32)
int freeMemory()
{
    return ESP.getFreeHeap();
}
#else

    #ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char *sbrk(int incr);
    #else   // __ARM__
extern char *__brkval;
    #endif  // __arm__

int freeMemory()
{
    char top;
    #ifdef __arm__
    return &top - reinterpret_cast<char *>(sbrk(0));
    #elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
    return &top - __brkval;
    #else   // __arm__
    return __brkval ? &top - __brkval : &top - __malloc_heap_start;
    #endif  // __arm__
}
#endif

#if DEBUG_LEVEL > 0
    #if TEST_BUILD == 1
void logv(int levelFlags, const char *input, ...)
    #else
void logv(int levelFlags, String input, ...)
    #endif
{
    if ((levelFlags & DEBUG_LEVEL) != 0)
    {
        va_list argp;
        va_start(argp, input);
        char strOut[256];
    #if TEST_BUILD == 1
        vsnprintf(strOut, ARRAY_SIZE(strOut), input, argp);
    #else
        vsnprintf_P(strOut, ARRAY_SIZE(strOut), input.c_str(), argp);
    #endif
    #if BUFFER_LOGS == true
        addToLogBuffer(strOut);
    #else
        DEBUG_SERIAL_PORT.print("[");
        DEBUG_SERIAL_PORT.print(millis());
        DEBUG_SERIAL_PORT.print("]:");
        DEBUG_SERIAL_PORT.print(String(freeMemory()));
        DEBUG_SERIAL_PORT.print(": ");
        DEBUG_SERIAL_PORT.println(strOut);
        DEBUG_SERIAL_PORT.flush();
    #endif
        va_end(argp);
    }
}

#endif
