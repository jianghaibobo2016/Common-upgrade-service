#ifndef RCSP_H
#define RCSP_H
#include <iostream>
// using namespace std;
template<typename T>
class SmartPtr;
template<typename T>
class Ptr {
	friend class SmartPtr<T> ;

	T *m_ptr;
	size_t m_count;

	Ptr(T *p = NULL) :
			m_ptr(p), m_count(1) {
	}
	~Ptr() {
		delete m_ptr;
	}
};
template<typename T>
class SmartPtr {
public:
	SmartPtr(T *p = NULL) :
			m_p(new Ptr<T>(p)) {
	}
	SmartPtr(const SmartPtr &sp) :
			m_p(sp.m_p) {
		++m_p->m_count;
	}

	bool operator!=(const SmartPtr &sp) {
		return sp.m_p != this->m_p;
	}
	SmartPtr &operator=(const SmartPtr &sp) {
		++sp.m_p->m_count;
		if (--m_p->m_count == 0) {
			delete m_p;
		}
		m_p = sp.m_p;

		return *this;
	}

	T *operator->() {
		return m_p->m_ptr;
	}
	const T *operator->() const {
		return m_p->m_ptr;
	}

	T operator*() {
		return *m_p->m_ptr;
	}
	T *get() {
		return m_p->m_ptr;
	}
	~SmartPtr() {
		if (--m_p->m_count == 0)
			delete m_p;
	}

private:
	Ptr<T> *m_p;
};
#endif /* RCSP_H */
