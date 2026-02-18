#pragma once
#include "Logger.h"

#define TH_LOG_TRACE(hostID, accountUID, fmt, ...) \
 ::th::Logger::GetInstance().Trace("hostID:% accountUID:% % " fmt, hostID, accountUID, static_cast<const char*>(__FUNCTION__), ##__VA_ARGS__)

#define TH_LOG_DEBUG(hostID, accountUID, fmt, ...) \
 ::th::Logger::GetInstance().Debug("hostID:% accountUID:% % " fmt, hostID, accountUID, static_cast<const char*>(__FUNCTION__), ##__VA_ARGS__)

#define TH_LOG_INFO(hostID, accountUID, fmt, ...) \
 ::th::Logger::GetInstance().Info("hostID:% accountUID:% % " fmt, hostID, accountUID, static_cast<const char*>(__FUNCTION__), ##__VA_ARGS__)

#define TH_LOG_WARN(hostID, accountUID, fmt, ...) \
 ::th::Logger::GetInstance().Warn("hostID:% accountUID:% % " fmt, hostID, accountUID, static_cast<const char*>(__FUNCTION__), ##__VA_ARGS__)

#define TH_LOG_ERROR(hostID, accountUID, fmt, ...) \
 ::th::Logger::GetInstance().Error("hostID:% accountUID:% % " fmt, hostID, accountUID, static_cast<const char*>(__FUNCTION__), ##__VA_ARGS__)

#define TH_LOG_CRITICAL(hostID, accountUID, fmt, ...) \
 ::th::Logger::GetInstance().Critical("hostID:% accountUID:% % " fmt, hostID, accountUID, static_cast<const char*>(__FUNCTION__), ##__VA_ARGS__)