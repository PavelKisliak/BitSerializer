#include <iostream>
#include "bitserializer/bit_serializer.h"
#include "bitserializer/rapidjson_archive.h"
#include "bitserializer/types/std/chrono.h"

using namespace BitSerializer;
using JsonArchive = BitSerializer::Json::RapidJson::JsonArchive;

// Incoming message structure (from external system)
struct ExternalEvent
{
	std::string EventId;
	std::string EventType;
	std::chrono::system_clock::time_point Timestamp;
	Json::RapidJson::Raw Payload;  // Opaque payload

	template <typename TArchive>
	void Serialize(TArchive& archive)
	{
		archive << KeyValue("event_id", EventId, Required());
		archive << KeyValue("event_type", EventType, Required(), Validate::MaxSize(32));
		archive << KeyValue("timestamp", Timestamp, Required());
		archive << KeyValue("payload", Payload, Required("Must contain valid JSON payload"));
	}
};

// Internal routing structure (for our system)
struct RoutingEnvelope
{
	std::string RouteId;
	Json::RapidJson::Raw Payload;  // Pass-through payload

	template <typename TArchive>
	void Serialize(TArchive& archive)
	{
		archive << KeyValue("route_id", RouteId);
		archive << KeyValue("payload", Payload);
	}
};

int main()	// NOLINT(bugprone-exception-escape)
{
	const char* incomingMsg = R"({
        "event_id": "1BE3185E-ABBE-4FBC-92E2-F2FC44D06223",
        "event_type": "USER_CREATED",
        "timestamp": "2023-11-15T14:30:00Z",
        "payload": {
            "user_id": "usr_789",
            "name": "Jane Doe",
            "preferences": { },
            "metadata": ["beta-tester", "premium"]
        }
    })";

	// Deserialize external event (payload remains raw)
	ExternalEvent externalEvent;
	BitSerializer::LoadObject<JsonArchive>(externalEvent, incomingMsg);

	// Process metadata
	std::cout << "[AUDIT] Processing event '" << externalEvent.EventId << "' (type: " << externalEvent.EventType << ")\n";

	// Forward payload to internal routing system WITHOUT PROCESSING
	RoutingEnvelope envelope{ "route_789", std::move(externalEvent.Payload) };
	std::string routedMessage;
	BitSerializer::SaveObject<JsonArchive>(envelope, routedMessage);
	std::cout << "[FORWARD] To internal service:\n" << routedMessage << '\n';
}
