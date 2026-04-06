#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <iostream>
#include <fstream>

inline void DebugLog(const std::string& message) {
    try {
        std::ofstream logFile("ue4_extractor_debug.log", std::ios::app);
        if (logFile.is_open()) {
            SYSTEMTIME st;
            GetSystemTime(&st);
            logFile << "[" << st.wHour << ":" << st.wMinute << ":" << st.wSecond << "] "
                << message << std::endl;
            logFile.close();
        }
    }
    catch (...) {
    }
}