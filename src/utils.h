#ifndef SDM_UTILS_H
#define SDM_UTILS_H

#include <string>
#include <vector>

std::string trim(const std::string &s);
std::vector<std::string> split(const std::string &s, char delim);

class TimeMeasure {
public:
	std::vector<std::pair<std::string, long long> > measures;
	long long ref_time;

	void start();
	void mark(const std::string &name);
	std::string str();

	long long get_time();
};

#endif
