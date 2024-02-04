#ifndef PRIVMSGCOMMAND_HPP
#define PRIVMSGCOMMAND_HPP

#include "Command.hpp"

class PrivmsgCommand: public Command {
	public:
		PrivmsgCommand(const std::vector<std::string> &token_list);
		void	Run(void);
		void	AnyOfError(void);
		void	CheckTarget(void);
		void	UniCast(const std::string& client_name, const std::string& text);
		void	BroadCast(const std::string& channel_name, const std::string& text);
};
#endif
