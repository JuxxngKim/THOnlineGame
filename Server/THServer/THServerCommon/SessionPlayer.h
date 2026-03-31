#pragma once
#include "ILogicUnit.h"

namespace th
{
	class PacketDistributor;
	class SessionPlayer : public IPlayerLogicUnit
	{
	protected:
		HostID_t m_hostID;
		AccountUID_t m_accountUID;

	public:
		SessionPlayer() = delete;
		SessionPlayer(const HostID_t m_hostId) : m_hostID{ m_hostId }, m_accountUID{ 0 }
		{
			//CommonMetric::GetInstance()->IncrementObject("session_player");
		}
		virtual ~SessionPlayer()
		{
			//CommonMetric::GetInstance()->DecrementObject("session_player");
		}

		void Execute(const PTR<PacketDistributor>& distributor) override = 0;
		void Idle(const HostID_t& hostID) override = 0;
		void Clear() override
		{
			m_hostID = 0;
			m_accountUID = 0;
		}

	public:
		virtual void SetHostID(const HostID_t& hostID) { m_hostID = hostID; }
		HostID_t FindHostID() const override { return m_hostID; }
		virtual void SetAccountUID(const AccountUID_t& accountUID) { m_accountUID = accountUID; }
		AccountUID_t FindAccountUID() const override { return m_accountUID; }
	};
}
