#pragma once

#include "Platform/Platform.h"

#include <string>
using CharString = std::string;

#if PLATFORM_WINDOWS
using String = std::wstring;
#else
using String = std::string;
#endif


#include <vector>
template<class T> using vector = std::vector<T>;

#include <memory_resource>
template<class T> using pmr_vector = std::pmr::vector<T>;
using pmr_memory_resource = std::pmr::memory_resource;
using pmr_monotonic_buffer_resource = std::pmr::monotonic_buffer_resource;
using pmr_polymorphic_allocator = std::pmr::polymorphic_allocator<char>;
using pmr_string = std::pmr::string;
using pmr_wstring = std::pmr::wstring;

#include <list>
template<class T>using pmr_list = std::pmr::list<T>;
#include <deque>
template<class T> using pmr_deque = std::pmr::deque<T>;
#include <unordered_map>
template<class Key, class T> using pmr_unordered_map = std::pmr::unordered_map<Key, T>;
template<class Key, class T> using unordered_map = std::unordered_map<Key, T>;
#include <unordered_set>
template<class T> using pmr_unordered_set = std::pmr::unordered_set<T>;
template<class T> using unordered_set = std::unordered_set<T>;

inline pmr_memory_resource* pmr_new_delete_resource()
{
	return std::pmr::new_delete_resource();
}

#include <queue>
template<class T> using queue = std::queue<T>;

#include <memory>
template<class T> using SharedPtr = std::shared_ptr<T>;

#include <assert.h>

#ifdef TEXT
    #undef TEXT
#endif

#if PLATFORM_WINDOWS
#define TEXT(x) L##x
#else
#define TEXT(x) x
#endif

#include <type_traits>
#include <functional>

#include <iostream>

using std::cout;
using std::endl;

using int8 = signed char;
using uint8 = unsigned char;

using int16 = short;
using uint16 = unsigned short;

using int32 = int;
using uint32 = unsigned int;

using int64 = long long;
using uint64 = unsigned long long;

#include "Vector.h"
#include "Misc/Matrix.h"

using MStartupParams = MPlatformStartupParams;
extern MStartupParams GStartupParams;

enum EShaderType
{
	VS,
	PS
};

#if PLATFORM_WINDOWS
#define verify(x) assert( x == S_OK )
#else
#define verify(x) assert( x == 0 )
#endif
