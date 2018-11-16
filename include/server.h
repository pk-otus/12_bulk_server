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

					while (!q.empty())
					{
						q.front()->log_all();
						q.pop();						
					}
					lock.unlock();
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

						if (cmd.empty()) continue;
						
						if (1 == cmd.size())
						{
							switch (cmd[0])
							{
							case '{':
								if (!handler_bracketed.inside_brackets())
								{
									std::lock_guard<std::mutex> lock(guard_mutex);
									needs_notify = flush_fixed();
								}
								handler_bracketed.open_bracket();
								continue;
							case '}':
								if (handler_bracketed.inside_brackets())
								{
									handler_bracketed.close_bracket();
									if (!handler_bracketed.inside_brackets())
									{
										needs_notify = flush_bracketed(handler_bracketed);
									}
								}
								continue;
							}								
						}
						
						if (handler_bracketed.inside_brackets())
						{
							handler_bracketed.add_command(cmd);
						}
						else								
						{							
							std::lock_guard<std::mutex> lock(guard_mutex);
							if (handler_fixed.add_command(cmd))
							{
								needs_notify = flush_fixed();
							}																
						}										
					}
					if (needs_notify)
					{
						cond_var.notify_one();
					}

				}
				catch (const std::exception &e) 
				{
					std::cout << "producer thread error: " << e.what() << '\n';
					break;
				}
			}
		}

		bool flush_fixed()
		{			
			if (auto cmd = handler_fixed.flush_data())
			{				
				q.push(std::move(cmd));
				return true;
			}
			return false;
		}

		bool flush_bracketed(bracketed_handler& handler)
		{				
			if (auto cmd = handler.flush_data())
			{				
				std::lock_guard<std::mutex> lock(guard_mutex);
				q.push(std::move(cmd));
				return true;
			}
			return false;
		}

		ba::io_service			service;
		ba::ip::tcp::acceptor	acc;

		size_t					connection_counter;
		fixed_size_handler		handler_fixed;

		std::mutex				guard_mutex;
		std::condition_variable	cond_var;
		std::queue<std::unique_ptr<commands_block>> q;

	};
}
