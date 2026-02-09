#pragma once

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
#define NOMINMAX1

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <deque>
#include <queue>
#include <utility>
#include <conio.h>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <format>
#include <regex>
#include <stdexcept>
#include <comutil.h>
#include <fstream>
#include <algorithm>

#include "DefineType.h"
#include "DefineConst.h"
#include "DefineMacro.h"
#include "DefineEnum.h"
#include "THDateTime.h"
#include "Singleton.h"
#include "TimeUtil.h"
