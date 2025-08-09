#ifndef LOGGER_H
#define LOGGER_H
#pragma once

#include <string>
#include <mutex>
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    SUCCESS
};

class Logger {
public:
    // Singleton instance
    static Logger& getInstance();

    // Initialize logger
    void init(LogLevel level, bool writeToFile = false, const std::string& filePath = "logs/app.log");

    // Logging methods
    static void debug(const std::string& msg);
    static void info(const std::string& msg);
    static void warn(const std::string& msg);
    static void error(const std::string& msg);
    static void success(const std::string& msg);

private:
    Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void log(LogLevel level, const std::string& msg);
    std::string getTimestamp();
    std::string levelToString(LogLevel level);
    std::string levelToColor(LogLevel level);

    LogLevel currentLevel = LogLevel::INFO;
    bool logToFile = false;
    std::ofstream logFile;
    std::mutex logMutex;
};

#endif
