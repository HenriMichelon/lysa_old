/*
 * Copyright (c) 2024-present Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <cstdio>
export module lysa.log;

import std;
import lysa.enums;

export namespace lysa {

#ifdef DISABLE_LOG

    constexpr bool ENABLE_LOG = false;

    class LogStream {
    public:
        inline LogStream(LogLevel level) {};
    private:
    };

#else

    constexpr bool ENABLE_LOG = true;

    class LogStreamBuf : public std::streambuf {
    public:
        std::streamsize xsputn(const char* s, std::streamsize n) override;

        int_type overflow(int_type c) override;

        void setLevel(const LogLevel level) { this->level = level; }

    private:
        LogLevel level{LogLevel::ERROR};
        bool newLine{true};
        void log(const std::string& message);
    };

    class LogStream : public std::ostream {
    public:
        LogStream(LogLevel level);
    private:
        LogStreamBuf logStreamBuf;
    };

#endif

    struct Log {
        LogStream trace{LogLevel::TRACE};
        LogStream _internal{LogLevel::INTERNAL};
        LogStream debug{LogLevel::DEBUG};
        LogStream info{LogLevel::INFO};
        LogStream game1{LogLevel::GAME1};
        LogStream game2{LogLevel::GAME2};
        LogStream game3{LogLevel::GAME3};
        LogStream warning{LogLevel::WARNING};
        LogStream error{LogLevel::ERROR};
        LogStream critical{LogLevel::CRITICAL};

        FILE* logFile;

        static void open(const std::shared_ptr<Log>&);
        static void close();
        static inline std::shared_ptr<Log> loggingStreams{nullptr};
    };

    consteval bool isLoggingEnabled() {
        return ENABLE_LOG;
    }

    template <typename... Args>
    void _LOG(Args... args) { if constexpr (isLoggingEnabled()) { (Log::loggingStreams->_internal << ... << args) << std::endl; } }

    inline void TRACE(const std::source_location& location = std::source_location::current()) {
        if constexpr (isLoggingEnabled()) {
            Log::loggingStreams->trace << location.function_name() << " line " << location.line() << std::endl;
        }
    }

    template <typename... Args>
    void DEBUG(Args... args) { if constexpr (isLoggingEnabled()) { (Log::loggingStreams->debug << ... << args) << std::endl; } }

    template <typename... Args>
    void INFO(Args... args) { if constexpr (isLoggingEnabled()) { (Log::loggingStreams->info << ... << args) << std::endl; } }

    template <typename... Args>
    void GAME1(Args... args) { if constexpr (isLoggingEnabled()) { (Log::loggingStreams->game1 << ... << args) << std::endl; } }

    template <typename... Args>
    void GAME2(Args... args) { if constexpr (isLoggingEnabled()) { (Log::loggingStreams->game2 << ... << args) << std::endl; } }

    template <typename... Args>
    void GAME3(Args... args) { if constexpr (isLoggingEnabled()) { (Log::loggingStreams->game3 << ... << args) << std::endl; } }

    template <typename... Args>
    void WARNING(Args... args) { if constexpr (isLoggingEnabled()) { (Log::loggingStreams->warning << ... << args) << std::endl; } }

    template <typename... Args>
    void ERROR(Args... args) { if constexpr (isLoggingEnabled()) { (Log::loggingStreams->error << ... << args) << std::endl; } }

    template <typename... Args>
    void CRITICAL(Args... args) { if constexpr (isLoggingEnabled()) { (Log::loggingStreams->critical << ... << args) << std::endl; } }

}
