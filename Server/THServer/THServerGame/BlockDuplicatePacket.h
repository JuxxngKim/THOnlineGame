#pragma once

namespace th
{
    class BlockDuplicatePacket
    {
    private:
        using ClientMsgID_t = EMessageID;
        using BlockDuplicatePacketKey_t = int32_t;
        using GenerateKeyCount_t = int32_t;
        using DBMsgKeyValue_t = std::pair<ClientMsgID_t, GenerateKeyCount_t>;

        BlockDuplicatePacketKey_t m_blockKeyIdx;
        std::unordered_map<ClientMsgID_t, BlockDuplicatePacketKey_t> m_clientMsgMap;
        std::unordered_map<BlockDuplicatePacketKey_t, DBMsgKeyValue_t> m_blockKeyCountMap;
        std::set<BlockDuplicatePacketKey_t> m_infiniteSet;
        std::unordered_map<ClientMsgID_t, int64_t> m_packetReceivedTimeMs;
        int32_t m_duplicateCount;

    public:
        BlockDuplicatePacket();
        virtual ~BlockDuplicatePacket();

    public:
        void Clear();
        bool IsProcessing(const ClientMsgID_t& msgID) const;
        void CheckDuplicate(const ClientMsgID_t& msgID);
        int32_t FindDuplicateCount() const;
        void ResetDuplicateCount();
        void PacketProcess(const BlockDuplicatePacketKey_t& dbkey, const ClientMsgID_t& clientMsgID, bool isInfinite);
        void PacketComplete(const BlockDuplicatePacketKey_t& blockKey);
        void PacketInfiniteComplete(const ClientMsgID_t& clientMsgID);
        BlockDuplicatePacketKey_t GenerateKey(const ClientMsgID_t& msgID);
    };
}