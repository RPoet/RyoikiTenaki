#pragma once
#include "Definitions.h"

template<class T>
class Singleton
{
public:
	static T& Get()
	{
		static T GlobalObject{};
		return GlobalObject;
	}
};
