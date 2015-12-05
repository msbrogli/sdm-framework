
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>

#include "server.h"

const char msg_busy[] = "Server busy.\n";
const char msg_invalid_cmd[] = "Invalid command.\n";

Client::Client(int fd) {
	this->fd = fd;
}

void Client::disconnect() {
	close(this->fd);
	std::cout << "Client connection closed." << std::endl;
	exit(0);
}

void Client::parse(const std::string &line) {
	if (line == "BYE") {
		this->disconnect();
	}
}

void Client::handle_connection() {
	char buffer[256];
	int len;
	while((len = read(this->fd, buffer, 255)) > 0) {
		if (buffer[len-1] == '\n') {
			buffer[len-1] = '\0';
			this->parse(std::string(buffer));
			write(this->fd, msg_invalid_cmd, sizeof(msg_invalid_cmd));
		}
	}
	this->disconnect();
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
	strncpy(srv_addr.sun_path, this->config->sock_path.c_str(), sizeof(srv_addr.sun_path)-1);
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
				std::cout << "Client connection rejected." << std::endl;
				write(cli_fd, msg_busy, sizeof(msg_busy));
				close(cli_fd);
				continue;
			}
		}

		std::cout << "Client connection accepted." << std::endl;
		pid = fork();
		if (pid == 0) {
			// child process
			Client client(cli_fd);
			client.handle_connection();
		} else {
			// parent process
			close(cli_fd);
		}
	}
}

int main(void) {
	ServerConfig config;
	config.sock_path = "./sdm.sock";
	config.address_spaces_dir = "../data/address_space/";
	config.counters_dir = "../data/counters/";

	Server server;
	server.run(config);
	return 0;
}

