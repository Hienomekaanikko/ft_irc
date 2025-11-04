#include <iostream>
#include <cstring>
#include <cstdlib>
#include <vector>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>

#define PORT 6667
#define BUFFER_SIZE 1024

int main()
{
	// 1. Create socket fd
	int server_fd;
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Socket failed");
		return EXIT_FAILURE;
	}

	// 2. Set socket options: SO_REUSEADDR
	int opt = 1;

	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		perror("setsockopt SO_REUSEADDR");
		close(server_fd);
		return EXIT_FAILURE;
	}

	// 3. Define the server addr and bind address (IPv4, any interface, port 6667)
	struct sockaddr_in address;
	std::memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	// Bind the socket to the specified port
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("Bind");
		close(server_fd);
		return EXIT_FAILURE;
	}

	// Start listening for incoming connections
	if (listen(server_fd, 10) < 0)
	{
		perror("Listen");
		close(server_fd);
		return EXIT_FAILURE;
	}

	std::cout << "IRC server is running on port " << PORT << "..." << std::endl;

	// 5. Prepare pollfd list
	std::vector<pollfd> fds;
	pollfd server_poll_fd;
	server_poll_fd.fd = server_fd;
	server_poll_fd.events = POLLIN;
	server_poll_fd.revents = 0;
	fds.push_back(server_poll_fd);

	// 6. Main event loop
	char buffer[BUFFER_SIZE] = {0};
	while (true)
	{
		int ready = poll(fds.data(), fds.size(), -1); // -1 = wait forever
		if (ready < 0)
		{
			perror("poll");
			break;
		}
		// Loop over all fds and handle those that have events
		for (std::size_t i = 0; i < fds.size() && ready > 0; ++i)
		{
			if (!(fds[i].revents & POLLIN))
				continue;
			--ready;
			if (fds[i].fd == server_fd)
			{
				// 6a. New incoming connection on the listening socket
				socklen_t addrlen = sizeof(address);
				int client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
				if (client_fd < 0)
				{
					perror("accept");
					continue;
				}
				std::cout << "New client connected, fd = " << client_fd << std::endl;

				pollfd client_poll_fd;
				client_poll_fd.fd = client_fd;
				client_poll_fd.events = POLLIN;
				client_poll_fd.revents = 0;
				fds.push_back(client_poll_fd);
			}
			else
			{
				// 6b. Data from existing client
				int client_fd = fds[i].fd;
				std::memset(buffer, 0, BUFFER_SIZE);
				ssize_t bytes_read = recv(client_fd, buffer, BUFFER_SIZE, 0);

				if (bytes_read <= 0)
				{
					// client closed connection or error
					if (bytes_read < 0)
						perror("recv");
					std::cout << "Client disconnected, fd = " << client_fd << std::endl;
					close(client_fd);

					// Remove this fd from the vector (swap with last, pop_back, adjust index)
					fds[i] = fds.back();
					fds.pop_back();
					--i;
				}
				else
				{
					// Echo back to client
					std::cout << "Received from fd " << client_fd << ": " << std::string(buffer, bytes_read);
					send(client_fd, buffer, bytes_read, 0);
				}

			}
		}
	}
	for (std::size_t i = 0; i < fds.size(); ++i)
		close(fds[i].fd);
	return 0;
}
