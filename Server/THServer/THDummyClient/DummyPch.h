#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX1
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "protocol.pb.h"

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
#include <fstream>
#include <algorithm>
#include <thread>
#include <atomic>
#include <functional>
#include <iostream>

#include <google/protobuf/message.h>

#include "THDateTime.h"
#include "DefineConst.h"
#include "DefineEnum.h"
#include "DefineType.h"
#include "DefineMacro.h"
#include "Singleton.h"
#include "TimeUtil.h"
#include "Logger.h"
#include "LogMacro.h"
#include "PacketWrapper.h"
#include "NetworkManager.h"
#include "MessageCreator.h"
#include "ServiceProfile.h"
