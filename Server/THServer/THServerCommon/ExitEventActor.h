#pragma once
#include "ConcurrentActor.h"

namespace th
{
	class ExitEventActor : public ConcurrentActor
	{
	public:
		ExitEventActor()
		{
			//m_state->Exit();
		}

		virtual ~ExitEventActor() = default;

		void Run(const PTR<PacketWrapper>& msg) override {}
	};
}