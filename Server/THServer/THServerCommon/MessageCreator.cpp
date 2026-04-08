#include "CommonPch.h"
#include "MessageCreator.h"

namespace th
{
    MessageCreator::MessageCreator()
    {
        RegisterNetDisconnect();
        RegisterNetAliveReq();
        RegisterNetAliveAck();
        RegisterCALoginReq();
    }

    PTR<google::protobuf::Message> MessageCreator::CreateMessage(const int32_t& messageID)
    {
        auto it = m_creators.find(messageID);
        if (it == m_creators.end()) return nullptr;
        return (it->second)();
    }

    void MessageCreator::RegisterNetConnect()
    {
        auto msg = NEW(NetConnect);
        m_creators.insert({ msg->messageid(), []() { return NEW(NetConnect); } });
    }

    void MessageCreator::RegisterNetDisconnect()
    {
        auto msg = NEW(NetDisconnect);
        m_creators.insert({ msg->messageid(), []() { return NEW(NetDisconnect); } });
    }

    void MessageCreator::RegisterNetAliveReq()
    {
        auto msg = NEW(NetAliveReq);
        m_creators.insert({ msg->messageid(), []() { return NEW(NetAliveReq); } });
    }

    void MessageCreator::RegisterNetAliveAck()
    {
        auto msg = NEW(NetAliveAck);
        m_creators.insert({ msg->messageid(), []() { return NEW(NetAliveAck); } });
    }

    void MessageCreator::RegisterCALoginReq()
    {
        auto msg = NEW(CALoginReq);
        m_creators.insert({ msg->messageid(), []() { return NEW(CALoginReq); } });
    }
}
