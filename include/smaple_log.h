#pragma once

#include <ctime>  
#include <iostream>  
#include <fstream>  
#include <direct.h>

using namespace std;

#ifndef __EASYLOG_PIPI_0813
#define __EASYLOG_PIPI_0813

class sample_log
{
public:
	static void Write(std::string log) {
		try
		{
			std::ofstream ofs;
			time_t t = time(0);
			char tmp[64];
			strftime(tmp, sizeof(tmp), "[%Y-%m-%d %X]", localtime(&t));
			ofs.open("sample_log.log", std::ofstream::app);

			ofs << tmp << " - ";
			ofs.write(log.c_str(), log.size());
			ofs << std::endl;
			ofs.close();
		}
		catch (...)
		{
		}
	}
};

#endif
