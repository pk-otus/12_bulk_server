#pragma once
#include <memory>
#include <ctime>

#include "commands_block.h"

namespace bulk_server
{
	class basic_handler
	{
	public:
		explicit basic_handler(size_t instance_id) :
			instance_suffix(instance_id),
			block_suffix(0)
		{ }

		virtual ~basic_handler()
		{
			if (commands)
				commands->final_flush();		
		}

		void handle_command(const std::string& cmd)
		{
			if (!commands)
				commands = create_command_block(generate_file_name());

			commands->add_command(cmd);
			if (commands->is_full())
				flush();
		}
	protected:
		void flush()
		{
			if (commands)
			{
				commands->log_all();
				commands = nullptr;
			}
		}
	private:
		virtual std::unique_ptr<commands_block> create_command_block(const std::string& fname) = 0;
		
		std::string generate_file_name()
		{
			return "bulk" + std::to_string(time(nullptr)) +
				'_' + std::to_string(instance_suffix) +
				'_' + std::to_string(block_suffix++) + ".log";
		}

		const size_t	instance_suffix;
		size_t			block_suffix;

		std::unique_ptr<commands_block>	commands;
	};

	class fixed_size_handler : public basic_handler
	{
	public:
		fixed_size_handler(size_t instance_id, size_t num_commands) :
			basic_handler	(instance_id),			
			sz_fixed_buffer	(num_commands)
			
		{	}
		
	protected:
		std::unique_ptr<commands_block> create_command_block(const std::string& fname) override
		{
			return std::unique_ptr<commands_block>(new limited_commands_block(fname, sz_fixed_buffer));
		}

	private:
		const size_t sz_fixed_buffer;					
	};

	class bracketed_handler : public basic_handler
	{
	public:
		bracketed_handler(size_t instance_id) :
			basic_handler	(instance_id),
			count_brackets	(0)
		{ }

		bool inside_brackets() const { return 0 != count_brackets; }

		bool try_handle_special(char ch)
		{
			if ('{' == ch)
			{
				if (!inside_brackets()) flush();
				++count_brackets;
				return true;
			}
			if ('}' == ch)
			{
				if (inside_brackets())
				{
					--count_brackets;
					if (!inside_brackets()) flush();
				}
				return true;
			}
			return false;
		}

	private:
		std::unique_ptr<commands_block> create_command_block(const std::string& fname) override
		{
			if (inside_brackets())
				return std::make_unique<commands_block>(fname);

			throw std::logic_error("creating command block not inside brackets");
		}

		size_t			count_brackets;
	};


}
