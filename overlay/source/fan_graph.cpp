#include "fan_graph.hpp"

#include <algorithm>
#include <cmath>
#include <string>

namespace {

constexpr s32 PanelInset = 7;
constexpr s32 PanelBleed = 1;
constexpr s32 PadTop     = 30;
constexpr s32 PadBottom  = 26;
constexpr s32 PadLeft    = 40;
constexpr s32 PadRight   = 14;
constexpr s32 MaxTicks   = 6;

constexpr u32 TickFont   = 13;
constexpr u32 HeaderFont = 15;

constexpr tsl::Color ColorCold  (0x3, 0xA, 0xF, 0xF);
constexpr tsl::Color ColorHot   (0xF, 0x4, 0x2, 0xF);
constexpr tsl::Color ColorLive  (0x3, 0xF, 0x8, 0xF);
constexpr tsl::Color ColorPlot  (0x0, 0x0, 0x0, 0x5);
constexpr tsl::Color ColorGrid  (0x5, 0x5, 0x5, 0xA);
constexpr tsl::Color ColorAxis  (0x8, 0x8, 0x8, 0xF);
constexpr tsl::Color ColorTick  (0x9, 0x9, 0x9, 0xF);
constexpr tsl::Color ColorPoint (0xF, 0xF, 0xF, 0xF);

int FloorTo(int v, int step) {
    return (int)std::floor((float)v / (float)step) * step;
}

int CeilTo(int v, int step) {
    return (int)std::ceil((float)v / (float)step) * step;
}

tsl::Color CurveColor(float level, u8 alpha) {
    const tsl::Color c = tsl::lerpColor(ColorHot, ColorCold, level);
    return tsl::Color(c.r, c.g, c.b, alpha);
}

tsl::Color Fade(const tsl::Color& c, u8 alpha) {
    return tsl::Color(c.r, c.g, c.b, alpha);
}

}

void DrawFanCurveGraph(tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h, const TemperaturePoint* table, u32 count, float liveTempC) {
    if (count == 0) {
        return;
    }
    if (count > MAX_TABLE_ENTRIES) {
        count = MAX_TABLE_ENTRIES;
    }

    TemperaturePoint curve[MAX_TABLE_ENTRIES];
    memcpy(curve, table, sizeof(TemperaturePoint) * count);
    SortFanCurveTable(curve, count);

    const s32 gx = x + PanelInset;
    const s32 gw = w + PanelBleed;

    const s32 px = gx + PadLeft;
    const s32 py = y + PadTop;
    const s32 pw = gw - PadLeft - PadRight;
    const s32 ph = h - PadTop - PadBottom;

    const bool haveLive = liveTempC >= 0.0f;

    int minT = curve[0].temperature_c;
    int maxT = curve[count - 1].temperature_c;
    if (haveLive) {
        minT = std::min(minT, (int)std::floor(liveTempC));
        maxT = std::max(maxT, (int)std::ceil(liveTempC));
    }

    int domainMin = std::max(0, FloorTo(minT - 5, 10));
    int domainMax = std::min(100, CeilTo(maxT + 5, 10));
    if (domainMax - domainMin < 20) {
        domainMax = std::min(100, domainMin + 20);
        domainMin = std::max(0, domainMax - 20);
    }

    int tickStep = 10;
    while ((domainMax - domainMin) / tickStep > MaxTicks) {
        tickStep += 10;
    }

    const float span = (float)(domainMax - domainMin);

    const auto xAt = [&](float t) -> s32 { return px + (s32)std::lround((std::clamp(t, (float)domainMin, (float)domainMax) - domainMin) / span * (float)(pw - 1)); };
    const auto yAt = [&](float level) -> s32 { return py + (s32)std::lround((1.0f - std::clamp(level, 0.0f, 1.0f)) * (float)(ph - 1)); };

    renderer->drawRoundedRect(gx, y + 3, gw, h - 8, 8, renderer->a(tsl::tableBGColor));
    renderer->drawRect(px, py, pw, ph, renderer->a(ColorPlot));

    for (int level = 0; level <= 100; level += 25) {
        const s32 gridY = yAt((float)level / 100.0f);
        renderer->drawRect(px, gridY, pw, 1, renderer->a(level == 0 ? ColorAxis : ColorGrid));

        const std::string label = std::to_string(level);
        const s32 labelWidth = renderer->getTextDimensions(label, false, TickFont).first;
        renderer->drawString(label, false, px - 7 - labelWidth, gridY + 4, TickFont, renderer->a(ColorTick));
    }

    for (int temp = domainMin; temp <= domainMax; temp += tickStep) {
        const s32 gridX = xAt((float)temp);
        renderer->drawRect(gridX, py, 1, ph, renderer->a(temp == domainMin ? ColorAxis : ColorGrid));

        const std::string label = std::to_string(temp) + (temp == domainMax ? "C" : "");
        const s32 labelWidth = renderer->getTextDimensions(label, false, TickFont).first;
        const s32 labelX = std::clamp(gridX - labelWidth / 2, gx + 4, gx + gw - 4 - labelWidth);
        renderer->drawString(label, false, labelX, py + ph + 15, TickFont, renderer->a(ColorTick));
    }

    const s32 baseY = yAt(0.0f);
    s32 prevY = -1;
    for (s32 i = 0; i < pw; i++) {
        const float temp = (float)domainMin + span * (float)i / (float)(pw - 1);
        const float level = std::clamp(InterpolateFanLevel(curve, count, temp), 0.0f, 1.0f);
        const s32 curveY = yAt(level);

        if (curveY < baseY) {
            renderer->drawRect(px + i, curveY, 1, baseY - curveY, renderer->a(CurveColor(level, 0x4)));
        }

        const s32 top = (prevY < 0) ? curveY : std::min(prevY, curveY);
        const s32 bottom = (prevY < 0) ? curveY : std::max(prevY, curveY);
        renderer->drawRect(px + i, top, 1, std::min(bottom - top + 2, py + ph - top), renderer->a(CurveColor(level, 0xF)));

        prevY = curveY;
    }

    for (u32 i = 0; i < count; i++) {
        const float level = std::clamp(curve[i].fanLevel_f, 0.0f, 1.0f);
        const s32 pointX = xAt((float)curve[i].temperature_c);
        const s32 pointY = yAt(level);

        renderer->drawCircle(pointX, pointY, 3, true, renderer->a(ColorPoint));
        renderer->drawCircle(pointX, pointY, 2, true, renderer->a(CurveColor(level, 0xF)));
    }

    float liveLevel = 0.0f;
    if (haveLive) {
        liveLevel = std::clamp(InterpolateFanLevel(curve, count, liveTempC), 0.0f, 1.0f);

        const s32 liveX = xAt(liveTempC);
        const s32 liveY = yAt(liveLevel);

        for (s32 dashY = liveY + 7; dashY < py + ph; dashY += 5) {
            renderer->drawRect(liveX, dashY, 1, std::min(3, py + ph - dashY), renderer->a(Fade(ColorLive, 0x9)));
        }

        renderer->drawCircle(liveX, liveY, 6, true, renderer->a(Fade(ColorLive, 0x4)));
        renderer->drawCircle(liveX, liveY, 4, true, renderer->a(ColorLive));
        renderer->drawCircle(liveX, liveY, 4, false, renderer->a(ColorPoint));
    }

    const s32 headerBaseline = y + 22;
    renderer->drawString("FAN %", false, gx + 12, headerBaseline, TickFont, renderer->a(ColorTick));

    const std::string readout = haveLive ? std::to_string((int)std::lround(liveTempC)) + "C / " + std::to_string((int)std::lround(liveLevel * 100.0f)) + "%" : std::string("--C / --%");
    const s32 readoutWidth = renderer->getTextDimensions(readout, false, HeaderFont).first;
    const s32 readoutX = gx + gw - 12 - readoutWidth;

    renderer->drawCircle(readoutX - 11, headerBaseline - 5, 3, true, renderer->a(haveLive ? ColorLive : ColorTick));
    renderer->drawString(readout, false, readoutX, headerBaseline, HeaderFont, renderer->a(haveLive ? ColorLive : ColorTick));
}
