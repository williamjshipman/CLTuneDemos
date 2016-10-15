/*
 * CCLIArgumentsBase.cpp
 *
 *  Created on: 14 Feb 2016
 *      Author: wjs
 */

#include <CLIArgumentsBase.h>
#include <cstring>
#include <cstdlib>

CCLIArgumentsBase::CCLIArgumentsBase() {
	Init(0, nullptr);
}

CCLIArgumentsBase::CCLIArgumentsBase(int argc_, char* argv_[])
{
	this->Init(argc_, argv_);
}

void CCLIArgumentsBase::Init(int argc_, char* argv_[])
{
	m_iPlatformIdx = 0;
	m_iDeviceIdx = 0;
	m_iNumKernelRuns = 3;
	for (int idx = 1; idx < argc_; idx++)
	{
		if (std::strcmp(argv_[idx], "-p") == 0)
		{
			if (idx+1 < argc_)
				this->m_iPlatformIdx = static_cast<size_t>(std::atoi(argv_[idx+1]));
		}
		else if (std::strcmp(argv_[idx], "-d") == 0)
		{
			if (idx+1 < argc_)
				this->m_iDeviceIdx = static_cast<size_t>(std::atoi(argv_[idx+1]));
		}
		else if (std::strcmp(argv_[idx], "-r") == 0)
		{
			if (idx+1 < argc_)
				this->m_iNumKernelRuns = std::atoi(argv_[idx+1]);
		}
		else if ((std::strcmp(argv_[idx], "-h") == 0) || (std::strcmp(argv_[idx], "--help") == 0))
			PrintHelpMessage();
	}
}

CCLIArgumentsBase::~CCLIArgumentsBase() {
	// TODO Auto-generated destructor stub
}

size_t CCLIArgumentsBase::GetPlatformIdx() const
{
	return this->m_iPlatformIdx;
}

size_t CCLIArgumentsBase::GetDeviceIdx() const
{
	return this->m_iDeviceIdx;
}

size_t CCLIArgumentsBase::GetNumberOfKernelRuns() const
{
	return this->m_iNumKernelRuns;
}
