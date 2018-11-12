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
		size_t port = std::stoi(argv[1]);
		size_t sz = std::stoi(argv[2]);

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
