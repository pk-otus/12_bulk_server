#pragma once
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>

#include <boost/asio.hpp>

#include "command_handler.h"

namespace bulk_server
{
	namespace ba = boost::asio;

	class server
	{
	public:
		server(uint16_t port, size_t sz) :
			acc					(service, ba::ip::tcp::endpoint(ba::ip::tcp::v4(), port)),
			connection_counter	(0),
			handler_fixed		(connection_counter, sz)
		{
			std::thread(&server::consumer_thread, this).detach();
		}

		void accept_connection()
		{
			auto sock = std::make_shared<ba::ip::tcp::socket>(service);
			acc.accept(*sock);
			std::thread(&server::producer_thread, this, ++connection_counter, sock).detach();
		}

	private:

		void consumer_thread()
		{
			while (true)
			{
				try
				{
					std::unique_lock<std::mutex> lock(guard_mutex);
					cond_var.wait(lock, [this]() { return !q.empty(); });

					auto cmd = q.front();
					q.pop();
					lock.unlock();

					handler_fixed.handle_command(cmd);
				}
				catch (const std::exception &e) 
				{
					std::cout << "consumer thread error: " << e.what() << '\n';
					break;
				}

			}
		}

		void producer_thread(size_t handler_id, std::shared_ptr<ba::ip::tcp::socket> sock)
		{
			boost::system::error_code ec;

			bracketed_handler handler_bracketed(handler_id);
			std::string	unhandled;

			while (true)
			{
				try 
				{
					bool needs_notify = false;

					ba::streambuf buf;
					read_until(*sock, buf, '\n', ec);

					if (ec && boost::asio::error::eof != ec)
						throw std::runtime_error("socket read error");

					unhandled += std::string(std::istreambuf_iterator<char>(&buf),
						std::istreambuf_iterator<char>());

					auto pos_end = unhandled.find('\n');
					while (std::string::npos != pos_end)
					{
						auto cmd = unhandled.substr(0, pos_end);
						unhandled = unhandled.substr(pos_end + 1);
						pos_end = unhandled.find('\n');

						if (!cmd.empty())
						{
							if (1 == cmd.size() && handler_bracketed.try_handle_special(cmd[0]))
								continue;

							if (handler_bracketed.inside_brackets())
							{
								handler_bracketed.handle_command(cmd);
							}
							else								
							{
								std::lock_guard<std::mutex> lock(guard_mutex);
								q.push(cmd);
								needs_notify = true;
							}
						}						
					}
					if (needs_notify)
						cond_var.notify_one();

				}
				catch (const std::exception &e) 
				{
					std::cout << "producer thread error: " << e.what() << '\n';
					break;
				}
			}
		}
		ba::io_service			service;
		ba::ip::tcp::acceptor	acc;

		size_t					connection_counter;
		fixed_size_handler		handler_fixed;

		std::mutex				guard_mutex;
		std::condition_variable	cond_var;
		std::queue<std::string> q;

	};
}
