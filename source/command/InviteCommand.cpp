#include "InviteCommand.hpp"

InviteCommand::InviteCommand(const std::vector<std::string> &token_list) : Command(token_list) {
}

void	InviteCommand::SetInfo(void) {
	this->client_nick_ = this->server_->SearchClientBySock(this->client_sock_);
	this->receiver_ = this->server_->SearchClientByNick(this->params_[0]);
	this->receive_nick_ = this->params_[0];
	this->channel_name_ = this->params_[1];
}

void	InviteCommand::CheckChannel(const std::string& nick, const std::string& channel_name) {
	std::map<std::string, Channel> *channel_list;
	std::map<std::string, Channel>::iterator chan;

	this->server_->LockChannelListMutex();
	channel_list = &(this->server_->get_channels());
	chan = channel_list->find(channel_name);
	if (chan == channel_list->end()) {
		this->server_->UnlockChannelListMutex();
		this->resp_ = this->resp_ + ERR_NOSUCHCHANNEL + " " + channel_name + " :No such channel.";
		return ;
	}
	this->server_->UnlockChannelListMutex();

	this->server_->LockChannelMutex(chan->first);
	if ((chan->second).IsMember(this->client_sock_) == false)
		this->resp_ = this->resp_ + ERR_NOTONCHANNEL + " " + channel_name + " :You're not on that channel";
	else if ((chan->second).IsOperator(this->client_sock_) == false)
		this->resp_ = this->resp_ + ERR_CHANOPRIVSNEEDED + " " + channel_name + " :You're not channel operator";
	else if ((chan->second).IsMember(this->receiver_) == true)
		this->resp_ = this->resp_ + ERR_USERONCHANNEL + " " + nick +" " + channel_name + " :is already on cahnnel";
	else {
		this->is_success_ = true;
		(chan->second).Invite(this->receiver_);
	}
	this->server_->UnlockChannelMutex(chan->first);
}

void	InviteCommand::AnyOfError(void) {
	if (Command::IsRegistered(this->client_sock_) == false)
		this->resp_ = this->resp_ + ERR_NOTREGISTERED + " :You have not registered";
	else if (this->params_.size() < 2)
		this->resp_ = this->resp_ + ERR_NEEDMOREPARAMS + " Invite :Not enough parameters";
	else {
		SetInfo();
		if (this->receiver_ == FT_INIT_CLIENT_FD)
			this->resp_ = this->resp_ + ERR_NOSUCHNICK + " " + this->receive_nick_ + " :No such user";
		else
			CheckChannel(this->params_[0], this->params_[1]);
	}
}

void	InviteCommand::Run() {
	try {
		AnyOfError();
		if (this->is_success_ == false)
			SendResponse(this->client_sock_, this->resp_.get_format_str());
		else {
			this->resp_ = this->resp_ + ":" + this->client_nick_ +  " INVITE " + this->receive_nick_ + " " + this->channel_name_;
			SendResponse(this->receiver_, this->resp_.get_format_str());
			this->resp_ = this->resp_ + ":" + this->client_nick_ + " " + RPL_INVITING + " " + this->receive_nick_ + " " + this->channel_name_;
			SendResponse(this->client_sock_, this->resp_.get_format_str());
		}
	} catch (std::exception& e) {
		log::cout << BOLDRED << e.what() << RESET << "\n";
	}
}

