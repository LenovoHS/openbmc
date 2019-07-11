#include <mapper.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <systemd/sd-bus.h>
#include <unistd.h>
#include <ipmid/api.h>

namespace ipmi::chassis
{
	struct Identify
		{
			uint8_t identifyInterval;
			uint8_t forceIdentify;
		} __attribute__((packed));
		
	/** The status of the update mechanism or the verification mechanism */
	enum class ActionStatus : std::uint8_t
	{
			running = 0,
			success = 1,
			failed = 2,
			unknown = 3,
	};

	class TriggerableActionInterface
	{
			public:
			virtual ~TriggerableActionInterface() = default;

			/**
			* Trigger action.
			*
			* @return true if successfully started, false otherwise.
			*/
			virtual bool trigger() = 0;

			/** Abort the action if possible. */
			virtual void abort() = 0;

			/** Check the current state of the action. */
			virtual ActionStatus status() = 0;
	};
	class UIDIdentify : public TriggerableActionInterface
	{
			public:
			/**
			* Create a default Verification object that uses systemd to trigger the
			* process.
			*
			* @param[in] bus - an sdbusplus handler for a bus to use.
			* @param[in] path - the path to check for verification status.
			* @param[in[ service - the systemd service to start to trigger
			* verification.
			*/
			static std::unique_ptr<TriggerableActionInterface>
			CreateIdentify(sdbusplus::bus::bus&& bus,
                           const std::string& service);

			UIDIdentify(sdbusplus::bus::bus&& bus,
                        const std::string& service) :
			bus(std::move(bus)),triggerService(service)
			{
			}

		~UIDIdentify() = default;
		UIDIdentify(const UIDIdentify&) = delete;
		UIDIdentify& operator=(const UIDIdentify&) = delete;
		UIDIdentify(UIDIdentify&&) = default;
		UIDIdentify& operator=(UIDIdentify&&) = default;

		bool trigger() override;
		void abort() override;
		ActionStatus status() override;

		private:
		sdbusplus::bus::bus bus;
		const std::string triggerService;
		ActionStatus state = ActionStatus::unknown;
	};
}