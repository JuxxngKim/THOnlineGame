#pragma once
#include "Singleton.h"

namespace th
{
	class LogicUnit;
	class IPlayerLogicUnit;
	class BattleField;
	class LogicUnitArchive : public Singleton<LogicUnitArchive>
	{
	public:
		LogicUnitArchive();
		virtual ~LogicUnitArchive() = default;

		void SetLogicUnitPlayerCount(int32_t count);
		//PTR<LogicUnit> EnterTo(const PTR<SessionPlayer>& player, const PTR<BattleField>& field);
		PTR<LogicUnit> EnterTo(const PTR<IPlayerLogicUnit>& player);
		std::vector<PTR<LogicUnit>>& FindAllUnits();
		void AddUnfilledUnit(const PTR<LogicUnit>& unit);
		int32_t FindCount() const;
		void Arrange(const PTR<LogicUnit>& unit);

	private:
		void CreateUnit();
		void Remove(const Int64Key uid);

	private:
		std::atomic<Int64Key> m_key;
		std::unordered_map<Int64Key, PTR<LogicUnit>> m_unfilledUnit;
		std::vector<PTR<LogicUnit>> m_allUnit;
		int32_t m_maxLogicUnitPlayerCount;
	};
}