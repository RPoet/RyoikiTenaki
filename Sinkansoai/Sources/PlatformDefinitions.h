﻿#pragma once

// Containter overriding
#include <string>
using CharString = std::string;
using String = std::wstring;

#include <vector>
template<class T> using vector = std::vector<T>;

#include <queue>
template<class T> using queue = std::queue<T>;


#include <unordered_map>
template<class Key, class T> using unordered_map = std::unordered_map<Key, T>;

#include <unordered_set>
template<class T> using unordered_set = std::unordered_set<T>;

#include <memory>
template<class T> using SharedPtr = std::shared_ptr<T>;

#include <assert.h>

#define TEXT(x) L##x

#include <type_traits>

#include <functional>

// IOS overriding
#include <iostream>

#define cout std::cout

#define endl std::endl

using uint8 = unsigned char;

using int32 = int;
using uint32 = unsigned int;

using int16 = short;
using uint16 = unsigned short;

using int8 = char;
using uint8 = unsigned char;

#include < windows.h >
#include "Vector.h"
#include "Misc/Matrix.h"

struct MStartupParams
{
	HINSTANCE hInstance;
	HINSTANCE hPrecInstace;
	LPWSTR lpCmdLine;
	int32 nCmdShow;
};

extern MStartupParams GStartupParams;

enum EShaderType
{
	VS,
	PS
};


#define verify(x) assert( x == S_OK )
