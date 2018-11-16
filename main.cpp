#include "include/server.h"

int main(int argc, char const *argv[])
{
	if (argc < 3)
	{
		std::cout << "Usage: bulk_server <server port> <bulk size>\n";
		return 1;
	}

	try
	{
		size_t port = 9999, sz = 3;
		try
		{
			port = std::stoi(argv[1]);
		}
		catch (std::exception& e)
		{
			throw std::invalid_argument("cannot parse server port");
		}

		try
		{
			sz = std::stoi(argv[2]);
		}
		catch (std::exception& e)
		{
			throw std::invalid_argument("cannot bulk size");
		}
				
		bulk_server::server srv(port, sz);

		while (true)
		{
			srv.accept_connection();
		}
	}
	catch (std::exception& e)
	{
		std::cout << "main thread error: " << e.what() << '\n';
	}
	return 0;
}
