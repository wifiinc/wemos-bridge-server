#ifndef PACKETS_H
#define PACKETS_H

#include <cstdint>

enum class SensorType : uint8_t {
    NOOP = 0,
    BUTTON = 1,
    TEMPERATURE = 2,
    CO2 = 3,
    HUMIDITY = 4,
    PRESSURE = 5,
    LIGHT = 6,
    MOTION = 7,
};

enum class PacketType : uint8_t {
    DATA = 0,
    HEARTBEAT = 1,
};

struct sensor_header {
    uint8_t length;  // Total length *excluding* this header
    PacketType ptype;
} __attribute__((packed));

// Specific packet structures (ensure alignment/packing matches expected format)
struct sensor_heartbeat {
    SensorType type;
    uint8_t id;
} __attribute__((packed));

struct sensor_packet_generic {
    SensorType type;
    uint8_t id;
    bool value;
} __attribute__((packed));

struct sensor_packet_temperature {
    SensorType type;
    uint8_t id;
    float value;
} __attribute__((packed));

struct sensor_packet_co2 {
    SensorType type;
    uint8_t id;
    uint16_t value;
} __attribute__((packed));

struct sensor_packet_humidity {
    SensorType type;
    uint8_t id;
    float value;
} __attribute__((packed));

struct sensor_packet_light {
    SensorType type;
    uint8_t id;
    uint8_t target_state;
} __attribute__((packed));
// --- End Structures ---

struct sensor_packet {
    struct sensor_header header;

    union {
        struct sensor_packet_generic generic;
        struct sensor_packet_temperature temperature;
        struct sensor_packet_co2 co2;
        struct sensor_packet_humidity humidity;
        struct sensor_packet_light light;
    } data;
};

#endif  // PACKETS_H
