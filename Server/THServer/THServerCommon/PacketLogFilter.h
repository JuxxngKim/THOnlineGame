#pragma once
namespace util
{
    class PacketLogFilter
    {
    public:
        static void Write(const HostID_t& hostID, const th::AccountUID_t& auid, const std::string& body
            , const th::EMessageID& msgId, const google::protobuf::Message& message);
    private:
        static bool PacketFilter(const th::EMessageID& msgID);
    };
}