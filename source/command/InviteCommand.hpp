#ifndef INVITECOMMAND_HPP
#define INVITECOMMAND_HPP

#include "Command.hpp"

class InviteCommand: public Command {
	public:
		InviteCommand(const std::vector<std::string> &token_list);
		void	Run(void);
		void	AnyOfError(void);
		void	CheckChannel(const std::string& nick, const std::string& chan);
		void	SetInfo(void);

	private:
		std::string	channel_name_;
		std::string receive_nick_;
		std::string	client_nick_;
		int	receiver_;
};
#endif
