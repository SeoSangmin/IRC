#include "TopicCommand.hpp"

TopicCommand::TopicCommand(const std::vector<std::string> &token_list) : Command(token_list) {
}

void	TopicCommand::NoticeTopic(Channel* c, const std::string& topic) {
	std::string	nick = this->server_->SearchClientBySock(this->client_sock_);
	std::string notice = ":" + nick + " TOPIC " + c->get_name() + " :" + topic + CRLF;
	const std::map<int, char>	&member_list = c->get_members();

	this->is_success_ = true;
	for (std::map<int, char>::const_iterator it = member_list.begin(); it != member_list.end(); ++it) {
		SendResponse(it->first, notice);
	}
}

void	TopicCommand::CheckChannel(const std::string& channel_name, const std::string& topic) {
	std::map<std::string, Channel> *channel_list;
	std::map<std::string, Channel>::iterator chan;

	this->server_->LockChannelListMutex();
	channel_list = &(this->server_->get_channels());
	chan = channel_list->find(channel_name);
	if (chan == channel_list->end()) {
		this->server_->UnlockChannelListMutex();
		this->resp_ = this->resp_ + ERR_NOSUCHCHANNEL + " :No such channel.";
	}
	this->server_->UnlockChannelListMutex();

	this->server_->LockChannelMutex(chan->first);
	if ((chan->second).IsMember(this->client_sock_) == false)
		this->resp_ = this->resp_ + ERR_NOTONCHANNEL + " " + channel_name + " :You're not on that channel.";
	else if (((chan->second).get_mode() & MODE_TOPIC)) {
		if ((chan->second).IsOperator(this->client_sock_) == false)
			this->resp_ = this->resp_ + ERR_CHANOPRIVSNEEDED + " " + channel_name + " :You're not channel operator";
		else {
			chan->second.set_mode(MODE_TOPIC, true);
			chan->second.set_topic(topic);
			NoticeTopic(&(chan->second), topic);
		}
	}
	else {
		chan->second.set_mode(MODE_TOPIC, true);
		chan->second.set_topic(topic);
		NoticeTopic(&(chan->second), topic);
	}
	this->server_->UnlockChannelMutex(chan->first);
}

void	TopicCommand::AnyOfError(void) {
	if (this->params_.empty())
		this->resp_ = this->resp_ + ERR_NEEDMOREPARAMS + " :Not enough params";
	else if (this->params_.size() == 1)
		this->resp_ = this->resp_ + RPL_NOTOPIC + " :No topic is set";
	else
		CheckChannel(this->params_[0], this->params_[1]);
}

void	TopicCommand::Run(void) {
	try {
		AnyOfError();
		if (this->is_success_ == false) 
			SendResponse(this->client_sock_, this->resp_.get_format_str());
	} catch(std::exception& e) {
		log::cout << BOLDRED << e.what() << RESET << "\n";
	}
}
