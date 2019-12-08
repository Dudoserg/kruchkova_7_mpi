#pragma once

#include "DUDOS.h"
#include <iostream>
#include <vector>
#include "Individ.h"
#include "Path.h"




#include <fstream>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

using std::vector;
using std::pair;
using namespace std;


class GA
{
public:

	GA();

	~GA();

	// fields

	// Количество потоков в вычислении  FITNESS
	static const int THREAD_FITNESS_COUNT = 2;


	// Количество потоков в вычислении REPRODUCTION
	static const int THREAD_REPRODUCTION_COUNT = 2;


	// количество островов
	static const int THREAD_COUNT_ISLAND = 4;

	static const int INF = 1000000000;
	// размер окна в кроссовере
	static const int sizeCrossOverWindow = 3;
	// количество мутаций 
	static const int countSwapInMutation = 3;


	vector<Individ*> newPopulation;

	int sizePopulation = 100;
	int startVertex = 0;

	int minimalFitnes = 9999;
	int minimalFitnesIteration = 9999;
	int iterationNum = 0;

	vector<vector<pair<int, int>>> data;
	int n;
	vector<vector<Path*>> paths;

	vector<Individ*> population;



	// Serialize the std::vector member of Info
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar &newPopulation;
		ar &sizePopulation;
		ar &startVertex;
		ar &minimalFitnes;
		ar &minimalFitnesIteration;
		ar &iterationNum;
		ar &data;
		ar &n;
		ar &paths;
		ar &population;
	}

	

/////////////////////////////////////////////////////////	

	// methods
	void start();

	// начальная популяция
	void createFirstPopulation();

	void fitness();

	//void startFitness(int startIndex, int secondIndex);

	int fitnessForIndivid(Individ* individ);

	void calculatePercent();

	void reproduction();


	Individ* crossOver(Individ* firstParent, Individ* secondParent);

	int BinSearch(vector<int>  &arr,  int key);

	void mutation();

	void killWeakIndivid();

	void printPopulation();

private:
	// fields

	// methods
	void init();

	void readData();

	void calculatePathFromVertexToAll(int startVertex);

	void calculateAllPath();

	int calculateWeightPath(Path* currPath);

	// создаем одну особь
	Individ* createIndividual();

	

	void mutationIndivid(Individ* individ);

	static bool vertexPairCompare(const pair<int, int> &first, const pair<int, int> &second) {
		return first.first < second.first;
	}

};

