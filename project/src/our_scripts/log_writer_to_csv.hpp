#ifdef GENERATE_LOG
#pragma once


#include "utils/Singleton.h"
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>


#define file_name "log_"

class log_writer_to_csv : public Singleton<log_writer_to_csv>{
	friend class Singleton<log_writer_to_csv>;
public:
	//Adds new row to csv file
	/*inline void add_new_log(std::string log) {
		//assert(log_file.good());

		auto current = std::chrono::system_clock::now();
		std::time_t current_time = std::chrono::system_clock::to_time_t(current);
		std::string date = std::ctime(&current_time);
		date.erase(date.size() - 1);

		log_file << date << ";" << log << std::endl;
	}
	*/
	//Adds new row to csv file
	template<class T>
	inline void add_new_log(T log) {
		auto current = std::chrono::system_clock::now();
		std::time_t current_time = std::chrono::system_clock::to_time_t(current);
		std::string date = std::ctime(&current_time);
		date.erase(date.size() - 1);

		log_file << date << ";" << log << std::endl;
	}
	template <class ...Ts>
	inline void add_new_log(const Ts&... logs) {
		auto current = std::chrono::system_clock::now();
		std::time_t current_time = std::chrono::system_clock::to_time_t(current);
		std::string date = std::ctime(&current_time);
		date.erase(date.size() - 1);

		log_file << date << ";";
		(add_partial_log(logs), ...);
		log_file << std::endl;
	}
private:
	template<class T>
	inline void add_partial_log(T arg) {
		log_file << arg << ";";
	}
	std::ofstream log_file;
	inline bool init() {
		auto current = std::chrono::system_clock::now();
		std::time_t current_time = std::chrono::system_clock::to_time_t(current);
		std::string date = std::ctime(&current_time);
		date.erase(date.size()-1);
		for (auto& c : date) {
			if (c == ' ' || c == ':') {
				c = '_';
			}
		}
		std::string name = file_name + date + ".csv";
		log_file = std::ofstream(name);
		//log_file.open(name);
		add_new_log("Game Started");
		return true;
	}
	inline ~log_writer_to_csv() {
		log_file.close();
	}
};
#endif