#include "CommonPch.h"
#include "NetworkBuffer.h"
#include "NetworkContext.h"
#include "DefineConst.h"
#include "DefineMacro.h"
#include <tuple>

// TODO Crypto
//#include "Crypto.h"

namespace network
{
	NetworkBuffer::NetworkBuffer(const bool& ignoreEncrypt)
		: m_buffer(nullptr)
		, m_bufferSize(0)
		, m_writeSize(0)
		, m_readSize(0)
		, m_ignoreEncrypt{ ignoreEncrypt }
	{
	}

	NetworkBuffer::~NetworkBuffer()
	{
		SAFE_DELETE_ARRAY(m_buffer);
	}

	NetworkBuffer::NetworkBuffer(const PTR<NetworkBuffer>& other)
	{
		m_buffer = new char[other->m_bufferSize];
		memcpy(m_buffer, other->m_buffer, other->m_bufferSize);
		m_bufferSize = other->m_bufferSize;
		m_writeSize = other->m_writeSize;
		m_readSize = other->m_readSize;
		m_ignoreEncrypt = other->m_ignoreEncrypt;
	}

	bool NetworkBuffer::Write(void* src, int32_t srcSize)
	{
		if (src == nullptr || srcSize <= 0)
		{
			TH_LOG_ERROR(0, 0, "failed - src");
			return false;
		}

		if (srcSize > GetEmptySize())
		{
			TH_LOG_ERROR(0, 0, "failed - GetEmptySize");
			return false;
		}

		memcpy(GetEmpty(), (char*)src, srcSize);
		m_writeSize += srcSize;

		return true;
	}

	bool NetworkBuffer::CompleteWrite(int32_t size)
	{
		if (size > 0)
		{
			if (size > GetEmptySize())
			{
				TH_LOG_ERROR(0, 0, "failed - GetEmptySize");
				return false;
			}

			m_writeSize += size;
		}

		return true;
	}

	bool NetworkBuffer::Read(void* dest, int32_t destSize)
	{
		if (dest == nullptr || destSize <= 0)
		{
			TH_LOG_ERROR(0, 0, "failed - dest");
			return false;
		}

		char* src = GetData();
		int32_t srcSize = GetDataSize();
		if (src == nullptr || srcSize < destSize)
		{
			TH_LOG_ERROR(0, 0, "failed - src");
			return false;
		}

		memcpy((char*)dest, src, destSize);
		m_readSize += destSize;

		return true;
	}

	bool NetworkBuffer::CompleteRead(int32_t size)
	{
		if (size > 0)
		{
			if (size > GetDataSize())
			{
				TH_LOG_ERROR(0, 0, "failed - size");
				return false;
			}

			m_readSize += size;
		}

		return true;
	}

	char* NetworkBuffer::GetData()
	{
		if (m_buffer == nullptr) return nullptr;
		if (m_bufferSize < m_writeSize) return nullptr;
		if (m_writeSize <= 0 || m_writeSize <= m_readSize) return nullptr;

		return m_buffer + m_readSize;
	}

	int32_t NetworkBuffer::GetDataSize()
	{
		if (m_buffer == nullptr) return 0;
		if (m_bufferSize < m_writeSize) return 0;
		if (m_writeSize <= 0 || m_writeSize <= m_readSize) return 0;

		return m_writeSize - m_readSize;
	}

	char* NetworkBuffer::GetEmpty()
	{
		if (m_buffer == nullptr) return nullptr;
		if (m_bufferSize <= m_writeSize) return nullptr;

		return m_buffer + m_writeSize;
	}

	int32_t NetworkBuffer::GetEmptySize()
	{
		if (m_buffer == nullptr) return 0;
		if (m_bufferSize <= m_writeSize) return 0;

		return (int32_t)m_bufferSize - m_writeSize;
	}

	bool NetworkBuffer::PacketToByte(int32_t packetId, google::protobuf::Message& packet)
	{
		//이미 버퍼가 할당되어있으면 실패
		if (m_buffer) return false;

		//사이즈 체크
		int32_t messageSize = static_cast<int32_t>(packet.ByteSizeLong());
		if (messageSize < 0)
		{
			TH_LOG_ERROR(0, 0, "packet size under : [id:%, size:%]", packetId, messageSize);
			messageSize = 0;
		}
		else if (messageSize >= g_PacketBodySize)
		{
			TH_LOG_ERROR(0, 0, "packet size over : [id:%, size:%]", packetId, messageSize);
		}

		//버퍼 할당
		m_bufferSize = g_PacketHeaderSize + messageSize;
		m_buffer = new char[m_bufferSize];

		//데이터 복사
		u_long tempMessageSize = 0;
		u_long tempMessageID = 0;

		tempMessageSize = htonl(sizeof(tempMessageID) + messageSize);
		Write(&tempMessageSize, sizeof(tempMessageSize));

		tempMessageID = htonl(packetId);
		Write(&tempMessageID, sizeof(tempMessageID));

		if (messageSize > 0)
		{
			auto pos = GetEmpty();
			packet.SerializeToArray(pos, messageSize);

			CompleteWrite(messageSize);
		}

		return true;
	}

	PacketInfo NetworkBuffer::ByteToPacket(const bool& isEncrypted, const unsigned char* nonce)
	{
		char* packet = GetData();
		int32_t packetSize = GetDataSize();

		//데이터가 패킷최소사이즈보다 큰 경우인지 체크
		if (packetSize >= g_PacketHeaderSize)
		{
			int32_t messageSize = ntohl(*(u_long*)(packet)) - sizeof(u_long);
			int32_t messageID = ntohl(*(u_long*)(packet + sizeof(u_long)));

			//데이터 사이즈 예외 체크
			if (messageSize >= 0 && messageSize <= packetSize - g_PacketHeaderSize)
			{
				//읽기 처리
				CompleteRead(messageSize + g_PacketHeaderSize);

				// 복호화
				Decrypt(packet + g_PacketHeaderSize, isEncrypted, nonce, messageSize);

				//받기 완료 처리
				return std::forward_as_tuple(messageID, packet + g_PacketHeaderSize, messageSize);
			}
		}

		return std::forward_as_tuple(0, nullptr, 0);;
	}

	bool NetworkBuffer::ReadyToRecv(WSABUF& wsabuf)
	{
		//버퍼가 없으면 생성
		if (m_buffer == nullptr)
		{
			m_writeSize = 0;
			m_readSize = 0;
			m_bufferSize = DEFAULT_CAPACITY_SIZE;
			m_buffer = new char[m_bufferSize];
		}

		//읽기와 쓰기가 같은지 체크
		if (m_readSize == m_writeSize)
		{
			//버퍼 축소
			if (m_bufferSize > DEFAULT_CAPACITY_SIZE)
			{
				SAFE_DELETE_ARRAY(m_buffer);

				m_bufferSize = DEFAULT_CAPACITY_SIZE;
				m_buffer = new char[m_bufferSize];
			}

			//읽기,쓰기 초기화
			m_writeSize = 0;
			m_readSize = 0;
		}

		//빈공간 체크
		int32_t emptySize = GetEmptySize();
		if (emptySize < MIN_CAPACITY_SIZE)
		{
			if (emptySize + m_readSize >= MIN_CAPACITY_SIZE)
			{
				//재정렬만 함
				memmove(m_buffer, m_buffer + m_readSize, m_writeSize - m_readSize);
				m_writeSize -= m_readSize;
				m_readSize = 0;
			}
			else
			{
				//최대 사이즈를 넘어가는지 체크
				if (m_bufferSize >= MAX_CAPACITY_SIZE) return false;

				//새버퍼를 생성하여 바꿔치기
				auto temp = m_buffer;

				m_bufferSize += DEFAULT_CAPACITY_SIZE;
				m_buffer = new char[m_bufferSize];
				memmove(m_buffer, temp + m_readSize, m_writeSize - m_readSize);
				m_writeSize -= m_readSize;
				m_readSize = 0;

				SAFE_DELETE_ARRAY(temp);
			}
		}

		wsabuf.buf = GetEmpty();
		wsabuf.len = GetEmptySize();

		if (wsabuf.len <= 0) return false;

		return true;
	}

	bool NetworkBuffer::ReadyToSend(WSABUF& wsabuf, int32_t total, std::deque<PTR<NetworkBuffer>>& datas)
	{
		if (total <= 0) return false;

		//이미 버퍼가 할당되어있으면 실패
		if (m_buffer) return false;

		//버퍼 할당
		m_bufferSize = total;
		m_buffer = new char[m_bufferSize];

		for (auto& data : datas)
		{
			//데이터 복사
			Write(data->GetData(), data->GetDataSize());
		}

		wsabuf.buf = GetData();
		wsabuf.len = GetDataSize();

		return true;
	}

	void NetworkBuffer::Encrypt(const bool& isEncrypted, const unsigned char* nonce)
	{
		if (m_ignoreEncrypt || !isEncrypted || nonce == nullptr) return;
		//util::Crypto::Convert(GetData() + g_PacketHeaderSize, GetDataSize() - g_PacketHeaderSize, nonce);
	}

	void NetworkBuffer::Decrypt(char* buffer, const bool& isEncrypted, const unsigned char* nonce, const int32_t& messageSize)
	{
		if (m_ignoreEncrypt || !isEncrypted || nonce == nullptr) return;
		//util::Crypto::Convert(buffer, messageSize, nonce);
	}
}