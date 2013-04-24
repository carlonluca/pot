// Written by Matt Ownby, August 2012
// You are free to use this for educational/non-commercial use

#ifndef OMXCORE_H
#define OMXCORE_H

#include <QMutex>

#include "OMXComponent.h"
#include "MyDeleter.h"

using namespace std;

class OMX_Core;
typedef std::tr1::shared_ptr<OMX_Core> OMXCoreSPtr;

class OMX_Core : public MyDeleter
{
public:
    static OMX_Core* instance();
    virtual ~OMX_Core();
    static void destroyInstance();

private:
    OMX_Core();

    void DeleteInstance() {delete this;}

    bool init();
    void shutdown();

    static OMX_Core* m_instance;
    static QMutex m_mutex;

    static int refs;

    friend class OMX_Component; /* Not inherited */
};

#endif // OMXCORE_H
