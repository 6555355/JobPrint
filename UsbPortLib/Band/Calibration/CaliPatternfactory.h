/*!
  ******************************************************************************
  * @file    patternfactory.h
  * @author  EastSun    
  * @version V1.0.0
  * @date    2018/08/11 14:08
  * @brief   Cali Pattern Class factory.
  ******************************************************************************
  * @copy
  *
  * 
  *
  * <h2><center>&copy; COPYRIGHT 2017 BeiJing BYHX </center></h2>
  */ 
  


#ifndef PATTERN_FACTORY_H
#define PATTERN_FACTORY_H

#include <string>
#include <map>
using namespace std;

typedef void* (*Constructor)();

class CCaliPatternFactory
{
public:
	static void registerClass(int classID, string className, Constructor constructor)
	{
		strTbl()[classID] = className;
		registerClass(className, constructor);
	}
	static void registerClass(string className, Constructor constructor)
	{
		constructors()[className] = constructor;
	}

	static void* CreateObject(const int classID)
	{

		if (strTbl().find(classID) != strTbl().end())
			return CreateObject(strTbl().find(classID)->second);

		return NULL;

	}

	static void* CreateObject(const string& className)
	{
		Constructor constructor = NULL;

		if (constructors().find(className) != constructors().end())
			constructor = constructors().find(className)->second;

		if (constructor == NULL)
			return NULL;

		return (*constructor)();
	}

private:
	inline static map<int, string>& strTbl()
	{
		static map<int, string> tbl;
		return tbl;
	};
	inline static map<string, Constructor>& constructors()
	{
		static map<string, Constructor> instance;
		return instance;
	}
};


#define REGISTER_CLASS(classID,class_name) \
class class_name##Helper{ \
public: \
	class_name##Helper() \
{ \
	CCaliPatternFactory::registerClass(classID,#class_name, class_name##Helper::creatObjFunc); \
} \
	static void* creatObjFunc() \
{ \
	return new class_name; \
} \
}; \
	class_name##Helper class_name##helper;


#endif