// Written by Matt Ownby, August 2012
// You are free to use this for educational/non-commercial use

#ifndef OMXCOMPONENT_H
#define OMXCOMPONENT_H

//#include "IL/OMX_Broadcom.h"
#include "IL/OMX_Component.h"
#include "MyDeleter.h"
#include "IEvent.h"
#include "Locker.h"
#include <list>
#include <stdexcept>

#include <QObject>
#include <QHash>
#include <QMutex>
#include <QWaitCondition>

// uncomment this to get verbose logging of events
//#define VERBOSE

using namespace std;

#if 0
class IOMXComponent;

class IOMXComponent
{
public:
    virtual OMX_HANDLETYPE GetHandle() = 0;
    virtual void GetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pPtr) = 0;
    virtual void SetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pPtr) = 0;
    virtual void sendCommand(OMX_COMMANDTYPE cmd, int nParam, void *pCmdData) = 0;
    virtual void getState(OMX_STATETYPE& state) = 0;
    virtual void SetupTunnel(OMX_U32 u32SrcPort, IOMXComponent *pDstComponent, OMX_U32 u32DstPort) = 0;
    virtual void RemoveTunnel(OMX_U32 u32Port) = 0;
    virtual void enablePortBuffers(OMX_U32 portIndex);
    virtual void UseBuffer(OMX_BUFFERHEADERTYPE **ppHeader, OMX_U32 nPortIndex, OMX_PTR pAppPrivate, OMX_U32 nSizeBytes, OMX_U8 *pBuffer) = 0;
    virtual void EmptyThisBuffer(OMX_BUFFERHEADERTYPE *pHeader) = 0;
    virtual void FillThisBuffer(OMX_BUFFERHEADERTYPE *pHeader) = 0;
    virtual void FreeBuffer(OMX_U32 nPortIdx, OMX_BUFFERHEADERTYPE *pBuffer) = 0;
    virtual bool IsEventPending(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2) = 0;
    virtual bool waitForInputBufferReady(OMX_U32 port, OMX_BUFFERHEADERTYPE*& buffer);
    virtual IEventSPtr waitForEvent(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, unsigned int uTimeoutMs) = 0;
    virtual IEventSPtr WaitForEmpty(const OMX_BUFFERHEADERTYPE* pBuf, unsigned int uTimeoutMs) = 0;
    virtual IEventSPtr WaitForFill(const OMX_BUFFERHEADERTYPE* pBuf, unsigned int uTimeoutMs) = 0;
    virtual IEventSPtr WaitForAnything(unsigned int uTimeouts) = 0;
    virtual IEventSPtr WaitForEventOrEmpty(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, const OMX_BUFFERHEADERTYPE* pEmptyBuffer, unsigned int uTimeoutMs) = 0;
    virtual size_t GetPendingEventCount() = 0;
    virtual size_t GetPendingEmptyCount() = 0;
    virtual size_t GetPendingFillCount() = 0;
};

typedef shared_ptr<IOMXComponent> IOMXComponentSPtr;
#endif

class OMXComponent;
typedef std::tr1::shared_ptr<OMXComponent> OMXComponentShared;

class OMXComponent : public QObject, public MyDeleter
{
    friend class OMXCore;	// only let the core allocate instances to ensure that everything is properly shut down
    Q_OBJECT
public:
    OMX_HANDLETYPE GetHandle();
    OMX_ERRORTYPE GetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pPtr);
    void SetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pPtr);
    void sendCommand(OMX_COMMANDTYPE cmd, int nParam, OMX_PTR pCmdData);
    void getState(OMX_STATETYPE& state);
    void SetupTunnel(OMX_U32 u32SrcPort, OMXComponent *pDstComponent, OMX_U32 u32DstPort);
    void RemoveTunnel(OMX_U32 u32Port);
    void enablePortBuffers(OMX_U32 portIndex);
    void disablePortBuffers(OMX_U32 portIndex);
    void UseBuffer(
            OMX_BUFFERHEADERTYPE** ppHeader,
            OMX_U32 nPortIndex,
            OMX_PTR pAppPrivate,
            OMX_U32 nSizeBytes,
            OMX_U8 *pBuffer
            );
    void EmptyThisBuffer(OMX_BUFFERHEADERTYPE *pHeader);
    void FillThisBuffer(OMX_BUFFERHEADERTYPE *pHeader);
    void FreeBuffer(OMX_U32 nPortIdx, OMX_BUFFERHEADERTYPE *pBuffer);
    bool IsEventPending(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2);
    bool waitForInputBufferReady(OMX_U32 port, OMX_BUFFERHEADERTYPE*& buffer);
    IEventSPtr waitForEvent(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, int timeoutMs);
    IEventSPtr WaitForEmpty(const OMX_BUFFERHEADERTYPE* pBuf, unsigned int uTimeoutMs);
    IEventSPtr WaitForFill(const OMX_BUFFERHEADERTYPE* pBuf, unsigned int uTimeoutMs);
    IEventSPtr WaitForAnything(unsigned int uTimeouts);
    IEventSPtr WaitForEventOrEmpty(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, const OMX_BUFFERHEADERTYPE* pEmptyBuffer, unsigned int uTimeoutMs);
    size_t GetPendingEventCount();
    size_t GetPendingEmptyCount();
    size_t GetPendingFillCount();

protected:
    OMXComponent();
    virtual ~OMXComponent();

private:
    IEventSPtr WaitForGeneric(const list<IEventSPtr> &lstEvents, int timeoutMs);

    static OMXComponentShared GetInstance();

    void DeleteInstance() { delete this; }

    // this must be deferred because OMXCore doesn't know what its value is until after we are instantiated
    void SetHandle(OMX_HANDLETYPE hComponent);

    void Lock();

    void Unlock();

    static OMX_ERRORTYPE EventHandlerCallback(OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
                                              OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, OMX_PTR pEventData);
    static OMX_ERRORTYPE EmptyBufferDoneCallback(
            OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer);
    static OMX_ERRORTYPE FillBufferDoneCallback(
            OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBufferHeader);
    static OMX_CALLBACKTYPE callbacks;

    // TODO: Virtual calls are worst than non-virtual, but might be ignored for the moment.
    virtual OMX_ERRORTYPE eventHandler(
            OMX_EVENTTYPE eEvent,
            OMX_U32 nData1,
            OMX_U32 nData2,
            OMX_PTR pEventData
            );
    virtual OMX_ERRORTYPE emptyBufferDone(OMX_BUFFERHEADERTYPE* pBuffer);
    virtual OMX_ERRORTYPE fillBufferDone(OMX_BUFFERHEADERTYPE* pBuffer);

    OMX_HANDLETYPE m_handle;
    ILockerSPtr m_locker;
    ILocker *m_pLocker;

    list<OMXEventData> m_lstEvents;
    list<EmptyBufferDoneData> m_lstEmpty;
    list<FillBufferDoneData> m_lstFill;

    // Available buffers for each port.
    QMutex mutexBuffers;
    QWaitCondition condBufferAvailable;
    QHash<OMX_U32, QList<OMX_BUFFERHEADERTYPE*> > buffers;

    template<class T> friend class OMXComponentFactory;
};

template<class T>
class OMXComponentFactory
{
public:
    static std::tr1::shared_ptr<T> getInstance(const char* componentName) {
        OMX_HANDLETYPE hComponent = 0;

        std::tr1::shared_ptr<T> component(new T(), OMXComponent::deleter());
        OMXComponent* pComponent = (OMXComponent*)component.get();

        OMX_ERRORTYPE err = OMX_GetHandle(
                    &hComponent,
                    (char*)componentName,
                    pComponent,
                    &OMXComponent::callbacks
                    );
        if (err != OMX_ErrorNone)
            throw runtime_error("OMX_GetHandle failed.");

        // Set the handle.
        pComponent->SetHandle(hComponent);
        return component;
    }
};

class OMXCompVideoDecoder;
typedef std::tr1::shared_ptr<OMXCompVideoDecoder> OMXCompVideoDecoderSPtr;
class OMXCompVideoDecoder : public OMXComponent
{
    Q_OBJECT
public:
    static OMXCompVideoDecoderSPtr GetInstance() {
        return OMXCompVideoDecoderSPtr(new OMXCompVideoDecoder(), OMXComponent::deleter());
    }

    static int inputPort;
    static int outputPort;

private:
    OMXCompVideoDecoder() :
        OMXComponent() {
        // Do nothing.
    }
};

class OMXCompVideoScheduler : public OMXComponent
{
    Q_OBJECT
public:
    static int inputVideoPort;
    static int inputClockPort;
    static int outputPort;
};

#endif // OMXCOMPONENT_H
