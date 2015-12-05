
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>

#include "server.h"

const char msg_busy[] = "Server busy.\n";
const char msg_invalid_cmd[] = "Invalid command.\n";

void handle_connection(int fd) {
	char buffer[256];
	int len;
	while((len = read(fd, buffer, 255)) > 0) {
		buffer[len] = '\0';

		if (!strcmp(buffer, "BYE\n")) {
			break;
		}

		write(fd, msg_invalid_cmd, sizeof(msg_invalid_cmd));
	}
	close(fd);
}

void Server::run(const ServerConfig &config) {
	this->config = &config;
	std::cout << "Running SDM server..." << std::endl;

	int srv_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (srv_fd < 0) {
		perror("Error opening socket");
		exit(1);
	}

	struct sockaddr_un srv_addr;
	bzero((char *) &srv_addr, sizeof(srv_addr));
	strncpy(srv_addr.sun_path, "../sdm.sock", sizeof(srv_addr.sun_path)-1);
	if (bind(srv_fd, (struct sockaddr*) &srv_addr, sizeof(srv_addr)) != 0) {
		perror("Error binding");
		exit(1);
	}

	if (listen(srv_fd, 2) != 0) {
		perror("Error listening");
		exit(1);
	}

	int cli_fd;
	struct sockaddr_un cli_addr;
	socklen_t cli_addr_len = sizeof(cli_addr);
	char buffer[256];
	int len;
	int pid = 0;
	while((cli_fd = accept(srv_fd, (struct sockaddr *) &cli_addr, &cli_addr_len)) >= 0) {

		if (pid) {
			int status;
			pid_t result = waitpid(pid, &status, WNOHANG);
			if (result == 0) {
				std::cout << "New client connection rejected." << std::endl;
				write(cli_fd, msg_busy, sizeof(msg_busy));
				close(cli_fd);
				continue;
			}
		}

		std::cout << "New client connection accepted." << std::endl;
		pid = fork();
		if (pid == 0) {
			// child process
			handle_connection(cli_fd);
			std::cout << "Client connection closed." << std::endl;
			exit(0);
		} else {
			// parent process
			close(cli_fd);
		}
	}
}

int main(void) {
	ServerConfig config;
	config.address_spaces_dir = "../data/address_space/";
	config.counters_dir = "../data/counters/";

	Server server;
	server.run(config);
	return 0;
}

