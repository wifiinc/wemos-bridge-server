/**
 * @file packets.h
 * @brief Header file for packets.h.
 * @details This files origin is from the Wemos project
 * @warning THIS FILE MUST BE KEPT IN SYNC IN OTHER PROJECTS
 * @author Daan Breur
 * @author Erynn Scholtes
 */

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
    RGB_LIGHT = 8,
    LICHTKRANT = 9,
};

enum class PacketType : uint8_t {
    DATA = 0,
    HEARTBEAT = 1,
    DASHBOARD_POST = 2,
    DASHBOARD_GET = 3,
    DASHBOARD_RESPONSE = 4
};

/**
 * @struct sensor_header
 * @brief Header structure for sensor packets.
 * @ingroup Packets
 */
struct sensor_header {
    /** @brief Length of the packet excluding the header. */
    uint8_t length;
    /** @brief Type of the packet as PacketType (DATA, HEARTBEAT, etc.). */
    PacketType ptype;
} __attribute__((packed));

/**
 * @struct sensor_metadata
 * @brief Structure for sensor metadata, which is always included in any packet.
 * @ingroup Packets
 */
struct sensor_metadata {
    /** @brief Type of the sensor being addressed as SensorType (one byte) */
    SensorType sensor_type;
    /** @brief ID of the sensor being addressed */
    uint8_t sensor_id;
} __attribute__((packed));

// Specific packet structures (ensure alignment/packing matches expected format)

/**
 * @struct sensor_heartbeat
 * @brief Structure for heartbeat packets.
 * @details This structure contains the type and ID of the sensor being addressed. This structure
 is
 * used for heartbeat packets sent by the sensors to indicate they are still alive.
 * @ingroup Packets
 */
struct sensor_heartbeat {
    struct sensor_metadata metadata;
} __attribute__((packed));

/**
 * @struct sensor_packet_generic
 * @brief Structure for generic sensor packets.
 * @details This structure contains the type and ID of the sensor being addressed. This structure
 * is used for generic sensor packets that do not require additional data. For example, it can be
 * used for a simple button press event.
 * @ingroup Packets
 */
struct sensor_packet_generic {
    struct sensor_metadata metadata;
    // /** @brief Whether the sensor did or did not trigger */
    // bool value;
} __attribute__((packed));

/**
 * @struct sensor_packet_temperature
 * @brief Structure for temperature sensor packets.
 * @details This structure contains the type, ID, and value of the temperature sensor reading.
 * @note The temperature value is represented in Celsius.
 * @ingroup Packets
 */
struct sensor_packet_temperature {
    struct sensor_metadata metadata;
    /** @brief Value of the sensor reading the temperature represented in Celcius */
    float value;
} __attribute__((packed));

/**
 * @struct sensor_packet_co2
 * @brief Structure for CO2 sensor packets.
 * @details This structure contains the type, ID, and value of the CO2 sensor reading.
 * @note The CO2 value is represented in parts per million (ppm).
 * @ingroup Packets
 */
struct sensor_packet_co2 {
    struct sensor_metadata metadata;
    /** @brief Value of the sensor reading the CO2 level represented in ppm */
    uint16_t value;
} __attribute__((packed));

/**
 * @struct sensor_packet_humidity
 * @brief Structure for humidity sensor packets.
 * @details This structure contains the type, ID, and value of the humidity sensor reading.
 * @note The humidity value is represented in percentage.
 * @ingroup Packets
 */
struct sensor_packet_humidity {
    struct sensor_metadata metadata;
    /** @brief Value of the sensor reading the humidity level represented in percentage */
    float value;
} __attribute__((packed));

/**
 * @struct sensor_packet_light
 * @brief Structure for light sensor packets.
 * @details This structure contains the type, ID, and target state of the light/led. This structure
 * is used for light control packets sent to the light/led.
 * @ingroup Packets
 */
struct sensor_packet_light {
    struct sensor_metadata metadata;
    /** @brief Target state of the light (on 1/off 0) represented as a boolean value */
    uint8_t target_state;
} __attribute__((packed));

/**
 * @struct sensor_packet_rgb_light
 * @brief Structure for RGB light sensor packets.
 * @details This structure contains the type, ID, and target color of the RGB light. This structure
 * is used for RGB light control packets sent to the RGB light.
 * @note The RGB values are represented as 8-bit integers (0-255).
 * @ingroup Packets
 */
struct sensor_packet_rgb_light {
    struct sensor_metadata metadata;
    /** @brief Target state of the red color (0-255) represented as an 8-bit integer */
    uint8_t red_state;
    /** @brief Target state of the green color (0-255) represented as an 8-bit integer */
    uint8_t green_state;
    /** @brief Target state of the blue color (0-255) represented as an 8-bit integer */
    uint8_t blue_state;
} __attribute__((packed));

/**
 * @struct sensor_packet_lichtkrant
 * @brief structure for the lichkrant packets
 * @details Contains the string to display on the lichtkrant
 * @ingroupo Packets
 */
struct sensor_packet_lichtkrant {
    struct sensor_metadata metadata;
    char text[16];
} __attribute__((packed));
// --- End Structures ---

/**
 * @struct sensor_packet
 * @brief Union structure for the entire sensor packet.
 * @details This structure is used to encapsulate the different types of sensor packets that can be
 * sent and has the shape of a valid packet.
 *
 * It contains a sensor_header followed by a union of different sensor data types.
 * The union allows for different types of sensor data to be stored in the same memory location,
 * depending on the packet type.
 *
 * Example usage:
 * ```cpp
 * sensor_packet packet;
 * packet.header.length = sizeof(sensor_packet_generic);
 * packet.header.ptype = PacketType::DATA;
 * packet.data.generic.metadata.sensor_type = SensorType::BUTTON;
 * packet.data.generic.metadata.sensor_id = 1;
 *
 * // Accessing the packet data
 * if (packet.header.ptype == PacketType::DATA) {
 *     if (packet.data.generic.metadata.sensor_type == SensorType::BUTTON) {
 *         uint8_t sensor_id = packet.data.generic.metadata.sensor_id;
 *         // Process button press event for sensor_id
 *     }
 * }
 * ```
 *
 * To use this structure to request data from the dashboard, you can set the ptype to DASHBOARD_GET
 * to indicate that you want to request data from the backend (wemos bridge). Then, you use a
 * sensor_packet_generic
 * to specify the type of sensor you want to request data for and the ID of that sensor.
 *
 * Example:
 * We want to request temperature data from the backend (wemos bridge) for sensor ID 1.
 *
 * ```cpp
 * sensor_packet packet;
 * packet.header.length = sizeof(sensor_packet_generic);
 * packet.header.ptype = PacketType::DASHBOARD_GET;
 * packet.data.generic.metadata.sensor_type = SensorType::TEMPERATURE;
 * packet.data.generic.metadata.sensor_id = 1;
 * ```
 *
 * The backend (wemos bridge) will then respond with a packet of type DASHBOARD_RESPONSE containing
 * the requested data. Following the correct type packet for this example would be a
 * sensor_packet_temperature.
 *
 * Example:
 * We want to change the color of an RGB light with ID 1 to red (255, 0, 0).
 *
 * ```cpp
 * sensor_packet packet;
 * packet.header.length = sizeof(sensor_packet_rgb_light);
 * packet.header.ptype = PacketType::DASHBOARD_POST;
 * packet.data.rgb_light.metadata.sensor_type = SensorType::RGB_LIGHT;
 * packet.data.rgb_light.metadata.sensor_id = 1;
 * packet.data.rgb_light.red_state = 255;
 * packet.data.rgb_light.green_state = 0;
 * packet.data.rgb_light.blue_state = 0;
 * ```
 *
 * @note The data field is a union that can hold different types of sensor data.
 * @ingroup Packets
 */
struct sensor_packet {
    /** @brief Header of the packet containing length and type information */
    struct sensor_header header;

    /** @brief  */
    union sensor_data {
        struct sensor_heartbeat heartbeat;
        struct sensor_packet_generic generic;
        struct sensor_packet_temperature temperature;
        struct sensor_packet_co2 co2;
        struct sensor_packet_humidity humidity;
        struct sensor_packet_light light;
        struct sensor_packet_rgb_light rgb_light;
        struct sensor_packet_lichtkrant lichtkrant;
    } data;
} __attribute__((packed));

#endif  // PACKETS_H
