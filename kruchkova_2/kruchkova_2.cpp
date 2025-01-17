// kruchkova_2.cpp: определяет точку входа для консольного приложения.
//
#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "mpi.h"
#include "DUDOS.h"
#include "GA.h"
#include "Mylog.h"
#include "Tag.h"

#include <iostream>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>



#include <fstream>
#include <string>
#include <fstream>
#include <sstream>


#include <cstring>

#include <boost/serialization/vector.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>


#include <algorithm>
#include <time.h>
#include <set>  // заголовочный файл множеств и мультимножеств
#include <iterator>
#include <ctime>
#include<cstdlib> 


const int INF = 1000000000;

using namespace std;

void startGA(int argc, char *argv[]);



int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "Russian");
	srand(time(NULL));

	startGA(argc, argv);
	int s;


	system("pause");

	return 0;
}




void startGA(int argc, char *argv[]) {

	unsigned int start_time = clock(); // начальное время
									   ////////////////////////////////////////////////////////////////////////////////////
	cout << argc << endl;
	for (int i = 0; i < argc; i++) {
		cout << i << "  " << argv[0] << endl;
	}
	int rank_mpi, size_mpi, n, i, ibeg, iend;
	int root = 0;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size_mpi);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank_mpi);

	////////////////////////////////////////////////////////////////////////////////////
	vector<int> bestResult_prev(size_mpi);
	vector<int> bestResult(size_mpi);

	GA *answer = NULL;
	boolean flagStop = false;
	// Первичная отправка островов по потокам
	if (rank_mpi == root) {
		for (int i = 0; i < size_mpi; i++) {
			bestResult_prev[i] = INT_MAX;
			bestResult[i] = INT_MAX;
		}

		// Создаем остров
		GA* ga = new GA();
		ga->start();
		ga->createFirstPopulation();

		// Сериализуем остров в строку
		// Передаем остров всем остальным потокам
		for (int i = 1; i < size_mpi; i++) {
			// Save filename data contained in Info object
			std::string serialized;
			{
				// Create an output archive
				std::ostringstream oss;
				boost::archive::text_oarchive ar(oss);
				// Save the data
				ar & (*ga);
				serialized = oss.str();
			}
			// Получаем размер строки
			int len = serialized.size();
			len++;
			// Переводим строку в массив символов
			char *helloStr = new char[len];
			strcpy(helloStr, serialized.c_str());	// or pass &s[0]

													// Посылаем размер строки (в которой сериализован передаваемый объект) в главный поток
			MPI_Send(
				&len,					//	адрес начала расположения пересылаемых данных; 
				1,						//	число пересылаемых элементов;
				MPI_INT,				//	тип посылаемых элементов;
				i,						//	номер процесса-получателя в группе, связанной с коммуникатором comm;
				(int)Tag::ENUM::SIZE,	//	идентификатор сообщения (аналог типа сообщения функций nread и nwrite PSE nCUBE2);
				MPI_COMM_WORLD			//	коммуникатор области связи.
			);
			// Посылаем саму строку (в которой сериализован передаваемый объект)
			MPI_Send(helloStr, len, MPI_CHAR, i, (int)Tag::ENUM::MESSAGE, MPI_COMM_WORLD);
		}

	}


	int maxIteration = 10;
	for (int countIteration = 0; countIteration < maxIteration; countIteration++) {
		if (rank_mpi == root) {

			// Получаем обработанные острова из потоков i
			vector<GA *> ga_array;
			for (int i = 1; i < size_mpi; i++) {
				GA* ga_get = new GA();
				int size;
				// Получаем сообщение в котором лежит размер передаваемой строки (в которой сериализован передаваемый объект)
				//		от конкретного потока
				MPI_Recv(
					&size,								//	адрес начала расположения принимаемого сообщения;
					1,									//	максимальное число принимаемых элементов;
					MPI_INT,							//	тип элементов принимаемого сообщения;
					i,								//	номер процесса-отправителя;
					(int)Tag::ENUM::SIZE,				//	идентификатор сообщения;
					MPI_COMM_WORLD,						//	коммуникатор области связи;
					MPI_STATUS_IGNORE					//	атрибуты принятого сообщения.
				);
				cout << "rank_mpi # " << rank_mpi << "\t size_mpi : " << size << " from  rank_mpi # " << root << endl;

				// Создаем указатель на строку, с размером полученным в предыдущем сообщении
				char *get = new char[size];
				// Получаем строку (в которой сериализован передаваемый объект) от конкретного потока, размер уже знаем
				MPI_Recv(get, size, MPI_CHAR, i, (int)Tag::ENUM::MESSAGE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				//printf("rank_mpi 0 get string %s from rank_mpi 1\n", get);

				// Restore from saved data and print to verify contents

				{
					// Create and input archive
					std::istringstream iss(get);
					boost::archive::text_iarchive ar(iss);

					// Load the data
					ar & (*ga_get);
				}
				cout << "========rank_mpi " << rank_mpi << " get value from root =============== " << endl;
				ga_array.push_back(ga_get);
			}

			std::cout << "start obrabotka island" << std::endl;

			// Обрабатываем остров
			{
				// Объединяем в один остров
				for (int i = 1; i < ga_array.size(); i++) {
					GA * ga_first = ga_array[0];
					GA * ga_tmp = ga_array[i];

					//
					for (int j = 0; j < ga_tmp->population.size(); j++) {
						ga_first->population.push_back(ga_tmp->population[j]);
					}
				}

				// вычисляем фитнес для всех особей
				ga_array[0]->fitness();
				ga_array[0]->calculatePercent();

				// размножение особей
				ga_array[0]->reproduction();

				// пересчитываем фитнес
				ga_array[0]->fitness();
				ga_array[0]->calculatePercent();
				std::cout << "start mutation" << std::endl;

				// мутируем нужные особи
				ga_array[0]->mutation();

				std::cout << "start killWeakIndivid" << std::endl;

				// удаляем лишние особи
				ga_array[0]->killWeakIndivid();

				// очищаем все острова кроме первого
				for (int i = 1; i < ga_array.size(); i++)
					ga_array[i]->population.clear();

				// копируем особи с первого острова на другие острова
				/*for (int i = 1; i < ga_array.size(); i++) {
				GA *ga_first = ga_array[0];
				GA *ga_tmp = ga_array[i];

				for (int i = 0; i < ga_first->population.size(); i++) {
				ga_tmp->population.push_back(ga_first->population[i]->copy());
				}
				}*/
			}

			std::cout << "start fitness every island" << std::endl;
			//// вычисляем фитнес на каждом острове
			for (int i = 0; i < ga_array.size(); i++) {
				ga_array[i]->fitness();
				ga_array[i]->calculatePercent();
			}
			std::cout << "end obrabotka island" << std::endl;
			if (countIteration == maxIteration - 1) {
				answer = ga_array[0];
				flagStop = true;
				break;
			}

			// Рассылаем остров обратно по потокам
			{
				GA *first = ga_array[0];

				// Save filename data contained in Info object
				std::string serialized;
				{
					// Create an output archive
					std::ostringstream oss;
					boost::archive::text_oarchive ar(oss);
					// Save the data
					ar & (*first);
					serialized = oss.str();
				}
				// Получаем размер строки
				int len = serialized.size();
				len++;
				// Переводим строку в массив символов
				char *helloStr = new char[len];
				strcpy(helloStr, serialized.c_str());	// or pass &s[0]

				for (int i = 1; i < size_mpi; i++) {
					// Посылаем размер строки (в которой сериализован передаваемый объект) в главный поток
					MPI_Send(
						&len,					//	адрес начала расположения пересылаемых данных; 
						1,						//	число пересылаемых элементов;
						MPI_INT,				//	тип посылаемых элементов;
						i,						//	номер процесса-получателя в группе, связанной с коммуникатором comm;
						(int)Tag::ENUM::SIZE,	//	идентификатор сообщения (аналог типа сообщения функций nread и nwrite PSE nCUBE2);
						MPI_COMM_WORLD			//	коммуникатор области связи.
					);
					// Посылаем саму строку (в которой сериализован передаваемый объект)
					MPI_Send(helloStr, len, MPI_CHAR, i, (int)Tag::ENUM::MESSAGE, MPI_COMM_WORLD);
				}
			}
			std::cout << "end  (Рассылаем остров обратно по потокам)" << std::endl;
		}
		else {
			for (int i = 1; i < size_mpi; i++) {
				if (i == rank_mpi) {
					GA *ga = new GA();
					// Получаем от главного потока объект
					{
						int size;
						// Получаем сообщение в котором лежит размер передаваемой строки (в которой сериализован передаваемый объект)
						//		от конкретного потока
						MPI_Recv(
							&size,								//	адрес начала расположения принимаемого сообщения;
							1,									//	максимальное число принимаемых элементов;
							MPI_INT,							//	тип элементов принимаемого сообщения;
							root,									//	номер процесса-отправителя;
							(int)Tag::ENUM::SIZE,				//	идентификатор сообщения;
							MPI_COMM_WORLD,						//	коммуникатор области связи;
							MPI_STATUS_IGNORE					//	атрибуты принятого сообщения.
						);
						cout << "rank_mpi # " << rank_mpi << "\t size_mpi : " << size << " from  rank_mpi # " << root << endl;

						// Создаем указатель на строку, с размером полученным в предыдущем сообщении
						char *get = new char[size];
						// Получаем строку (в которой сериализован передаваемый объект) от конкретного потока, размер уже знаем
						MPI_Recv(get, size, MPI_CHAR, root, (int)Tag::ENUM::MESSAGE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
						//printf("rank_mpi 0 get string %s from rank_mpi 1\n", get);

						// Restore from saved data and print to verify contents

						{
							// Create and input archive
							std::istringstream iss(get);
							boost::archive::text_iarchive ar(iss);

							// Load the data
							ar & (*ga);
						}
						cout << "========rank_mpi " << rank_mpi << " get value from root =============== " << endl;
					}


					// обрабатываем остров
					{
						// Начинаем выполнять над ним итерации
						int countIteration = 5;
						for (int i = 0; i < countIteration; i++) {
							// считаем фитнес и процент выживаемости
							MyLog::log("1 start", MyLog::Status::DEBUG);
							ga->fitness();
							MyLog::log("2 start", MyLog::Status::DEBUG);

							ga->calculatePercent();


							// удаляем слабые особи, чтобы их осталось столько, сколько было
							MyLog::log("3 start", MyLog::Status::DEBUG);
							ga->killWeakIndivid();


#if DEBUG_ON

							std::cout << endl << "it #" << i;
							std::cout << "_ind #1___";
							for (int i = 0; i < ga->population[0]->path.size_mpi(); i++)
								std::cout << ga->population[0]->path[i] << " ";
							std::cout << "___fit = " << ga->population[0]->fitness;

							std::cout << "_ind #2___";
							for (int i = 0; i < ga->population[1]->path.size_mpi(); i++)
								std::cout << ga->population[1]->path[i] << " ";
							std::cout << "___fit = " << ga->population[1]->fitness;

#endif // DEBUG_ON

							// пересчитываем фитнес и процент выживаемости
							MyLog::log("4 start", MyLog::Status::DEBUG);
							ga->fitness();
							MyLog::log("5 start", MyLog::Status::DEBUG);
							ga->calculatePercent();

							//std::sort(ga->population.begin(), ga->population.end(), Individ::testSort);

							// размножение
							MyLog::log("6 start", MyLog::Status::DEBUG);
							ga->reproduction();

							MyLog::log("7 start", MyLog::Status::DEBUG);
							ga->fitness();
							MyLog::log("8 start", MyLog::Status::DEBUG);
							ga->calculatePercent();

							//std::cout << "=====================  " << i << " =====================";
							//ga->printPopulation();

							MyLog::log("9 start", MyLog::Status::DEBUG);
							ga->mutation();
							MyLog::log("10 start", MyLog::Status::DEBUG);

							{
								int s = rank_mpi;
								cout << "i == " << i << "  p(" << s << ")" << endl;
							}

						}

						ga->fitness();
						ga->calculatePercent();

						ga->killWeakIndivid();

						ga->fitness();
						ga->calculatePercent();

						std::sort(ga->population.begin(), ga->population.end(), Individ::testSort);
					}


					// Посылаем остров главному потоку
					{
						// Save filename data contained in Info object
						std::string serialized;
						{
							// Create an output archive
							std::ostringstream oss;
							boost::archive::text_oarchive ar(oss);
							// Save the data
							ar & (*ga);
							serialized = oss.str();
						}
						// Получаем размер строки
						int len = serialized.size();
						len++;
						// Переводим строку в массив символов
						char *helloStr = new char[len];
						strcpy(helloStr, serialized.c_str());	// or pass &s[0]

																// Посылаем размер строки (в которой сериализован передаваемый объект) в главный поток
						MPI_Send(
							&len,					//	адрес начала расположения пересылаемых данных; 
							1,						//	число пересылаемых элементов;
							MPI_INT,				//	тип посылаемых элементов;
							root,						//	номер процесса-получателя в группе, связанной с коммуникатором comm;
							(int)Tag::ENUM::SIZE,	//	идентификатор сообщения (аналог типа сообщения функций nread и nwrite PSE nCUBE2);
							MPI_COMM_WORLD			//	коммуникатор области связи.
						);
						// Посылаем саму строку (в которой сериализован передаваемый объект)
						MPI_Send(helloStr, len, MPI_CHAR, root, (int)Tag::ENUM::MESSAGE, MPI_COMM_WORLD);
					}

				}
			}
		}
		if (flagStop)
			break;
	}






	if (rank_mpi == root) {
		std::cout << endl << endl << "=======================================================" << endl;
		std::cout << "result = " << answer->population[0]->fitness << endl;
		std::cout << "minimalFitnes = " << answer->minimalFitnes << endl;
		std::cout << "minimalFitnesIteration = " << answer->minimalFitnesIteration << endl;
		std::cout << endl << "=======================================================" << endl;

		unsigned int end_time = clock(); // конечное время
		unsigned int search_time = end_time - start_time; // искомое время
		std::cout << endl << "time = "; std::printf("%.3f\n", search_time / 1000.0);

		std::cout << endl;
		for (int i = 0; i < answer->population[0]->path.size(); i++)
			std::cout << answer->population[0]->path[i] << " ";
		std::cout << endl;

		int x = 0; cin >> x;
	}

}



