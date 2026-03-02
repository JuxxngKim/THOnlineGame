#pragma once
#include <tuple>

namespace network
{
	using PacketInfo = std::tuple<int32_t /*messageID*/, char* /*messageBuffer*/, int32_t /*messageSize*/>;

	class NetworkBuffer
	{
	private:
		char* m_buffer;
		int32_t m_bufferSize;
		int32_t m_writeSize;
		int32_t m_readSize;
		bool m_ignoreEncrypt;

	public:
		NetworkBuffer(const bool& ignoreEncrypt = false);
		virtual ~NetworkBuffer();
		NetworkBuffer(const PTR<NetworkBuffer>& other);

		bool Write(void* src, int32_t srcSize);
		bool CompleteWrite(int32_t size);

		bool Read(void* dest, int32_t destSize);
		bool CompleteRead(int32_t size);

		char* GetData();
		int32_t GetDataSize();

		char* GetEmpty();
		int32_t GetEmptySize();

		bool PacketToByte(int32_t packetId, google::protobuf::Message& packet);
		PacketInfo ByteToPacket(const bool& isEncrypted, const unsigned char* nonce);

		bool ReadyToRecv(WSABUF& wsabuf);
		bool ReadyToSend(WSABUF& wsabuf, int32_t total, std::deque<PTR<NetworkBuffer>>& datas);

		void Encrypt(const bool& isEncrypted, const unsigned char* nonce);
		void Decrypt(char* buffer, const bool& isEncrypted, const unsigned char* nonce, const int32_t& messageSize);
	};
}
