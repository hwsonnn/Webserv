#ifndef SERVER_HPP
# define SERVER_HPP

# include <map>
# include <string>

# ifdef __APPLE__
#  include <sys/event.h>
# elif __linux__
#  include <kqueue/sys/event.h>
# endif
# include <sys/stat.h>
# include <dirent.h>
# include <time.h>
# include <fcntl.h>
# include <unistd.h>

class	Request;

# include "Exception.hpp"
# include "Response.hpp"
# include "Location.hpp"
# include "Dialogue.hpp"
# include "Resource.hpp"

class Server
{
public:
	Server();
	~Server();

	//config parsing
	void		addLocation(std::string path, Location* loc); // Success : 0, Fail : 1
	void		addErrorPage(int error_code, std::string page_path); // Success : 0, Fail : 1

	void		setBodyLimit(int limit);
	void		setAutoindex(std::string on_off_string);
	void		setReturnInfo(int code, std::string uri);


	//make Response
	void		makeErrorResponse(Dialogue *, Location *location, size_t);
	void		makeReturnResponse(Dialogue *, Location *location, size_t);
	std::string	makeAutoIndexPage(std::string path, std::string uri, Location *location);
	void		makeGETResponse(Dialogue *, Location *, std::string);
	void		makePOSTResponse(Dialogue *, Location *, std::string);
	void		makeDELETEResponse(Dialogue *, Location *, std::string);

	void		makeGeneralHeaders(Dialogue *dial);
	std::string	makeHTMLPage(std::string body);
	std::string	generateErrorPage(size_t error_code);

	int							checkPath(std::string path);
	int							deleteDirectory(std::string path);

	std::string					dateHeader();
	std::string					lastModifiedHeader();
	std::string					connectionHeader(Request &req);
	std::string					contentTypeHeader(std::string extension);
	static std::string			statusMessage(size_t code);

	//getter
	Location					*getLocation(std::string uri);
	unsigned int				getBodyLimit();
	std::map<int, std::string>	getErrorPages();

private:
	enum FileType { NotFound, File, Directory };

	Server(const Server& s);

	Server& operator= (const Server& s);

	std::map<std::string, Location*>	locations;
	std::map<int, std::string>			error_page;

	bool								auto_index;
	int									body_limit;
};

#endif
