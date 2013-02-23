#ifndef OMXTUNNEL_H
#define OMXTUNNEL_H

#include "OMXComponent.h"

class OMXTunnel
{
public:
    OMXTunnel(OMXComponent* compSource, OMX_U32 portSource, OMXComponent* compDest, OMX_U32 portDest);
    ~OMXTunnel();
    void setupTunnel(
            unsigned int portStream,
            unsigned int timeout,
            bool waitForPortSettingsChanged = false
            );
    void flush(unsigned int timeout);
    void disable(unsigned int timeout);
    void enable(unsigned int timeout);

    OMXComponent* compSource;
    OMX_U32 portSource;
    OMXComponent* compDest;
    OMX_U32 portDest;
};

#endif // OMXTUNNEL_H
