#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <queue>
#include "Request.hpp"

class Command;
class ThreadPool
{
	public:
		std::vector<pthread_t>	threads_;
		std::queue<Command*> 	queue_;
		pthread_mutex_t			lock_;
		pthread_cond_t			notify_;
		int						thread_count_;
		int						count_;
		bool					shutdown_;

		ThreadPool(void);
		ThreadPool(int size);
		~ThreadPool();

		void		Enqueue(void *arg);
		static void	*Worker(void *arg);
};
#endif