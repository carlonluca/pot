/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    11.01.2012
 *
 * Copyright (c) 2012, 2013 Luca Carlon. All rights reserved.
 *
 * This file is part of PiOmxTextures.
 *
 * PiOmxTextures is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PiOmxTextures is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PiOmxTextures.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LGL_LOGGING_H
#define LGL_LOGGING_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#if defined(ANDROID) && defined(ENABLE_LOGCAT)
#include <android/log.h>
#else
#include <stdio.h>
#endif
#include <assert.h>

/*------------------------------------------------------------------------------
|    definitions
+-----------------------------------------------------------------------------*/
// Colors.
#define RESET     0
#define BRIGHT    1
#define DIM       2
#define UNDERLINE 3
#define BLINK     4
#define REVERSE   7
#define HIDDEN    8

#define BLACK 		0
#define RED			1
#define GREEN		2
#define YELLOW		3
#define BLUE		4
#define MAGENTA     5
#define CYAN		6
#define WHITE		7

#ifdef ANDROID
#ifndef LOG_TAG
#define LOG_TAG "DefaultTag"
#endif
#define LOG_ASSERT(cond, text) assert(cond)
#else
#define LOG_ASSERT(cond, text) assert(cond)
#endif

#define COLOR_CHANGE(a, f) fprintf(stderr, "%c[%d;%dm", 0x1B, a, f + 30)
#define COLORED_OUTPUT(a, f, ...) {                \
    COLOR_CHANGE(a, f);                             \
    fprintf(stderr, __VA_ARGS__);                   \
    fprintf(stderr, "\n");                          \
    COLOR_CHANGE(RESET, WHITE);                     \
    }
#define PRINTFN(...) { \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n"); \
    }

// Common definitions.
#if defined(ANDROID) && defined(ENABLE_LOGCAT)
#define LOG_ERROR(tag, ...) \
   __android_log_print(ANDROID_LOG_ERROR, tag, __VA_ARGS__)
#define LOG_WARNING(tag, ...) \
   __android_log_print(ANDROID_LOG_WARN, tag, __VA_ARGS__)
#define LOG_INFORMATION(tag, ...) \
   __android_log_print(ANDROID_LOG_INFO, tag, __VA_ARGS__)
#define LOG_DEBUG(tag, ...) \
   __android_log_print(ANDROID_LOG_DEBUG, tag, __VA_ARGS__)
#define LOG_VERBOSE(tag, ...) \
   __android_log_print(ANDROID_LOG_VERBOSE, tag, __VA_ARGS__)
#elif defined(__APPLE__)
#define LOG_ERROR(tag, ...)      \
   fprintf(stderr, __VA_ARGS__); \
   fprintf(stderr, "\n");        \
   fflush(stderr)
#define LOG_WARNING(tag, ...)    \
   fprintf(stderr, __VA_ARGS__); \
   fprintf(stderr, "\n");        \
   fflush(stderr)
#define LOG_INFORMATION(tag, ...) \
   fprintf(stdout, __VA_ARGS__);  \
   fprintf(stdout, "\n");         \
   fflush(stdout)
#define LOG_DEBUG(tag, ...) \
   fprintf(stderr, __VA_ARGS__); \
   fflush(stderr)
#define LOG_VERBOSE(tag, ...) \
   fprintf(stderr, __VA_ARGS__); \
   fflush(stderr)
#else
#define LOG_ERROR(tag, ...) \
   COLORED_OUTPUT(RESET, RED, __VA_ARGS__); \
   fflush(stderr)
#define LOG_WARNING(tag, ...) \
   COLORED_OUTPUT(RESET, YELLOW, __VA_ARGS__); \
   fflush(stderr)
#define LOG_INFORMATION(tag, ...) \
   COLORED_OUTPUT(RESET, GREEN, __VA_ARGS__); \
   fflush(stderr)
#define LOG_DEBUG(tag, ...) \
   COLORED_OUTPUT(RESET, BLUE, __VA_ARGS__); \
   fflush(stderr)
#define LOG_VERBOSE(tag, ...) \
   COLORED_OUTPUT(RESET, WHITE, __VA_ARGS__); \
   fflush(stderr)
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

#ifdef LOG_LEVEL_DEBUG
#undef LOG_VERBOSE
#define LOG_VERBOSE(...) do {} while(0)
#elif LOG_LEVEL_INFORMATION
#undef LOG_VERBOSE
#undef LOG_DEBUG
#define LOG_VERBOSE(...)  do {} while(0)
#define LOG_DEBUG(...)  do {} while(0)
#elif LOG_LEVEL_WARNING
#undef LOG_VERBOSE
#undef LOG_DEBUG
#undef LOG_INFORMATION
#define LOG_VERBOSE(...)  do {} while(0)
#define LOG_DEBUG(...)  do {} while(0)
#define LOG_INFORMATION(...)  do {} while(0)
#elif LOG_LEVEL_ERROR
#undef LOG_VERBOSE
#undef LOG_DEBUG
#undef LOG_INFORMATION
#undef LOG_WARNING
#define LOG_VERBOSE(...)  do {} while(0)
#define LOG_DEBUG(...)  do {} while(0)
#define LOG_INFORMATION(...)  do {} while(0)
#define LOG_WARNING(...)  do {} while(0)
#endif

#endif // LGL_LOGGING_H
