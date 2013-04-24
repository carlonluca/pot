// Written by Matt Ownby, August 2012
// You are free to use this for educational/non-commercial use

#ifndef ILOCKER_H
#define ILOCKER_H

#include <time.h>	// for timespec

class ILocker
{
public:
	virtual void Lock() = 0;
	virtual void Unlock() = 0;

	// must be locked before calling, returns true if event was received or false on timeout/error
	virtual bool WaitForEvent(const struct timespec *ptsEnd) = 0;

	// must be locked before calling!
	virtual void GenerateEvent() = 0;
};

#endif // ILOCKER_H
