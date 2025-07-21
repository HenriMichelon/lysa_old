/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <cstdio>
#include <ctime>
module lysa.log;

import lysa.application;
import lysa.exception;

namespace lysa {

    void Log::open(const std::shared_ptr<Log>&log) {
        const auto& appConfig = Application::getConfiguration();
        loggingStreams = log;
        if (appConfig.loggingMode & LOGGING_MODE_FILE) {
            loggingStreams->logFile = fopen("log.txt", "w");
            if(loggingStreams->logFile == nullptr) {
                throw Exception("Error opening log.txt file");
            }
        }
        _LOG("START OF LOG");
    }

    void Log::close() {
        _LOG("END OF LOG");
        const auto& appConfig = Application::getConfiguration();
        if (appConfig.loggingMode & LOGGING_MODE_FILE) {
            fclose(loggingStreams->logFile);
        }
        loggingStreams.reset();
    }

#ifndef DISABLE_LOG

    std::streamsize LogStreamBuf::xsputn(const char* s, const std::streamsize n) {
        const std::string message(s, n);
        log(message);
        return n;
    }

    std::streambuf::int_type LogStreamBuf::overflow(const int_type c) {
        if (c != EOF) {
            const char ch = static_cast<char>(c);
            log(std::string(1, ch));
        }
        return c;
    }

    LogStream::LogStream(const LogLevel level) : std::ostream{&logStreamBuf} { logStreamBuf.setLevel(level); }

    void LogStreamBuf::log(const std::string& message) {
        const auto& config = Application::getConfiguration();
        if (config.loggingMode == LOGGING_MODE_NONE || config.logLevelMin > level) {
            return;
        }
        if (newLine) {
            using namespace std::chrono;
            const auto in_time_t = system_clock::to_time_t(system_clock::now());
            std::tm tm;
            localtime_s(&tm, &in_time_t);
            std::string item = std::format("{:02}:{:02}:{:02}", tm.tm_hour, tm.tm_min, tm.tm_sec);
            item.append(" ");
            switch (level) {
                case LogLevel::TRACE:    item.append("TRACE"); break;
                case LogLevel::DEBUG:    item.append("DEBUG"); break;
                case LogLevel::INFO:     item.append("INFO "); break;
                case LogLevel::GAME1:    item.append("GAME1"); break;
                case LogLevel::GAME2:    item.append("GAME2"); break;
                case LogLevel::GAME3:    item.append("GAME3"); break;
                case LogLevel::WARNING:  item.append("WARN "); break;
                case LogLevel::ERROR:    item.append("ERROR"); break;
                case LogLevel::CRITICAL: item.append("CRIT "); break;
                case LogLevel::INTERNAL: item.append("====="); break;
            }
            item.append(" ");
            if (config.loggingMode & LOGGING_MODE_STDOUT) {
                std::cout << item;
            }
            if (config.loggingMode & LOGGING_MODE_FILE) {
                fwrite(item.c_str(), item.size(), 1, Log::loggingStreams->logFile);
            }
        }
        newLine = message == "\n";
        if (config.loggingMode & LOGGING_MODE_STDOUT) {
            std::cout << message;
            if (newLine) { std::cout.flush(); }
        }
        if (config.loggingMode & LOGGING_MODE_FILE) {
            fwrite(message.c_str(), message.size(), 1, Log::loggingStreams->logFile);
            if (newLine) { fflush(Log::loggingStreams->logFile); }
        }
    }

#endif
}
