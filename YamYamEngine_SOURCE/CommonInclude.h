#pragma once
#include <Windows.h>
#include <assert.h>
#include <iostream>

#include <vector>
#include <algorithm>
#include <string>
#include <stack>
#include <queue>
#include <map>
#include <bitset>
#include <functional>
#include <filesystem>
#include <mutex>
#include <thread>

#include <mmsystem.h>
#include <dinput.h>

#include "yaEnums.h"
#include "yaMath.h"

#define CAST_UINT(value) static_cast<UINT>(value)
#define CAST_FLOAT(value) static_cast<float>(value)

#define SAFE_DELETE(p) if (p) { delete p; p = nullptr; }
#define SAFE_DELETE_ARRAY(p) if (p) { delete[] p; p = nullptr; }
#define SAFE_RELEASE(p) if (p) { p->Release(); p = nullptr; }
