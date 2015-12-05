
#include "utils.h"

/*
LIST MEMORIES
CREATE MEMORY mem_name USING space_name
CREATE MEMORY mem_name(bits, sample)
DROP MEMORY mem_name

CREATE ADDRESS SPACE space_name(bits, sample)
CREATE ADDRESS SPACE space_name(bits, sample)
LIST ADDRESS SPACES
DROP ADDRESS SPACE space_name
*/

void parse(const std::string &line) {
	std::vector<std::string> parts = split(line, ' ');
	std::string cmd = parts[0];
	std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
}

