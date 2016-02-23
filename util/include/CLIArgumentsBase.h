/*
 * CCLIArgumentsBase.h
 *
 *  Created on: 14 Feb 2016
 *      Author: wjs
 */

#ifndef UTIL_INCLUDE_CLIARGUMENTSBASE_H_
#define UTIL_INCLUDE_CLIARGUMENTSBASE_H_

#include <memory>

class CCLIArgumentsBase {
public:
	CCLIArgumentsBase();
	CCLIArgumentsBase(int argc_, char* argv_[]);
	virtual ~CCLIArgumentsBase();

protected:
	virtual void Init(int argc_, char* argv_[]);

	virtual void PrintHelpMessage() {};

private:
	size_t m_iPlatformIdx;
	size_t m_iDeviceIdx;

public:
	size_t GetPlatformIdx() const;
	size_t GetDeviceIdx() const;

	virtual explicit operator std::string() const {};
};


/*
 * Output a CConfig object to a stream using the std::string type cast.
 */
std::ostream& operator<<(std::ostream& os, const CCLIArgumentsBase& obj)
{
	os << (std::string) obj;
	return os;
}

#endif /* UTIL_INCLUDE_CLIARGUMENTSBASE_H_ */
