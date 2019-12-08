#pragma once


#include "DUDOS.h"
#include <iostream>
#include <vector>

#include <fstream>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

using namespace std;


class Path
{
public:
	Path();
	~Path();
	int from;
	int to;
	vector<int> path;
	int weight;


	// Serialize the std::vector member of Info
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar &from;
		ar &to;
		ar &path;
		ar &weight;
	}
};

