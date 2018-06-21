/*
 * Timer.h
 *
 *  Created on: May 22, 2018
 *      Author: jhb
 */

#ifndef INCLUDE_TIMER_H_
#define INCLUDE_TIMER_H_

/**
 * 定时器的实现
 * 支持int setTimer(T interval,function action):设置一个定时器，指定间隔interval和回调函数action,返回定时器id
 * 支持void deleteTimer(int timerId):删除一个定时器
 * 数据结构:最小堆模型，按照定时器触发的时间排序
 */
#include <iostream>
#include <pthread.h>
#include "SortedHeap.hpp"

class Timer {
private:
//	std::chrono::milliseconds tick;
	unsigned long tick;
	double timeline; //当前时间线,long double的字节数为12
	bool isStart;    //标志当前定时器的启动状态
	struct SchedulerEvent {
		unsigned int id;              //定时事件的唯一标示id
		double interval;              //事件的触发间隔，在重复事件中会用到这个属性
		double deadline;              //定时事件的触发时间
		void (*action)(void*args);
		void *args;
		bool isRepeat;                //是否是重复执行事件
		SchedulerEvent(unsigned int id, double interval, double timeline,
				void (*action)(void*args), void *args, bool isRepeat) {
			this->id = id;
			this->interval = interval;
			this->deadline = interval + timeline;
			this->action = action;
			this->args = args;
			this->isRepeat = isRepeat;
		}
	};

	/**
	 * 执行到达期限的定时器
	 */
	void loopForExecute();

	static inline bool Campare(SchedulerEvent &a, SchedulerEvent &b) {
		return a.deadline < b.deadline;
	}

public:

	SortedHeap<SchedulerEvent> eventQueue;

//	Timer(const Timer &timer) :
//			eventQueue(timer.eventQueue), timeline(timer.timeline), tick(
//					timer.tick), isStart(timer.isStart) {
//	}

	const unsigned long getTick() const {
		return tick;
	}

	void setTick(const unsigned long m_tick) {
		tick = m_tick;
	}

	Timer(const unsigned long tick) :
			eventQueue(Campare) {
		this->timeline = 0;
		this->tick = tick;
		this->isStart = false;
	}
//	static Timer *getInstance(unsigned long tick) {
//		static Timer timer(tick);
//		return &timer;
//	}

	/**
	 * 设置定时器
	 * @param interval 定时间隔
	 * @param action 定时执行的动作
	 * @param isRepeat 是否重复执行,默认不重复执行
	 * @return unsigned int 定时器的id,可以根据这个id执行删除操作
	 */
	unsigned int addEvent(unsigned int id, double interval,
			void (*action)(void*args), void *args, bool isRepeat = false);

//	virtual void OnTimer(void) = 0;

	/**
	 * 删除定时器
	 * @param timerId 定时器id
	 *
	 */
	void deleteEvent(unsigned int timerId);

	/**
	 * 同步执行启动定时器
	 */
	static void *syncStart(void *);

	/**
	 * 异步执行启动定时器
	 */
	void asyncStart();
	virtual ~Timer() {
	}
};

#endif /* INCLUDE_TIMER_H_ */
