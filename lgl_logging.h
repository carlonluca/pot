/*
 * Author:  Luca Carlon
 * Company: -
 * Date:    05.12.2011
 */

#ifndef LC_LOGGING_H
#define LC_LOGGING_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <sstream>
#include <string>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <iostream>
#ifdef __ANDROID__
#include <android/log.h>
#else
#include <assert.h>
#endif
#include <execinfo.h>
#include <cxxabi.h>

// Convenience inlines. These functions can be used as a common shurtcut to invoke the
// desidered logger.
#define VA_LIST_CONTEXT(last, i) \
   {va_list args; va_start(args, last); i; va_end(args);}
#define LOG_UNUSED(x) \
   (void)x

// Only enabled coloring for non-Win platforms. Unless specified externally.
#ifndef COLORING_ENABLED
#if __APPLE__
#include "TargetConditionals.h"
#if TARGET_IPHONE_SIMULATOR
// iOS Simulator
#elif TARGET_OS_IPHONE
// iOS device
#elif TARGET_OS_MAC
// Other kinds of Mac OS
#define COLORING_ENABLED
#else
// Unsupported platform
#endif
#elif !defined(_WIN32) && !defined(_WIN32_WCE)
#define COLORING_ENABLED
#endif
#endif // COLORING_ENABLED

// Define a LOG_TAG macro project-widely or none is used.
#ifndef LOG_TAG
#define LOG_TAG NULL
#endif

// Enable logs according to log level.
#ifdef LC_COMPILE_LOG_LEVEL_DEBUG
#define LC_COMPILE_LOG_LEVEL_ALL
#elif LC_COMPILE_LOG_LEVEL_VERBOSE
#define ENABLE_LOG_CRITICAL
#define ENABLE_LOG_ERROR
#define ENABLE_LOG_WARNING
#define ENABLE_LOG_INFORMATION
#define ENABLE_LOG_VERBOSE
#elif LC_COMPILE_LOG_LEVEL_INFORMATION
#define ENABLE_LOG_CRITICAL
#define ENABLE_LOG_ERROR
#define ENABLE_LOG_WARNING
#define ENABLE_LOG_INFORMATION
#elif LC_COMPILE_LOG_LEVEL_WARNING
#define ENABLE_LOG_CRITICAL
#define ENABLE_LOG_ERROR
#define ENABLE_LOG_WARNING
#elif LC_COMPILE_LOG_LEVEL_ERROR
#define ENABLE_LOG_CRITICAL
#define ENABLE_LOG_ERROR
#elif LC_COMPILE_LOG_LEVEL_CRITICAL
#define ENABLE_LOG_CRITICAL
#else
// If none is selected select all.
#ifndef LC_COMPILE_LOG_LEVEL_ALL
#define LC_COMPILE_LOG_LEVEL_ALL
#endif
#endif // LC_COMPILE_LOG_LEVEL_DEBUG

#ifdef LC_COMPILE_LOG_LEVEL_ALL
#define ENABLE_LOG_CRITICAL
#define ENABLE_LOG_ERROR
#define ENABLE_LOG_WARNING
#define ENABLE_LOG_INFORMATION
#define ENABLE_LOG_VERBOSE
#define ENABLE_LOG_DEBUG
#endif // LC_COMPILE_LOG_LEVEL_ALL


/*------------------------------------------------------------------------------
|    definitions
+-----------------------------------------------------------------------------*/
enum LC_LogLevel {
   // Do not mess with the order.
   LC_LOG_LEVEL_CRITICAL,
   LC_LOG_LEVEL_ERROR,
   LC_LOG_LEVEL_WARN,
   LC_LOG_LEVEL_INFO,
   LC_LOG_LEVEL_VERBOSE,
   LC_LOG_LEVEL_DEBUG
};

// Text attributes.
#define C_RESET     0
#define C_BRIGHT    1 // Bold.
#define C_DIM       2
#define C_UNDERLINE 4 // Underscore
#define C_BLINK     5
#define C_REVERSE   7
#define C_HIDDEN    8

enum LC_LogTextAttrib {
   LC_LOG_TEXT_ATTRIB_RESET      = C_RESET,
   LGL_LOG_TEXT_ATTRIB_BRIGHT    = C_BRIGHT,
   LGL_LOG_TEXT_ATTRIB_DIM       = C_DIM,
   LGL_LOG_TEXT_ATTRIB_UNDERLINE = C_UNDERLINE,
   LGL_LOG_TEXT_ATTRIB_BLINK     = C_BLINK,
   LGL_LOG_TEXT_ATTRIB_REVERSE   = C_REVERSE,
   LGL_LOG_TEXT_ATTRIB_HIDDEN    = C_HIDDEN
};

// Foreground colors.
#define C_F_BLACK     0
#define C_F_RED       1
#define C_F_GREEN		 2
#define C_F_YELLOW    3
#define C_F_BLUE		 4
#define C_F_MAGENTA   5
#define C_F_CYAN		 6
#define C_F_WHITE		 7
#define C_F_DEFAULT   9

enum LC_LogFgColor {
   LC_LOG_FG_COLOR_BLACK   = C_F_BLACK,
   LC_LOG_FG_COLOR_RED     = C_F_RED,
   LC_LOG_FG_COLOR_GREEN   = C_F_GREEN,
   LC_LOG_FG_COLOR_YELLOW  = C_F_YELLOW,
   LC_LOG_FG_COLOR_BLUE    = C_F_BLUE,
   LC_LOG_FG_COLOR_MAGENTA = C_F_MAGENTA,
   LC_LOG_FG_COLOR_CYAN    = C_F_CYAN,
   LC_LOG_FG_COLOR_WHITE   = C_F_WHITE,
   LC_LOG_FG_COLOR_DEFAULT = C_F_DEFAULT
};

inline std::string lc_current_time();

/*------------------------------------------------------------------------------
|    LC_LogPriv class
+-----------------------------------------------------------------------------*/
/**
 * Internal class used for logging. This class is only internal. Methods must be accessed
 * using the type passed to it.
 */
template <typename T>
class LC_Log
{
public:
   LC_Log(const char* log_tag);
   LC_Log(LC_LogLevel level);
   LC_Log(const char* log_tag, LC_LogLevel level);

   ~LC_Log();

   void printf(const char* format, ...);
   void printf(const char* format, va_list args);

   static std::string toString(LC_LogLevel level);
   static LC_LogLevel fromString(const std::string& level);

   std::stringstream m_string;
   LC_LogLevel m_level;
   const char* m_log_tag;

private:
   LC_Log(const LC_Log&);
   LC_Log& operator =(const LC_Log&);

   void appendHeader();
   void appendlog_tagIfNeeded();
};

/*------------------------------------------------------------------------------
|    LC_Output2Std class
+-----------------------------------------------------------------------------*/
class LC_Output2Std
{
public:
   static void printf(LC_Log<LC_Output2Std>& logger, va_list args);
   static LC_LogFgColor getColorForLevel(LC_LogLevel level);
};
typedef LC_Log<LC_Output2Std> LC_LogStd;

/*------------------------------------------------------------------------------
|    LC_Output2File class
+-----------------------------------------------------------------------------*/
class LC_Output2FILE
{
public:
   static void printf(LC_Log<LC_Output2Std>& logger, va_list args);

private:
   static FILE*& stream();
};
typedef LC_Log<LC_Output2FILE> LC_LogFile;

// Define the default logger.
#ifdef __ANDROID__
typedef LC_Log<LC_OutputAndroid> LC_LogDef;
#else
typedef LC_Log<LC_Output2Std> LC_LogDef;
#endif

#ifdef ENABLE_LOG_CRITICAL
/*------------------------------------------------------------------------------
|    log_critical
+-----------------------------------------------------------------------------*/
inline bool
log_critical(const char* log_tag, const char* format, va_list args)
{
   LC_LogDef(log_tag, LC_LOG_LEVEL_CRITICAL).printf(format, args);
   return false;
}

/*------------------------------------------------------------------------------
|    log_err
+-----------------------------------------------------------------------------*/
inline bool
log_critical(const char* log_tag, const char* format, ...)
{
   VA_LIST_CONTEXT(format, log_critical(log_tag, format, args));
   return false;
}

/*------------------------------------------------------------------------------
|    log_err
+-----------------------------------------------------------------------------*/
inline bool
log_critical(const char* format, ...)
{
   VA_LIST_CONTEXT(format, log_critical(LOG_TAG, format, args));
   return false;
}
#else
#define log_critical(...) (void)0
#endif // ENABLE_LOG_CRITICAL

#ifdef ENABLE_LOG_ERROR
/*------------------------------------------------------------------------------
|    log_err
+-----------------------------------------------------------------------------*/
inline bool
log_err(const char* log_tag, const char* format, va_list args)
{
   LC_LogDef(log_tag, LC_LOG_LEVEL_ERROR).printf(format, args);
   return false;
}

/*------------------------------------------------------------------------------
|    log_err
+-----------------------------------------------------------------------------*/
inline bool
log_err(const char* log_tag, const char* format, ...)
{
   VA_LIST_CONTEXT(format, log_err(log_tag, format, args));
   return false;
}

/*------------------------------------------------------------------------------
|    log_err
+-----------------------------------------------------------------------------*/
inline bool
log_err(const char* format, ...)
{
   VA_LIST_CONTEXT(format, log_err(LOG_TAG, format, args));
   return false;
}
#else
#define log_err(...) (void)0
#endif // ENABLE_LOG_ERROR

#ifdef ENABLE_LOG_WARNING
/*------------------------------------------------------------------------------
|    log_warn
+-----------------------------------------------------------------------------*/
inline bool
log_warn(const char* log_tag, const char* format, va_list args)
{
   LC_LogDef(log_tag, LC_LOG_LEVEL_WARN).printf(format, args);
   return false;
}

/*------------------------------------------------------------------------------
|    log_warn
+-----------------------------------------------------------------------------*/
inline bool
log_warn(const char* log_tag, const char* format, ...)
{
   VA_LIST_CONTEXT(format, log_warn(log_tag, format, args));
   return false;
}

/*------------------------------------------------------------------------------
|    log_warn
+-----------------------------------------------------------------------------*/
inline bool
log_warn(const char* format, ...)
{
   VA_LIST_CONTEXT(format, log_warn(LOG_TAG, format, args));
   return false;
}
#else
#define log_warn(...) (void)0
#endif // ENABLE_LOG_WARNING

#ifdef ENABLE_LOG_INFORMATION
/*------------------------------------------------------------------------------
|    log_info
+-----------------------------------------------------------------------------*/
inline bool
log_info(const char* log_tag, const char* format, va_list args)
{
   LC_LogDef(log_tag, LC_LOG_LEVEL_INFO).printf(format, args);
   return true;
}

/*------------------------------------------------------------------------------
|    log_info
+-----------------------------------------------------------------------------*/
inline bool
log_info(const char* log_tag, const char* format, ...)
{
   VA_LIST_CONTEXT(format, log_info(log_tag, format, args));
   return true;
}

/*------------------------------------------------------------------------------
|    log_info
+-----------------------------------------------------------------------------*/
inline bool
log_info(const char* format, ...)
{
   VA_LIST_CONTEXT(format, log_info(LOG_TAG, format, args));
   return true;
}
#else
#define log_info(...) (void)0
#endif // ENABLE_LOG_INFORMATION

#ifdef ENABLE_LOG_VERBOSE
/*------------------------------------------------------------------------------
|    log_verbose
+-----------------------------------------------------------------------------*/
inline bool
log_verbose(const char* log_tag, const char* format, va_list args)
{
   LC_LogDef(log_tag, LC_LOG_LEVEL_VERBOSE).printf(format, args);
   return true;
}

/*------------------------------------------------------------------------------
|    log_verbose
+-----------------------------------------------------------------------------*/
inline bool
log_verbose(const char* log_tag, const char* format, ...)
{
   VA_LIST_CONTEXT(format, log_verbose(log_tag, format, args));
   return true;
}

/*------------------------------------------------------------------------------
|    log_verbose
+-----------------------------------------------------------------------------*/
inline bool
log_verbose(const char* format, ...)
{
   VA_LIST_CONTEXT(format, log_verbose(LOG_TAG, format, args));
   return true;
}
#else
#define log_verbose(...) (void)0
#endif // ENABLE_LOG_VERBOSE

#ifdef ENABLE_LOG_DEBUG
/*------------------------------------------------------------------------------
|    log_debug
+-----------------------------------------------------------------------------*/
inline void
log_debug(const char* log_tag, const char* format, va_list args)
{
   LC_LogStd(log_tag, LC_LOG_LEVEL_DEBUG).printf(format, args);
}

/*------------------------------------------------------------------------------
|    log_debug
+-----------------------------------------------------------------------------*/
inline void
log_debug(const char* log_tag, const char* format, ...)
{
   VA_LIST_CONTEXT(format, log_debug(log_tag, format, args));
}

/*------------------------------------------------------------------------------
|    log_debug
+-----------------------------------------------------------------------------*/
inline void
log_debug(const char* format, ...)
{
   VA_LIST_CONTEXT(format, log_debug(LOG_TAG, format, args));
}
#else
#define log_debug(...) (void)0
#endif // ENABLE_LOG_DEBUG

// Convenience macros. The same as using the inlined functions.
// TODO: ## is a gcc extension. Get rid of it somehow.
#ifdef __GNUC__
#define LOG_CRITICAL(tag, f, ...) \
   log_critical(tag, f, ##__VA_ARGS__)
#define LOG_ERROR(tag, f, ...) \
   log_err(tag, f, ##__VA_ARGS__)
#define LOG_WARNING(tag, f, ...) \
   log_warn(tag, f, ##__VA_ARGS__)
#define LOG_INFORMATION(tag, f, ...) \
   log_info(tag, f, ##__VA_ARGS__)
#define LOG_VERBOSE(tag, f, ...) \
   log_verbose(tag, f, ##__VA_ARGS__)
#define LOG_DEBUG(tag, f, ...) \
   log_debug(tag, f, ##__VA_ARGS__)
#endif

/*------------------------------------------------------------------------------
|    log_stacktrace
+-----------------------------------------------------------------------------*/
/**
 * Prints a demangled stack backtrace of the caller function. Remember to build adding the
 * flag -rdynamic.
 *
 * @param log_tag The log tag to be placed in the log line.
 * @param level The log level of the log.
 * @param max_frames The maximum number of lines of stack trace to log.
 */
inline
void log_stacktrace(const char* log_tag, LC_LogLevel level, unsigned int max_frames = 100)
{
   std::stringstream stream;
   stream << std::endl;

   // storage array for stack trace address data
   void* addrlist[max_frames + 1];

   // retrieve current stack addresses
   int addrlen = backtrace(addrlist, sizeof(addrlist)/sizeof(void*));

   if (addrlen == 0) {
      stream << "<empty, possibly corrupt>";
      LC_LogDef(log_tag, level).printf("%s", stream.str().c_str());
      return;
   }

   // resolve addresses into strings containing "filename(function+address)",
   // this array must be free()-ed
   char** symbollist = backtrace_symbols(addrlist, addrlen);

   // allocate string which will be filled with the demangled function name
   size_t funcnamesize = 256;
   char* funcname = (char*)malloc(funcnamesize);

   // iterate over the returned symbol lines. skip the first, it is the
   // address of this function.
   for (int i = 1; i < addrlen; i++) {
      char* begin_name = 0, * begin_offset = 0, * end_offset = 0;

      // find parentheses and +address offset surrounding the mangled name:
      // ./module(function+0x15c) [0x8048a6d]
      for (char* p = symbollist[i]; *p; ++p) {
         if (*p == '(')
            begin_name = p;
         else if (*p == '+')
            begin_offset = p;
         else if (*p == ')' && begin_offset) {
            end_offset = p;
            break;
         }
      }

      if (begin_name && begin_offset && end_offset && begin_name < begin_offset) {
         *begin_name++   = '\0';
         *begin_offset++ = '\0';
         *end_offset     = '\0';

         // mangled name is now in [begin_name, begin_offset) and caller
         // offset in [begin_offset, end_offset). now apply
         // __cxa_demangle():

         int status;
         char* ret = abi::__cxa_demangle(begin_name,
                                         funcname, &funcnamesize, &status);
         if (status == 0) {
            funcname = ret; // use possibly realloc()-ed string
            stream << "  " << symbollist[i] << ": "
                   << funcname << "+" << begin_offset << std::endl;
         }
         else
            // demangling failed. Output function name as a C function with
            // no arguments.
            stream << "  " << symbollist[i] << ": "
                   << begin_name << "()+" << begin_offset << std::endl;
      }
      else
         // couldn't parse the line? print the whole line.
         stream << "  " << symbollist[i] << std::endl;
   }

   LC_LogDef(log_tag, level).printf("%s", stream.str().c_str());

   free(funcname);
   free(symbollist);
}

/*------------------------------------------------------------------------------
|    log_stacktrace
+-----------------------------------------------------------------------------*/
inline
void log_stacktrace(LC_LogLevel level, unsigned int max_frames = 100)
{
   log_stacktrace(LOG_TAG, level, max_frames);
}

/*------------------------------------------------------------------------------
|    log_stacktrace
+-----------------------------------------------------------------------------*/
inline
void log_stacktrace(unsigned int max_frames = 100)
{
   log_stacktrace(LOG_TAG, LC_LOG_LEVEL_DEBUG, max_frames);
}

/*------------------------------------------------------------------------------
|    log_stacktrace
+-----------------------------------------------------------------------------*/
inline
void log_stacktrace(const char* log_tag, unsigned int max_frames = 100)
{
   log_stacktrace(log_tag, LC_LOG_LEVEL_DEBUG, max_frames);
}

/*------------------------------------------------------------------------------
|    LC_Log<T>::LC_Log
+-----------------------------------------------------------------------------*/
template <typename T> inline
LC_Log<T>::LC_Log(const char* log_tag, LC_LogLevel level) :
   m_level(level)
 , m_log_tag(log_tag)
{
   // Do nothing.
}

/*------------------------------------------------------------------------------
|    LC_Log<T>::LC_Log
+-----------------------------------------------------------------------------*/
template <typename T> inline
LC_Log<T>::LC_Log(const char *log_tag) :
   m_level(LC_LOG_LEVEL_INFO)
 , m_log_tag(log_tag)
{
   // Do nothing.
}

/*------------------------------------------------------------------------------
|    LC_Log<T>::LC_Log
+-----------------------------------------------------------------------------*/
template <typename T> inline
LC_Log<T>::LC_Log(LC_LogLevel level) :
   m_level(level)
 , m_log_tag(LOG_TAG)
{
   // Do nothing.
}

/*------------------------------------------------------------------------------
|    LC_Log<T>::appendHeader
+-----------------------------------------------------------------------------*/
template <typename T> inline
void LC_Log<T>::appendHeader()
{
   m_string << "- " << lc_current_time();
   m_string << " " << toString(m_level) << ": ";
   m_string << std::string(m_level > LC_LOG_LEVEL_DEBUG ? m_level - LC_LOG_LEVEL_DEBUG : 0, '\t');
}

/*------------------------------------------------------------------------------
|    LC_Log<T>::appendlog_tagIfNeeded
+-----------------------------------------------------------------------------*/
template <typename T> inline
void LC_Log<T>::appendlog_tagIfNeeded()
{
   if (m_log_tag)
      m_string << "[" << m_log_tag << "]: ";
}

/*------------------------------------------------------------------------------
|    LC_Log<T>::printf
+-----------------------------------------------------------------------------*/
template <typename T> inline
void LC_Log<T>::printf(const char* format, ...)
{
   VA_LIST_CONTEXT(format, printf(format, args));
}

/*------------------------------------------------------------------------------
|    LC_Log<T>::printf
+-----------------------------------------------------------------------------*/
template <typename T> inline
void LC_Log<T>::printf(const char* format, va_list args)
{
   appendlog_tagIfNeeded();
   appendHeader();
   m_string << format;

   // Delegate log handling.
   T::printf(*this, args);
}

/*------------------------------------------------------------------------------
|    LC_Log<T>::~LC_Log
+-----------------------------------------------------------------------------*/
template <typename T> inline
LC_Log<T>::~LC_Log()
{
   // Do nothing.
}

/*------------------------------------------------------------------------------
|    LC_Log<T>::toString
+-----------------------------------------------------------------------------*/
template <typename T> inline
std::string LC_Log<T>::toString(LC_LogLevel level)
{
   static const char* const buffer[] = {
      "CRITICAL", "ERROR", "WARNING", "INFORMATION",
      "VERBOSE", "DEBUG"
   };

   return buffer[level];
}

/*------------------------------------------------------------------------------
|    LC_Log<T>::fromString
+-----------------------------------------------------------------------------*/
template <typename T> inline
LC_LogLevel LC_Log<T>::fromString(const std::string& level)
{
   if (level == "DEBUG")
      return LC_LOG_LEVEL_DEBUG;
   if (level == "VERBOSE")
      return LC_LOG_LEVEL_VERBOSE;
   if (level == "INFO")
      return LC_LOG_LEVEL_INFO;
   if (level == "WARNING")
      return LC_LOG_LEVEL_WARN;
   if (level == "ERROR")
      return LC_LOG_LEVEL_ERROR;
   if (level == "CRITICAL")
      return LC_LOG_LEVEL_CRITICAL;

   LC_Log<T>().appendHeader(LC_LOG_LEVEL_WARN)
         << "Unknown logging level '" << level << "'. Using INFO level as default.";
   return LC_LOG_LEVEL_INFO;
}

/*------------------------------------------------------------------------------
|    LC_Output2Std::output
+-----------------------------------------------------------------------------*/
inline void
LC_Output2Std::printf(LC_Log<LC_Output2Std>& logger, va_list args)
{
#ifdef COLORING_ENABLED
   std::stringstream sink;
   sink << (char)0x1B
        << "[" << (int)LC_LOG_TEXT_ATTRIB_RESET << ";"
        << (int)(getColorForLevel(logger.m_level) + 30) << "m";
   sink << logger.m_string.str();
   sink << (char)0x1B
        << "[" << (int)LC_LOG_TEXT_ATTRIB_RESET << "m" << std::endl;
   std::string final = sink.str();
#else
   std::string final = logger.m_string.str();
#endif // COLORING_ENABLED

   // I prefer to flush to avoid missing buffered logs in case of crash.
   FILE* stdOut = stdout;
   if (logger.m_level == LC_LOG_LEVEL_ERROR || logger.m_level == LC_LOG_LEVEL_CRITICAL)
      stdOut = stderr;
   ::vfprintf(stdOut, final.c_str(), args);
   ::fflush(stdOut);
}

/*------------------------------------------------------------------------------
|    LC_Output2Std::getColorForLevel
+-----------------------------------------------------------------------------*/
inline LC_LogFgColor
LC_Output2Std::getColorForLevel(LC_LogLevel level)
{
   static const LC_LogFgColor LC_COLOR_MAP[] = {
      LC_LOG_FG_COLOR_RED,
      LC_LOG_FG_COLOR_RED,
      LC_LOG_FG_COLOR_YELLOW,
      LC_LOG_FG_COLOR_GREEN,
      LC_LOG_FG_COLOR_WHITE,
      LC_LOG_FG_COLOR_BLUE
   };

   return LC_COLOR_MAP[level];
}

/*------------------------------------------------------------------------------
|    LC_Output2File::stream
+-----------------------------------------------------------------------------*/
inline
FILE*& LC_Output2FILE::stream()
{
   static FILE* pStream = fopen("output.log", "a");
   return pStream;
}

/*------------------------------------------------------------------------------
|    LC_Output2File::output
+-----------------------------------------------------------------------------*/
inline
void LC_Output2FILE::printf(LC_Log<LC_Output2Std>& logger, va_list args)
{
   FILE* pStream = stream();
   if (!pStream)
      return;

   vfprintf(pStream, logger.m_string.str().c_str(), args);
   fflush(pStream);
}
typedef LC_Log<LC_Output2FILE> LC_LogFile;

#ifdef __ANDROID__
/*------------------------------------------------------------------------------
|    LC_OutputAndroid class
+-----------------------------------------------------------------------------*/
class LC_OutputAndroid
{
public:
   static void printf(LC_Log<LC_OutputAndroid>& logger, va_list args);
};
typedef LC_Log<LC_OutputAndroid> LC_LogAndroid;

/*------------------------------------------------------------------------------
|    LC_OutputAndroid::printf
+-----------------------------------------------------------------------------*/
inline
void LC_OutputAndroid::printf(LC_Log<LC_OutputAndroid>& logger, va_list args)
{
   static const android_LogPriority android_logPriority[] = {
      // Do not mess with the order. Must map the LC_LogLevel enum.
      ANDROID_LOG_FATAL,
      ANDROID_LOG_ERROR,
      ANDROID_LOG_WARN,
      ANDROID_LOG_INFO,
      ANDROID_LOG_VERBOSE,
      ANDROID_LOG_DEBUG
   };

   __android_log_vprint(
         android_logPriority[logger.m_level],
         logger.m_log_tag,
         logger.m_string.str().c_str(),
         args);
}
#endif // __ANDROID__

#ifdef QT_VERSION

#endif // QT_VERSION

// Assertions.
#ifdef __ANDROID__
#define LOG_ASSERT(cond, text) \
   {if (!(cond)) __android_log_assert(0, LOG_TAG, text);}
#else
#define LOG_ASSERT(cond, text) assert(cond)
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#include <windows.h>

inline std::string current_time()
{
   const int MAX_LEN = 200;
   char buffer[MAX_LEN];
   if (GetTimeFormatA(LOCALE_USER_DEFAULT, 0, 0,
                      "HH':'mm':'ss", buffer, MAX_LEN) == 0)
      return "Error in NowTime()";

   char result[100] = {0};
   static DWORD first = GetTickCount();
   std::sprintf(result, "%s.%03ld", buffer, (long)(GetTickCount() - first) % 1000);
   return result;
}
#else
#include <sys/time.h>

inline std::string
lc_current_time()
{
   char buffer[11];
   time_t t;
   time(&t);

   tm r;
   memset(&r, 0, sizeof(tm));
   strftime(buffer, sizeof(buffer), "%X", localtime_r(&t, &r));

   struct timeval tv;
   gettimeofday(&tv, 0);

   char result[100] = {0};
   std::sprintf(result, "%s.%03ld", buffer, (long)tv.tv_usec / 1000);

   return result;
}

#if 0
#ifdef ANDROID
#ifndef LOG_TAG
#define LOG_TAG "DefaultTag"
#endif
#define LOG_ASSERT(cond, text)                \
   if (!(cond))                               \
      __android_log_assert(0, LOG_TAG, text);
#else
#define LOG_ASSERT(cond, text) assert(cond)
#endif

// Database logging.
#define LOG_ERROR_QUERY(q, ...) qCritical(__VA_ARGS__); \
   qCritical("Executed query: %s.", qPrintable(q.executedQuery())); \
   if (q.lastError().isValid()) { \
      qCritical("Error description: %s.", qPrintable(q.lastError().databaseText())); \
      qCritical("Error code: %d.", q.lastError().number()); \
   }

#define LOG_ERROR_DB(q, ...) qCritical(__VA_ARGS__); \
   if (q.lastError().isValid()) { \
      qCritical("Error description: %s.", qPrintable(q.lastError().databaseText())); \
      qCritical("Error code: %d.", q.lastError().number()); \
   }

// Definitions relative to log level.
#ifdef LOG_LEVEL_VERBOSE
#define LOG_FUNCTION \
   { \
   fprintf(stderr, "%s: invoked.\n", __FUNCTION__); \
   fflush(stderr); \
   }
#else
#define LOG_FUNCTION
#endif
#endif

// Prevent from using outside.
#undef VA_LIST_CONTEXT
#undef LOG_UNUSED

#endif //WIN32
#endif // LC_LOGGING_H
