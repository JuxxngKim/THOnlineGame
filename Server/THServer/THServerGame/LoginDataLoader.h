#pragma once

namespace th
{
    class LoginDataLoader
    {
    private:
        PTR<Mailbox_t> m_db;
        std::set<ELoginData> m_completeTag;

        int32_t m_gameDBID;
        std::string m_pid;
        AccountUID_t m_accountUID;
        HostID_t m_hostID;
        THDateTime m_loginDateTime;
        int64_t m_loadStartMs;
        std::unordered_map<ELoginData, int64_t> m_loadTimeMs;

    public:
        LoginDataLoader();
        virtual ~LoginDataLoader();

    public:
        void Clear();
        void Start(const PTR<Mailbox_t>& receiver);
        void SetPID(const std::string& pid);
        void SetGameDBID(const int32_t gameDbID);
        void SetAccountUID(const AccountUID_t accountUID);
        void SetHostID(const HostID_t& hostID);
        void SetLoginDateTime(const THDateTime& loginDateTime);

        void Receive(const int32_t& msgID);
        bool IsAllCompleted() const;
        bool IsCompleted(const ELoginData& type) const;
        std::set<ELoginData> FindCompleteDatas() const;
        const std::unordered_map<ELoginData, int64_t>& FindLoadTimeMs() const;

    private:
        void Complete(const ELoginData& type);
        void RequestPlayerInfo() const;
    };
}