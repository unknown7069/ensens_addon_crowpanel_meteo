#pragma once

// Periodically publishes the BM8563 RTC time to the dashboard clock label.
class RtcClockUpdater
{
    RtcClockUpdater()                              = default;
    RtcClockUpdater(const RtcClockUpdater&)            = delete;
    RtcClockUpdater& operator=(const RtcClockUpdater&) = delete;

public:
    static RtcClockUpdater& instance()
    {
        static RtcClockUpdater instance;
        return instance;
    }

    void init();
};
