#pragma once

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

#include <assert.h>

#define TEXT(x) L##x

#include <type_traits>


// IOS overriding
#include <iostream>

#define cout std::cout

#define endl std::endl

using int32 = int;
using uint32 = unsigned int;

using int16 = short;
using uint16 = unsigned short;

using int8 = char;
using uint8 = unsigned char;

#include "Vector.h"