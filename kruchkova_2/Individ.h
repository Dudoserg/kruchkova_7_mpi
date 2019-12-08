#pragma once


#include "DUDOS.h"
#include <iostream>
#include <vector>



#include <fstream>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

using std::vector;
using std::pair;
using namespace std;

class Individ
{
public:
	Individ();
	~Individ();
	vector<int> path;
	int fitness;
	double percent;

	static bool testSort(const Individ* a, const Individ* b)
	{
		return a->fitness < b->fitness; //return a->percent > b->percent;
	}

	Individ* copy() {
		Individ* tmp = new Individ;
		for (int i = 0; i < this->path.size(); i++)
			tmp->path.push_back(this->path[i]);
		tmp->fitness = this->fitness;
		tmp->percent = this->percent;
		return tmp;
	};


	// Serialize the std::vector member of Info
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar &path;
		ar &fitness;
		ar &percent;
	}

private:
};

