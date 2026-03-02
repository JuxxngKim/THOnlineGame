#pragma once

#include "NetworkContext.h"
#include "NetworkBuffer.h"

namespace network
{
	struct NetworkTask : OVERLAPPED
	{
		void* m_owner{ nullptr };

		void Failed();

		virtual ENetworkTask GetType() = 0;
		virtual void Prepare() = 0;
		virtual void Complete(bool result, int32_t transferred) = 0;
	};

	struct NetworkTaskConnect : NetworkTask
	{
		virtual ENetworkTask GetType() override;
		virtual void Prepare() override;
		virtual void Complete(bool result, int32_t transferred) override;
	};

	struct NetworkTaskAccept : NetworkTask
	{
		SOCKET m_socket{ INVALID_SOCKET };
		char m_address[MAX_ADDRESS_SIZE]{};

		virtual ENetworkTask GetType() override;
		virtual void Prepare() override;
		virtual void Complete(bool result, int32_t transferred) override;
	};

	struct NetworkTaskReceive : NetworkTask
	{
		NetworkBuffer m_buffer;

		virtual ENetworkTask GetType() override;
		virtual void Prepare() override;
		virtual void Complete(bool result, int32_t transferred) override;
	};

	struct NetworkTaskSend : NetworkTask
	{
		std::deque<PTR<NetworkBuffer>> m_datas{};
		std::vector<WSABUF> m_buffers{};
		int32_t m_total{ 0 };

		virtual ENetworkTask GetType() override;
		virtual void Prepare() override;
		virtual void Complete(bool result, int32_t transferred) override;
	};

}