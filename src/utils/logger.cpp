#include "utils/logger.h"

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::init(LogLevel level, bool writeToFile, const std::string& filePath) {
    currentLevel = level;
    logToFile = writeToFile;
    if (writeToFile) {
        logFile.open(filePath, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Logger: Failed to open log file: " << filePath << std::endl;
        }
    }
}
void Logger::debug(const std::string& msg) { getInstance().log(LogLevel::DEBUG, msg); }
void Logger::info(const std::string& msg)  { getInstance().log(LogLevel::INFO, msg); }
void Logger::warn(const std::string& msg)  { getInstance().log(LogLevel::WARN, msg); }
void Logger::error(const std::string& msg) { getInstance().log(LogLevel::ERROR, msg); }
void Logger::success(const std::string& msg){ getInstance().log(LogLevel::SUCCESS, msg); }


void Logger::log(LogLevel level, const std::string& msg) {
    std::lock_guard<std::mutex> lock(logMutex);

    if (level < currentLevel) return; // Skip lower-level logs

    std::ostringstream formatted;
    formatted << getTimestamp() << " [" << levelToString(level) << "] " << msg;

    // Console output with color
    std::cout << levelToColor(level) << formatted.str() << "\033[0m" << std::endl;

    // File output if enabled
    if (logToFile && logFile.is_open()) {
        logFile << formatted.str() << std::endl;
    }
}

std::string Logger::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::SUCCESS: return "SUCCESS";
    }
    return "UNKNOWN";
}

std::string Logger::levelToColor(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "\033[36m"; // Cyan
        case LogLevel::INFO:  return "\033[32m"; // Green
        case LogLevel::SUCCESS: return "\033[32m]";// Green
        case LogLevel::WARN:  return "\033[33m"; // Yellow
        case LogLevel::ERROR: return "\033[31m"; // Red
    }
    return "\033[0m";
}
