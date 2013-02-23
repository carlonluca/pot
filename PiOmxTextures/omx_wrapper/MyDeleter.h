// Written by Matt Ownby, August 2012
// You are free to use this for educational/non-commercial use

#ifndef MPO_DELETER_H
#define MPO_DELETER_H

#include <tr1/memory>

using namespace std::tr1;

class MyDeleter
{
protected:

	virtual void DeleteInstance() = 0;

	class deleter;
	friend class deleter;

	// THIS DELETE CODE FROM BOOST EXAMPLE
	class deleter
	{
	public:
		void operator()(MyDeleter *p) { p->DeleteInstance(); }
	};
	// END BOOST EXAMPLE

};

#endif // MPO_DELETER_H
