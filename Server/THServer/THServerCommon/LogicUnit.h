#pragma once

namespace th
{
	class IPlayerLogicUnit;
	class IFieldLogicUnit;
	class PacketDistributor;
	class LogicUnit
	{
	public:
		LogicUnit() = delete;
		LogicUnit(const Int64Key id, const int32_t maxLogicUnitPlayerCount);
		virtual ~LogicUnit();

		void Execute(const PTR<PacketDistributor>& distributor) const;

		void Add(const PTR<IPlayerLogicUnit>& player);
		void Remove(const HostID_t& hostID);
		bool IsEmpty() const;
		bool IsMax() const;

		Int64Key FindId() const;

	private:
		Int64Key m_id;
		int32_t m_maxLogicUnitPlayerCount;
		std::vector<PTR<IPlayerLogicUnit>> m_players;
	};
}