#pragma once

namespace th
{
	class PacketDistributor;
	class IPlayerLogicUnit
	{
	public:
		IPlayerLogicUnit() = default;
		virtual ~IPlayerLogicUnit() = default;

		virtual AccountUID_t FindAccountUID() const = 0;
		virtual void Idle(const HostID_t& hostID) = 0;
		virtual HostID_t FindHostID() const = 0;
		virtual void Clear() = 0;
		virtual void Execute(const PTR<PacketDistributor>& distributor) = 0;
	};

	class IFieldLogicUnit
	{
	public:
		IFieldLogicUnit() = default;
		virtual ~IFieldLogicUnit() = default;

		virtual bool Remove(const HostID_t& hostID) = 0;
		virtual bool IsEmpty() const = 0;
		virtual void Execute() = 0;

		virtual int32_t FindStageID() const = 0;
	};
}