#pragma once

//------------------------------------------------------------------------------
// Keep it to the color formats you are realistically going to render to.
// Depth/stencil usually live in a separate enum (`RHIDepthFormat`) so they
// don’t show up in swap-chain or render-target picks.
//
enum class RHIFormat : uint8_t   // 1-byte enum is plenty
{
    // 8-bit RGBA – the workhorse on *all* platforms
    RGBA8_UNorm,
    RGBA8_SRGB,

    // 8-bit BGRA – preferred on Windows / Wayland because it matches the OS
    BGRA8_UNorm,
    BGRA8_SRGB,

    // 10-bit HDR formats (optional; add only if you target HDR)
    RGB10A2_UNorm,
    RGB10A2_UNorm_Rev,   // (Metal / Apple reverses channels)

    // 16-bit float HDR (optional but nice for off-screen buffers)
    RGBA16_Float,

    // Sentinel
    Unknown
};