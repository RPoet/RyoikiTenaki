#pragma once
#include "PlatformDefinitions.h"

using int32 = int;
using uint32 = unsigned int;

using int16 = short;
using uint16 = unsigned short;

using int8 = char;
using uint8 = unsigned char;

#define CLASS_DECORATOR(ClassName) \
	protected:\
		using Super = Base;\
		using Base = ClassName;


#define MODULE_CLASS_DECORATOR(MoudleName)\
	CLASS_DECORATOR( MoudleName )\
	public:\
		MoudleName()\
			: Super()\
		{\
			MEngine::Get().RegisterModule(this);\
		}\
\
\
	private:\
		static MoudleName GlobalReference;\
\
	public:\
		static MoudleName& Get()\
		{\
			GlobalReference.Check();\
			return GlobalReference;\
		};


#define IMPLEMENT_MODULE(MoudleName) MoudleName MoudleName::GlobalReference{};
