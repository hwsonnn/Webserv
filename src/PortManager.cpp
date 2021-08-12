#include <sys/socket.h>
#include <arpa/inet.h>

#include "EventHandlerInstance.hpp"
#include "Exception.hpp"
#include "PortManager.hpp"
#include "Client.hpp"

/* static */

static int
	openSocket(int port)
{
	struct sockaddr_in	server_addr;
	int					server_socket;

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);

	if ((server_socket = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		throw SystemCallError("socket");
	if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
		throw SystemCallError("bind");
	if (listen(server_socket, PORT_BACKLOG) == -1)
		throw SystemCallError("listen");

	return (server_socket);
}

/* public */

PortManager::PortManager(int port) : FDManager(openSocket(port))
{}

void
	PortManager::addServer(std::string server_name, Server *server)
{
	servers.push_back(std::pair<std::string, Server*>(server_name, server));
}

void
	PortManager::readEvent()
{
	Client	*client = new Client(this);

	EventHandlerInstance::getInstance().enableReadEvent(client->getFD());
}

void
	PortManager::writeEvent()
{
	throw UnexceptedEventOccured("PortManager write event");
}

void
	PortManager::timerEvent()
{
	throw UnexceptedEventOccured("PortManager timer event");
}

/* priavte */
