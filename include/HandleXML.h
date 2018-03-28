/*
 * HandleXML.h
 *
 *  Created on: Mar 23, 2018
 *      Author: jhb
 */

#ifndef HANDLEXML_H_
#define HANDLEXML_H_


template<typename T>
class HandleXML {
public:

	HandleXML(T* p = 0) {
		this->_ptr = p;

		if (this->_ptr) {
			this->_ptr->__incRef();
		}
	}

	template<typename Y>
	HandleXML(const HandleXML<Y>& r) {
		this->_ptr = r._ptr;

		if (this->_ptr) {
			this->_ptr->__incRef();
		}
	}

	HandleXML(const HandleXML& r) {
		this->_ptr = r._ptr;

		if (this->_ptr) {
			this->_ptr->__incRef();
		}
	}

	~HandleXML() {
		if (this->_ptr) {
			this->_ptr->__decRef();
		}
	}

	HandleXML& operator=(T* p) {
		if (this->_ptr != p) {
			if (p) {
				p->__incRef();
			}

			T* ptr = this->_ptr;
			this->_ptr = p;

			if (ptr) {
				ptr->__decRef();
			}
		}
		return *this;
	}

	template<typename Y>
	HandleXML& operator=(const HandleXML<Y>& r) {
		if (this->_ptr != r._ptr) {
			if (r._ptr) {
				r._ptr->__incRef();
			}

			T* ptr = this->_ptr;
			this->_ptr = r._ptr;

			if (ptr) {
				ptr->__decRef();
			}
		}
		return *this;
	}

	HandleXML& operator=(const HandleXML& r) {
		if (this->_ptr != r._ptr) {
			if (r._ptr) {
				r._ptr->__incRef();
			}

			T* ptr = this->_ptr;
			this->_ptr = r._ptr;

			if (ptr) {
				ptr->__decRef();
			}
		}
		return *this;
	}
	T* get() const {
		return _ptr;
	}

	T* operator->() const {
		if (!_ptr) {
			// throw SystemException(string("HandleXML -> nullptr."));
		}

		return _ptr;
	}

	T& operator*() const {
		if (!_ptr) {
			// throw SystemException(string("HandleXML operator*nullptr."));
		}
		return *_ptr;
	}

	operator bool() const {
		return _ptr ? true : false;
	}
	template<class Y>
	static HandleXML dynamicCast(Y* p) {
		return HandleXML(dynamic_cast<T*>(p));
	}
private:
	T* _ptr;
};



#endif /* HANDLEXML_H_ */
