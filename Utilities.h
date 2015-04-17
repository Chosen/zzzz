#ifndef UTILITIES_H
#define UTILITIES_H

/// output log to debug view
void Log2DebugView(const char* format, ...);

/// a high time counter
double QueryTickCount();

typedef unsigned int uint32_t;

/// declare the automic operations on win32 platform
/// @brief atomically adds a_count to the variable pointed by a_ptr
/// @return the value that had previously been in memory
#define AtomicAdd(ptr, count) (InterlockedExchangeAdd((uint32_t*)(ptr), (count)))

/// @brief atomically substracts a_count from the variable pointed by a_ptr
/// @return the value that had previously been in memory
#define AtomicDec(ptr, count) (InterlockedExchangeAdd((uint32_t*)(ptr), -(count)))

/// @brief Compare And Swap
/// If the current value of *ptr is equal to compVal, then write newVal into *ptr
/// @return true if the comparison is successful and newVal was written
#define CAS(ptr, compVal, newVal) ((compVal) == InterlockedCompareExchange((ptr), (newVal), (compVal)))


#endif//UTILITIES_H
