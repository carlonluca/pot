#ifndef EVENT_H
#define EVENT_H

#include "IEvent.h"

class OMXEvent : public IEvent
{
public:
	OMXEvent(OMXEventData ev);

	OMXEventData *ToEvent();
	EmptyBufferDoneData *ToEmpty();
	FillBufferDoneData *ToFill();

private:
	OMXEventData m_ev;
};

class EmptyBufferDoneEvent : public IEvent
{
public:
	EmptyBufferDoneEvent(EmptyBufferDoneData ev);
	OMXEventData *ToEvent();
	EmptyBufferDoneData *ToEmpty();
	FillBufferDoneData *ToFill();

private:
	EmptyBufferDoneData m_ev;
	
};

class FillBufferDoneEvent : public IEvent
{
public:
	FillBufferDoneEvent(FillBufferDoneData ev);
	OMXEventData *ToEvent();
	EmptyBufferDoneData *ToEmpty();
	FillBufferDoneData *ToFill();

private:
	FillBufferDoneData m_ev;
	
};

#endif // EVENT_H
