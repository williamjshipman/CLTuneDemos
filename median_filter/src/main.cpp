/*
 *  CLTuneDemos/median_filter: A demo showing CLTune applied to a median filter.
 *
 *  Copyright (C) 2016 William John Shipman
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
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

// Application configuration data from CLI.
class CConfig
{
public: // Public fields.
	size_t m_iPlatformID;
	size_t m_iDeviceID;
	size_t m_iImageWidth;
	size_t m_iImageHeight;

public: // Public member functions.
	CConfig() : m_iPlatformID(size_t{0}),
				m_iDeviceID(size_t{0}),
				m_iImageWidth(size_t{256}),
				m_iImageHeight(size_t{256}) {}

	static std::shared_ptr<CConfig> parse(int argc, char* argv[])
	{
		std::shared_ptr<CConfig> Cfg(new CConfig);
		for (int idx = 1; idx < argc; idx++)
		{
			if (std::strcmp(argv[idx], "-p") == 0)
			{
				if (idx+1 < argc)
					Cfg->m_iPlatformID = static_cast<size_t>(std::atoi(argv[idx+1]));
			}
			else if (std::strcmp(argv[idx], "-d") == 0)
			{
				if (idx+1 < argc)
					Cfg->m_iDeviceID = static_cast<size_t>(std::atoi(argv[idx+1]));
			}
			else if (std::strcmp(argv[idx], "-W") == 0)
			{
				if (idx+1 < argc)
					Cfg->m_iImageWidth = static_cast<size_t>(std::atoi(argv[idx+1]));
			}
			else if (std::strcmp(argv[idx], "-H") == 0)
			{
				if (idx+1 < argc)
					Cfg->m_iImageHeight = static_cast<size_t>(std::atoi(argv[idx+1]));
			}
		}
		return Cfg;
	}

	explicit operator std::string() const
	{
		std::ostringstream ss;
		ss << "Platform " << m_iPlatformID << " device " << m_iDeviceID << " - image size: " << m_iImageWidth << "x" << m_iImageHeight;
		return ss.str();
	}
};

std::ostream& operator<<(std::ostream& os, const CConfig& obj)
{
	os << (std::string) obj;
	return os;
}

/**
 * Generate a random 2D gray-scale image.
 */
std::shared_ptr<std::vector<cl_float>> GenerateImage(size_t width_, size_t height_)
{
	std::shared_ptr<std::vector<cl_float>> Img(new std::vector<cl_float>(width_*height_));
	const auto random_seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(static_cast<unsigned int>(random_seed));
	std::uniform_real_distribution<cl_float> distribution(0.9f, 1.0f);
	for (auto &item: *Img) { item = distribution(generator); }
	return Img;
}

void PrintHelpMessage()
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

void ParseCLI(int argc, char* argv[])
{
	for (int idx = 0; idx < argc; idx++)
	{
		if ((std::strcmp(argv[idx], "-h") == 0) || (std::strcmp(argv[idx], "--help") == 0))
			PrintHelpMessage();
	}
}

int main(int argc, char* argv[])
{
	ParseCLI(argc, argv);
	std::shared_ptr<CConfig> Cfg = CConfig::parse(argc, argv);
	std::cout << "Configuration: " << *Cfg << std::endl;

	auto Kernels = std::vector<std::string>{"./src/medfilt.cl"};
	auto BaselineKernel = std::vector<std::string>{"./src/medfilt_baseline.cl"};

	cltune::Tuner tuner(Cfg->m_iPlatformID, Cfg->m_iDeviceID);
	tuner.UseFullSearch();
	// Outputs the search process to a file
	tuner.OutputSearchLog("search_log.txt");

	auto ImgIn = GenerateImage(Cfg->m_iImageWidth, Cfg->m_iImageHeight);
	std::shared_ptr<std::vector<cl_float>> ImgOut(new std::vector<cl_float>(Cfg->m_iImageWidth*Cfg->m_iImageHeight));
	for (auto &item: *ImgOut) { item = 0.0f; }

	size_t work_x = Cfg->m_iImageWidth;
	size_t work_y = Cfg->m_iImageHeight;
	work_x = work_x + work_x % 64;
	work_y = work_y + work_y % 64;

	auto kernelID = tuner.AddKernel(Kernels, "medfilt", {work_x, work_y}, {1, 1});
	tuner.AddParameter(kernelID, "USE_LOCAL_MEM", {0, 1});
	tuner.AddParameter(kernelID, "TBX", {1, 4, 8, 16, 32});
	tuner.AddParameter(kernelID, "TBY", {1, 4, 8, 16, 32});

	auto LocalMemorySize = [] (std::vector<size_t> v) {
		if (v[0] != 0) { return (v[1] + 2) * (v[2] + 2); }
		else           { return size_t{0}; }
	};
	tuner.SetLocalMemoryUsage(kernelID, LocalMemorySize, {"USE_LOCAL_MEM", "TBX", "TBY"});

	tuner.DivGlobalSize(kernelID, {"TBX", "TBY"});
	tuner.MulGlobalSize(kernelID, {"TBX", "TBY"});
	tuner.MulLocalSize(kernelID, {"TBX", "TBY"});

	tuner.SetReference(BaselineKernel, "medfilt", {work_x, work_y}, {8, 8});

	tuner.AddArgumentInput(*ImgIn);
	tuner.AddArgumentOutput(*ImgOut);
	tuner.AddArgumentScalar(static_cast<int>(Cfg->m_iImageWidth));
	tuner.AddArgumentScalar(static_cast<int>(Cfg->m_iImageHeight));

	// Starts the tuner
	tuner.Tune();

	// Prints the results to screen and to file
	auto time_ms = tuner.PrintToScreen();
	tuner.PrintToFile("output.csv");
	tuner.PrintFormatted();

//	std::system("pause");
}
