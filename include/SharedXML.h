/*
 * SharedXML.h
 *
 *  Created on: Mar 23, 2018
 *      Author: jhb
 */

#ifndef SHAREDXML_H_
#define SHAREDXML_H_

#include <iostream>
class SharedXML {
public:
	SharedXML():_ref(0){

	}
	SharedXML(const SharedXML&):_ref(0){

	}
	virtual ~SharedXML(){

	}
	SharedXML& operator=(const SharedXML&) {
		return *this;
	}
	void __incRef(){
		// AutoLock lock(&_mutex);
		_ref++;
	}
	void __decRef(){
		{
			// AutoLock lock(&_mutex);
			_ref--;
		}

		if(_ref<=0){
			delete this;
			//::std::cout<<"delete sharedXML object"<<::std::endl;
		}
	}
	int  __getRef()const{
		return _ref;
	}
private:
	// Mutex _mutex;
	int _ref;
};




#endif /* SHAREDXML_H_ */
