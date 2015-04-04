// Written by Matt Ownby, August 2012
// You are free to use this for educational/non-commercial use

#include "Locker.h"
#include <stdexcept>

using namespace std;

ILockerSPtr Locker::GetInstance()
{
	ILockerSPtr pRes;
	Locker *pInstance = 0;

	try
	{
		pInstance = new Locker();
		pInstance->Init();
		pRes = ILockerSPtr(pInstance, Locker::deleter());
	}
	catch (std::exception &)
	{
		if (pInstance)
		{
			delete pInstance;
		}
	}

	return pRes;
}

void Locker::Lock()
{
	if (pthread_mutex_lock(&m_Mutex) != 0)
	{
		throw runtime_error("Mutex lock failed");
	}
}

void Locker::Unlock()
{
	if (pthread_mutex_unlock(&m_Mutex) != 0)
	{
		throw runtime_error("Mutex unlock failed");
	}
}

// this method can't throw an exception because  we are locked
bool Locker::WaitForEvent(const struct timespec *ptsEnd)
{
	bool bRes = false;

	int iRes = pthread_cond_timedwait(&m_Cond, &m_Mutex, ptsEnd);

	if (iRes == 0)
	{
		bRes = true;
	}

	return bRes;
}

// this method can't throw an exception because  we are locked
void Locker::GenerateEvent()
{
	// assumes we are locked!
	pthread_cond_broadcast(&m_Cond);
}

void Locker::Init()
{
	if (pthread_mutex_init(&m_Mutex, 0) != 0)
	{
		throw runtime_error("Mutex init failed");
	}

	if (pthread_cond_init(&m_Cond, NULL) != 0)
	{
		throw runtime_error("Condition init failed");
	}

	m_bInitialized = true;
}

void Locker::Shutdown()
{
	if (m_bInitialized)
	{
		pthread_mutex_destroy(&m_Mutex);
		pthread_cond_destroy(&m_Cond);
	}
}
