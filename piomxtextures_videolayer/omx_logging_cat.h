#ifndef OMX_LOGGING_CAT_H
#define OMX_LOGGING_CAT_H

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(vl)

#define log_debug_func qCDebug(vl) << Q_FUNC_INFO
#define log_debug(...) qCDebug(vl, __VA_ARGS__)
#define log_info(...) qCInfo(vl, __VA_ARGS__)
#define log_verbose(...) qCDebug(vl, __VA_ARGS__)
#define log_warn(...) qCWarning(vl, __VA_ARGS__)
#define log_err(...) qCCritical(vl, __VA_ARGS__)

#endif // OMX_LOGGING_CAT_H
