#pragma once

enum class ConnectionState {
    Connecting,
    Connected
};

struct BmsData {
    float voltage;
    float current;
    float power;
    uint16_t soc;
    uint32_t latency_ms;
};