#pragma once

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
