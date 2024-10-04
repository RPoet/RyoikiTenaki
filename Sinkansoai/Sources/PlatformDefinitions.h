#pragma once

// Containter overriding
#include <string>
using CharString = std::string;
using String = std::wstring;

#include <vector>
template<class T> using vector = std::vector<T>;

#include <queue>
template<class T> using Queue = std::queue<T>;


#include <assert.h>

#define TEXT(x) L##x



// IOS overriding
#include <iostream>

#define cout std::cout

#define endl std::endl
