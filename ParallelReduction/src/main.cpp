/*
 * main.cpp
 *
 *  Created on: 14 Feb 2016
 *      Author: wjs
 */

#include <iostream>
#include <sstream>
#include <vector>
#include <memory>
#include <chrono>
#include <random>
#include <cstdlib>
#include <cstring>

#include "CL/opencl.h"

#include "cltune.h"

#include <CLIArgumentsBase.h>

class CConfig : public CCLIArgumentsBase
{
private:
	size_t m_iNumElements;

public: // Public member functions.
	CConfig()
	{
		Init(0, nullptr);
	}

	CConfig(int argc_, char* argv_[])
	{
		Init(argc_, argv_);
	}

	size_t GetNumberOfElements() { return m_iNumElements; }

protected:
	virtual void Init(int argc_, char* argv_[])
	{
		CCLIArgumentsBase::Init(argc_, argv_);
		InitThisClass(argc_, argv_);
	}

	virtual void PrintHelpMessage()
	{
		std::cout << "CLTuneDemos/ParallelReduction: A demo showing CLTune applied to parallel reduction." << std::endl;
		std::cout << "Copyright (C) 2016 William John Shipman" << std::endl;
		std::cout << std::endl;
		std::cout << "    This program is free software: you can redistribute it and/or modify" << std::endl;
		std::cout << "    it under the terms of the GNU General Public License as published by" << std::endl;
		std::cout << "    the Free Software Foundation, either version 3 of the License, or" << std::endl;
		std::cout << "    any later version." << std::endl;
		std::cout << std::endl;
		std::cout << "USAGE:" << std::endl;
		std::cout << "    ParallelReduction [-h] [-p platform] [-d device] [-N elements]" << std::endl;
		std::cout << std::endl;
		std::cout << "Options:" << std::endl;
		std::cout << "  -h, --help : Print this help message and exit." << std::endl;
		std::cout << " -p platform : Use platform number \'platform\', defaults to 0." << std::endl;
		std::cout << "   -d device : Use device number \'device\' in the platform, default: 0." << std::endl;
		std::cout << " -N elements : Number of elements (i.e. data size) for the parallel reduction, default: 1024." << std::endl;
		std::cout << std::endl;
		std::cout << "This program is distributed in the hope that it will be useful," << std::endl;
		std::cout << "but WITHOUT ANY WARRANTY; without even the implied warranty of" << std::endl;
		std::cout << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the" << std::endl;
		std::cout << "GNU General Public License for more details." << std::endl;
		std::cout << std::endl;
		std::cout << "You should have received a copy of the GNU General Public License" << std::endl;
		std::cout << "along with this program.  If not, see <http://www.gnu.org/licenses/>." << std::endl;

		std::exit(0);
	}

private:
	void InitThisClass(int argc_, char* argv_[])
	{
		m_iNumElements = 1024;
		for (int idx = 1; idx < argc_; idx++)
		{
			if (std::strcmp(argv_[idx], "-N") == 0)
			{
				if (idx+1 < argc_)
					this->m_iNumElements = static_cast<size_t>(std::atoi(argv_[idx+1]));
			}
		}
	}

public:
	/*
	 * Type cast operator to convert to an std::string.
	 */
	virtual explicit operator std::string() const
	{
		std::ostringstream ss;
		ss << "Platform " << GetPlatformIdx() << " device " << GetDeviceIdx() << " - data size: " << m_iNumElements << " elements";
		return ss.str();
	}
};

std::shared_ptr<std::vector<cl_float>> GenerateData(size_t N_)
{
	std::shared_ptr<std::vector<cl_float>> Data(new std::vector<cl_float>(N_));

	// This sets up the random number generator seed as the current time in ticks since the epoch.
	const auto random_seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(static_cast<unsigned int>(random_seed));

	// Set up a uniform distribution with minimum 0.9 and maximum 1.0.
	std::uniform_real_distribution<cl_float> distribution(0.9f, 1.0f);

	// Repeatedly sample the uniform distribution and store the random number in the image.
	for (auto &item: *Data) { item = distribution(generator); }
	return Data;
}


int main(int argc, char* argv[])
{
	CConfig Cfg(argc, argv);
	std::cout << "Configuration: " << Cfg << std::endl;

	// Create the tuner using the platform and device IDs passed on the command line.
	cltune::Tuner tuner(Cfg.GetPlatformIdx(), Cfg.GetDeviceIdx());

	// All combinations of values for every tuning parameter will be tested.
	// For a lot of parameters or values for each parameter, this can be
	// slow.
	tuner.UseFullSearch();

	// Outputs the search process to a file
	tuner.OutputSearchLog("search_log.txt");

	auto pDataIn = GenerateData(Cfg.GetNumberOfElements());
	std::vector<cl_float> DataOut;
	DataOut.push_back(0.0f);

	return 0;
}
