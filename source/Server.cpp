#include "Server.hpp"

Server::Server(int argc, char **argv)
{
	if (argc != 3)
	{
		std::string error_message;
		std::string program_name = argv[0];
		error_message = "Usage: " + program_name + " <port> <password>\n";
		error_handling(error_message);
	}

	port = atoi(argv[1]);
	password = argv[2];
	server_socket_init();
	kqueue_init();
}

void	Server::server_socket_init(void)
{
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		error_handling("socket() error\n");

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1)
		error_handling("socket bind() error\n");
	if (listen(sock, FT_SOCK_QUEUE_SIZE) == -1)
		error_handling("socket listen() error\n");
	fcntl(sock, F_SETFL, O_NONBLOCK);
}

void	Server::kqueue_init(void)
{
	if ((kq = kqueue()) == -1)
		error_handling("kqueue() error\n");
	struct kevent	server_event;
	EV_SET(&server_event, sock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	timeout.tv_sec = FT_TIMEOUT_SEC;
	timeout.tv_nsec = FT_TIMEOUT_NSEC;
}

bool	Server::run(void)
{
	// main loop of ircserv with kqueue
	int nev;
	while (true)
	{
		nev = kevent(kq, &(chlist[0]), chlist.size(), evlist, FT_KQ_EVENT_SIZE, &timeout);
		chlist.clear();//why should I write this line?
		if (nev == -1)
			error_handling("kevent() error\n");
		else if (nev == 0)
			handle_timeout();
		else if (nev > 0)
			handle_events(nev);
	}
	return true;
}

void	Server::handle_events(int nev)
{
	struct kevent	event;
	for (int i = 0; i < nev; ++i)
	{
		event = evlist[i];
		print_event(&event, i);
		if (event.flags & EV_ERROR)
			handle_event_error(event);
		else if (event.flags & EV_EOF)
			disconnect_client(event);//need to implement
		else if (event.filter == EVFILT_READ && event.ident == (uintptr_t)sock)
			connect_client();
		else if (event.filter == EVFILT_READ)
			handle_client_event(event);
		else if (event.filter == EVFILT_WRITE)
		{
			/* handle write event ... */
		}
	}
}

void	Server::handle_client_event(struct kevent event)
{
	std::map<int, Client>::iterator	it = clients.find(event.ident);
	if (it != clients.end())
	{
		char buff[FT_BUFF_SIZE];
		int n = read(event.ident, buff, sizeof(buff));
		if (n == -1)
			std::cerr << "client read error\n";
		else if (n == 0)
			disconnect_client(event);
		else
		{
			buff[n] = 0;
			clients[event.ident].buffer += buff;
			std::cout << "received data from " << event.ident << ": " << clients[event.ident].buffer << "\n";
		}
	}
}

void	Server::connect_client(void)
{
	Client	client;
	if ((client.sock = accept(sock, (struct sockaddr*)&client.addr, &client.addr_size)) == -1)
		error_handling("accept() error\n");

	/* Authenticate client password */
	if (!authenticate_client(client))
	{
		/* handle when the client connection rejected */
		std::cout << "authentification failed with: " << client.sock << "\n";
		close(client.sock);
	}
	else
	{
		/* handle new client */
		std::cout << "accent new client: " << client.sock << "\n";
		add_event(client.sock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
		add_event(client.sock, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
		clients[client.sock] = client;
	}
}

bool	Server::authenticate_client(Client& client)
{
	// client가 보낸 메시지를 확인한다.
	//		1. client가 보낸 password 와 Server의 password 의 일치
	//		2. client가 보낸 nick이 기존 clients의 nick과 겹치지 않아야함.
	//	1, 2 조건을 만족하는 client에 한해서 참을 반환.
	std::cout << "client authenticate call: " << client.sock << "\n";
	return true;
}

void	Server::add_event(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata)
{
	struct kevent	event;
	EV_SET(&event, ident, filter, flags, fflags, data, udata);
	chlist.push_back(event);
}

void	Server::handle_event_error(struct kevent event)
{
	if (event.ident == (uintptr_t)this->sock)
		error_handling("server socket event error\n");
	else
	{
		std::cerr << "client socket event error\n";
		disconnect_client(event);//need to implement fucntion
	}
}

void	Server::disconnect_client(struct kevent event)
{
	struct kevent delete_event;
	EV_SET(&delete_event, event.ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	kevent(kq, &delete_event, 1, NULL, 0, NULL);
	close(event.ident);
	clients.erase(event.ident);
	/* channel.. clients... etc */
}

void	Server::handle_timeout(void)
{
	/* handle timeout */
	std::cout << "time out!\n";
}

/* wooseoki functions */
void	Server::print_event(struct kevent *event, int i)
{
	std::cout << "=============================\n";
	std::cout << "index : " << i << "\n";
	std::cout << "ident : " << event->ident << "\n";
	std::cout << "flags : " << event->flags << "\n";
	std::cout << "filter : " << event->filter << "\n";
	std::cout << "=============================\n";
}