#ifndef ARGUMENTS_EX32_H
#define ARGUMENTS_EX32_H

#include <map>
#include <string>
#include <typeinfo>


#define __ARG_VAL(key,type,obj) (*(type*)(obj.get(key)))
#define __ARG_PTR(key,type,obj) ((type*)(obj.get(key)))
class Arguments{
	public:
		Arguments();
		void set(std::string, void*);
		void* get(std::string);
	private:
		std::map<std::string, void*> args_;
		// std::map<std::string, std::type_info> dess_;
};


#endif