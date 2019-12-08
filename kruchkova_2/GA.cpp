#include "stdafx.h"

#include "GA.h"
#include "Individ.h"
#include "Path.h"
#include <time.h>
#include <sstream>
#include <algorithm>
#include <set>  // заголовочный файл множеств и мультимножеств
#include <iterator>
#include <iostream>
#include <fstream>
#include <string>
#include<cstdlib> 
#include<thread>



GA::GA()
{


}


GA::~GA()
{
}

void GA::start()
{
	init();
}

//
void GA::init()
{
	readData();
	calculateAllPath();
}


// Считываем данные в g
void GA::readData()
{

	std::string str;

	vector<vector<int>> readedInfo;

	std::ifstream in("data.txt"); // окрываем файл для чтения

	if (in.is_open())
	{
		while (getline(in, str))
		{
			if (str.size() == 0)
				continue;
			string tmp;
			remove_copy(str.begin(), str.end(), back_inserter(tmp), ' ');
			str = tmp;

			str.replace(str.find("="), 1, "-");

			vector<string>  result;
			stringstream  data(str);
			std::string line;
			while (std::getline(data, line, '-'))
			{
				result.push_back(line);
			}
			vector<int> resultNum;
			for (int i = 0; i < result.size(); i++) {
				resultNum.push_back(stoi(result[i]));
			}
			readedInfo.push_back(resultNum);
#ifdef DEBUG_ON
			std::cout << str << std::endl;
#endif
		}
	}
	in.close();     // закрываем файл

	// считаем количество разных чисел
	set<int> differentVertex;
	int countDifferentVertex = -1;
	for (int i = 0; i < readedInfo.size(); i++) {
		differentVertex.insert(readedInfo[i][0]);
		differentVertex.insert(readedInfo[i][1]);
	}
	countDifferentVertex = differentVertex.size();

	// инициализируем вектор данных
	for (int i = 0; i < countDifferentVertex; i++) {
		vector<pair<int, int>> tmp;
		data.push_back(tmp);
	}
	// заносим данные из прочитанного файла
	for (int i = 0; i < readedInfo.size(); i++) {
		int first = readedInfo[i][0];
		int second = readedInfo[i][1];
		int weight = readedInfo[i][2];
		pair<int, int> para(second, weight);
		data[first].push_back(para);
	}

	// сортируем данные по парам, чтобы у вершины соседнией ей вершины шли в порядке возрастания индекса вершины
	for (int i = 0; i < data.size(); i++) {
		sort(data[i].begin(), data[i].end(), GA::vertexPairCompare);
	}

	/*vector<pair<int, int>> zero;
	zero.push_back(pair<int, int>(1, 1));

	vector<pair<int, int>> one;
	one.push_back(pair<int, int>(0, 1));
	one.push_back(pair<int, int>(2, 1));
	one.push_back(pair<int, int>(3, 1));
	one.push_back(pair<int, int>(4, 1));

	vector<pair<int, int>> two;
	two.push_back(pair<int, int>(1, 1));

	vector<pair<int, int>> three;
	three.push_back(pair<int, int>(1, 1));

	vector<pair<int, int>> four;
	four.push_back(pair<int, int>(1, 1));
	four.push_back(pair<int, int>(5, 1));
	four.push_back(pair<int, int>(6, 1));

	vector<pair<int, int>> five;
	five.push_back(pair<int, int>(4, 1));

	vector<pair<int, int>> six;
	six.push_back(pair<int, int>(4, 1));

	data.push_back(zero);
	data.push_back(one);
	data.push_back(two);
	data.push_back(three);
	data.push_back(four);
	data.push_back(five);
	data.push_back(six);*/

	//g[0] = zero;
	//g[1] = one;
	//g[2] = two;
	//g[3] = three;
	//g[4] = four;
	//g[5] = five;
	//g[6] = six;

	n = data.size();
}


void GA::calculateAllPath()
{
	for (int i = 0; i < data.size(); i++)
		calculatePathFromVertexToAll(i);
}


// генерируем особь
Individ * GA::createIndividual()
{
	//int *currentPopulation = new int[n + 1];

	vector<int> currentPopulation(n + 1);

	for (int i = 0; i < n; i++) {
		currentPopulation[i] = i;
	}
	// на первое место нужно поставить стартовую вершину, и потом ее не трогать, при перемешивании

	for (int i = 0; i < n; i++) {
		if (currentPopulation[i] == startVertex) {
			int tmp = currentPopulation[0];
			currentPopulation[0] = currentPopulation[i];
			currentPopulation[i] = tmp;
		}
	}

	// В конце должны вернуться в стартовую вершину, и потом ее не трогать, при перемешивании
	currentPopulation[n] = startVertex;

	int firstRand = 0;
	int secondRand = 0;
	int tmp = 0;
	for (int i = 0; i < n * 10; i++) {
		firstRand = rand() % (n - 1) + 1;
		secondRand = rand() % (n - 1) + 1;

		tmp = currentPopulation[firstRand];
		currentPopulation[firstRand] = currentPopulation[secondRand];
		currentPopulation[secondRand] = tmp;
	}

	vector<int> result;
	for (int i = 0; i < n + 1; i++)
		result.push_back(currentPopulation[i]);

	Individ* individ = new Individ();
	individ->path = result;

	return individ;
}


// Ищем путь от вершины До всех остальных
void GA::calculatePathFromVertexToAll(int startVertex)
{
	int s = startVertex;
	vector<int> d(n, INF), p(n);
	d[s] = 0;
	vector<char> u(n);
	for (int i = 0; i < n; ++i) {
		int v = -1;
		for (int j = 0; j < n; ++j)
			if (!u[j] && (v == -1 || d[j] < d[v]))
				v = j;
		if (d[v] == INF)
			break;
		u[v] = true;

		for (size_t j = 0; j < data[v].size(); ++j) {
			int to = data[v][j].first,
				len = data[v][j].second;
			if (d[v] + len < d[to]) {
				d[to] = d[v] + len;
				p[to] = v;
			}
		}
	}

	// сохраняем путь
	vector<Path*> pathFromTo;

	for (int t = 0; t < data.size(); t++) {

		Path* currPath = new Path();
		currPath->from = startVertex;
		currPath->to = t;



		vector<int> path;
		for (int v = t; v != s; v = p[v])
			path.push_back(v);
		path.push_back(s);
		reverse(path.begin(), path.end());

#ifdef DEBUG_ON
		cout << "path from " << s << " to " << t << ": ";

		for (int i = 0; i < path.size(); i++) {
			cout << path[i] << " ";
		}
		cout << endl;
#endif

		//pathFromTo.push_back(path);

		// устанавливаем маршрут в текущий путь
		currPath->path = path;

		// Вычисляем длину пути
		currPath->weight = calculateWeightPath(currPath);

		// Сохраняем путь в глобальный массив
		pathFromTo.push_back(currPath);
	}
#ifdef DEBUG_ON
	cout << endl;
#endif

	this->paths.push_back(pathFromTo);
}

// Расчет длины пути
int GA::calculateWeightPath(Path * currPath)
{
	int sum = 0;
	for (int i = 0; i < currPath->path.size() - 1; i++) {
		int first = currPath->path[i];
		int second = currPath->path[i + 1];

		int weight = -1;

		for (int j = 0; j < data[first].size(); j++) {
			if (data[first][j].first == second) {
				weight = data[first][j].second;
				break;
			}
		}

		if (weight != -1)
			sum += weight;
		else
			cout << "====================error=========================" << endl;
	}
	return sum;
}


void GA::createFirstPopulation()
{
	for (int i = 0; i < sizePopulation; i++) {
		Individ* individ = createIndividual();
#ifdef  DEBUG_ON
		cout << "individ # " << i + 1 << endl;
		for (int j = 0; j < individ->path.size(); j++)
			cout << individ->path[j] << " ";
		cout << endl;
#endif //  DEBUG_ON

		population.push_back(individ);
	}
}


void GA::fitness()
{
	
#if THREAD_FITNESS == 1
	

	vector<pair<int, int>> indexForThread;

	// рассчитываем сколько задач каждый поток должен выполнить
	int countIndividInThread = population.size() / THREAD_FITNESS_COUNT;
	int countFreeTask = population.size();

	vector<int> countTaskForThread(THREAD_FITNESS_COUNT);
	for (int i = 0; i < THREAD_FITNESS_COUNT; i++) {
		countTaskForThread[i] += countIndividInThread;
		countFreeTask -= countIndividInThread;
	}
	for (int i = 0; i < THREAD_FITNESS_COUNT && countFreeTask != 0; i++) {
		countTaskForThread[i]++;
		countFreeTask--;
	}
	// рассчитываем индексы индивидов в популяции, которые будут вычислятьяс в потоках
	// индексы зависят от количества потоков
	int sum = 0;
	for (int i = 0; i < THREAD_FITNESS_COUNT; i++) {
		sum += countTaskForThread[i];
		indexForThread.push_back(pair<int, int>(sum - countTaskForThread[i], sum));
	}
	////////////////////////////////////////////////////////

	// массив потоков
	vector<thread> thread_array(THREAD_FITNESS_COUNT);
	// создаем и запускаем потоки
	for (int i = 0; i < THREAD_FITNESS_COUNT; i++) {
		// используем лябду функцию
		thread_array[i] = thread(
			[&](int startIndex, int secondIndex)
		{
			for (int i = startIndex; i < secondIndex; i++) {
				int x = fitnessForIndivid(population[i]);
				population[i]->fitness = x;
			}
		}, indexForThread[i].first, indexForThread[i].second
			);
	}
	// ждем остановки потоков
	for (int i = 0; i < THREAD_FITNESS_COUNT; i++) {
		if (thread_array[i].joinable()) {
			thread_array[i].join();
		}
	}
	// теперь можем двигаться дальше
	cout << "";
#endif // THREAD_FITNESS_ON

#if THREAD_FITNESS == 0
	for (int i = 0; i < population.size(); i++) {
		int x = fitnessForIndivid(population[i]);
		population[i]->fitness = x;
	}
	cout << "";
#endif // THREAD_FITNESS_OFF
}

//void  GA::startFitness(int startIndex, int secondIndex) {
//	for (int i = startIndex; i < secondIndex; i++) {
//		int x = fitnessForIndivid(population[i]);
//		population[i]->fitness = x;
//	}
//}


int GA::fitnessForIndivid(Individ * individ)
{
	int sum = 0;
	for (int i = 0; i < individ->path.size() - 1; i++) {
		int first = individ->path[i];
		int second = individ->path[i + 1];

		int weight = -1;

		weight = paths[first][second]->weight;

		if (weight != -1)
			sum += weight;
		else
			cout << "=========error==========" << endl;
	}

	if (sum < minimalFitnes) {
		minimalFitnes = sum;
		minimalFitnesIteration = iterationNum;
	}

	return sum;

}


void GA::calculatePercent() {
	std::sort(population.begin(), population.end(), Individ::testSort);
	double ss = 0.0;

	for (int i = 0; i < population.size(); i++) {
		double tmp = 1.0 / population[i]->fitness;
		ss += tmp;
	}

	for (int i = 0; i < population.size(); i++) {
		double tmp = 1.0 / population[i]->fitness;
		tmp = tmp / ss;
		population[i]->percent = tmp;
	}
	cout << "";
}


void GA::reproduction() {

	std::sort(population.begin(), population.end(), Individ::testSort);

	#if THREAD_REPRODUCTION == 1
	// Генерируем индексы родителей для получения потомства
	// индексы генерируются в соответсвтии с вероятностью на выживание
	// чем меньше длина маршрута тем выше индекс выживания
	iterationNum++;
	vector<pair<int, int>> indexForReproduction;
	for (int i = 0; i < population.size(); i++) {
		double random = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		double sum = 0.0;

		int indexFirstParent = 0;

		for (int i = 0; i < population.size(); i++) {
			sum += population[i]->percent;
			if (random < sum) {
				indexFirstParent = i;
				break;
			}
		}

		int indexSecondParent = -1;
		do {
			random = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			sum = 0.0;

			for (int i = 0; i < population.size(); i++) {
				sum += population[i]->percent;
				if (random < sum) {
					indexSecondParent = i;
					break;
				}
			}
		} while (indexSecondParent == indexFirstParent);

		//std::cout << indexFirstParent << " " << indexSecondParent << endl;

		indexForReproduction.push_back(std::pair<int, int>(indexFirstParent, indexSecondParent));
	}
	////////////////////////////////////////////

	vector<pair<int, int>> indexForThread;

	// рассчитываем сколько задач каждый поток должен выполнить
	int countIndividInThread = population.size() / THREAD_REPRODUCTION_COUNT;
	int countFreeTask = population.size();

	vector<int> countTaskForThread(THREAD_REPRODUCTION_COUNT);
	for (int i = 0; i < THREAD_REPRODUCTION_COUNT; i++) {
		countTaskForThread[i] += countIndividInThread;
		countFreeTask -= countIndividInThread;
	}
	for (int i = 0; i < THREAD_REPRODUCTION_COUNT && countFreeTask != 0; i++) {
		countTaskForThread[i]++;
		countFreeTask--;
	}
	// рассчитываем индексы индивидов в популяции, которые будут вычислятьяс в потоках
	// индексы зависят от количества потоков
	int sum = 0;
	for (int i = 0; i < THREAD_REPRODUCTION_COUNT; i++) {
		sum += countTaskForThread[i];
		indexForThread.push_back(pair<int, int>(sum - countTaskForThread[i], sum));
	}

	// массив потоков
	vector<thread> thread_array(THREAD_REPRODUCTION_COUNT);

	// создаем и запускаем потоки
	newPopulation.resize(population.size());
	for (int i = 0; i < THREAD_REPRODUCTION_COUNT; i++) {
		// используем лябду функцию
		thread_array[i] = thread(
			[&](int startIndex, int secondIndex, vector<pair<int, int>> indexForReproduction)
		{
			for (int i = startIndex; i < secondIndex; i++) {
				int x = 4;
				Individ * tmpIndivid;
				tmpIndivid = crossOver(population[indexForReproduction[i].first], population[indexForReproduction[i].second]);

				//newPopulation.push_back(tmpIndivid);
				newPopulation[i] = tmpIndivid;
			}
		}, indexForThread[i].first, indexForThread[i].second, indexForReproduction
			);
	}
	// ждем остановки потоков
	for (int i = 0; i < THREAD_REPRODUCTION_COUNT; i++) {
		if (thread_array[i].joinable()) {
			thread_array[i].join();
		}
	}
	// теперь можем двигаться дальше
	//population.clear();
	// копируем новых потомков в основную популяцию
	for (int i = 0; i < newPopulation.size(); i++) {
		population.push_back(newPopulation[i]);
	}
	// чистим новых потомков
	newPopulation.clear();



	/*for (int i = 0; i < indexForReproduction.size(); i++) {
		newPopulation.push_back(crossOver(population[indexForReproduction[i].first], population[indexForReproduction[i].second]));
	}
	*/
	#endif // THREAD_REPRODUCTION == 1

	#if THREAD_REPRODUCTION == 0

	iterationNum++;
	vector<pair<int, int>> indexForReproduction;
	cout << "1" << endl;
	// Ãåíåðèðóåì èíäåêñû ðîäèòåëåé äëÿ ïîëó÷åíèÿ ïîòîìñòâà
	// èíäåêñû ãåíåðèðóþòñÿ â ñîîòâåòñâòèè ñ âåðîÿòíîñòüþ íà âûæèâàíèå
	// ÷åì ìåíüøå äëèíà ìàðøðóòà òåì âûøå èíäåêñ âûæèâàíèÿ
	for (int i = 0; i < population.size(); i++) {
		double random = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		//random /= 2.0;
		double sum = 0.0;

		int indexFirstParent = 0;

		for (int i = 0; i < population.size(); i++) {
			sum += population[i]->percent;
			if (random < sum) {
				indexFirstParent = i;
				break;
			}
		}

		int indexSecondParent = -1;
		do {
			random = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			//random /= 2.0;
			sum = 0.0;

			for (int i = 0; i < population.size(); i++) {
				sum += population[i]->percent;
				if (random < sum) {
					indexSecondParent = i;
					break;
				}
			}
		} while (indexSecondParent == indexFirstParent);

		//std::cout << indexFirstParent << " " << indexSecondParent << endl;
		if (indexFirstParent < 0)
			indexFirstParent = 1;
		if (indexSecondParent < 0)
			indexSecondParent = 1;
		indexForReproduction.push_back(std::pair<int, int>(indexFirstParent, indexSecondParent));
	}
	cout << "2" << endl;

	vector<Individ*> newPopulation;
	for (int i = 0; i < indexForReproduction.size(); i++) {
		newPopulation.push_back(crossOver(population[indexForReproduction[i].first], population[indexForReproduction[i].second]));
	}

	cout << "3" << endl;

	for (int i = 0; i < newPopulation.size(); i++) {
		//delete(population[i]);
		population.push_back(newPopulation[i]);
	}
	cout << "4" << endl;

	newPopulation.clear();

	cout << "5" << endl;
	#endif // THREAD_REPRODUCTION == 0

	
}

// Получаем потомка от двух родителей методом Кросс-Овер
// потомок составляется из трех частей
Individ* GA::crossOver(Individ* firstParent, Individ* secondParent)
{
	int index = (rand() % (firstParent->path.size() - sizeCrossOverWindow - 1)) + 1;
	//index = 2;

	vector<int> pth(firstParent->path.size());
	for (int i = 0; i < pth.size(); i++)
		pth[i] = INT_MIN;

	int numParent = rand() % 2;
	//numParent = 1;

	Individ** mas = new Individ*[2];
	mas[0] = firstParent;
	mas[1] = secondParent;

	// Копируем среднюю часть
	vector<int> centralPart_1;
	vector<int> centralPart_2;
	vector<vector<int>*> centralPart;

	centralPart.push_back(&centralPart_1);
	centralPart.push_back(&centralPart_2);

	/*if (numParent == 0) {
		centralPart.push_back(&centralPart_1);
		centralPart.push_back(&centralPart_2);
	}
	else {
		centralPart.push_back(&centralPart_2);
		centralPart.push_back(&centralPart_1);
	}*/

	for (int i = index; i < index + sizeCrossOverWindow; i++) {
		int firstParentСhromosome = mas[0]->path[i];
		int secondParentСhromosome = mas[1]->path[i];

		if (firstParentСhromosome != secondParentСhromosome) {
			centralPart_1.push_back(firstParentСhromosome);
			centralPart_2.push_back(secondParentСhromosome);
		}

		pth[i] = mas[abs(1 - numParent)]->path[i];
	}

	// удалим повторяющиеся элементы
	for (int i = 0; i < centralPart_1.size(); i++) {

		int elem = centralPart_1[i];
		int index = BinSearch(centralPart_2, elem);

		if (index != -1) {
			centralPart_2.erase(centralPart_2.begin() + index);
			centralPart_1.erase(centralPart_1.begin() + i);
			i--;
		}
	}

	// Перемешиваем центральную часть для дальнейшей замены повторяющихся элементов на невошедшие
	for (int i = 0; i < centralPart_1.size() * 10; i++) {
		int rnd1 = rand() % centralPart_1.size();
		int rnd2 = rand() % centralPart_1.size();
		// перемешиваем  родительский массив
		int tmp = (*centralPart[numParent])[rnd1];
		(*centralPart[numParent])[rnd1] = (*centralPart[numParent])[rnd2];
		(*centralPart[numParent])[rnd2] = tmp;
	}
	// сортируем НЕ родительскую середину 
	sort((*centralPart[abs(1 - numParent)]).begin(), (*centralPart[abs(1 - numParent)]).end());

	// Копируем первую часть до индекса из нужного родителя
	for (int i = 0; i < index; i++) {
		pth[i] = mas[numParent]->path[i];

		// ищем в НЕ родительской середине
		int it = BinSearch(*centralPart[abs(1 - numParent)], pth[i]);
		if (it != -1) {
			// вставляем соответствующий элемент из родительской середины вместо повторяющегося
			pth[i] = (*centralPart[numParent])[it];
		}

	}



	// Копируем оставшуюся часть
	for (int i = index + sizeCrossOverWindow; i < firstParent->path.size(); i++) {
		pth[i] = mas[numParent]->path[i];

		// ищем в НЕ родительской середине
		int it = BinSearch(*centralPart[abs(1 - numParent)], pth[i]);
		if (it != -1) {
			// вставляем соответствующий элемент из родительской середины вместо повторяющегося
			pth[i] = (*centralPart[numParent])[it];
		}

	}
#ifdef DEBUG_ON
	for (int i = 0; i < pth.size(); i++) {
		cout << pth[i] << " ";
	}
	cout << endl;
#endif
	Individ* result = new Individ();
	result->path = pth;

	/// проверяем
	set<int> testEquals;
	for (int i = 1; i < pth.size(); i++) {
		if (find(testEquals.begin(), testEquals.end(), pth[i]) != testEquals.end()) {
			cout << "same elem " << pth[i] << endl;
		}
		else {
			testEquals.insert(pth[i]);
		}
	}

	return result;

}

// Бинарный поиск в массиве
int GA::BinSearch(vector<int>  &arr, int key)
{
	/*
	int count = arr.size();
	int l = 0;            // нижняя граница
	int u = count - 1;    // верхняя граница

	while (l <= u) {
		int m = (l + u) / 2;
		if (arr[m] == key) return m;
		if (arr[m] < key) l = m + 1;
		if (arr[m] > key) u = m - 1;
	}
	return -1;
	*/

	for (int i = 0; i < arr.size(); i++) {
		if (arr[i] == key)
			return i;
	}
	return -1;
}


void GA::mutationIndivid(Individ* individ)
{
	int first = 1 + rand() % (n - 1);
	int second = 1 + rand() % (n - 1);
	for (int i = 0; i < countSwapInMutation; i++) {
		// Меняем два элемента местами
		int tmp = individ->path[first];
		individ->path[first] = individ->path[second];
		individ->path[second] = tmp;

		// Дальше будем менять только что поменянный элемент с новым, чтобы за раз новое место получал один элемент а не 2
		first = second;
		second = 1 + rand() % (n - 1);
	}
}


void GA::mutation()
{
	/*std::sort(population.begin(), population.end(), Individ::testSort);
	for (int i = population.size() ; i < population.size(); i++) {
		mutationIndivid(population[i]);
	}*/
}

void GA::killWeakIndivid()
{
	std::sort(population.begin(), population.end(), Individ::testSort);
	while (population.size() != sizePopulation) {
		population.erase(population.begin() + population.size() - 1);
	}
}

void GA::printPopulation()
{
	cout << endl << endl;
	for (int i = 0; i < population.size(); i++) {
		for (int j = 0; j < population[i]->path.size(); j++)
			cout << population[i]->path[j] << " ";
		cout << "\tfitness = " << population[i]->fitness << endl;
	}
	cout << endl << endl;
}
