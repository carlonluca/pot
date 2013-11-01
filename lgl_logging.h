/*
* Author:  Luca Carlon
* Company: -
* Date:    05.12.2011
*
* Copyright (c) 2013 Luca Carlon. All rights reserved.
*
* All rights reserved. This program and the accompanying materials
* are made available under the terms of the GNU Lesser General Public License
* (LGPL) version 2.1 which accompanies this distribution, and is available at
* http://www.gnu.org/licenses/lgpl-2.1.html
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*/

/**
* Classes and structures for logging purposes. Tested on Android, iOS, Linux Embedded,
* Mac OS X, Windows Vista/7/8, Linux.
*
* Available conf macros:
* 1. COLORING_ENABLED: if defined it forces coloring. If not defined, it is defined or
*    not according to the platform.
* 2. ENABLE_LOG_*: if defined, it enables the specific log level.
* 3. BUILD_LOG_LEVEL_*: if defined it enables all the levels with equal or higher
*    priority.
* 4. BUILD_LOG_LEVEL_ALL: enables all the logs.
* 5. XCODE_COLORING_ENABLED: Enables coloring with XCode coloring format. This also
*    enables COLORING_ENABLED automatically.
*
* Version: 1.0.0
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
#if !defined(_WIN32) && !defined(_WIN32_WCE) && !defined(__ANDROID__)
#include <unistd.h>
#include <execinfo.h>
#include <cxxabi.h>
#elif defined(_WIN32) || defined(_WIN32_WCE)
#include <WinBase.h>
#if WINVER < 0x0602
// It seems Windows 8.1 does not support DbgHelp yet.
#include <DbgHelp.h>
#else
// On Windows 8 this seems to be needed to get timing functions.
#include <datetimeapi.h>
#endif // WINVER<0x0602
#endif // !defined(_WIN32) && !defined(_WIN32_WCE) && !defined(NTDDI_WIN8)
#ifdef __ANDROID__
#include <android/log.h>
#else
#include <assert.h>
#endif // __ANDROID__
#ifdef QT_SQL_LIB
#include <QSqlQuery>
#include <QSqlError>
#endif // QT_SQL_LIB
#if defined(__APPLE__) && (__OBJC__ == 1)
#include <Foundation/Foundation.h>
#endif

#define VA_LIST_CONTEXT(last, i) \
   {va_list args; va_start(args, last); i; va_end(args); }
#define LOG_UNUSED(x) \
   (void) x

// Coloring is automatically enabled if XCODE_COLORING_ENABLED is defined.
#ifdef XCODE_COLORING_ENABLED
#define COLORING_ENABLED
#endif

// Coloring is automatically enabled on some platforms.
#ifndef COLORING_ENABLED
#if __APPLE__
#include "TargetConditionals.h"
#if TARGET_IPHONE_SIMULATOR
// iOS Simulator
#elif TARGET_OS_IPHONE
// iOS device
#elif TARGET_OS_MAC
#define COLORING_ENABLED
#else
// Unsupported platform
#endif
#elif !defined(_WIN32) && !defined(_WIN32_WCE)
#define COLORING_ENABLED
#else
// Probably some Windows. You might want to enable coloring if you intend
// to run from cygwin or similar.
#endif // __APPLE__
#endif // COLORING_ENABLED

// Define a LOG_TAG macro project-widely or none is used.
#ifndef LOG_TAG
#define LOG_TAG NULL
#endif

// Enable logs according to log level.
#ifdef BUILD_LOG_LEVEL_DEBUG
#define BUILD_LOG_LEVEL_ALL
#elif defined(BUILD_LOG_LEVEL_VERBOSE)
#define ENABLE_LOG_CRITICAL
#define ENABLE_LOG_ERROR
#define ENABLE_LOG_WARNING
#define ENABLE_LOG_INFORMATION
#define ENABLE_LOG_VERBOSE
#elif defined(BUILD_LOG_LEVEL_INFORMATION)
#define ENABLE_LOG_CRITICAL
#define ENABLE_LOG_ERROR
#define ENABLE_LOG_WARNING
#define ENABLE_LOG_INFORMATION
#elif defined(BUILD_LOG_LEVEL_WARNING)
#define ENABLE_LOG_CRITICAL
#define ENABLE_LOG_ERROR
#define ENABLE_LOG_WARNING
#elif defined(BUILD_LOG_LEVEL_ERROR)
#define ENABLE_LOG_CRITICAL
#define ENABLE_LOG_ERROR
#elif defined(BUILD_LOG_LEVEL_CRITICAL)
#define ENABLE_LOG_CRITICAL
#else
// If none is selected select all.
#ifndef BUILD_LOG_LEVEL_ALL
#define BUILD_LOG_LEVEL_ALL
#endif
#endif // BUILD_LOG_LEVEL_DEBUG

#ifdef BUILD_LOG_LEVEL_ALL
#define ENABLE_LOG_CRITICAL
#define ENABLE_LOG_ERROR
#define ENABLE_LOG_WARNING
#define ENABLE_LOG_INFORMATION
#define ENABLE_LOG_VERBOSE
#define ENABLE_LOG_DEBUG
#endif // BUILD_LOG_LEVEL_ALL

/*------------------------------------------------------------------------------
|    definitions
+-----------------------------------------------------------------------------*/
enum LC_LogLevel {
   // Do not mess with the order.
   LC_LOG_CRITICAL = 0,
   LC_LOG_ERROR = 1,
   LC_LOG_WARN = 2,
   LC_LOG_INFO = 3,
   LC_LOG_VERBOSE = 4,
   LC_LOG_DEBUG = 5,
   LC_LOG_NONE = 1000
};

// Text attributes.
#define C_RESET     0
#define C_BRIGHT    1 // Bold.
#define C_DIM       2
#define C_UNDERLINE 4 // Underscore
#define C_BLINK     5
#define C_REVERSE   7
#define C_HIDDEN    8

enum LC_LogAttrib {
   LC_LOG_ATTR_RESET = C_RESET,
   LC_LOG_ATTR_BRIGHT = C_BRIGHT,
   LC_LOG_ATTR_DIM = C_DIM,
   LC_LOG_ATTR_UNDERLINE = C_UNDERLINE,
   LC_LOG_ATTR_BLINK = C_BLINK,
   LC_LOG_ATTR_REVERSE = C_REVERSE,
   LC_LOG_ATTR_HIDDEN = C_HIDDEN
};

// Foreground colors.
#define C_F_BLACK     0
#define C_F_RED       1
#define C_F_GREEN     2
#define C_F_YELLOW    3
#define C_F_BLUE      4
#define C_F_MAGENTA   5
#define C_F_CYAN      6
#define C_F_WHITE     7
#define C_F_DEFAULT   9

enum LC_LogColor {
   LC_LOG_COL_BLACK = C_F_BLACK,
   LC_LOG_COL_RED = C_F_RED,
   LC_LOG_COL_GREEN = C_F_GREEN,
   LC_LOG_COL_YELLOW = C_F_YELLOW,
   LC_LOG_COL_BLUE = C_F_BLUE,
   LC_LOG_COL_MAGENTA = C_F_MAGENTA,
   LC_LOG_COL_CYAN = C_F_CYAN,
   LC_LOG_COL_WHITE = C_F_WHITE,
   LC_LOG_COL_DEFAULT = C_F_DEFAULT
};

#ifdef XCODE_COLORING_ENABLED
static const NSString* LC_XC_COL [] = {
   @"fg0,0,0;",
   @"fg255,0,0;",
   @"fg34,139,34;",
   @"fg255,215,0;",  // Gold should be more visible.
   @"fg0,0,255;",
   @"fg255,20,147;", // Magenta.
   @"fg0,255,255;",
   @"fg255,255,255;",
   @"fg0,0,0;"
};
#endif // XCODE_COLORING_ENABLED

#ifdef ENABLE_LOG_CRITICAL
inline bool log_critical_t_v(const char* log_tag, const char* format, va_list args);
inline bool log_critical_t(const char* log_tag, const char* format, ...);
inline bool log_critical_v(const char* format, va_list args);
inline bool log_critical(const char* format, ...);
#if defined(__APPLE__) && __OBJC__ == 1
inline BOOL log_critical_t_v(const char* log_tag, NSString* format, va_list args);
inline BOOL log_critical_t(const char* log_tag, NSString* format, ...);
inline BOOL log_critical_v(NSString* format, va_list args);
inline BOOL log_critical(NSString* format, ...);
#endif // defined(__APPLE__) && __OBJC__ == 1
#else
inline bool log_critical_t_v(...) { return false; }
inline bool log_critical_t(...)   { return false; }
inline bool log_critical_v(...)   { return false; }
inline bool log_critical(...)     { return false; }
#endif // ENABLE_LOG_CRITICAL

#ifdef ENABLE_LOG_ERROR
inline bool log_err_t_v(const char* log_tag, const char* format, va_list args);
inline bool log_err_t(const char* log_tag, const char* format, ...);
inline bool log_err_v(const char* format, va_list args);
inline bool log_err(const char* format, ...);
#if defined(__APPLE__) && __OBJC__ == 1
inline BOOL log_err_t_v(const char* log_tag, NSString* format, va_list args);
inline BOOL log_err_t(const char* log_tag, NSString* format, ...);
inline BOOL log_err_v(NSString* format, va_list args);
inline BOOL log_err(NSString* format, ...);
#endif // defined(__APPLE__) && __OBJC__ == 1
#else
inline bool log_err_t_v(...) { return false; }
inline bool log_err_t(...)   { return false; }
inline bool log_err_v(...)   { return false; }
inline bool log_err(...)     { return false; }
#endif // ENABLE_LOG_ERROR

#ifdef ENABLE_LOG_WARNING
inline bool log_warn_t_v(const char* log_tag, const char* format, va_list args);
inline bool log_warn_t(const char* log_tag, const char* format, ...);
inline bool log_warn_v(const char* format, va_list args);
inline bool log_warn(const char* format, ...);
#if defined(__APPLE__) && __OBJC__ == 1
inline BOOL log_warn_t_v(const char* log_tag, NSString* format, va_list args);
inline BOOL log_warn_t(const char* log_tag, NSString* format, ...);
inline BOOL log_warn_v(NSString* format, va_list args);
inline BOOL log_warn(NSString* format, ...);
#endif // defined(__APPLE__) && __OBJC__ == 1
#else
inline bool log_warn_t_v(...) { return false; }
inline bool log_warn_t(...)   { return false; }
inline bool log_warn_v(...)   { return false; }
inline bool log_warn(...)     { return false; }
#endif // ENABLE_LOG_WARNING

#ifdef ENABLE_LOG_INFORMATION
inline bool log_info_t_v(const char* log_tag, const char* format, va_list args);
inline bool log_info_t(const char* log_tag, const char* format, ...);
inline bool log_info_v(const char* format, va_list args);
inline bool log_info(const char* format, ...);
// TODO: Some more functions might be needed!
inline bool log_formatted_t_v(const char* log_tag, LC_LogAttrib a, LC_LogColor c, const char* format, va_list args);
inline bool log_formatted_t(const char* log_tag, LC_LogAttrib a, LC_LogColor c, const char* format, ...);
inline bool log_formatted_v(LC_LogAttrib a, LC_LogColor c, const char* format, va_list args);
inline bool log_formatted(LC_LogAttrib a, LC_LogColor c, const char* format, ...);
inline bool log_formatted(LC_LogColor c, const char* format, ...);
#if defined(__APPLE__) && __OBJC__ == 1
inline BOOL log_info_t_v(const char* log_tag, NSString* format, va_list args);
inline BOOL log_info_t(const char* log_tag, NSString* format, ...);
inline BOOL log_info_v(NSString* format, va_list args);
inline BOOL log_info(NSString* format, ...);
#endif // defined(__APPLE__) && __OBJC__ == 1
#else
inline bool log_info_t_v(...)      { return true; }
inline bool log_info_t(...)        { return true; }
inline bool log_info_v(...)        { return true; }
inline bool log_info(...)          { return true; }
inline bool log_formatted_t_v(...) { return true; }
inline bool log_formatted_t(...)   { return true; }
inline bool log_formatted_v(...)   { return true; }
inline bool log_formatted(...)     { return true; }
#endif // ENABLE_LOG_INFORMATION

#ifdef ENABLE_LOG_VERBOSE
inline bool log_verbose_t_v(const char* log_tag, const char* format, va_list args);
inline bool log_verbose_t(const char* log_tag, const char* format, ...);
inline bool log_verbose_v(const char* format, va_list args);
inline bool log_verbose(const char* format, ...);
#if defined(__APPLE__) && __OBJC__ == 1
inline BOOL log_vervose_t_v(const char* log_tag, NSString* format, va_list args);
inline BOOL log_verbose_t(const char* log_tag, NSString* format, ...);
inline BOOL log_verbose_v(NSString* format, va_list args);
inline BOOL log_verbose(NSString* format, ...);
#endif // defined(__APPLE__) && __OBJC__ == 1
#else
inline bool log_verbose_t_v(...) { return true; }
inline bool log_verbose_t(...)   { return true; }
inline bool log_verbose_v(...)   { return true; }
inline bool log_verbose(...)     { return true; }
#endif // ENABLE_LOG_VERBOSE

#ifdef ENABLE_LOG_DEBUG
inline void log_debug_t_v(const char* log_tag, const char* format, va_list args);
inline void log_debug_t(const char* log_tag, const char* format, ...);
inline void log_debug_v(const char* format, va_list args);
inline void log_debug(const char* format, ...);
#if defined(__APPLE__) && __OBJC__ == 1
inline void log_debug_t_v(const char* log_tag, NSString* format, va_list args);
inline void log_debug_t(const char* log_tag, NSString* format, ...);
inline void log_debug_v(NSString* format, va_list args);
inline void log_debug(NSString* format, ...);
#endif // defined(__APPLE__) && __OBJC__ == 1
#else
inline void log_debug_t_v(...) {}
inline void log_debug_t(...)   {}
inline void log_debug_v(...)   {}
inline void log_debug(...)     {}
#endif // ENABLE_LOG_DEBUG

#if !defined(__ANDROID__) && (!defined(WINVER) || WINVER < 0x0602)
inline void log_stacktrace(const char* log_tag, LC_LogLevel level, unsigned int max_frames = 65);
inline void log_stacktrace(const char* log_tag, unsigned int max_frames = 65);
inline void log_stacktrace(LC_LogLevel level, unsigned int max_frames = 65);
inline void log_stacktrace(unsigned int max_frames = 65);
#endif // !defined(__ANDROID__) && (!defined(WINVER) || WINVER < 0x06020000)

inline std::string lc_current_time();

/*------------------------------------------------------------------------------
|    lc_font_change
+-----------------------------------------------------------------------------*/
/**
* @brief lc_font_change Changes font attributes. These will be used for the text
* following this call.
* @param f The FILE* on which the color change should be written.
* @param attrib An attribute.
* @param color A color.
*/
inline void lc_font_change(FILE* f, LC_LogAttrib attrib, LC_LogColor color)
{
   fprintf(f, "%c[%d;%dm", 0x1B, attrib, color + 30);
}

/*------------------------------------------------------------------------------
|    lc_font_reset
+-----------------------------------------------------------------------------*/
/**
* @brief lc_font_reset Resets font attributes and color to the default params.
* @param f The file descriptor to use.
*/
inline void lc_font_reset(FILE* f)
{
   fprintf(f, "%c[%dm", 0x1B, C_RESET);
}

/*------------------------------------------------------------------------------
|    lc_color_change
+-----------------------------------------------------------------------------*/
/**
* @brief lc_color_change Changes the color that will be used for the following text.
* @param f The file descriptor the color change will be written.
* @param color The color to be used.
*/
inline void lc_color_change(FILE* f, LC_LogColor color)
{
   lc_font_change(f, LC_LOG_ATTR_RESET, color);
}

/*------------------------------------------------------------------------------
|    lc_formatted_printf
+-----------------------------------------------------------------------------*/
/**
* @brief lc_formatted_printf printf-like function accepting attributes and color for
* the text being written.
* @param f The output file descriptor to use.
* @param attrib Attribute to use for the font.
* @param color Color to use for the font.
* @param format The text to write followed by the parameters.
*/
inline void lc_formatted_printf(FILE* f, LC_LogAttrib attrib, LC_LogColor color, const char* format, ...)
{
   std::stringstream sink;
   sink << (char) 0x1B
      << "[" << (int) attrib << ";"
      << (int) ((int) color + 30) << "m";
   sink << format;
   sink << (char) 0x1B
      << "[" << (int) LC_LOG_ATTR_RESET << "m";
   std::string final = sink.str();

   VA_LIST_CONTEXT(format, vfprintf(f, final.c_str(), args));
}

// Assertions.
#ifdef __ANDROID__
#define LOG_ASSERT(cond, text) \
{if (!(cond)) __android_log_assert(0, LOG_TAG, text); }
#else
#define LOG_ASSERT(cond, text) assert(cond)
#endif

/*------------------------------------------------------------------------------
|    LC_NullStreamBuf class
+-----------------------------------------------------------------------------*/
/**
* @brief The LC_NullStreamBuf class implements a null sink for a char stream.
*/
class LC_NullStreamBuf : public std::basic_streambuf<char> {};

/*------------------------------------------------------------------------------
|    LC_NullStream class
+-----------------------------------------------------------------------------*/
/**
* @brief The LC_NullStream struct A type that can be used as a null stream. This should
* be optimized by the compiler.
*/
class LC_NullStream : public std::ostream {
public:
   LC_NullStream() : std::ostream(&buf) {}
private:
   LC_NullStreamBuf buf;
};

/*------------------------------------------------------------------------------
|    LC_LogPriv class
+-----------------------------------------------------------------------------*/
/**
* Internal class used for logging. This class is used to prepare the string that
* will be printed by some other delegate class.
*/
template <typename T>
class LC_Log
{
public:
   LC_Log(LC_LogLevel level);
   LC_Log(LC_LogColor color);
   LC_Log(const char* log_tag);
   LC_Log(const char* log_tag, LC_LogLevel level);
   LC_Log(const char* log_tag, LC_LogAttrib attrib, LC_LogColor color);

   ~LC_Log();

   std::ostream& stream();

   void printf(const char* format, ...);
   void printf(const char* format, va_list args);

#ifdef QT_SQL_LIB
   void printf(QSqlQuery& query, const char* format, ...);
   void printf(QSqlQuery& query, const char *format, va_list args);
#endif // QT_SQL_LIB

#if defined(__APPLE__) && defined(__OBJC__)
   void printf(NSString* format, ...);
   void printf(NSString* format, va_list args);
#endif

   static std::string toString(LC_LogLevel level);
   static LC_LogLevel fromString(const std::string& level);

   void prependHeader(std::string& s);
   void prependLogTagIfNeeded(std::string& s);

   std::stringstream m_string;

   // If level is set, then attrib and color are not unless level
   // is none.
   LC_LogLevel m_level;
   const char* m_log_tag;
   LC_LogAttrib m_attrib;
   LC_LogColor m_color;

private:
   LC_Log(const LC_Log&);
   LC_Log& operator =(const LC_Log&) ;

   void initForLevel(const LC_LogLevel& level);

   std::ostringstream m_stream;
};

/*------------------------------------------------------------------------------
|    LC_Output2Std class
+-----------------------------------------------------------------------------*/
class LC_Output2Std
{
public:
   static void printf(LC_Log<LC_Output2Std>& logger, va_list args);
   static LC_LogColor getColorForLevel(LC_LogLevel level);
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

#ifdef ENABLE_MSVS_OUTPUT
/*------------------------------------------------------------------------------
|    LC_Output2File class
+-----------------------------------------------------------------------------*/
/**
* Used to send debugging messages to Visual Studio instead of standard output.
*/
class LC_Output2MSVS
{
public:
   static void printf(LC_Log<LC_Output2MSVS>& logger, va_list args);
};
typedef LC_Log<LC_Output2MSVS> LC_LogMSVC;
#endif // ENABLE_MSVS_OUTPUT

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
#endif // __ANDROID__

#ifdef XCODE_COLORING_ENABLED
/*------------------------------------------------------------------------------
|    LC_OutputXCodeColors class
+-----------------------------------------------------------------------------*/
class LC_Output2XCodeColors
{
public:
   static void printf(LC_Log<LC_Output2XCodeColors>& logger, va_list args);
   static bool checkEnv();
   static NSString* getColorForLevel(LC_LogLevel level);
};
typedef LC_Log<LC_Output2XCodeColors> LC_LogXCodeColors;
#endif

// Define the default logger.
#ifdef __ANDROID__
typedef LC_Log<LC_OutputAndroid> LC_LogDef;
#elif defined(XCODE_COLORING_ENABLED)
typedef LC_Log<LC_Output2XCodeColors> LC_LogDef;
#elif defined(ENABLE_MSVS_OUTPUT)
typedef LC_Log<LC_Output2MSVS> LC_LogDef;
#else
typedef LC_Log<LC_Output2Std> LC_LogDef;
#endif

#ifdef ENABLE_LOG_CRITICAL
/*------------------------------------------------------------------------------
|    log_critical_t_v
+-----------------------------------------------------------------------------*/
inline bool log_critical_t_v(const char* log_tag, const char* format, va_list args)
{
   LC_LogDef(log_tag, LC_LOG_CRITICAL).printf(format, args);
   return false;
}

/*------------------------------------------------------------------------------
|    log_err_t
+-----------------------------------------------------------------------------*/
inline bool log_critical_t(const char* log_tag, const char* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogDef(log_tag, LC_LOG_CRITICAL).printf(format, args));
   return false;
}

/*------------------------------------------------------------------------------
|    log_err_v
+-----------------------------------------------------------------------------*/
inline bool log_critical_v(const char* format, va_list args)
{
   LC_LogDef(LC_LOG_CRITICAL).printf(format, args);
   return false;
}

/*------------------------------------------------------------------------------
|    log_err
+-----------------------------------------------------------------------------*/
inline bool log_critical(const char* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogDef(LC_LOG_CRITICAL).printf(format, args));
   return false;
}

#if defined(__APPLE__) && __OBJC__ == 1
/*------------------------------------------------------------------------------
|    log_critical_t_v
+-----------------------------------------------------------------------------*/
inline bool log_critical_t_v(const char* log_tag, NSString* format, va_list args)
{
   LC_LogDef(log_tag, LC_LOG_CRITICAL).printf(format, args);
   return NO;
}

/*------------------------------------------------------------------------------
|    log_critical_t
+-----------------------------------------------------------------------------*/
inline bool log_critical_t(const char* log_tag, NSString* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogDef(log_tag, LC_LOG_CRITICAL).printf(format, args));
   return NO;
}

/*------------------------------------------------------------------------------
|    log_critical_v
+-----------------------------------------------------------------------------*/
inline bool log_critical_v(NSString* format, va_list args)
{
   LC_LogDef(LC_LOG_CRITICAL).printf(format, args);
   return NO;
}

/*------------------------------------------------------------------------------
|    log_critical
+-----------------------------------------------------------------------------*/
inline bool log_critical(NSString* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogDef(LC_LOG_CRITICAL).printf(format, args));
   return NO;
}
#endif // defined(__APPLE__) && __OBJC__ == 1
#endif // ENABLE_LOG_CRITICAL

#ifdef ENABLE_LOG_ERROR
/*------------------------------------------------------------------------------
|    log_err_t_v
+-----------------------------------------------------------------------------*/
inline bool log_err_t_v(const char* log_tag, const char* format, va_list args)
{
   LC_LogDef(log_tag, LC_LOG_ERROR).printf(format, args);
   return false;
}

/*------------------------------------------------------------------------------
|    log_err_t
+-----------------------------------------------------------------------------*/
inline bool log_err_t(const char* log_tag, const char* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogDef(log_tag, LC_LOG_ERROR).printf(format, args));
   return false;
}

/*------------------------------------------------------------------------------
|    log_err_v
+-----------------------------------------------------------------------------*/
inline bool log_err_v(const char* format, va_list args)
{
   LC_LogDef(LC_LOG_ERROR).printf(format, args);
   return false;
}

/*------------------------------------------------------------------------------
|    log_err
+-----------------------------------------------------------------------------*/
inline bool log_err(const char* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogDef(LC_LOG_ERROR).printf(format, args));
   return false;
}

#if defined(__APPLE__) && __OBJC__ == 1
/*------------------------------------------------------------------------------
|    log_err_t_v
+-----------------------------------------------------------------------------*/
inline bool log_err_t_v(const char* log_tag, NSString* format, va_list args)
{
   LC_LogDef(log_tag, LC_LOG_ERROR).printf(format, args);
   return NO;
}

/*------------------------------------------------------------------------------
|    log_err_t
+-----------------------------------------------------------------------------*/
inline bool log_err_t(const char* log_tag, NSString* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogDef(log_tag, LC_LOG_ERROR).printf(format, args));
   return NO;
}

/*------------------------------------------------------------------------------
|    log_err_v
+-----------------------------------------------------------------------------*/
inline bool log_err_v(NSString* format, va_list args)
{
   LC_LogDef(LC_LOG_ERROR).printf(format, args);
   return NO;
}

/*------------------------------------------------------------------------------
|    log_critical
+-----------------------------------------------------------------------------*/
inline bool log_err(NSString* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogDef(LC_LOG_ERROR).printf(format, args));
   return NO;
}
#endif // defined(__APPLE__) && __OBJC__ == 1
#endif // ENABLE_LOG_ERROR

#ifdef ENABLE_LOG_WARNING
/*------------------------------------------------------------------------------
|    log_warn_t_v
+-----------------------------------------------------------------------------*/
inline bool log_warn_t_v(const char* log_tag, const char* format, va_list args)
{
   LC_LogDef(log_tag, LC_LOG_WARN).printf(format, args);
   return false;
}

/*------------------------------------------------------------------------------
|    log_warn_t
+-----------------------------------------------------------------------------*/
inline bool log_warn_t(const char* log_tag, const char* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogDef(log_tag, LC_LOG_WARN).printf(format, args));
   return false;
}

/*------------------------------------------------------------------------------
|    log_warn_v
+-----------------------------------------------------------------------------*/
inline bool log_warn_v(const char* format, va_list args)
{
   LC_LogDef(LC_LOG_WARN).printf(format, args);
   return false;
}

/*------------------------------------------------------------------------------
|    log_warn
+-----------------------------------------------------------------------------*/
inline bool log_warn(const char* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogDef(LC_LOG_WARN).printf(format, args));
   return false;
}

#if defined(__APPLE__) && __OBJC__ == 1
/*------------------------------------------------------------------------------
|    log_critical_t_v
+-----------------------------------------------------------------------------*/
inline bool log_warn_t_v(const char* log_tag, NSString* format, va_list args)
{
   LC_LogDef(log_tag, LC_LOG_CRITICAL).printf(format, args);
   return NO;
}

/*------------------------------------------------------------------------------
|    log_critical_t
+-----------------------------------------------------------------------------*/
inline bool log_warn_t(const char* log_tag, NSString* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogDef(log_tag, LC_LOG_WARN).printf(format, args));
   return NO;
}

/*------------------------------------------------------------------------------
|    log_critical_v
+-----------------------------------------------------------------------------*/
inline bool log_warn_v(NSString* format, va_list args)
{
   LC_LogDef(LC_LOG_WARN).printf(format, args);
   return NO;
}

/*------------------------------------------------------------------------------
|    log_critical
+-----------------------------------------------------------------------------*/
inline bool log_warn(NSString* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogDef(LC_LOG_WARN).printf(format, args));
   return NO;
}
#endif // defined(__APPLE__) && __OBJC__ == 1
#endif // ENABLE_LOG_WARNING

#ifdef ENABLE_LOG_INFORMATION
/*------------------------------------------------------------------------------
|    log_info_t_v
+-----------------------------------------------------------------------------*/
inline bool log_info_t_v(const char* log_tag, const char* format, va_list args)
{
   LC_LogDef(log_tag, LC_LOG_INFO).printf(format, args);
   return true;
}

/*------------------------------------------------------------------------------
|    log_info_t
+-----------------------------------------------------------------------------*/
inline bool log_info_t(const char* log_tag, const char* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogDef(log_tag, LC_LOG_INFO).printf(format, args));
   return true;
}

/*------------------------------------------------------------------------------
|    log_info_vs
+-----------------------------------------------------------------------------*/
inline bool log_info_v(const char* format, va_list args)
{
   LC_LogDef(LC_LOG_INFO).printf(format, args);
   return true;
}

/*------------------------------------------------------------------------------
|    log_info
+-----------------------------------------------------------------------------*/
inline bool log_info(const char* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogDef(LC_LOG_INFO).printf(format, args));
   return true;
}

/*------------------------------------------------------------------------------
|    log_formatted_t
+-----------------------------------------------------------------------------*/
inline bool log_formatted_t_v(const char* log_tag, LC_LogAttrib a, LC_LogColor c, const char* format, va_list args)
{
   LC_LogDef(log_tag, a, c).printf(format, args);
   return true;
}

/*------------------------------------------------------------------------------
|    log_formatted_t
+-----------------------------------------------------------------------------*/
inline bool log_formatted_t(const char* log_tag, LC_LogAttrib a, LC_LogColor c, const char* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogDef(log_tag, a, c).printf(format, args));
   return true;
}

/*------------------------------------------------------------------------------
|    log_formatted_v
+-----------------------------------------------------------------------------*/
inline bool log_formatted_v(LC_LogAttrib a, LC_LogColor c, const char* format, va_list args)
{
   LC_LogDef(LOG_TAG, a, c).printf(format, args);
   return true;
}

/*------------------------------------------------------------------------------
|    log_formatted
+-----------------------------------------------------------------------------*/
inline bool log_formatted(LC_LogAttrib a, LC_LogColor c, const char* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogDef(LOG_TAG, a, c).printf(format, args));
   return true;
}

/*------------------------------------------------------------------------------
|    log_formatted
+-----------------------------------------------------------------------------*/
inline bool log_formatted(LC_LogColor c, const char* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogDef(LOG_TAG, LC_LOG_ATTR_RESET, c).printf(format, args));
   return true;
}

#if defined(__APPLE__) && __OBJC__ == 1
/*------------------------------------------------------------------------------
|    log_info_t_v
+-----------------------------------------------------------------------------*/
inline bool log_info_t_v(const char* log_tag, NSString* format, va_list args)
{
   LC_LogDef(log_tag, LC_LOG_INFO).printf(format, args);
   return YES;
}

/*------------------------------------------------------------------------------
|    log_info_t
+-----------------------------------------------------------------------------*/
inline bool log_info_t(const char* log_tag, NSString* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogDef(log_tag, LC_LOG_INFO).printf(format, args));
   return YES;
}

/*------------------------------------------------------------------------------
|    log_info_v
+-----------------------------------------------------------------------------*/
inline bool log_info_v(NSString* format, va_list args)
{
   LC_LogDef(LC_LOG_INFO).printf(format, args);
   return YES;
}

/*------------------------------------------------------------------------------
|    log_info
+-----------------------------------------------------------------------------*/
inline bool log_info(NSString* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogDef(LC_LOG_INFO).printf(format, args));
   return YES;
}

/*------------------------------------------------------------------------------
|    log_formatted_t
+-----------------------------------------------------------------------------*/
inline bool log_formatted_t(const char* log_tag, LC_LogAttrib a, LC_LogColor c, NSString* format, va_list args)
{
   LC_LogDef(log_tag, a, c).printf(format, args);
   return YES;
}

/*------------------------------------------------------------------------------
|    log_formatted
+-----------------------------------------------------------------------------*/
inline bool log_formatted_t(const char* log_tag, LC_LogAttrib a, LC_LogColor c, NSString* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogDef(log_tag, a, c).printf(format, args));
   return YES;
}

/*------------------------------------------------------------------------------
|    log_formatted
+-----------------------------------------------------------------------------*/
inline bool log_formatted(LC_LogAttrib a, LC_LogColor c, NSString* format, ...)
{
   VA_LIST_CONTEXT(format, log_formatted_t(LOG_TAG, a, c, format, args));
   return YES;
}

/*------------------------------------------------------------------------------
|    log_formatted
+-----------------------------------------------------------------------------*/
inline bool log_formatted(LC_LogColor c, NSString* format, ...)
{
   VA_LIST_CONTEXT(format, log_formatted_t(LOG_TAG, LC_LOG_ATTR_RESET, c, format, args));
   return YES;
}
#endif // defined(__APPLE__) && __OBJC__ == 1
#endif // ENABLE_LOG_INFORMATION

#ifdef ENABLE_LOG_VERBOSE
/*------------------------------------------------------------------------------
|    log_verbose_t_v
+-----------------------------------------------------------------------------*/
inline bool log_verbose_t_v(const char* log_tag, const char* format, va_list args)
{
   LC_LogDef(log_tag, LC_LOG_VERBOSE).printf(format, args);
   return true;
}

/*------------------------------------------------------------------------------
|    log_verbose_t
+-----------------------------------------------------------------------------*/
inline bool log_verbose_t(const char* log_tag, const char* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogDef(log_tag, LC_LOG_VERBOSE).printf(format, args));
   return true;
}

/*------------------------------------------------------------------------------
|    log_verbose_v
+-----------------------------------------------------------------------------*/
inline bool log_verbose_v(const char* format, va_list args)
{
   LC_LogDef(LC_LOG_VERBOSE).printf(format, args);
   return true;
}

/*------------------------------------------------------------------------------
|    log_verbose
+-----------------------------------------------------------------------------*/
inline bool log_verbose(const char* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogDef(LC_LOG_VERBOSE).printf(format, args));
   return true;
}

#if defined(__APPLE__) && __OBJC__ == 1
/*------------------------------------------------------------------------------
|    log_verbose_t_v
+-----------------------------------------------------------------------------*/
inline bool log_verbose_t_v(const char* log_tag, NSString* format, va_list args)
{
   LC_LogDef(log_tag, LC_LOG_VERBOSE).printf(format, args);
   return YES;
}

/*------------------------------------------------------------------------------
|    log_verbose_t
+-----------------------------------------------------------------------------*/
inline bool log_verbose_t(const char* log_tag, NSString* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogDef(log_tag, LC_LOG_VERBOSE).printf(format, args));
   return YES;
}

/*------------------------------------------------------------------------------
|    log_verbose_v
+-----------------------------------------------------------------------------*/
inline bool log_verbose_v(NSString* format, va_list args)
{
   LC_LogDef(LC_LOG_VERBOSE).printf(format, args);
   return YES;
}

/*------------------------------------------------------------------------------
|    log_verbose
+-----------------------------------------------------------------------------*/
inline bool log_verbose(NSString* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogDef(LC_LOG_VERBOSE).printf(format, args));
   return YES;
}
#endif // defined(__APPLE__) && __OBJC__ == 1
#endif // ENABLE_LOG_VERBOSE

#ifdef ENABLE_LOG_DEBUG
/*------------------------------------------------------------------------------
|    log_debug_t_v
+-----------------------------------------------------------------------------*/
inline void log_debug_t_v(const char* log_tag, const char* format, va_list args)
{
   LC_LogStd(log_tag, LC_LOG_DEBUG).printf(format, args);
}

/*------------------------------------------------------------------------------
|    log_debug_t
+-----------------------------------------------------------------------------*/
inline void log_debug_t(const char* log_tag, const char* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogStd(log_tag, LC_LOG_DEBUG).printf(format, args));
}

/*------------------------------------------------------------------------------
|    log_debug_v
+-----------------------------------------------------------------------------*/
inline void log_debug_v(const char* format, va_list args)
{
   LC_LogStd(LC_LOG_DEBUG).printf(format, args);
}

/*------------------------------------------------------------------------------
|    log_debug
+-----------------------------------------------------------------------------*/
inline void log_debug(const char* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogStd(LC_LOG_DEBUG).printf(format, args));
}

#if defined(__APPLE__) && __OBJC__ == 1
/*------------------------------------------------------------------------------
|    log_debug_t_v
+-----------------------------------------------------------------------------*/
inline void log_debug_t_v(const char* log_tag, NSString* format, va_list args)
{
   LC_LogDef(log_tag, LC_LOG_DEBUG).printf(format, args);
}

/*------------------------------------------------------------------------------
|    log_debug_t
+-----------------------------------------------------------------------------*/
inline void log_debug_t(const char* log_tag, NSString* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogDef(log_tag, LC_LOG_DEBUG).printf(format, args));
}

/*------------------------------------------------------------------------------
|    log_debug_v
+-----------------------------------------------------------------------------*/
inline void log_debug_v(NSString* format, va_list args)
{
   LC_LogDef(LC_LOG_DEBUG).printf(format, args);
}

/*------------------------------------------------------------------------------
|    log_debug
+-----------------------------------------------------------------------------*/
inline void log_debug(NSString* format, ...)
{
   VA_LIST_CONTEXT(format, LC_LogDef(LC_LOG_DEBUG).printf(format, args));
}
#endif // defined(__APPLE__) && __OBJC__ == 1
#endif // ENABLE_LOG_DEBUG

// Convenience macros. The same as using the inlined functions.
#ifdef __GNUC__
#define LOG_CRITICAL(tag, f, ...) \
   log_critical_t(tag, f, ##__VA_ARGS__)
#define LOG_ERROR(tag, f, ...) \
   log_err_t(tag, f, ##__VA_ARGS__)
#define LOG_WARNING(tag, f, ...) \
   log_warn_t(tag, f, ##__VA_ARGS__)
#define LOG_INFORMATION(tag, f, ...) \
   log_info_t(tag, f, ##__VA_ARGS__)
#define LOG_VERBOSE(tag, f, ...) \
   log_verbose_t(tag, f, ##__VA_ARGS__)
#define LOG_DEBUG(tag, f, ...) \
   log_debug_t(tag, f, ##__VA_ARGS__)
#endif

#if !defined(__ANDROID__) && (!defined(WINVER) || WINVER < 0x0602)
/* Unfortunately backtrace() is not supported by Bionic */

#if !defined(_WIN32) && !defined(_WIN32_WCE)
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
inline void log_stacktrace(const char* log_tag, LC_LogLevel level, unsigned int max_frames)
{
   std::stringstream stream;
   stream << std::endl;

   // storage array for stack trace address data
   void* addrlist[max_frames + 1];

   // retrieve current stack addresses
   int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*) );

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
   char* funcname = (char*) malloc(funcnamesize);

   // iterate over the returned symbol lines. skip the first, it is the
   // address of this function.
   for (int i = 1; i < addrlen; i++) {
      char* begin_name = 0, *begin_offset = 0, *end_offset = 0;

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
         *begin_name++ = '\0';
         *begin_offset++ = '\0';
         *end_offset = '\0';

         // mangled name is now in [begin_name, begin_offset) and caller
         // offset in [begin_offset, end_offset). now apply
         // __cxa_demangle():

         int status;
         char* ret = abi::__cxa_demangle(
            begin_name,
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
#else
/*------------------------------------------------------------------------------
|    log_stacktrace
+-----------------------------------------------------------------------------*/
inline void log_stacktrace(const char* log_tag, LC_LogLevel level, unsigned int max_frames)
{
   unsigned int i;
   void* stack = malloc(sizeof(void*) *max_frames);
   unsigned short frames;
   SYMBOL_INFO* symbol;
   HANDLE process;

   process = GetCurrentProcess();

   SymInitialize(process, NULL, TRUE);

   frames = CaptureStackBackTrace(0, max_frames, (PVOID*) stack, NULL);
   symbol = (SYMBOL_INFO*) calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char) , 1);
   symbol->MaxNameLen = 255;
   symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

   LC_LogDef logger(log_tag, level);
   logger.stream() << std::endl;
   for (i = 0; i < frames; i++) {
      // intptr_t should be guaranteed to be able to keep a pointer on any
      // platform. Anyway... anything can happen on Win :-(
      DWORD64 addr = (DWORD64) (*(((intptr_t*) stack + i)));
      SymFromAddr(process, addr, 0, symbol);
      logger.stream() << (frames - i - 1) << ": "
         << symbol->Name << " 0x" << (void*) symbol->Address << std::endl;
   }

   free(symbol);
   free(stack);
}
#endif // !defined(_WIN32) && !defined(_WIN32_WCE)

/*------------------------------------------------------------------------------
|    log_stacktrace
+-----------------------------------------------------------------------------*/
inline void log_stacktrace(LC_LogLevel level, unsigned int max_frames)
{
   log_stacktrace(LOG_TAG, level, max_frames);
}

/*------------------------------------------------------------------------------
|    log_stacktrace
+-----------------------------------------------------------------------------*/
inline void log_stacktrace(unsigned int max_frames)
{
   log_stacktrace(LOG_TAG, LC_LOG_DEBUG, max_frames);
}

/*------------------------------------------------------------------------------
|    log_stacktrace
+-----------------------------------------------------------------------------*/
inline void log_stacktrace(const char* log_tag, unsigned int max_frames)
{
   log_stacktrace(log_tag, LC_LOG_DEBUG, max_frames);
}
#endif // !defined(__ANDROID__) && (!defined(WINVER) || WINVER < 0x0602)

/*------------------------------------------------------------------------------
|    LC_Log<T>::LC_Log
+-----------------------------------------------------------------------------*/
template <typename T> inline LC_Log<T>::LC_Log(LC_LogColor color) :
   m_level(LC_LOG_NONE)
   , m_log_tag(LOG_TAG)
   , m_attrib(LC_LOG_ATTR_RESET)
   , m_color(color)
{
   // Do nothing.
}

/*------------------------------------------------------------------------------
|    LC_Log<T>::LC_Log
+-----------------------------------------------------------------------------*/
template <typename T> inline LC_Log<T>::LC_Log(const char* log_tag, LC_LogLevel level) :
   m_level(level)
   , m_log_tag(log_tag)
{
   // Do nothing.
}

/*------------------------------------------------------------------------------
|    LC_Log<T>::LC_Log
+-----------------------------------------------------------------------------*/
template <typename T> inline LC_Log<T>::LC_Log(const char *log_tag) :
   m_level(LC_LOG_INFO)
   , m_log_tag(log_tag)
{
   // Do nothing.
}

/*------------------------------------------------------------------------------
|    LC_Log<T>::LC_Log
+-----------------------------------------------------------------------------*/
template <typename T> inline LC_Log<T>::LC_Log(LC_LogLevel level) :
   m_level(level)
   , m_log_tag(LOG_TAG)
{
   // Do nothing.
}

/*------------------------------------------------------------------------------
|    LC_Log<T>::LC_Log
+-----------------------------------------------------------------------------*/
template <typename T> inline LC_Log<T>::LC_Log(const char* log_tag, LC_LogAttrib attrib, LC_LogColor color) :
   m_level(LC_LOG_NONE)
   , m_log_tag(log_tag)
   , m_attrib(attrib)
   , m_color(color)
{
   // Do nothing.
}

/*------------------------------------------------------------------------------
|    LC_Log<T>::appendHeader
+-----------------------------------------------------------------------------*/
template <typename T> inline void LC_Log<T>::prependHeader(std::string& s)
{
   if (m_level != LC_LOG_NONE)
      s.insert(0, toString(m_level) + ": ");
   //m_string << lc_current_time() << " ";
   s.insert(0, lc_current_time() + " ");
}

/*------------------------------------------------------------------------------
|    LC_Log<T>::appendlog_tagIfNeeded
+-----------------------------------------------------------------------------*/
template <typename T> inline void LC_Log<T>::prependLogTagIfNeeded(std::string& s)
{
   if (m_log_tag)
      s.insert(0, std::string("[") + std::string(m_log_tag) + std::string("]: "));
}

/*------------------------------------------------------------------------------
|    LC_Log<T>::printf
+-----------------------------------------------------------------------------*/
template <typename T> inline void LC_Log<T>::printf(const char* format, ...)
{
   if (m_level != LC_LOG_NONE) {
#ifdef BUILD_LOG_LEVEL_DEBUG
      // Always return a valid stream.
#elif defined(BUILD_LOG_LEVEL_VERBOSE)
      if (m_level > LC_LOG_VERBOSE)
         return;
#elif defined(BUILD_LOG_LEVEL_INFORMATION)
      if (m_level > LC_LOG_INFO)
         return;
#elif defined(BUILD_LOG_LEVEL_WARNING)
      if (m_level > LC_LOG_WARN)
         return;
#elif defined(BUILD_LOG_LEVEL_ERROR)
      if (m_level > LC_LOG_ERROR)
         return;
#elif defined(BUILD_LOG_LEVEL_CRITICAL)
      if (m_level > LC_LOG_CRITICAL)
         return;
#endif
   }

   VA_LIST_CONTEXT(format, this->printf(format, args));
}

/*------------------------------------------------------------------------------
|    LC_Log<T>::printf
+-----------------------------------------------------------------------------*/
template <typename T> inline void LC_Log<T>::printf(const char* format, va_list args)
{
   if (m_level != LC_LOG_NONE) {
#ifdef BUILD_LOG_LEVEL_DEBUG
      // Always return a valid stream.
#elif defined(BUILD_LOG_LEVEL_VERBOSE)
      if (m_level > LC_LOG_VERBOSE)
         return;
#elif defined(BUILD_LOG_LEVEL_INFORMATION)
      if (m_level > LC_LOG_INFO)
         return;
#elif defined(BUILD_LOG_LEVEL_WARNING)
      if (m_level > LC_LOG_WARN)
         return;
#elif defined(BUILD_LOG_LEVEL_ERROR)
      if (m_level > LC_LOG_ERROR)
         return;
#elif defined(BUILD_LOG_LEVEL_CRITICAL)
      if (m_level > LC_LOG_CRITICAL)
         return;
#endif
   }

   m_string << format;

   // Delegate log handling.
   T::printf(*this, args);
}

#ifdef QT_SQL_LIB
/*------------------------------------------------------------------------------
|    LC_Log<T>::printf
+-----------------------------------------------------------------------------*/
template <typename T> inline void LC_Log<T>::printf(QSqlQuery& query, const char* format, ...)
{
   if (m_level != LC_LOG_NONE) {
#ifdef BUILD_LOG_LEVEL_DEBUG
      // Always return a valid stream.
#elif defined(BUILD_LOG_LEVEL_VERBOSE)
      if (m_level > LC_LOG_VERBOSE)
         return;
#elif defined(BUILD_LOG_LEVEL_INFORMATION)
      if (m_level > LC_LOG_INFO)
         return;
#elif defined(BUILD_LOG_LEVEL_WARNING)
      if (m_level > LC_LOG_WARN)
         return;
#elif defined(BUILD_LOG_LEVEL_ERROR)
      if (m_level > LC_LOG_ERROR)
         return;
#elif defined(BUILD_LOG_LEVEL_CRITICAL)
      if (m_level > LC_LOG_CRITICAL)
         return;
#endif
   }

   VA_LIST_CONTEXT(format, printf(query, format, args));
}

/*------------------------------------------------------------------------------
|    LC_Log<T>::printf
+-----------------------------------------------------------------------------*/
template <typename T> inline void LC_Log<T>::printf(QSqlQuery& query, const char* format, va_list args)
{
   if (m_level != LC_LOG_NONE) {
#ifdef BUILD_LOG_LEVEL_DEBUG
      // Always return a valid stream.
#elif defined(BUILD_LOG_LEVEL_VERBOSE)
      if (m_level > LC_LOG_VERBOSE)
         return;
#elif defined(BUILD_LOG_LEVEL_INFORMATION)
      if (m_level > LC_LOG_INFO)
         return;
#elif defined(BUILD_LOG_LEVEL_WARNING)
      if (m_level > LC_LOG_WARN)
         return;
#elif defined(BUILD_LOG_LEVEL_ERROR)
      if (m_level > LC_LOG_ERROR)
         return;
#elif defined(BUILD_LOG_LEVEL_CRITICAL)
      if (m_level > LC_LOG_CRITICAL)
         return;
#endif
   }

   prependLogTagIfNeeded();
   prependHeader();
   m_string << format << std::endl;

   prependLogTagIfNeeded();
   m_string << "Failed to execute query: " << qPrintable(query.lastQuery()) << "." << std::endl;

   prependLogTagIfNeeded();
   m_string << "Error description: " << qPrintable(query.lastError().databaseText()) << "." << std::endl;

   prependLogTagIfNeeded();
   m_string << "Error code: " << query.lastError().number() << "." << std::endl;

   // Delegate.
   T::printf(*this, args);
}
#endif // QT_SQL_LIB

#if defined(__APPLE__) && (__OBJC__ == 1)
/*------------------------------------------------------------------------------
|    LC_Log<T>::printf
+-----------------------------------------------------------------------------*/
template <typename T> inline void LC_Log<T>::printf(NSString* format, ...)
{
   if (m_level != LC_LOG_NONE) {
#ifdef BUILD_LOG_LEVEL_DEBUG
      // Always return a valid stream.
#elif defined(BUILD_LOG_LEVEL_VERBOSE)
      if (m_level > LC_LOG_VERBOSE)
         return;
#elif defined(BUILD_LOG_LEVEL_INFORMATION)
      if (m_level > LC_LOG_INFO)
         return;
#elif defined(BUILD_LOG_LEVEL_WARNING)
      if (m_level > LC_LOG_WARN)
         return;
#elif defined(BUILD_LOG_LEVEL_ERROR)
      if (m_level > LC_LOG_ERROR)
         return;
#elif defined(BUILD_LOG_LEVEL_CRITICAL)
      if (m_level > LC_LOG_CRITICAL)
         return;
#endif
   }

   VA_LIST_CONTEXT(format, printf(format, args));
}

/*------------------------------------------------------------------------------
|    LC_Log<T>::printf
+-----------------------------------------------------------------------------*/
template <typename T> inline void LC_Log<T>::printf(NSString* format, va_list args)
{
   if (m_level != LC_LOG_NONE) {
#ifdef BUILD_LOG_LEVEL_DEBUG
      // Always return a valid stream.
#elif defined(BUILD_LOG_LEVEL_VERBOSE)
      if (m_level > LC_LOG_VERBOSE)
         return;
#elif defined(BUILD_LOG_LEVEL_INFORMATION)
      if (m_level > LC_LOG_INFO)
         return;
#elif defined(BUILD_LOG_LEVEL_WARNING)
      if (m_level > LC_LOG_WARN)
         return;
#elif defined(BUILD_LOG_LEVEL_ERROR)
      if (m_level > LC_LOG_ERROR)
         return;
#elif defined(BUILD_LOG_LEVEL_CRITICAL)
      if (m_level > LC_LOG_CRITICAL)
         return;
#endif
   }

   // Build the NSString from the format. This includes NSString's in args.
   NSString* s1 = [NSString stringWithUTF8String : m_string.str().c_str()];
   NSString* s2 = [[[NSString alloc] initWithFormat:format arguments : args] autorelease];
   NSString* s = [NSString stringWithFormat : @"%@%@", s1, s2];

   printf([s cStringUsingEncoding : NSUTF8StringEncoding]);
}
#endif // defined(__APPLE__) && (__OBJC__ == 1)

/*------------------------------------------------------------------------------
|    LC_Log<T>::~LC_Log
+-----------------------------------------------------------------------------*/
template <typename T> inline LC_Log<T>::~LC_Log()
{
   if (m_level != LC_LOG_NONE) {
#ifdef BUILD_LOG_LEVEL_DEBUG
      // Always return a valid stream.
#elif defined(BUILD_LOG_LEVEL_VERBOSE)
      if (m_level > LC_LOG_VERBOSE)
         return;
#elif defined(BUILD_LOG_LEVEL_INFORMATION)
      if (m_level > LC_LOG_INFO)
         return;
#elif defined(BUILD_LOG_LEVEL_WARNING)
      if (m_level > LC_LOG_WARN)
         return;
#elif defined(BUILD_LOG_LEVEL_ERROR)
      if (m_level > LC_LOG_ERROR)
         return;
#elif defined(BUILD_LOG_LEVEL_CRITICAL)
      if (m_level > LC_LOG_CRITICAL)
         return;
#endif
   }

   std::string s = m_stream.str();
   if (s.empty())
      return;
   printf(s.c_str());
}

/*------------------------------------------------------------------------------
|    LC_Log<T>::stream
+-----------------------------------------------------------------------------*/
template <typename T> inline std::ostream& LC_Log<T>::stream()
{
   static LC_NullStream nullStream;

   if (m_level != LC_LOG_NONE) {
#ifdef BUILD_LOG_LEVEL_DEBUG
      // Always return a valid stream.
#elif defined(BUILD_LOG_LEVEL_VERBOSE)
      if (m_level > LC_LOG_VERBOSE)
         return nullStream;
#elif defined(BUILD_LOG_LEVEL_INFORMATION)
      if (m_level > LC_LOG_INFO)
         return nullStream;
#elif defined(BUILD_LOG_LEVEL_WARNING)
      if (m_level > LC_LOG_WARN)
         return nullStream;
#elif defined(BUILD_LOG_LEVEL_ERROR)
      if (m_level > LC_LOG_ERROR)
         return nullStream;
#elif defined(BUILD_LOG_LEVEL_CRITICAL)
      if (m_level > LC_LOG_CRITICAL)
         return nullStream;
#endif
   }

   return m_stream;
}

/*------------------------------------------------------------------------------
|    LC_Log<T>::toString
+-----------------------------------------------------------------------------*/
template <typename T> inline std::string LC_Log<T>::toString(LC_LogLevel level)
{
   static const char* const buffer [] = {
      "CRITICAL", "ERROR", "WARNING",
      "INFORMATION", "VERBOSE", "DEBUG"
   };

   return buffer[level];
}

/*------------------------------------------------------------------------------
|    LC_Log<T>::fromString
+-----------------------------------------------------------------------------*/
template <typename T> inline LC_LogLevel LC_Log<T>::fromString(const std::string& level)
{
   if (level == "DEBUG")
      return LC_LOG_DEBUG;
   if (level == "VERBOSE")
      return LC_LOG_VERBOSE;
   if (level == "INFO")
      return LC_LOG_INFO;
   if (level == "WARNING")
      return LC_LOG_WARN;
   if (level == "ERROR")
      return LC_LOG_ERROR;
   if (level == "CRITICAL")
      return LC_LOG_CRITICAL;

   LC_Log<T>().prependHeader(LC_LOG_WARN)
      << "Unknown logging level '" << level << "'. Using INFO level as default.";
   return LC_LOG_INFO;
}

/*------------------------------------------------------------------------------
|    LC_Output2Std::output
+-----------------------------------------------------------------------------*/
inline void LC_Output2Std::printf(LC_Log<LC_Output2Std>& logger, va_list args)
{
#ifdef COLORING_ENABLED
   LC_LogAttrib attrib;
   LC_LogColor  color;
   if (logger.m_level != LC_LOG_NONE) {
      attrib = LC_LOG_ATTR_RESET;
      color = getColorForLevel(logger.m_level);
   }
   else {
      attrib = logger.m_attrib;
      color = logger.m_color;
   }

   std::stringstream sink;
   sink << (char) 0x1B
      << "[" << (int) attrib << ";"
      << (int) (color + 30) << "m";
   sink << logger.m_string.str();
   sink << (char) 0x1B
      << "[" << (int) LC_LOG_ATTR_RESET << "m" << std::endl;
   std::string final = sink.str();
#else
   std::stringstream s;
   s << logger.m_string.str() << std::endl;
   std::string final = s.str();
#endif // COLORING_ENABLED

   // Prepend tags.
   logger.prependHeader(final);
   logger.prependLogTagIfNeeded(final);

   // I prefer to flush to avoid missing buffered logs in case of crash.
   FILE* stdOut = stdout;
   if (logger.m_level == LC_LOG_ERROR || logger.m_level == LC_LOG_CRITICAL)
      stdOut = stderr;
   ::vfprintf(stdOut, final.c_str(), args);
   ::fflush(stdOut);
}

/*------------------------------------------------------------------------------
|    LC_Output2Std::getColorForLevel
+-----------------------------------------------------------------------------*/
inline LC_LogColor LC_Output2Std::getColorForLevel(LC_LogLevel level)
{
   static const LC_LogColor LC_COLOR_MAP [] = {
      LC_LOG_COL_RED,
      LC_LOG_COL_RED,
      LC_LOG_COL_YELLOW,
      LC_LOG_COL_GREEN,
      LC_LOG_COL_WHITE,
      LC_LOG_COL_BLUE
   };

   return LC_COLOR_MAP[level];
}

/*------------------------------------------------------------------------------
|    LC_Output2File::stream
+-----------------------------------------------------------------------------*/
inline FILE*& LC_Output2FILE::stream()
{
#ifdef _MSC_VER
   static FILE* pStream = NULL;
   if (!pStream)
      if (errno_t err = fopen_s(&pStream, "output.log", "a"))
         ::printf("Failed to open output.log: %d,", err);
#else
   static FILE* pStream = fopen("output.log", "a");
#endif // _MSC_VER
   return pStream;
}

/*------------------------------------------------------------------------------
|    LC_Output2File::output
+-----------------------------------------------------------------------------*/
inline void LC_Output2FILE::printf(LC_Log<LC_Output2Std>& logger, va_list args)
{
   // Prepend.
   std::string final = logger.m_string.str();
   logger.prependHeader(final);
   logger.prependLogTagIfNeeded(final);

   FILE* pStream = stream();
   if (!pStream)
      return;

   vfprintf(pStream, final.c_str(), args);
   fflush(pStream);
}
typedef LC_Log<LC_Output2FILE> LC_LogFile;

#ifdef ENABLE_MSVS_OUTPUT
inline void LC_Output2MSVS::printf(LC_Log<LC_Output2MSVS>& logger, va_list args)
{
   // Prepend.
   std::string final = logger.m_string.str();
   logger.prependHeader(final);
   logger.prependLogTagIfNeeded(final);
   final.append("\n");

   OutputDebugStringA(final.c_str());
}
#endif // ENABLE_MSVS_OUTPUT

#ifdef __ANDROID__
/*------------------------------------------------------------------------------
|    LC_OutputAndroid::printf
+-----------------------------------------------------------------------------*/
inline void LC_OutputAndroid::printf(LC_Log<LC_OutputAndroid>& logger, va_list args)
{
   static const android_LogPriority android_logPriority [] = {
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

#ifdef XCODE_COLORING_ENABLED
/*------------------------------------------------------------------------------
|    LC_Output2XCodeColors::printf
+-----------------------------------------------------------------------------*/
inline void LC_Output2XCodeColors::printf(LC_Log<LC_Output2XCodeColors>& logger, va_list args)
{
#define XC_COL_ESC_MAC @"\033["
#define XC_COL_ESC_IOS @"\xC2\xA0["

#define XC_COL_ESC XC_COL_ESC_MAC

#define XC_COL_RESET_FG XC_COL_ESC @"fg;" // Clear any foreground color
#define XC_COL_RESET_BG XC_COL_ESC @"bg;" // Clear any background color
#define XC_COL_RESET XC_COL_ESC @";"      // Clear any foreground or background color

   std::string final = logger.m_string.str();
   logger.prependHeader(final);
   logger.prependLogTagIfNeeded(final);

   const NSString* colorCode;
   if (logger.m_level == LC_LOG_NONE)
      colorCode = LC_XC_COL[(int) logger.m_color];
   else
      colorCode = getColorForLevel(logger.m_level);

   NSString* s;
   if (checkEnv())
      s = [NSString stringWithFormat : XC_COL_ESC @"%@%s" XC_COL_RESET @"\n",
      colorCode, final.c_str()];
   else
      s = [NSString stringWithFormat : @"%s", final.c_str()];

   ::printf("%s", [s cStringUsingEncoding : NSUTF8StringEncoding]);
}

/*------------------------------------------------------------------------------
|    LC_Output2XCodeColors::checkEnv
+-----------------------------------------------------------------------------*/
inline bool LC_Output2XCodeColors::checkEnv()
{
   static bool checkPerformed = false;
   static bool available = true;
   if (checkPerformed)
      return available;

#define XCODE_COLORS "XcodeColors"

   char* xcode_colors = getenv(XCODE_COLORS);
   if (!(xcode_colors && (strcmp(xcode_colors, "YES") == 0))) {
      NSLog(@"XCodeColors not available.");
      checkPerformed = true;
      available = false;
   }

   return available;
}

/*------------------------------------------------------------------------------
|    LC_OutputXCodeColors::getColorForLevel
+-----------------------------------------------------------------------------*/
inline NSString* LC_Output2XCodeColors::getColorForLevel(LC_LogLevel level)
{
   switch (level) {
   case LC_LOG_VERBOSE:
      return @"fg0,0,0;";   // Black here. XCode is commonly on light bg.
   case LC_LOG_DEBUG:
      return @"fg0,0,255;";
   case LC_LOG_INFO:
      return @"fg34,139,34;";
   case LC_LOG_NONE:
      return @"fg0,0,0;";
   case LC_LOG_WARN:
      return @"fg255,165,0;";
   case LC_LOG_ERROR:
      return @"fg255,0,0;";
   case LC_LOG_CRITICAL:
      return @"fg255,0,0;";
   default:
      return @"fg0,0,255;";
   }

   return nil;
}
#endif // XCODE_COLORING_ENABLED

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
/*------------------------------------------------------------------------------
|    current_time
+----------------------------------------------------------------------------*/
inline std::string lc_current_time()
{
   const int MAX_LEN = 200;
   wchar_t buffer[MAX_LEN];

   // Get locale name. GetTimeFormatEx should be supported since Windows 7.
   if (GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, 0, 0, L"HH':'mm':'ss", buffer, MAX_LEN) == 0)
      return "Error in NowTime()";

   // GetTickTime64() should be supported since Windows Vista.
   char result[100] = { 0 };
   static ULONGLONG first = GetTickCount64();
#ifndef _MSC_VER
   sprintf(result, "%s.%03ld", buffer, (long) (GetTickCount() - first) % 1000);
#else
   sprintf_s(result, sizeof(result), "%ls.%03lld", (const char*) buffer, (ULONGLONG) (GetTickCount64() - first) % 1000);
#endif
   return result;
}
#else
#include <sys/time.h>

/*------------------------------------------------------------------------------
|    lc_current_time
+-----------------------------------------------------------------------------*/
inline std::string lc_current_time()
{
   char buffer[11];
   time_t t;
   time(&t);

   tm r;
   memset(&r, 0, sizeof(tm));
   strftime(buffer, sizeof(buffer), "%X", localtime_r(&t, &r));

   struct timeval tv;
   gettimeofday(&tv, 0);

   char result[100] = { 0 };
   std::sprintf(result, "%s.%03ld", buffer, (long) tv.tv_usec / 1000);

   return result;
}

// Prevent from using outside.
#undef VA_LIST_CONTEXT
#undef LOG_UNUSED

#endif // WIN32
#endif // LC_LOGGING_H
