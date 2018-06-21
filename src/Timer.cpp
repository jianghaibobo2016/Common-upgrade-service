/*
 * Timer.cpp
 *
 *  Created on: May 22, 2018
 *      Author: jhb
 */

#include <unistd.h>
#include "Timer.h"

unsigned int Timer::addEvent(unsigned int id, double interval,
		void (*action)(void*args), void *args, bool isRepeat) {
	SchedulerEvent event(id, interval, this->timeline, action, args, isRepeat);
	return this->eventQueue.insertNode(event);
}

void Timer::deleteEvent(unsigned int timerId) {
	this->eventQueue.deleteNode(timerId);
}

void Timer::loopForExecute() {
	SmartPtr<SchedulerEvent> top = this->eventQueue.getTopNode();
	while (top != NULL && top->deadline <= this->timeline) {
		//如果已经到了执行的时间,新开一个子线程执行任务
//		std::thread t(top->action);
//		t.detach(); //子线程分离
		top->action(top->args);
		if (top->isRepeat) {
			//如果是重复事件,则重新添加
			this->addEvent(top->id, top->interval, top->action, top->args,
					top->isRepeat);
		}

		//从堆中删除
		this->eventQueue.deleteTopNode();
		top = this->eventQueue.getTopNode();
	}
	//执行一次后等待一个周期
//	std::this_thread::sleep_for(this->tick);
	usleep(tick);

	//周期增1
	this->timeline++;
}

void Timer::asyncStart() {
	if (!this->isStart) {
		pthread_t tid;
//		cout <<
		pthread_create(&tid, NULL, syncStart, (void*) this);
		pthread_detach(tid);
//		std::thread daemon_thread(&Timer::syncStart, this);
//		daemon_thread.detach(); //从当前主线程分离
	}
}

void *Timer::syncStart(void *args) {
	Timer *time = (Timer *) args;
	if (!time->isStart) {
		while (1) {
			time->loopForExecute();
		}
	}
	return NULL;
}
