#pragma once

enum class RHIPresentMode : uint8_t
{
    // Guaranteed to exist on *every* WSI-backed API
    Fifo,           // V-sync, double-buffered

    // Extras – check support before using
    Mailbox,        // Triple-buffered, low latency (VK_PRESENT_MODE_MAILBOX_KHR)
    Immediate,      // Tear-allowed (VK_PRESENT_MODE_IMMEDIATE_KHR)
    FifoRelaxed,    // V-sync unless late (like VK fifo-relaxed)

    // Future-proof extension slot
    // (e.g. adaptive-sync once standardized across back-ends)
    Unknown
};
