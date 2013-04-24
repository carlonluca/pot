// Written by Matt Ownby, August 2012
// You are free to use this for educational/non-commercial use

#include "OMX_Core.h"

#include <stdexcept>
#include <bcm_host.h>

#include "lgl_logging.h"

using namespace std;

#define ACQUIRE_MUTEX              \
    QMutexLocker locker(&m_mutex); \
    Q_UNUSED(locker)

OMX_Core* OMX_Core::m_instance = NULL;
QMutex OMX_Core::m_mutex;
int OMX_Core::refs = 0;

OMX_Core* OMX_Core::instance()
{
    ACQUIRE_MUTEX;
    try {
        if (!m_instance) {
            LOG_INFORMATION(LOG_TAG, "OMX Core instantiated.");
            m_instance = new OMX_Core;
        }
    }
    catch (const runtime_error& e) {
        Q_UNUSED(e);
        return NULL;
    }

    refs++;
    return m_instance;
}

void OMX_Core::destroyInstance()
{
    ACQUIRE_MUTEX;
    refs--;
    if (!refs) {
        LOG_INFORMATION(LOG_TAG, "OMX Core is bring released.");
        delete m_instance;
        m_instance = NULL;
    }
}

OMX_Core::OMX_Core()
{
    if (!init())
        throw runtime_error("Failed to init OpenMAX core.");
}

OMX_Core::~OMX_Core()
{
    shutdown();
}

inline
bool OMX_Core::init()
{
    LOG_VERBOSE(LOG_TAG, "Initializing OMXCore...");

    bcm_host_init();
    if (OMX_Init() != OMX_ErrorNone) {
        bcm_host_deinit();
        return false;
    }
    return true;
}

inline
void OMX_Core::shutdown()
{
    assert(false);
    LOG_VERBOSE(LOG_TAG, "Shutting down OMXCore...");

    if (OMX_Deinit() != OMX_ErrorNone)
        throw runtime_error("OMX_Deinit failed");
    bcm_host_deinit();
}
