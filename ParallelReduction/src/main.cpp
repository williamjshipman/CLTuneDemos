/*
 * main.cpp
 *
 *  Created on: 14 Feb 2016
 *      Author: wjs
 */

#include <iostream>
#include <vector>

#include "cltune.h"

void PrintHelpMessage()
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
	std::cout << "    median_filter [-h] [-p platform] [-d device]" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -h, --help : Print this help message and exit." << std::endl;
	std::cout << " -p platform : Use platform number \'platform\', defaults to 0." << std::endl;
	std::cout << "   -d device : Use device number \'device\' in the platform, default: 0." << std::endl;
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

int main(int argc, char* argv[])
{
	return 0;
}
