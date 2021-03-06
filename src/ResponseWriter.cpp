#include <unistd.h>

#include "EventHandlerInstance.hpp"
#include "ResponseWriter.hpp"
#include "Response.hpp"
#include "Exception.hpp"

ResponseWriter::ResponseWriter(int client_fd)
	: client_fd(client_fd),
	  last_communication(false),
	  cgi(NULL),
	  is_chunked(false)
{}

ResponseWriter::~ResponseWriter()
{}

void
	ResponseWriter::pushResponse(Response &res)
{
	if (res.getHeaders().find("Connection")->second == "close")
		last_communication = true;
	buffer += res.getStartLine();
	buffer += "\r\n";

	for (std::map<std::string, std::string>::iterator iter = res.getHeaders().begin(); iter != res.getHeaders().end(); iter++)
	{
		buffer += iter->first;
		buffer += ": ";
		buffer += iter->second;
		buffer += "\r\n";
	}
	if ((cgi = res.getCGI()) != NULL)
	{
		buffer += "\r\n";
		buffer += res.getBody();
	}
	else if ((resource = res.getResource()) == NULL || resource->getStatus() == Resource::DONE)
	{
		buffer += "Content-Length: " + std::to_string(res.getBody().length()) + "\r\n";
		buffer += "\r\n";
		buffer += res.getBody();
		delete resource;
	}
	else // if (resource->getStatus() != Resource::DONE)
		is_chunked = true;
}

bool
	ResponseWriter::emptyBuffer()
{
	return (this->buffer.empty());
}

bool
	ResponseWriter::writeChunkedResponse(long write_size)
{
	write_size = (write_size < (ssize_t)buffer.size() ? write_size : buffer.size());

	ssize_t	write_bytes = write(client_fd, &buffer[0], write_size);

	if (write_bytes == 0)
		throw SystemCallError("Client's write system call returns 0");
	else if ((size_t)write_bytes == buffer.size())
	{
		buffer.clear();
		EventHandlerInstance::getInstance().disableWriteEvent(client_fd);
		if (resource->getStatus() == Resource::DONE)
		{
			delete resource;
			return (last_communication);
		}
	}
	else
	{
		buffer.erase(0, write_bytes);
		EventHandlerInstance::getInstance().enableWriteEvent(client_fd);
	}
	return (false);
}

bool
	ResponseWriter::writeResponse(long write_size)
{
	write_size = (write_size < (ssize_t)buffer.length() ? write_size : buffer.length());

	ssize_t	write_bytes = write(client_fd, &buffer[0], write_size);

	if (write_bytes == -1)
		// throw SystemCallError("write");
		return (true);
	else if ((size_t)write_bytes == buffer.length())
	{
		buffer.clear();
		return (last_communication);
	}
	else
	{
		buffer.erase(0, write_bytes);
		EventHandlerInstance::getInstance().enableWriteEvent(client_fd);
	}
	return (false);
}

bool
	ResponseWriter::isChunked()
{
	return (is_chunked);
}
