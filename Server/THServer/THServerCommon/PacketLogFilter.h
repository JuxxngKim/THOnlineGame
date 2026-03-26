#pragma once
namespace util
{
    class PacketLogFilter
    {
    public:
        static void Write(const HostID_t& hostID, const th::AccountUID_t& auid, const std::string& body
            , const th::EMessageID& msgId, const google::protobuf::Message& message);
    private:
        static bool PacketFilter(const th::EMessageID& msgId);
        static bool LoginDBPacket(const th::EMessageID& msgId);
        static bool LoginClientPacket(const th::EMessageID& msgId);
        static bool BattlePacket(const th::EMessageID& msgId);
        static bool LogicCyclePacket(const th::EMessageID& msgId);
    };
}