#include <sstream>
#include "utils.h"

// Original code: http://stackoverflow.com/a/21698913/465035
std::string trim(const std::string &s)
{
	std::string::const_iterator it = s.begin();
	while (it != s.end() && isspace(*it)) {
		it++;
	}
	std::string::const_reverse_iterator rit = s.rbegin();
	while (rit.base() != it && isspace(*rit)) {
		rit++;
	}
	return std::string(it, rit.base());
}

// Original code: http://stackoverflow.com/a/236803/465035
std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}
