#pragma once
#include "Singleton.h"

namespace th
{
    class MessageCreator : public Singleton<MessageCreator>
    {
    private:
        using Creator = std::function<PTR<google::protobuf::Message>()>;
        std::unordered_map<int32_t, Creator> m_creators;

    public:
        MessageCreator();
        virtual ~MessageCreator() = default;
        PTR<google::protobuf::Message> CreateMessage(const int32_t& messageID);

    private:
        void RegisterNetConnect();
        void RegisterNetDisconnect();
        void RegisterNetAliveReq();
        void RegisterNetAliveAck();
        void RegisterCALoginReq();
    };
}