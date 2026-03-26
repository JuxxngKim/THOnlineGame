#include "CommonPch.h"
#include "LogicUnitArchive.h"
#include "LogicUnit.h"

namespace th
{
	LogicUnitArchive::LogicUnitArchive() : m_maxLogicUnitPlayerCount{ g_MaxLogicUnitPlayerCount }
	{
	}

	void LogicUnitArchive::SetLogicUnitPlayerCount(int32_t count)
	{
		m_maxLogicUnitPlayerCount = count;
	}

	PTR<LogicUnit> LogicUnitArchive::EnterTo(const PTR<IPlayerLogicUnit>& player)
	{
		if (player == nullptr) return nullptr;

		if (m_unfilledUnit.empty()) CreateUnit();

		const auto unit = m_unfilledUnit.begin()->second;
		unit->Add(player);

		if (unit->IsMax()) m_unfilledUnit.erase(m_unfilledUnit.begin());

		return unit;
	}

	void LogicUnitArchive::CreateUnit()
	{
		auto id = ++m_key;
		auto unit = NEW(LogicUnit, id, m_maxLogicUnitPlayerCount);
		m_allUnit.push_back(unit);
		m_unfilledUnit.insert({ id, unit });
	}

	void LogicUnitArchive::Remove(const Int64Key uid)
	{
		const auto it = m_unfilledUnit.find(uid);
		if (it != m_unfilledUnit.end())
		{
			m_unfilledUnit.erase(it);
		}

		for (size_t i = 0; i < m_allUnit.size(); ++i)
		{
			const auto& unit = m_allUnit[i];
			if (unit->FindId() != uid) continue;

			m_allUnit.erase(m_allUnit.begin() + i);
			break;
		}
	}

	std::vector<PTR<LogicUnit>>& LogicUnitArchive::FindAllUnits()
	{
		return m_allUnit;
	}

	void LogicUnitArchive::AddUnfilledUnit(const PTR<LogicUnit>& unit)
	{
		if (unit == nullptr) return;
		if (m_unfilledUnit.find(unit->FindId()) != m_unfilledUnit.end()) return;

		m_unfilledUnit.insert({ unit->FindId(), unit });
	}

	int32_t LogicUnitArchive::FindCount() const
	{
		return static_cast<int32_t>(m_allUnit.size());
	}

	void LogicUnitArchive::Arrange(const PTR<LogicUnit>& unit)
	{
		if (unit == nullptr) return;

		if (unit->IsEmpty())
		{
			Remove(unit->FindId());
		}
		else if (!unit->IsMax())
		{
			AddUnfilledUnit(unit);
		}
	}
}
