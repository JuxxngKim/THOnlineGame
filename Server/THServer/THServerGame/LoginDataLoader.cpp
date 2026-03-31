#include "GamePch.h"
#include "LoginDataLoader.h"
#include "ProtoMemberAdapter.h"

namespace th
{
	LoginDataLoader::LoginDataLoader()
		: m_db{ nullptr }, m_gameDBID{ 0 }, m_pid{ "" }, m_accountUID{ 0 }, m_hostID{ 0 }, m_loginDateTime{}, m_loadStartMs{ 0 }
	{
		//CommonMetric::GetInstance().IncrementObject("login_data_loader");
	}

	LoginDataLoader::~LoginDataLoader()
	{
		//CommonMetric::GetInstance().DecrementObject("login_data_loader");
	}

	void LoginDataLoader::SetGameDBID(const int32_t gameDBID)
	{
		m_gameDBID = gameDBID;
	}

	void LoginDataLoader::SetAccountUID(const AccountUID_t accountUID)
	{
		m_accountUID = accountUID;
	}

	void LoginDataLoader::SetHostID(const HostID_t& hostID)
	{
		m_hostID = hostID;
	}

	void LoginDataLoader::SetLoginDateTime(const THDateTime& loginDateTime)
	{
		m_loginDateTime = loginDateTime;
	}

	void LoginDataLoader::Receive(const int32_t& msgID)
	{
		switch (msgID)
		{
		case DA_PLAYER_INFO_ACK: Complete(ELoginData::Player); return;
		default: TH_LOG_ERROR(0, 0, "undefined login loader packet : [msgID:%]", msgID);
		}
	}

	bool LoginDataLoader::IsAllCompleted() const
	{
		return m_completeTag.size() == static_cast<size_t>(ELoginData::Max);
	}

	bool LoginDataLoader::IsCompleted(const ELoginData& type) const
	{
		return m_completeTag.contains(type);
	}

	std::set<ELoginData> LoginDataLoader::FindCompleteDatas() const
	{
		std::set<ELoginData> results;
		std::ranges::copy(m_completeTag, std::insert_iterator(results, results.begin()));
		return results;
	}

	const std::unordered_map<ELoginData, int64_t>& LoginDataLoader::FindLoadTimeMs() const
	{
		return m_loadTimeMs;
	}

	void LoginDataLoader::Complete(const ELoginData& type)
	{
		m_completeTag.emplace(type);

		m_loadTimeMs[type] = util::TimeUtil::GetInstance().FindTickKSTMs() - m_loadStartMs;
	}

	void LoginDataLoader::Clear()
	{
		m_db = nullptr;
		m_completeTag.clear();
		m_gameDBID = 0;
		m_accountUID = 0;
		m_hostID = 0;
		m_loadStartMs = 0;
		m_loadTimeMs.clear();
		m_loginDateTime = {};
	}

	void LoginDataLoader::Start(const PTR<Mailbox_t>& receiver)
	{
		if (receiver == nullptr) return;
		m_db = receiver;

		RequestPlayerInfo();

		m_loadStartMs = util::TimeUtil::GetInstance().FindTickKSTMs();
	}

	void LoginDataLoader::SetPID(const std::string& pid)
	{
		m_pid = pid;
	}

	void LoginDataLoader::RequestPlayerInfo() const
	{
		// TODO
		//const auto& req = NEW(ADPlayerInfoReq);
		//req->set_pid(m_pid);
		//ProtoMemberAdapter::ConvertReDateTimeToMDateTime(util::TimeUtil::GetInstance().FindKSTDate(), req->mutable_updatetime());
		//m_db->Send(m_hostID, m_accountUID, req->messageid(), req);
	}
}