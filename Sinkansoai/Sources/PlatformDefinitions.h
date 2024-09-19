#pragma once

// Containter overriding
#include <string>
using String = std::wstring;


#include <vector>
template<class T> using vector = std::vector<T>;

#include <assert.h>

#define TEXT(x) L##x

// IOS overriding
#include <iostream>

#define cout std::cout

#define endl std::endl

