#pragma once
#include "common_types.h"
#include <functional>

class IBmsClient {
public:
    // Callback types
    using DataCallback = std::function<void(const BmsData&)>;
    using StatusCallback = std::function<void(ConnectionState)>;

    virtual ~IBmsClient() = default;
    virtual void setup() = 0;
    virtual void update() = 0;
    virtual bool isConnected() const = 0;
};