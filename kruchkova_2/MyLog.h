#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <stdarg.h>

class MyLog
{
public:

	MyLog();
	~MyLog();



	enum class Status { 
		DEBUG,
		RELEASE,
		INFO 
	};

	static Status status;

	static void log(std::string text, MyLog::Status vec) {

		if (vec == MyLog::status) {
			std::cout << text << std::endl;
		}
	}
};

