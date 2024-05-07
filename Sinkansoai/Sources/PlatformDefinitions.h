#pragma once

// Containter overriding
#include <string>
using String = std::wstring;

#define TEXT(x) L##x

// IOS overriding
#include <iostream>

#define cout std::cout

#define endl std::endl

