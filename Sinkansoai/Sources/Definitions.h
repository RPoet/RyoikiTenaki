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
		static CharString CStaticClassName;\
		static String StaticClassName;\
\
	public:\
		static MoudleName& Get()\
		{\
			GlobalReference.Check();\
			return GlobalReference;\
		};\
\
		static String& GetStaticClassName()\
		{\
			return MoudleName::StaticClassName;\
		};\
\
		virtual void PrintName() override final\
		{\
			cout << "Static Class Name : "<< GetStaticClassName().c_str() << endl;\
		};



#define IMPLEMENT_MODULE(MoudleName) \
	MoudleName MoudleName::GlobalReference{};\
	CharString MoudleName::CStaticClassName{ typeid(MoudleName).name() };\
	String MoudleName::StaticClassName{ MoudleName::CStaticClassName.begin(), MoudleName::CStaticClassName.end() };