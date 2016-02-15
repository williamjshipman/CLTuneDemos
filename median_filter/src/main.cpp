/*
 *  CLTuneDemos/median_filter: A demo showing CLTune applied to a median filter.
 *
 *  Copyright (C) 2016 William John Shipman
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Created on: 16 Jan 2016
 *      Author: William John Shipman (@williamjshipman).
 */

#include <iostream>
#include <sstream>
#include <vector>
#include <chrono>
#include <random>
#include <memory>

#include <cstdlib>
#include <cstring>

// Includes the OpenCL tuner library
#include "cltune.h"

#include "CL/opencl.h"

#include "CCLIArgumentsBase.h"

// Application configuration data from CLI.
class CConfig : public CCLIArgumentsBase
{
private:
	size_t m_iImageWidth;
	size_t m_iImageHeight;

public: // Public member functions.
	CConfig()
	{
		Init(0, nullptr);
	}

	CConfig(int argc_, char* argv_[])
	{
		Init(argc_, argv_);
	}

	size_t GetImageHeight() const { return m_iImageHeight; }
	size_t GetImageWidth()  const { return m_iImageWidth; }

protected:
	virtual void Init(int argc_, char* argv_[])
	{
		CCLIArgumentsBase::Init(argc_, argv_);
		InitThisClass(argc_, argv_);
	}

	virtual void PrintHelpMessage()
	{
		std::cout << "CLTuneDemos/median_filter: A demo showing CLTune applied to a median filter." << std::endl;
		std::cout << "Copyright (C) 2016 William John Shipman" << std::endl;
		std::cout << std::endl;
		std::cout << "    This program is free software: you can redistribute it and/or modify" << std::endl;
		std::cout << "    it under the terms of the GNU General Public License as published by" << std::endl;
		std::cout << "    the Free Software Foundation, either version 3 of the License, or" << std::endl;
		std::cout << "    any later version." << std::endl;
		std::cout << std::endl;
		std::cout << "USAGE:" << std::endl;
		std::cout << "    median_filter [-h] [-p platform] [-d device] [-w width] [-h height]" << std::endl;
		std::cout << std::endl;
		std::cout << "Options:" << std::endl;
		std::cout << "  -h, --help : Print this help message and exit." << std::endl;
		std::cout << " -p platform : Use platform number \'platform\', defaults to 0." << std::endl;
		std::cout << "   -d device : Use device number \'device\' in the platform, default: 0." << std::endl;
		std::cout << "    -W width : Image width (pixels), default: 256." << std::endl;
		std::cout << "   -H height : Image height (pixels), default: 256." << std::endl;
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
		m_iImageHeight = 256;
		m_iImageWidth = 256;
		for (int idx = 1; idx < argc_; idx++)
		{
			if (std::strcmp(argv_[idx], "-W") == 0)
			{
				if (idx+1 < argc_)
					this->m_iImageWidth = static_cast<size_t>(std::atoi(argv_[idx+1]));
			}
			else if (std::strcmp(argv_[idx], "-H") == 0)
			{
				if (idx+1 < argc_)
					this->m_iImageHeight = static_cast<size_t>(std::atoi(argv_[idx+1]));
			}
		}
	}

public:
	/*
	 * Type cast operator to conver to an std::string.
	 */
	virtual explicit operator std::string() const
	{
		std::ostringstream ss;
		ss << "Platform " << GetPlatformIdx() << " device " << GetDeviceIdx() << " - image size: " << m_iImageWidth << "x" << m_iImageHeight;
		return ss.str();
	}
};

/**
 * Generate a random 2D gray-scale image.
 */
std::shared_ptr<std::vector<cl_float>> GenerateImage(size_t width_, size_t height_)
{
	std::shared_ptr<std::vector<cl_float>> Img(new std::vector<cl_float>(width_*height_));

	// This sets up the random number generator seed as the current time in ticks since the epoch.
	const auto random_seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(static_cast<unsigned int>(random_seed));

	// Set up a uniform distribution with minimum 0.9 and maximum 1.0.
	std::uniform_real_distribution<cl_float> distribution(0.9f, 1.0f);

	// Repeatedly sample the uniform distribution and store the random number in the image.
	for (auto &item: *Img) { item = distribution(generator); }
	return Img;
}

int main(int argc, char* argv[])
{
	std::shared_ptr<CConfig> Cfg(new CConfig(argc, argv));
	std::cout << "Configuration: " << *Cfg << std::endl;

	// Create the list of files that define the kernel that will be autotuned.
	auto Kernels = std::vector<std::string>{"./median_filter/src/medfilt.cl"};

	// This is the list of files needed to compile the reference kernel.
	// The outputs of each kernel generated during the autotuning get compared
	// to this kernels output so that bugs get detected.
	auto BaselineKernel = std::vector<std::string>{"./median_filter/src/medfilt_baseline.cl"};

	// Create the tuner using the platform and device IDs passed on the command line.
	cltune::Tuner tuner(Cfg->GetPlatformIdx(), Cfg->GetDeviceIdx());

	// All combinations of values for every tuning parameter will be tested.
	// For a lot of parameters or values for each parameter, this can be
	// slow.
	tuner.UseFullSearch();

	// Outputs the search process to a file
	tuner.OutputSearchLog("search_log.txt");

	// Generate the random test image as an std::vector since CLTune doesn't understand images yet.
	auto ImgIn = GenerateImage(Cfg->GetImageWidth(), Cfg->GetImageHeight());

	// Allocate space for the output image and initialise it to zero.
	std::shared_ptr<std::vector<cl_float>> ImgOut(new std::vector<cl_float>(Cfg->GetImageWidth()*Cfg->GetImageHeight()));
	for (auto &item: *ImgOut) { item = 0.0f; }

	size_t work_x = Cfg->GetImageWidth();
	size_t work_y = Cfg->GetImageHeight();

	// Here I add a kernel for CLTune to autotune by passing in the list of source files and a kernel name.
	// The work group size parameters are the basis from which the final work group size gets calculated
	// below.
	auto kernelID = tuner.AddKernel(Kernels, "medfilt", {work_x, work_y}, {1, 1});

	// Define the tuning parameters and the values they can take on.
	// USE_LOCAL_MEM == 0 if local memory WILL NOT be used.
	// USE_LOCAL_MEM == 1 if local memory WILL be used.
	tuner.AddParameter(kernelID, "USE_LOCAL_MEM", {0, 1});
	// These two are used to define the local work size.
	tuner.AddParameter(kernelID, "TBX", {1, 4, 8, 16, 32});
	tuner.AddParameter(kernelID, "TBY", {1, 4, 8, 16, 32});

	// This tells CLTune how much local memory a given choice of parameters will consume.
	// It gets expressed as a function that takes an std::vector as input and returns the
	// size
	auto LocalMemorySize = [] (std::vector<size_t> v) {
		if (v[0] != 0) { return (v[1] + 2) * (v[2] + 2) * 4; }
		else           { return size_t{0}; }
	};
	// This tells CLTune which function to use to calculate local memory and which
	// tuning parameters to pass to this function.
	tuner.SetLocalMemoryUsage(kernelID, LocalMemorySize, {"USE_LOCAL_MEM", "TBX", "TBY"});

	// This is how one tells CLTune that TBX and TBY relate to the global and local
	// work size. Thes operations take the global and local work size specified when adding
	// the kernel and divide or multiply them by the variables listed before running each
	// kernel.
	tuner.DivGlobalSize(kernelID, {"TBX", "TBY"});
	tuner.MulGlobalSize(kernelID, {"TBX", "TBY"});
	tuner.MulLocalSize(kernelID, {"TBX", "TBY"});

	// The number of threads, i.e. the global work size, needs to be a multiple of
	// the local work size, which is (8,8) for the reference kernel.
	work_x = work_x + work_x % 8;
	work_y = work_y + work_y % 8;
	tuner.SetReference(BaselineKernel, "medfilt", {work_x, work_y}, {8, 8});

	// Now pass the kernel arguments, in the order they appear in the kernel signature.
	// Using AddArgumentOutput also tells CLTune which argument needs to be compared
	// between the tuned and reference kernels to check that the output is always the
	// same, irrespective of the choice of parameters.
	tuner.AddArgumentInput(*ImgIn);
	tuner.AddArgumentOutput(*ImgOut);
	tuner.AddArgumentScalar(static_cast<int>(Cfg->GetImageWidth()));
	tuner.AddArgumentScalar(static_cast<int>(Cfg->GetImageHeight()));

	// Starts the tuner
	tuner.Tune();

	// Prints the results to screen and to file
	auto time_ms = tuner.PrintToScreen();
	tuner.PrintToFile("output.csv");
	tuner.PrintFormatted();
}
