// Written by Matt Ownby, August 2012
// You are free to use this for educational/non-commercial use

#include <Qt>
#include <QThread>

#include "OMXComponent.h"
#include "Event.h"
#include <stdexcept>
#include <sys/time.h>	// for gettimeofday
#include <unistd.h>	// for gettimeofday
#include <stdio.h>	// for sprintf
#include <assert.h>

#include "interface/vcos/vcos.h"

#include "lgl_logging.h"

#define TIMEOUT_MS 10000

using namespace std;

int OMXCompVideoDecoder::inputPort  = 130;
int OMXCompVideoDecoder::outputPort = 131;

int OMXCompVideoScheduler::inputClockPort = 12;
int OMXCompVideoScheduler::inputVideoPort = 10;
int OMXCompVideoScheduler::outputPort     = 11;

OMX_CALLBACKTYPE OMXComponent::callbacks = {
    OMXComponent::EventHandlerCallback,
    OMXComponent::EmptyBufferDoneCallback,
    OMXComponent::FillBufferDoneCallback
};

OMX_HANDLETYPE OMXComponent::GetHandle()
{
	return m_handle;
}

OMX_ERRORTYPE OMXComponent::GetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pPtr)
{
    OMX_ERRORTYPE err = OMX_GetParameter(m_handle, nParamIndex, pPtr);
    if (err != OMX_ErrorNone && err != OMX_ErrorNoMore) {
        LOG_VERBOSE(LOG_TAG, "OMX_GetParameter failed with code: %x.", err);
		throw runtime_error("OMX_GetParameter failed");
	}
    return err;
}

void OMXComponent::SetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pPtr)
{
	OMX_ERRORTYPE err = OMX_SetParameter(m_handle, nParamIndex, pPtr);

    if (err != OMX_ErrorNone) {
        LOG_VERBOSE(LOG_TAG, "OMX_SetParameter failed with code: %x.", err);
		throw runtime_error("OMX_SetParameter failed");
	}
}

void OMXComponent::sendCommand(OMX_COMMANDTYPE cmd, int nParam, OMX_PTR pCmdData)
{
    OMX_ERRORTYPE err = OMX_SendCommand(m_handle, cmd, nParam, pCmdData);
    if (err != OMX_ErrorNone) {
        LOG_ERROR(LOG_TAG, "OMX_SendCommand failed with code: %x.", err);
		throw runtime_error("OMX_SendCommand failed");
	}
}

void OMXComponent::getState(OMX_STATETYPE& state)
{
    OMX_ERRORTYPE err;
    if ((err = OMX_GetState(m_handle, &state)) != OMX_ErrorNone) {
        LOG_ERROR(LOG_TAG, "OMX_GetState failed: %x.", err);
        throw runtime_error("OMX_GetState failed.");
    }
}

void OMXComponent::SetupTunnel(OMX_U32 u32SrcPort, OMXComponent *pDstComponent, OMX_U32 u32DstPort
        )
{
	OMX_HANDLETYPE hDst = pDstComponent->GetHandle();
	OMX_ERRORTYPE err = OMX_SetupTunnel(m_handle, u32SrcPort, hDst, u32DstPort);

	if (err != OMX_ErrorNone)
	{
		throw runtime_error("OMX_SetupTunnel failed");
	}
}

void OMXComponent::RemoveTunnel(OMX_U32 u32Port)
{
	OMX_ERRORTYPE err = OMX_SetupTunnel(m_handle, u32Port, NULL, 0);
	if (err != OMX_ErrorNone)
	{
		throw runtime_error("RemoveTunnel failed");
	}
}

void OMXComponent::UseBuffer(
        OMX_BUFFERHEADERTYPE** ppHeader,
        OMX_U32 nPortIndex,
        OMX_PTR pAppPrivate,
        OMX_U32 nSizeBytes,
        OMX_U8 *pBuffer
        )
{
	OMX_ERRORTYPE err = OMX_UseBuffer(m_handle, ppHeader, nPortIndex, pAppPrivate, nSizeBytes, pBuffer);
    if (err != OMX_ErrorNone) {
        LOG_ERROR(LOG_TAG, "OMX_UseBuffer failed with code: %x.", err);
		throw runtime_error("OMX_UseBuffer failed");
	}

    // Add to the list if input port.
    if ((*ppHeader)->nInputPortIndex == nPortIndex) {
        LOG_VERBOSE(LOG_TAG, "Adding buffer to port.");
        mutexBuffers.lock();
        QList<OMX_BUFFERHEADERTYPE*>& portBuffers = buffers[nPortIndex];
        portBuffers.append(*ppHeader);
        condBufferAvailable.wakeAll();
        mutexBuffers.unlock();
    }
}

void OMXComponent::enablePortBuffers(OMX_U32 portIndex)
{
    OMX_PARAM_PORTDEFINITIONTYPE portdef;
    memset(&portdef, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    portdef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
    portdef.nVersion.nVersion = OMX_VERSION;
    portdef.nPortIndex = portIndex;

    // Work out buffer requirements, check port is in the right state.
    //error = OMX_GetParameter(comp->comp, OMX_IndexParamPortDefinition, &portdef);
    GetParameter(OMX_IndexParamPortDefinition, &portdef);
    if (portdef.bEnabled != OMX_FALSE || portdef.nBufferCountActual == 0 || portdef.nBufferSize == 0)
        throw runtime_error("Port is not in the right state.");

    portdef.nBufferSize = 1000000;
    SetParameter(OMX_IndexParamPortDefinition, &portdef);

    OMX_STATETYPE state;
    getState(state);
    if (!(state == OMX_StateIdle || state == OMX_StateExecuting || state == OMX_StatePause))
        throw runtime_error("Wrong component state.");

    // Send the command. The port won't be enabled until the buffers are
    // provided.
    sendCommand(OMX_CommandPortEnable, portIndex, NULL);

    LOG_VERBOSE(LOG_TAG, "Providing %lu buffers...", portdef.nBufferCountActual);
    for (OMX_U32 i = 0; i < portdef.nBufferCountActual; i++) {
        unsigned char* buf;
        //buf = (unsigned char*)vcos_malloc_aligned(portdef.nBufferSize, portdef.nBufferAlignment, NULL);
        if (posix_memalign((void**)&buf, portdef.nBufferAlignment, portdef.nBufferSize) != 0)
            throw runtime_error("posix_memalign failed");

        OMX_BUFFERHEADERTYPE* pBufHeader;
        UseBuffer(&pBufHeader, portIndex, NULL, portdef.nBufferSize, buf);
    }
    assert((OMX_U32)buffers[portIndex].size() == portdef.nBufferCountActual);

    // TODO: Error management!

    // Wait for the port to get enabled.
    // TODO: Check timeout.
    waitForEvent(OMX_EventCmdComplete, OMX_CommandPortEnable, portIndex, TIMEOUT_MS);
    LOG_VERBOSE(LOG_TAG, "Input port enabled.");
#if 0
    // TODO: Implement this.
    OMX_ERRORTYPE error;
    OMX_PARAM_PORTDEFINITIONTYPE portdef;
    OMX_BUFFERHEADERTYPE *list = NULL, **end = &list;
    OMX_STATETYPE state;
    int i;

    memset(&portdef, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    portdef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
    portdef.nVersion.nVersion = OMX_VERSION;
    portdef.nPortIndex = portIndex;

    // work out buffer requirements, check port is in the right state
    error = OMX_GetParameter(comp->comp, OMX_IndexParamPortDefinition, &portdef);
    if(error != OMX_ErrorNone || portdef.bEnabled != OMX_FALSE || portdef.nBufferCountActual == 0 || portdef.nBufferSize == 0)
       return -1;

    // check component is in the right state to accept buffers
    error = OMX_GetState(comp->comp, &state);
    if (error != OMX_ErrorNone || !(state == OMX_StateIdle || state == OMX_StateExecuting || state == OMX_StatePause))
       return -1;

    // send the command
    error = OMX_SendCommand(comp->comp, OMX_CommandPortEnable, portIndex, NULL);
    vc_assert(error == OMX_ErrorNone);

    for (i=0; i != portdef.nBufferCountActual; i++)
    {
       unsigned char *buf;
       if(ilclient_malloc)
          buf = ilclient_malloc(private, portdef.nBufferSize, portdef.nBufferAlignment, comp->bufname);
       else
          buf = vcos_malloc_aligned(portdef.nBufferSize, portdef.nBufferAlignment, comp->bufname);

       if(!buf)
          break;

       error = OMX_UseBuffer(comp->comp, end, portIndex, NULL, portdef.nBufferSize, buf);
       if(error != OMX_ErrorNone)
       {
          if(ilclient_free)
             ilclient_free(private, buf);
          else
             vcos_free(buf);

          break;
       }
       end = (OMX_BUFFERHEADERTYPE **) &((*end)->pAppPrivate);
    }

    // queue these buffers
    vcos_semaphore_wait(&comp->sema);

    if(portdef.eDir == OMX_DirInput)
    {
       *end = comp->in_list;
       comp->in_list = list;
    }
    else
    {
       *end = comp->out_list;
       comp->out_list = list;
    }

    vcos_semaphore_post(&comp->sema);

    if(i != portdef.nBufferCountActual ||
       ilclient_wait_for_command_complete(comp, OMX_CommandPortEnable, portIndex) < 0)
    {
       ilclient_disable_port_buffers(comp, portIndex, NULL, ilclient_free, private);

       // at this point the first command might have terminated with an error, which means that
       // the port is disabled before the disable_port_buffers function is called, so we're left
       // with the error bit set and an error event in the queue.  Clear these now if they exist.
       ilclient_remove_event(comp, OMX_EventError, 0, 1, 1, 0);

       return -1;
    }

    // success
    return 0;
#endif
}

void OMXComponent::disablePortBuffers(OMX_U32 portIndex)
{
    OMX_PARAM_PORTDEFINITIONTYPE portdef;
    memset(&portdef, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    portdef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
    portdef.nVersion.nVersion = OMX_VERSION;
    portdef.nPortIndex = portIndex;

    // Work out buffer requirements, check port is in the right state.
    //error = OMX_GetParameter(comp->comp, OMX_IndexParamPortDefinition, &portdef);
    GetParameter(OMX_IndexParamPortDefinition, &portdef);
    if (portdef.bEnabled != OMX_TRUE || portdef.nBufferCountActual == 0 || portdef.nBufferSize == 0)
        throw runtime_error("Port is not in the right state.");

    OMX_U32 bufferCount = portdef.nBufferCountActual;
    const QList<OMX_BUFFERHEADERTYPE*>& bufferList = buffers.value(portIndex);
    LOG_VERBOSE(LOG_TAG, "Buffer list size: %d.", buffers.size());

    // Disable the port.
    sendCommand(OMX_CommandPortDisable, portIndex, NULL);
    for (OMX_U32 i = 0; i < bufferCount; i++) {
        OMX_BUFFERHEADERTYPE* buf = bufferList[i];
        if (OMX_FreeBuffer(GetHandle(), portIndex, buf) != OMX_ErrorNone)
            throw runtime_error("Failed to free buffer.");
    }

    // Remove the buffer list.
    assert(buffers.remove(portIndex) >= 1);
    // TODO: Handle timeout here...
    LOG_VERBOSE(LOG_TAG, "Waiting for port to be disabled...");
    waitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, portIndex, TIMEOUT_MS);
}

void OMXComponent::EmptyThisBuffer(OMX_BUFFERHEADERTYPE* pHeader)
{
    assert(m_handle);
    assert(pHeader);

    OMX_ERRORTYPE type;
    if ((type = OMX_EmptyThisBuffer(m_handle, pHeader)) != OMX_ErrorNone) {
        LOG_WARNING(LOG_TAG, "OMX_Empty this buffer failed with code: %x.", type);
		throw runtime_error("OMX_EmptyThisBuffer failed");
    }
}

void OMXComponent::FillThisBuffer(OMX_BUFFERHEADERTYPE *pHeader)
{
    OMX_ERRORTYPE omxError;
    if ((omxError = OMX_FillThisBuffer(m_handle, pHeader)) != OMX_ErrorNone) {
        LOG_VERBOSE(LOG_TAG, "OMX_FillThisBuffer failed with code: %x.", omxError);
        throw runtime_error("OMX_FillThisBuffer failed.");
    }
}

void OMXComponent::FreeBuffer(OMX_U32 nPortIdx, OMX_BUFFERHEADERTYPE *pBuffer)
{
	OMX_ERRORTYPE err = OMX_FreeBuffer(m_handle, nPortIdx, pBuffer);

	if (err != OMX_ErrorNone)
	{
		throw runtime_error("OMX_FreeBuffer failed");
	}
}

static void add_timespecs(struct timespec &time, long millisecs)
{
	time.tv_sec  += millisecs / 1000;
	time.tv_nsec += (millisecs % 1000) * 1000000;
	if (time.tv_nsec > 1000000000)
	{
		time.tv_sec  += 1;
		time.tv_nsec -= 1000000000;
	}
}

bool OMXComponent::IsEventPending(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2)
{
	bool bRes = false;

	Lock();

	for (list<OMXEventData>::iterator li = m_lstEvents.begin(); li != m_lstEvents.end(); li++)
	{
		// if we match the event type
		if (li->eEvent == eEvent)
		{
			// if we also match the data types
			if ((nData1 == li->nData1) && (nData2 == li->nData2))
			{
				bRes = true;
				break;
			}
		}
	}

	Unlock();

	return bRes;
}

bool OMXComponent::waitForInputBufferReady(OMX_U32 port, OMX_BUFFERHEADERTYPE*& buffer)
{
    // Check if any input buffer is available for that port.
    mutexBuffers.lock();
    if (!buffers.contains(port)) {
        LOG_ERROR(LOG_TAG, "No buffer for that port is available.");
        mutexBuffers.unlock();
        return false;
    }
    QList<OMX_BUFFERHEADERTYPE*>& portBuffers = buffers[port];
    if (portBuffers.size() == 0) {
        LOG_VERBOSE(LOG_TAG, "Waiting for an available buffer...");
        condBufferAvailable.wait(&mutexBuffers);
    }

    // If we're here it means a buffer is available.
    buffer = portBuffers.first();
    portBuffers.removeFirst();
    mutexBuffers.unlock();
    return true;
}

IEventSPtr OMXComponent::waitForEvent(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, int timeoutMs)
{
	list<IEventSPtr> lstEvents;

	OMXEventData evOMX;
	evOMX.eEvent = eEvent;
	evOMX.nData1 = nData1;
	evOMX.nData2 = nData2;
	evOMX.pEventData = NULL;

	lstEvents.push_back(IEventSPtr(new OMXEvent(evOMX)));

    return WaitForGeneric(lstEvents, timeoutMs);
}

IEventSPtr OMXComponent::WaitForEmpty(const OMX_BUFFERHEADERTYPE* pEmptyBuffer, unsigned int uTimeoutMs)
{
	list<IEventSPtr> lstEvents;

	EmptyBufferDoneData evEmpty;
	evEmpty.pBuffer = pEmptyBuffer;
	lstEvents.push_back(IEventSPtr(new EmptyBufferDoneEvent(evEmpty)));

	return WaitForGeneric(lstEvents, uTimeoutMs);
}

IEventSPtr OMXComponent::WaitForFill(const OMX_BUFFERHEADERTYPE* pFillBuffer, unsigned int uTimeoutMs)
{
	list<IEventSPtr> lstEvents;

	FillBufferDoneData evFill;
	evFill.pBuffer = pFillBuffer;
	lstEvents.push_back(IEventSPtr(new FillBufferDoneEvent(evFill)));

	return WaitForGeneric(lstEvents, uTimeoutMs);
}

IEventSPtr OMXComponent::WaitForAnything(unsigned int uTimeoutMs)
{
	struct timespec tsEnd;
	clock_gettime(CLOCK_REALTIME, &tsEnd);
	add_timespecs(tsEnd, uTimeoutMs);

	IEventSPtr pRes;
	bool bFound = false;
	bool bFailure = false;

	while (!bFound)
	{
		Lock();

		// if we have an event
		if (!m_lstEvents.empty())
		{
			IEvent *pEvent = new OMXEvent(m_lstEvents.front());
			pRes = IEventSPtr(pEvent);
			m_lstEvents.pop_front();
			bFound = true;
		}

		// if we have an empty buffer done
		else if (!m_lstEmpty.empty())
		{
			pRes = IEventSPtr(new EmptyBufferDoneEvent(m_lstEmpty.front()));
			m_lstEmpty.pop_front();
			bFound = true;
		}
		// else if we have a fill buffer done
		else if (!m_lstFill.empty())
		{
			pRes = IEventSPtr(new FillBufferDoneEvent(m_lstFill.front()));
			m_lstFill.pop_front();
			bFound = true;
		}

		// if we didn't get a match we need to wait for success or a timeout
		if (!bFound)
		{
			// If we got an error or timed out, then we're done
			// (this implicitly unlocks the mutex during the waiting period, then relocks it upon returning!)
			if (!m_pLocker->WaitForEvent(&tsEnd))
			{
				bFailure = true;
			}
		}

		Unlock();

		if (bFailure) throw runtime_error("Waiting timed out");

	}	// end for

	return pRes;
}

IEventSPtr OMXComponent::WaitForEventOrEmpty(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, const OMX_BUFFERHEADERTYPE* pEmptyBuffer, unsigned int uTimeoutMs)
{
	IEventSPtr pRes;

	list<IEventSPtr> lstEvents;

	EmptyBufferDoneData evEmpty;
	evEmpty.pBuffer = pEmptyBuffer;

	lstEvents.push_back(IEventSPtr(new EmptyBufferDoneEvent(evEmpty)));

	OMXEventData evOMX;
	evOMX.eEvent = eEvent;
	evOMX.nData1 = nData1;
	evOMX.nData2 = nData2;
	evOMX.pEventData = NULL;

	lstEvents.push_back(IEventSPtr(new OMXEvent(evOMX)));

	pRes = WaitForGeneric(lstEvents, uTimeoutMs);

	return pRes;
}

size_t OMXComponent::GetPendingEventCount()
{
	return m_lstEvents.size();
}

size_t OMXComponent::GetPendingEmptyCount()
{
	return m_lstEmpty.size();
}

size_t OMXComponent::GetPendingFillCount()
{
	return m_lstFill.size();
}

/**
 * @brief OMXComponent::WaitForGeneric Locks waiting for any of the provided events to happen.
 * @param lstEvents The events to look for.
 * @param timeoutMs If < 0 means no timeout. The method returns immediately. If the none of the
 * events are found NULL is returned.
 * @return
 */
IEventSPtr OMXComponent::WaitForGeneric(const list<IEventSPtr>& lstEvents, int timeoutMs)
{
	IEventSPtr pRes;

	struct timespec tsEnd;
	clock_gettime(CLOCK_REALTIME, &tsEnd);
    add_timespecs(tsEnd, timeoutMs);

	bool bMatch = false;
	bool bFailure = false;

    // Go until we either fail or succeed
    while ((!bFailure) && (!bMatch)) {
        // Go through all things we _can_ match with...
        for (list<IEventSPtr>::const_iterator lsi = lstEvents.begin(); lsi != lstEvents.end(); lsi++) {
            // Determine what type of event this is.
            IEvent* pEvent = lsi->get();
            OMXEventData* pOMX          = pEvent->ToEvent();
            EmptyBufferDoneData* pEmpty = pEvent->ToEmpty();
            FillBufferDoneData* pFill   = pEvent->ToFill();

			Lock();

            if (pEmpty != NULL) {
                for (list<EmptyBufferDoneData>::iterator li = m_lstEmpty.begin(); li != m_lstEmpty.end(); li++) {
                    if (li->pBuffer == pEmpty->pBuffer) {
						bMatch = true;
						m_lstEmpty.erase(li);
						pRes = *lsi;
						break;
					}
				}
            }
            else if (pOMX != NULL) {
                for (list<OMXEventData>::iterator li = m_lstEvents.begin(); li != m_lstEvents.end(); li++) {
					// if we found the match
                    if ((li->eEvent == pOMX->eEvent) && (pOMX->nData1 == li->nData1) && (pOMX->nData2 == li->nData2)) {
						bMatch = true;
						m_lstEvents.erase(li);	// remove this item from the list because the caller knows about it now
						pRes = *lsi;
						break;	// must break here to about segfault since we just called erase()
					}
				}
			}
            else if (pFill != NULL) {
                for (list<FillBufferDoneData>::iterator li = m_lstFill.begin(); li != m_lstFill.end(); li++) {
                    if (li->pBuffer == pFill->pBuffer) {
						bMatch = true;
						m_lstFill.erase(li);
						pRes = *lsi;
						break;
					}
				}
			}
            // else this should never happen.
            else {
				assert(false);
			}

			Unlock();

			// if we found a match, we're done
            if (bMatch) {
				break;
			}
		}	// end for

		// if we didn't get a match we need to wait for success or a timeout
        // lcarlon: if uTimeoutMs is negative, then do not wait at all and return
        // false.
        if (!bMatch && timeoutMs >= 0) {
			Lock();

			// If we got an error or timed out, then we're done
			// (this implicitly unlocks the mutex during the waiting period, then relocks it upon returning!)
            if (!m_pLocker->WaitForEvent(&tsEnd)) {
				bFailure = true;
			}

			Unlock();
		}
        else {
            LOG_VERBOSE(LOG_TAG, "Finished scanning event list.");
            return pRes;
        }
    }

    if (bFailure) {
		throw runtime_error("Waiting timed out");
	}

	return pRes;
}

////////////////////////////////////////////////////////////////////////////////////////////

OMXComponentShared OMXComponent::GetInstance()
{
    return OMXComponentShared(new OMXComponent(), OMXComponent::deleter());
}

OMXComponent::OMXComponent()
{
	m_locker = Locker::GetInstance();
	m_pLocker = m_locker.get();
}

OMXComponent::~OMXComponent()
{
    LOG_VERBOSE(LOG_TAG, "Destroying component.");
    assert(m_handle);
    if (OMX_FreeHandle(m_handle) != OMX_ErrorNone) {
        LOG_ERROR(LOG_TAG, "Failed to free handle %x.", (unsigned int)m_handle);
    }
}

void OMXComponent::SetHandle(OMX_HANDLETYPE hComponent)
{
	m_handle = hComponent;
}

void OMXComponent::Lock()
{
	m_pLocker->Lock();
}

void OMXComponent::Unlock()
{
	m_pLocker->Unlock();
}

OMX_ERRORTYPE OMXComponent::EventHandlerCallback(OMX_HANDLETYPE hComponent, OMX_PTR pAppData,
												 OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, OMX_PTR pEventData)
{
    Q_UNUSED(hComponent);

	OMXComponent *pInstance = (OMXComponent *) pAppData;
	return pInstance->eventHandler(eEvent, nData1, nData2, pEventData);
}

OMX_ERRORTYPE OMXComponent::EmptyBufferDoneCallback(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
    Q_UNUSED(hComponent);

	OMXComponent *pInstance = (OMXComponent *) pAppData;
	return pInstance->emptyBufferDone(pBuffer);
}

OMX_ERRORTYPE OMXComponent::FillBufferDoneCallback(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBufferHeader)
{
    Q_UNUSED(hComponent);

	OMXComponent *pInstance = (OMXComponent *) pAppData;
	return pInstance->fillBufferDone(pBufferHeader);
}

OMX_ERRORTYPE OMXComponent::eventHandler(OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, OMX_PTR pEventData)
{
#ifdef VERBOSE
	const char *cpszType = NULL;
	switch (eEvent)
	{
	case OMX_EventCmdComplete:
		cpszType = "OMX_EventCmdComplete";
		break;
	case OMX_EventError:
		cpszType = "OMX_EventError";
		break;
	case OMX_EventMark:
		cpszType = "OMX_EventMark";
		break;
	case OMX_EventPortSettingsChanged:
		cpszType = "OMX_EventPortSettingsChanged";
		break;
	case OMX_EventBufferFlag:
		cpszType = "OMX_EventBufferFlag";
		break;
	default:
		cpszType = "OMX_Event?? (add to switch statement)";
		break;
	}
	char s[160];
    snprintf(s, sizeof(s), "Handle %x got event: %s (%u) ndata1: %u (%x) ndata2: %u (%x) pEventData %x", (unsigned int) m_handle, cpszType, (unsigned int) eEvent, (unsigned int) nData1, (unsigned int) nData1, (unsigned int) nData2, (unsigned int) nData2, (unsigned int) pEventData);
#endif // VERBOSE

	OMXEventData e;
	e.eEvent = eEvent;
	e.nData1 = nData1;
	e.nData2 = nData2;
	e.pEventData = pEventData;

	Lock();
#ifdef VERBOSE
    LOG_VERBOSE(LOG_TAG, "%s", s);
#endif // VERBOSE
	m_lstEvents.push_back(e);
	m_pLocker->GenerateEvent();
    Unlock();
	return OMX_ErrorNone;
}

OMX_ERRORTYPE OMXComponent::emptyBufferDone(OMX_BUFFERHEADERTYPE* pBuffer)
{
    // Add to the list of available buffers.
    LOG_VERBOSE(LOG_TAG, "EmptyBufferDone entering mutex...");
    mutexBuffers.lock();
    QList<OMX_BUFFERHEADERTYPE*>& buffer = buffers[pBuffer->nInputPortIndex];
    buffer.append(pBuffer);
    condBufferAvailable.wakeAll();
    mutexBuffers.unlock();

	EmptyBufferDoneData dat;
	dat.pBuffer = pBuffer;

	Lock();
#ifdef VERBOSE
    LOG_VERBOSE(LOG_TAG, "Got EmptyBufferDone");
#endif // VERBOSE

	m_lstEmpty.push_back(dat);
	m_pLocker->GenerateEvent();
	Unlock();

    LOG_VERBOSE(LOG_TAG, "EmptyBufferDone exited mutex...");

	return OMX_ErrorNone;
}

OMX_ERRORTYPE OMXComponent::fillBufferDone(OMX_BUFFERHEADERTYPE* pBuffer)
{
	FillBufferDoneData dat;
	dat.pBuffer = pBuffer;

	Lock();
#ifdef VERBOSE
    LOG_VERBOSE(LOG_TAG, "Got FillBufferDone");
#endif // VERBOSE

	m_lstFill.push_back(dat);
	m_pLocker->GenerateEvent();
	Unlock();

	return OMX_ErrorNone;
}
