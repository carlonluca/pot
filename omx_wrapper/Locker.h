// Written by Matt Ownby, August 2012
// You are free to use this for educational/non-commercial use

#ifndef LOCKER_H
#define LOCKER_H

#include <pthread.h>
#include "ILocker.h"
#include "MyDeleter.h"

typedef std::tr1::shared_ptr<ILocker> ILockerSPtr;

class Locker : public ILocker, public MyDeleter
{
public:
	static ILockerSPtr GetInstance();
	void Lock();
	void Unlock();
	bool WaitForEvent(const struct timespec *ptsEnd);
	void GenerateEvent();

private:
	Locker() : m_bInitialized(false) { Init(); }
    virtual ~Locker() { Shutdown(); }

	void DeleteInstance() { delete this; }

	void Init();
	void Shutdown();

	bool m_bInitialized;
	pthread_mutex_t m_Mutex;
	pthread_cond_t m_Cond;
};

#endif // LOCKER_H

