#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <iterator>
#include <iostream>

namespace bulk_server
{
	class commands_block
	{
	public:
		commands_block(const std::string& fname) : file_name(fname) { }

		commands_block(const commands_block&) = delete;
		commands_block& operator=	(const commands_block&) = delete;

		virtual ~commands_block() = default;

		void log_all() const
		{
			auto str_result = get_string();
			std::cout << str_result << std::endl;

			auto stream = std::ofstream(file_name, std::ofstream::out);
			stream << str_result;
			stream.close();
		}

		virtual void final_flush() const noexcept { }
		virtual bool is_full() const noexcept { return false; }

		void add_command(const std::string& str)
		{
			pool_commands.emplace_back(str);
		}
	protected:
		size_t commands_count() const noexcept { return pool_commands.size(); }

	private:
		std::string get_string() const
		{
			std::string result;
			if (!pool_commands.empty())
			{
				const std::string DELIMITER = ", ";
				result += "bulk: ";
				for (const auto& item : pool_commands)
				{
					result += item + DELIMITER;
				}
				result = result.substr(0, result.size() - DELIMITER.size());
			}
			return result;
		}

		const std::string			file_name;
		std::vector<std::string>	pool_commands;
	};

	class limited_commands_block : public commands_block
	{
	public:
		explicit limited_commands_block(const std::string& fname, size_t sz) :
					commands_block(fname),
					sz_fixed_buffer(sz) { }

		void final_flush() const noexcept override
		{
			log_all();
		}

		bool is_full() const noexcept override
		{
			return sz_fixed_buffer == commands_count();
		}

	private:
		const size_t				sz_fixed_buffer;
	};
}