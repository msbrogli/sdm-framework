
#include "utils.h"

/*
LIST MEMORIES
CREATE MEMORY mem_name USING addr_name
CREATE MEMORY mem_name(bits, sample)
DROP MEMORY mem_name

CREATE ADDRESS SPACE addr_name(bits, sample)
*/

void parse(const std::string &line) {
	std::vector<std::string> parts = split(line, ' ');
	std::string cmd = parts[0];
	std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
}

