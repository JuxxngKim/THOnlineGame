#pragma once
#include "PacketCollector.h"

namespace th
{
	struct OutGamePackets
	{
		std::unordered_map<HostID_t, std::deque<PTR<PacketWrapper>>> HostPackets;
		std::unordered_map<AccountUID_t, std::deque<PTR<PacketWrapper>>> AccountPackets;
		std::deque<PTR<PacketWrapper>> CollectedPackets;

		OutGamePackets()
		{
			//CommonMetric::GetInstance()->IncrementObject("out_game_packets");
		}
		~OutGamePackets()
		{
			//CommonMetric::GetInstance()->DecrementObject("out_game_packets");
		}

		void Clear()
		{
			HostPackets.clear();
			AccountPackets.clear();
			CollectedPackets.clear();
		}

		bool Empty() const
		{
			return HostPackets.empty() && AccountPackets.empty();
		}
	};

	class PacketDualMap
	{
	private:
		std::recursive_mutex m_lock{};
		bool usedMain{ false };

		PTR<PacketCollector<PTR<PacketWrapper>>> m_collector;
		PTR<OutGamePackets> m_mainPackets;
		PTR<OutGamePackets> m_subPackets;

	public:
		PacketDualMap() : m_collector{ nullptr }, m_mainPackets{ NEW(OutGamePackets) }, m_subPackets{ NEW(OutGamePackets) }
		{
			//CommonMetric::GetInstance()->IncrementObject("logic_dual_map");
		}

		virtual ~PacketDualMap()
		{
			std::lock_guard guard(m_lock);

			usedMain = false;
			//CommonMetric::GetInstance()->DecrementObject("logic_dual_map");
		}

		void RegisterCollector(const PTR<PacketCollector<PTR<PacketWrapper>>>& collector)
		{
			m_collector = collector;
		}

	public:
		void Push(const PTR<PacketWrapper>& packet)
		{
			std::lock_guard guard(m_lock);

			PTR<OutGamePackets> packets = nullptr;
			if (usedMain)
			{
				packets = m_mainPackets;
			}
			else
			{
				packets = m_subPackets;
			}

			if (packet->AccountUID != 0)
			{
				const auto& it = packets->AccountPackets.find(packet->AccountUID);
				if (it != packets->AccountPackets.end())
				{
					it->second.push_back(packet);
				}
				else
				{
					std::deque<PTR<PacketWrapper>> values;
					values.push_back(packet);
					packets->AccountPackets.insert({ packet->AccountUID, values });
				}
			}
			else
			{
				const auto& it = packets->HostPackets.find(packet->HostID);
				if (it != packets->HostPackets.end())
				{
					it->second.push_back(packet);
				}
				else
				{
					std::deque<PTR<PacketWrapper>> values;
					values.push_back(packet);
					packets->HostPackets.insert({ packet->HostID, values });
				}
			}

			if (m_collector->IsMatched(packet->MessageID))
			{
				packets->CollectedPackets.push_back(packet);
			}
		}

		PTR<OutGamePackets> Pop()
		{
			std::lock_guard guard(m_lock);

			if (usedMain)
			{
				usedMain = false;
				return m_mainPackets;
			}
			else
			{
				usedMain = true;
				return m_subPackets;
			}
		}

		bool Empty()
		{
			std::lock_guard guard(m_lock);
			if (usedMain)
			{
				//usedMain = false;
				return m_mainPackets->Empty();
			}
			else
			{
				//usedMain = true;
				return m_subPackets->Empty();
			}
		}
	};
}