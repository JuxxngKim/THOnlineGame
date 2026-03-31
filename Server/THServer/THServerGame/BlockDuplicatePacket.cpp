#include "GamePch.h"
#include "BlockDuplicatePacket.h"
#include "Configuration.h"
#include "ProtoMemberAdapter.h"

namespace th
{
	BlockDuplicatePacket::BlockDuplicatePacket()
	{
		m_blockKeyIdx = 1;
		m_duplicateCount = 0;
	}

	BlockDuplicatePacket::~BlockDuplicatePacket()
	{
	}

	void BlockDuplicatePacket::Clear()
	{
		m_blockKeyIdx = 0;
		m_clientMsgMap.clear();
		m_blockKeyCountMap.clear();
		m_infiniteSet.clear();
		m_packetReceivedTimeMs.clear();
		m_duplicateCount = 0;
	}

	bool BlockDuplicatePacket::IsProcessing(const ClientMsgID_t& msgID) const
	{
		if (msgID == 0 || !ProtoMemberAdapter::IsClientRequestPacket(msgID)) return false;

		return m_clientMsgMap.find(msgID) != m_clientMsgMap.end();
	}

	void BlockDuplicatePacket::CheckDuplicate(const ClientMsgID_t& msgID)
	{
		if (msgID == 0 || !ProtoMemberAdapter::IsClientRequestPacket(msgID)) return;
		if (msgID == CA_GAME_MACRO_REQ)	return;	// 치트키 예외처리

		const auto& currentTickMs = util::TimeUtil::GetInstance().FindTickKSTMs();
		if (Configuration::GetInstance().Get("ServerConfig", "CheckDuplicatePacket").Valid()
			&& Configuration::GetInstance().Get("ServerConfig", "CheckDuplicatePacket").ToInt() == 1)
		{
			if (currentTickMs - m_packetReceivedTimeMs[msgID] <= g_LogicUpdateMs)
			{
				++m_duplicateCount;
			}

			m_packetReceivedTimeMs[msgID] = currentTickMs;
		}
	}

	int32_t BlockDuplicatePacket::FindDuplicateCount() const
	{
		return m_duplicateCount;
	}

	void BlockDuplicatePacket::ResetDuplicateCount()
	{
		m_duplicateCount = 0;
	}

	void BlockDuplicatePacket::PacketProcess(const BlockDuplicatePacketKey_t& dbKey,
		const ClientMsgID_t& clientMsgID, bool isInfinite)
	{
		if (dbKey == 0 || clientMsgID == 0) return;

		// DB를 수동으로 컨트롤 하는 경우
		if (isInfinite)
		{
			auto isValid = m_infiniteSet.insert(dbKey);
			if (!isValid.second) return;
		}

		// Client메세지에 DB Key값을 저장해둔다.
		m_clientMsgMap.insert({ clientMsgID, dbKey });

		auto findIter = m_blockKeyCountMap.find(dbKey);
		if (findIter == m_blockKeyCountMap.end())
		{
			m_blockKeyCountMap.emplace(dbKey, std::make_pair(clientMsgID, 1));
		}
		else
		{
			findIter->second.second += 1;
		}
	}

	void BlockDuplicatePacket::PacketComplete(const BlockDuplicatePacketKey_t& blockKey)
	{
		if (blockKey == 0) return;

		// 수동으로 설정인 경우에는 PacketInfiniteComplete 호출을 통해서 해제 해준다.
		if (m_infiniteSet.find(blockKey) != m_infiniteSet.end()) return;

		auto findIter = m_blockKeyCountMap.find(blockKey);
		if (findIter == m_blockKeyCountMap.end()) return;

		auto clientMsgID = findIter->second.first;
		auto dbRequestCount = findIter->second.second - 1;
		if (dbRequestCount <= 0)
		{
			m_blockKeyCountMap.erase(blockKey);
			m_clientMsgMap.erase(clientMsgID);
		}
		else
		{
			findIter->second.second = dbRequestCount;
		}
	}

	void BlockDuplicatePacket::PacketInfiniteComplete(const ClientMsgID_t& clientMsgID)
	{
		const auto& findClientMsgIter = m_clientMsgMap.find(clientMsgID);
		if (findClientMsgIter == m_clientMsgMap.end()) return;

		auto blockKey = findClientMsgIter->second;
		if (m_infiniteSet.find(blockKey) == m_infiniteSet.end()) return;

		auto findIter = m_blockKeyCountMap.find(blockKey);
		if (findIter == m_blockKeyCountMap.end()) return;

		auto dbRequestCount = findIter->second.second - 1;
		if (dbRequestCount <= 0)
		{
			m_infiniteSet.erase(blockKey);
			m_blockKeyCountMap.erase(blockKey);
			m_clientMsgMap.erase(clientMsgID);
		}
		else
		{
			findIter->second.second = dbRequestCount;
		}
	}

	int32_t BlockDuplicatePacket::GenerateKey(const ClientMsgID_t& msgID)
	{
		if (msgID == 0) return 0;

		const auto findIter = m_clientMsgMap.find(msgID);
		if (findIter != m_clientMsgMap.end())
			return findIter->second;

		auto blockKey = m_blockKeyIdx++;
		if (g_MaxBlcokDuplicatePacketCount < blockKey)
		{
			m_blockKeyIdx = 1;
		}

		m_clientMsgMap.insert({ msgID, blockKey });

		return blockKey;
	}
}