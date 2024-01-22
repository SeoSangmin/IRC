#include "Command.hpp"

Command::~Command(void){
}

bool	Command::IsRegistered(const int&fd) {
	this->server_->LockClientMutex(fd);
	bool result = this->client_->IsAuth();
	this->server_->UnlockClientMutex(fd);

	return result;
}

void	Command::SendResponse(const int& sock, const std::string& str) {
	this->server_->LockClientMutex(sock);
	send(sock, str.c_str(), str.size(), 0);
	this->server_->UnlockClientMutex(sock);
}

Command::Command(const std::vector<std::string> &token_list) {
	size_t	param_index;

	if (token_list[0][0] == ':') {
		this->prefix_ = token_list[0];
		this->command_ = token_list[1];
		param_index = 2;
	} else {
		this->command_ = token_list[0];
		param_index = 1;
	}

	for (size_t index = param_index; index < token_list.size(); ++index) {
		this->params_.push_back(token_list[index]);
	}
}

void	Command::set_server(Server *server) {
	this->server_ = server;
}

void	Command::set_client(Client *client) {
	this->client_ = client;
	this->client_sock_ = client->get_sock();
}

Server*	Command::get_server(void) {
	return server_;
}

Client*	Command::get_client(void) {
	return client_;
}

void	Command::DisconnectClient(void) {
	/* Add to Delete list in Server */
	this->server_->AddDeleteClient(this->client_sock_);
}

void	Command::AuthCheckReply(void) {
	log::cout << "Command::AuthCheckReply call()\n";
	Response	auth_message;
	auth_message << RPL_WELCOME;
	char	flag;
	bool	auth_status = false;
	bool	send_status = false;

	this->server_->LockClientMutex(this->client_sock_);//Lock
	auth_status = this->client_->IsAuth();
	flag = this->client_->get_auth_flag(FT_AUTH_ALL);
	if (auth_status == false && ((flag & FT_AUTH_ALL) == FT_AUTH_ALL)) {
		send_status = true;
		this->client_->SetAuthFlag(FT_AUTH);
	}
	this->server_->UnlockClientMutex(this->client_sock_);//Unlock
	
	if (send_status) {
		SendResponse(this->client_sock_, auth_message.get_format_str());
		log::cout << RED << "send 001\n" << RESET;
	}
}
