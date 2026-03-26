#pragma once
#include <type_traits>

namespace th
{
	template <typename T, typename = std::enable_if_t<std::is_base_of<PTR<PacketWrapper>, T>::value>>
	class PacketCollector
	{
	private:
		std::unordered_set<int32_t> m_matchedId;

	public:
		PacketCollector()
		{
			//CommonMetric::GetInstance()->IncrementObject("packet_collector");
		}

		virtual ~PacketCollector()
		{
			//CommonMetric::GetInstance()->DecrementObject("packet_collector");
		}

		void RegisterID(const int32_t msgId)
		{
			m_matchedId.insert(msgId);
		}

	public:
		bool IsMatched(const int32_t messageID) const
		{
			return m_matchedId.contains(messageID);
		}
	};
}