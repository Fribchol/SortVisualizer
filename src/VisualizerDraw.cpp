// ============================================================
// VisualizerDraw.cpp – Rendern der Visualisierung
//
// C++20 Features & Modernisierungen:
// ┌──────────────────────┬─────────────────────────────────────┐
// │ Anonymer Namespace   │ Ersetzt C-static für globale Konst. │
// │                      │ und Hilfs-Structs (Deleter).        │
// │ std::int32_t         │ Explizite Typen aus <cstdint>       │
// │ std::size_t          │ Für alle Container-Größen           │
// │ Umlaute (UTF-8)      │ Sauberes Deutsch in den Menüs       │
// │ RAII                 │ Smart Pointer für SDL-Surfaces      │
// └──────────────────────┴─────────────────────────────────────┘
// ============================================================
#include "Visualizer.hpp"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>
#include <format>
#include <ranges>
#include <sstream>
#include <cstdint> // Für explizite Integer-Typen

// ── Anonymer Namespace ──────────────────────────────────────
// Kapselt Layout-Konstanten und SDL-Deleter strikt in dieser Datei.
namespace {
    constexpr std::int32_t WIN_W     = 1400;
    constexpr std::int32_t WIN_H     = 860;
    constexpr float        METRICS_W = 380.0f;
    constexpr float        UI_H      = 170.0f;
    constexpr float        VIS_X     = 10.0f;
    constexpr float        VIS_Y     = 10.0f;
    constexpr float        VIS_W     = WIN_W - METRICS_W - 20.0f;
    constexpr float        VIS_H     = WIN_H - UI_H - 20.0f;
    constexpr float        MET_X     = WIN_W - METRICS_W;
    constexpr float        MET_H     = WIN_H - UI_H;

    // RAII Deleter für SDL-Ressourcen
    struct SdlSurfaceDeleter { void operator()(SDL_Surface* s) const noexcept { SDL_DestroySurface(s); } };
    struct SdlTextureDeleter { void operator()(SDL_Texture* t) const noexcept { SDL_DestroyTexture(t); } };

    using SurfacePtr = std::unique_ptr<SDL_Surface, SdlSurfaceDeleter>;
    using TexturePtr = std::unique_ptr<SDL_Texture, SdlTextureDeleter>;
}

void Visualizer::drawText(std::string_view text, float x, float y, SDL_Color color, TTF_Font* font)
{
    if (text.empty() || !font) return;

    // Moderne RAII-Verwaltung der rohen C-Pointer von SDL
    SurfacePtr surf{ TTF_RenderText_Blended(font, std::string(text).c_str(), 0, color) };
    if (!surf) return;

    TexturePtr tex{ SDL_CreateTextureFromSurface(m_renderer.get(), surf.get()) };
    surf.reset(); // Surface wird nicht mehr benötigt
    if (!tex) return;

    float tw{}, th{};
    SDL_GetTextureSize(tex.get(), &tw, &th);
    SDL_FRect destRect{x, y, tw, th};
    SDL_RenderTexture(m_renderer.get(), tex.get(), nullptr, &destRect);
}

void Visualizer::drawTextWrapped(std::string_view text, float x, float y, float maxWidth, SDL_Color color, TTF_Font* font, float& outHeight)
{
    outHeight = 0.0f;
    if (text.empty() || !font) return;

    constexpr float lineH = 14.0f;
    std::string currentLine;
    float       currentY = y;

    auto flushLine = [&](const std::string& line)
    {
        if (line.empty()) return;
        drawText(line, x, currentY, color, font);
        currentY += lineH;
        outHeight += lineH;
    };

    std::istringstream iss{std::string(text)};
    std::string word;
    while (iss >> word)
    {
        const std::string test = currentLine.empty() ? word : currentLine + " " + word;
        std::int32_t tw{}, th{};
        TTF_GetStringSize(font, test.c_str(), 0, &tw, &th);

        if (static_cast<float>(tw) > maxWidth && !currentLine.empty())
        {
            flushLine(currentLine);
            currentLine = word;
        }
        else {
            currentLine = test;
        }
    }
    flushLine(currentLine);
}

void Visualizer::drawMainMenu()
{
    drawText("Sort Visualizer", WIN_W / 2.0f - 140.0f, 150.0f, {100, 200, 255, 255}, m_fontTitle.get());

    auto drawBtn = [this](const Button& btn) {
        SDL_FRect r{btn.x, btn.y, btn.w, btn.h};
        SDL_SetRenderDrawColor(m_renderer.get(), 40, 40, 60, 255);
        SDL_RenderFillRect(m_renderer.get(), &r);
        SDL_SetRenderDrawColor(m_renderer.get(), 100, 150, 255, 255);
        SDL_RenderRect(m_renderer.get(), &r);
        drawText(btn.label, btn.x + 35.0f, btn.y + 20.0f, {220, 220, 220, 255}, m_fontLarge.get());
    };

    drawBtn(m_btnMenuStart);
    drawBtn(m_btnMenuSettings);
    drawBtn(m_btnMenuQuit);
}

void Visualizer::drawSettings()
{
    drawText("Einstellungen", WIN_W / 2.0f - 100.0f, 150.0f, {100, 200, 255, 255}, m_fontTitle.get());

    auto drawBtn = [this](const Button& btn) {
        SDL_FRect r{btn.x, btn.y, btn.w, btn.h};
        SDL_SetRenderDrawColor(m_renderer.get(), 40, 40, 60, 255);
        SDL_RenderFillRect(m_renderer.get(), &r);
        SDL_SetRenderDrawColor(m_renderer.get(), 100, 150, 255, 255);
        SDL_RenderRect(m_renderer.get(), &r);
        drawText(btn.label, btn.x + 40.0f, btn.y + 15.0f, {220, 220, 220, 255}, m_fontLarge.get());
    };

    drawBtn(m_btnSettingsFullscreen);
    drawBtn(m_btnSettingsBack);

    drawText(std::format("Lautstärke: {}%", static_cast<std::int32_t>(m_volume * 100)), m_volumeSliderBg.x, m_volumeSliderBg.y - 30.0f, {200, 200, 200, 255}, m_fontLarge.get());

    SDL_SetRenderDrawColor(m_renderer.get(), 30, 30, 45, 255);
    SDL_RenderFillRect(m_renderer.get(), &m_volumeSliderBg);

    SDL_FRect thumb = {m_volumeSliderBg.x + m_volume * m_volumeSliderBg.w - 10.0f, m_volumeSliderBg.y - 10.0f, 20.0f, m_volumeSliderBg.h + 20.0f};
    SDL_SetRenderDrawColor(m_renderer.get(), 100, 200, 100, 255);
    SDL_RenderFillRect(m_renderer.get(), &thumb);
}

void Visualizer::drawBarsView()
{
    std::lock_guard lock(m_mutex);
    if (m_array.empty() || m_history.empty()) return;

    const std::int32_t maxVal = std::ranges::max(m_array);
    const float        barW   = VIS_W / static_cast<float>(m_array.size());
    const float        fixedTopPadding = 50.0f;
    const float        effectiveVisHeight = VIS_H - fixedTopPadding;

    bool isFinished = (m_threadFinished && m_history.size() > 1 && m_historyIndex == static_cast<std::int32_t>(m_history.size()) - 1);

    std::vector<bool> touched(m_array.size(), false);

    if (m_historyIndex > 0 && m_historyIndex < static_cast<std::int32_t>(m_history.size())) {
        for (std::int32_t s = 1; s <= m_historyIndex; ++s) {
            std::int32_t a = m_history[s].indexA;
            std::int32_t b = m_history[s].indexB;
            if (a >= 0 && a < static_cast<std::int32_t>(touched.size())) touched[a] = true;
            if (b >= 0 && b < static_cast<std::int32_t>(touched.size())) touched[b] = true;

            const auto& prevArr = m_history[s-1].array;
            const auto& currArr = m_history[s].array;
            std::size_t minLen = std::min(prevArr.size(), currArr.size());
            for (std::size_t j = 0; j < minLen; ++j) {
                if (currArr[j] != prevArr[j] && j < touched.size()) {
                    touched[j] = true;
                }
            }
        }
    }

    for (std::int32_t i = 0; i < static_cast<std::int32_t>(m_array.size()); ++i)
    {
        const float barH = (static_cast<float>(m_array[i]) / static_cast<float>(maxVal)) * effectiveVisHeight;
        const float bx   = VIS_X + i * barW;
        const float by   = VIS_Y + VIS_H - barH;

        bool shouldBeGreen = isFinished;
        if (!shouldBeGreen && m_threadFinished && i < static_cast<std::int32_t>(m_finalStepForIndex.size())) {
            if (m_historyIndex > m_finalStepForIndex[i] && touched[i]) {
                shouldBeGreen = true;
            }
        }

        if (i == m_highlightA && !isFinished) {
            SDL_SetRenderDrawColor(m_renderer.get(), 255, 60, 60, 255);
        } else if (i == m_highlightB && !isFinished) {
            SDL_SetRenderDrawColor(m_renderer.get(), 255, 220, 50, 255);
        } else if (shouldBeGreen) {
            if (i % 2 == 0) SDL_SetRenderDrawColor(m_renderer.get(), 40, 190, 75, 255);
            else            SDL_SetRenderDrawColor(m_renderer.get(), 50, 210, 85, 255);
        } else {
            if (i % 2 == 0) SDL_SetRenderDrawColor(m_renderer.get(), 45, 90, 170, 255);
            else            SDL_SetRenderDrawColor(m_renderer.get(), 60, 110, 200, 255);
        }

        SDL_FRect barRect{bx, by, barW, barH};
        SDL_RenderFillRect(m_renderer.get(), &barRect);

        if (barW > 28.0f) {
            SDL_Color textColor;
            if (shouldBeGreen) textColor = {200, 255, 200, 255};
            else if (i == m_highlightA || i == m_highlightB) textColor = {255, 255, 255, 255};
            else textColor = {150, 150, 160, 255};

            drawText(std::format("{}", m_array[i]), bx + 2.0f, by - 15.0f, textColor, m_fontTiny.get());
        }
    }
}

void Visualizer::drawNumbersView()
{
    constexpr float lineH = 26.0f;
    constexpr float numW  = 40.0f;
    const float startArrayX = VIS_X + 45.0f;

    std::int32_t visibleHistSize = 0;
    std::int32_t fullHistSize = 0;
    std::int32_t currentArraySize = 0;

    std::vector<std::int32_t> sortedTarget;
    std::vector<std::int32_t> initialArray;

    {
        std::lock_guard lock(m_mutex);
        if (m_history.empty()) return;
        fullHistSize = static_cast<std::int32_t>(m_history.size());
        visibleHistSize = std::clamp(m_historyIndex + 1, 1, fullHistSize);
        currentArraySize = static_cast<std::int32_t>(m_array.size());
        sortedTarget = m_history.front().array;
        initialArray = m_history.front().array;
    }

    std::ranges::sort(sortedTarget);

    const std::int32_t maxLines = static_cast<std::int32_t>(VIS_H / lineH);
    std::int32_t maxScrollY = std::max(0, visibleHistSize - maxLines);

    if (m_autoScrollNumbers) {
        if (m_liveMode && m_sorting) {
            m_numbersScrollY = maxScrollY;
        } else if (m_historyIndex >= 0) {
            m_numbersScrollY = std::clamp(m_historyIndex - maxLines / 2, 0, maxScrollY);
        }
    }
    m_numbersScrollY = std::clamp(m_numbersScrollY, 0, maxScrollY);

    float maxItemWidth = currentArraySize * numW + 100.0f;
    float maxScrollX = std::max(0.0f, maxItemWidth - VIS_W);
    m_numbersScrollX = std::clamp(m_numbersScrollX, 0.0f, maxScrollX);

    const std::int32_t startIdx = m_numbersScrollY;
    const float offsetX = -m_numbersScrollX;

    SDL_Rect clipRect = { static_cast<std::int32_t>(VIS_X), static_cast<std::int32_t>(VIS_Y), static_cast<std::int32_t>(VIS_W), static_cast<std::int32_t>(VIS_H) };
    SDL_SetRenderClipRect(m_renderer.get(), &clipRect);

    std::vector<bool> touched(currentArraySize, false);
    for (std::int32_t s = 1; s <= startIdx && s < visibleHistSize; ++s) {
        std::lock_guard lock(m_mutex);
        std::int32_t a = m_history[s].indexA;
        std::int32_t b = m_history[s].indexB;
        if (a >= 0 && a < static_cast<std::int32_t>(touched.size())) touched[a] = true;
        if (b >= 0 && b < static_cast<std::int32_t>(touched.size())) touched[b] = true;

        const auto& prevArr = m_history[s-1].array;
        const auto& currArr = m_history[s].array;
        std::size_t minLen = std::min(prevArr.size(), currArr.size());
        for (std::size_t j = 0; j < minLen; ++j) {
            if (currArr[j] != prevArr[j] && j < touched.size()) touched[j] = true;
        }
    }

    for (std::int32_t s = startIdx; s < std::min(visibleHistSize, startIdx + maxLines); ++s)
    {
        SortStep step;
        {
            std::lock_guard lock(m_mutex);
            step = m_history[s];
        }

        if (s > 0) {
            std::lock_guard lock(m_mutex);
            std::int32_t a = step.indexA;
            std::int32_t b = step.indexB;
            if (a >= 0 && a < static_cast<std::int32_t>(touched.size())) touched[a] = true;
            if (b >= 0 && b < static_cast<std::int32_t>(touched.size())) touched[b] = true;

            const auto& prevArr = m_history[s-1].array;
            const auto& currArr = step.array;
            std::size_t minLen = std::min(prevArr.size(), currArr.size());
            for (std::size_t j = 0; j < minLen; ++j) {
                if (currArr[j] != prevArr[j] && j < touched.size()) touched[j] = true;
            }
        }

        const auto& [array, indexA, indexB, action] = step;
        const float   fy = VIS_Y + (s - startIdx) * lineH;
        const std::int32_t n  = static_cast<std::int32_t>(array.size());

        const bool isCur = (s == m_historyIndex);
        const bool lineIsFinished = (m_threadFinished && fullHistSize > 1 && s == fullHistSize - 1);

        if (isCur) {
            SDL_SetRenderDrawColor(m_renderer.get(), 40, 40, 70, 255);
            SDL_FRect curRect{VIS_X, fy, VIS_W - 10.0f, lineH};
            SDL_RenderFillRect(m_renderer.get(), &curRect);
        }

        drawText(std::format("{}:", s + 1), VIS_X, fy + 4.0f,
                 isCur ? SDL_Color{255,220,60,255} : SDL_Color{130,130,130,255}, m_fontTiny.get());

        for (std::int32_t i = 0; i < n; ++i)
        {
            const float fx = startArrayX + i * numW + offsetX;
            if (fx > VIS_X + VIS_W) break;
            if (fx + numW < startArrayX - 10.0f) continue;

            if (!lineIsFinished && (i == indexA || i == indexB)) {
                SDL_SetRenderDrawColor(m_renderer.get(),
                    i == indexA ? 200 : 180, i == indexA ?  60 : 180, i == indexA ?  60 :  40, 255);
                SDL_FRect highlightRect{fx - 2.0f, fy + 2.0f, numW - 2.0f, lineH - 4.0f};
                SDL_RenderFillRect(m_renderer.get(), &highlightRect);
            }

            bool isTouchedNow = false;
            if (i < static_cast<std::int32_t>(touched.size())) isTouchedNow = touched[i];

            bool shouldBeGreen = lineIsFinished;
            if (!shouldBeGreen && m_threadFinished && i < static_cast<std::int32_t>(m_finalStepForIndex.size())) {
                if (s > m_finalStepForIndex[i] && touched[i]) {
                    shouldBeGreen = true;
                }
            }

            SDL_Color textColor;
            if (shouldBeGreen) textColor = {100, 255, 100, 255};
            else if (!lineIsFinished && (i == indexA || i == indexB)) textColor = {255, 255, 255, 255};
            else textColor = {130, 130, 140, 255};

            drawText(std::format("{}", array[i]), fx, fy + 4.0f, textColor, m_fontTiny.get());
        }

        if (!lineIsFinished && indexA >= 0 && indexB >= 0 && indexA != indexB)
        {
            const float ax  = startArrayX + indexA * numW + numW * 0.5f + offsetX;
            const float bx2 = startArrayX + indexB * numW + numW * 0.5f + offsetX;
            const float ay  = fy + lineH * 0.5f;
            SDL_SetRenderDrawColor(m_renderer.get(), 255, 80, 80, 255);
            SDL_RenderLine(m_renderer.get(), ax, ay, bx2, ay);
            SDL_RenderLine(m_renderer.get(), ax, ay, ax + 6.0f, ay - 5.0f);
            SDL_RenderLine(m_renderer.get(), ax, ay, ax + 6.0f, ay + 5.0f);
            SDL_RenderLine(m_renderer.get(), bx2, ay, bx2 - 6.0f, ay - 5.0f);
            SDL_RenderLine(m_renderer.get(), bx2, ay, bx2 - 6.0f, ay + 5.0f);
        }
    }

    SDL_SetRenderClipRect(m_renderer.get(), nullptr);

    if (visibleHistSize > maxLines) {
        const float sbX = VIS_X + VIS_W - 6.0f;
        const float sbH = VIS_H - 10.0f;
        SDL_SetRenderDrawColor(m_renderer.get(), 35, 35, 55, 255);
        SDL_FRect sbRect{sbX, VIS_Y, 4.0f, sbH};
        SDL_RenderFillRect(m_renderer.get(), &sbRect);

        const float thumbH = std::max(sbH * static_cast<float>(maxLines) / visibleHistSize, 8.0f);
        const float thumbPos = sbH * static_cast<float>(m_numbersScrollY) / visibleHistSize;
        SDL_SetRenderDrawColor(m_renderer.get(), 100, 140, 200, 255);
        SDL_FRect thumbRect{sbX, VIS_Y + thumbPos, 4.0f, thumbH};
        SDL_RenderFillRect(m_renderer.get(), &thumbRect);
    }

    if (maxScrollX > 0.0f) {
        const float sbY = VIS_Y + VIS_H - 6.0f;
        const float sbW = VIS_W - 10.0f;
        SDL_SetRenderDrawColor(m_renderer.get(), 35, 35, 55, 255);
        SDL_FRect sbRect{VIS_X, sbY, sbW, 4.0f};
        SDL_RenderFillRect(m_renderer.get(), &sbRect);

        const float thumbW = std::max(sbW * VIS_W / (VIS_W + maxScrollX), 8.0f);
        const float thumbPos = sbW * m_numbersScrollX / (VIS_W + maxScrollX);
        SDL_SetRenderDrawColor(m_renderer.get(), 100, 140, 200, 255);
        SDL_FRect thumbRect{VIS_X + thumbPos, sbY, thumbW, 4.0f};
        SDL_RenderFillRect(m_renderer.get(), &thumbRect);
    }
}

void Visualizer::drawExplanationPanel()
{
    constexpr float lineH    = 16.0f;
    constexpr float panelX   = MET_X + 8.0f;
    constexpr float panelY   = 48.0f;
    constexpr float panelEnd = MET_H - 10.0f;

    constexpr SDL_Color colTitle   {100, 180, 255, 255};
    constexpr SDL_Color colCurrent {255, 220,  60, 255};
    constexpr SDL_Color colOld     {140, 160, 180, 255};

    drawText("Schritt-Erklärung:", panelX, panelY, colTitle, m_fontSmall.get());

    std::int32_t visibleHistSize = 0;
    std::int32_t fullHistSize = 0;
    {
        std::lock_guard lock(m_mutex);
        if (m_history.empty()) return;
        fullHistSize = static_cast<std::int32_t>(m_history.size());
        visibleHistSize = std::clamp(m_historyIndex + 1, 1, fullHistSize);
    }

    const std::int32_t maxLines = static_cast<std::int32_t>((panelEnd - panelY - 20.0f) / lineH);
    const std::int32_t curIdx   = m_historyIndex;

    const std::int32_t maxScrollY = std::max(0, visibleHistSize - maxLines);
    if (m_autoScrollNumbers) m_explanationScrollY = maxScrollY;
    m_explanationScrollY = std::clamp(m_explanationScrollY, 0, maxScrollY);

    float maxExpScrollX = std::max(0.0f, 1500.0f - METRICS_W);
    m_explanationScrollX = std::clamp(m_explanationScrollX, 0.0f, maxExpScrollX);

    const std::int32_t startIdx = m_explanationScrollY;
    const std::int32_t endIdx   = std::min(visibleHistSize, startIdx + maxLines);

    SDL_Rect clipRect = { static_cast<std::int32_t>(MET_X), static_cast<std::int32_t>(panelY + 18.0f), static_cast<std::int32_t>(METRICS_W), static_cast<std::int32_t>(MET_H - panelY - 18.0f) };
    SDL_SetRenderClipRect(m_renderer.get(), &clipRect);

    float cy = panelY + 20.0f;
    for (std::int32_t s = startIdx; s < endIdx && cy < panelEnd; ++s)
    {
        SortStep step;
        { std::lock_guard lock(m_mutex);
          step = m_history[s]; }

        const bool isCur = (s == curIdx);
        if (isCur)
        {
            SDL_SetRenderDrawColor(m_renderer.get(), 30, 50, 80, 255);
            SDL_FRect stepRect{MET_X + 2.0f, cy - 1.0f, METRICS_W - 4.0f, lineH + 2.0f};
            SDL_RenderFillRect(m_renderer.get(), &stepRect);
        }

        drawText(std::format("{}.", s + 1), panelX - m_explanationScrollX, cy,
                 isCur ? SDL_Color{255,200,50,255} : SDL_Color{100,120,140,255},
                 m_fontTiny.get());
        drawText(step.action, panelX + 32.0f - m_explanationScrollX, cy,
                 isCur ? colCurrent : colOld, m_fontTiny.get());
        cy += lineH;
    }

    SDL_SetRenderClipRect(m_renderer.get(), nullptr);

    if (visibleHistSize > maxLines)
    {
        const float sbX = static_cast<float>(WIN_W) - 6.0f;
        const float sbH = panelEnd - panelY - 20.0f;
        SDL_SetRenderDrawColor(m_renderer.get(), 35, 35, 55, 255);
        SDL_FRect sbRect{sbX, panelY + 20.0f, 4.0f, sbH};
        SDL_RenderFillRect(m_renderer.get(), &sbRect);

        const float thumbH   = std::max(sbH * static_cast<float>(maxLines) / visibleHistSize, 8.0f);
        const float thumbPos = sbH * static_cast<float>(m_explanationScrollY) / visibleHistSize;
        SDL_SetRenderDrawColor(m_renderer.get(), 100, 140, 200, 255);
        SDL_FRect thumbRect{sbX, panelY + 20.0f + thumbPos, 4.0f, thumbH};
        SDL_RenderFillRect(m_renderer.get(), &thumbRect);
    }

    if (maxExpScrollX > 0.0f) {
        const float sbX = MET_X + 4.0f;
        const float sbY = panelEnd;
        const float sbW = METRICS_W - 10.0f;
        SDL_SetRenderDrawColor(m_renderer.get(), 35, 35, 55, 255);
        SDL_FRect sbRect{sbX, sbY, sbW, 4.0f};
        SDL_RenderFillRect(m_renderer.get(), &sbRect);

        const float thumbW = std::max(sbW * METRICS_W / (METRICS_W + maxExpScrollX), 8.0f);
        const float thumbPos = sbW * m_explanationScrollX / (METRICS_W + maxExpScrollX);
        SDL_SetRenderDrawColor(m_renderer.get(), 100, 140, 200, 255);
        SDL_FRect thumbRect{sbX + thumbPos, sbY, thumbW, 4.0f};
        SDL_RenderFillRect(m_renderer.get(), &thumbRect);
    }
}

void Visualizer::drawMetricsPanel()
{
    SDL_SetRenderDrawColor(m_renderer.get(), 18, 18, 28, 255);
    SDL_FRect bgRect{MET_X, 0.0f, METRICS_W, MET_H};
    SDL_RenderFillRect(m_renderer.get(), &bgRect);

    SDL_SetRenderDrawColor(m_renderer.get(), 80, 80, 110, 255);
    SDL_RenderLine(m_renderer.get(),
                   static_cast<std::int32_t>(MET_X), 0,
                   static_cast<std::int32_t>(MET_X), static_cast<std::int32_t>(MET_H));

    drawText(m_algoInfo.name, MET_X + 12.0f, 12.0f, {120, 200, 255, 255}, m_fontLarge.get());

    SDL_SetRenderDrawColor(m_renderer.get(), 60, 60, 90, 255);
    SDL_RenderLine(m_renderer.get(), static_cast<std::int32_t>(MET_X + 6), 36, WIN_W - 4, 36);

    drawExplanationPanel();
}

void Visualizer::drawButtons()
{
    auto drawBtn = [this](const Button& btn, bool enabled = true)
    {
        SDL_FRect rect{btn.x, btn.y, btn.w, btn.h};

        if      (!enabled)   SDL_SetRenderDrawColor(m_renderer.get(), 35, 35, 45, 255);
        else if (btn.active) SDL_SetRenderDrawColor(m_renderer.get(), 20,150, 80, 255);
        else                 SDL_SetRenderDrawColor(m_renderer.get(), 50, 50, 70, 255);
        SDL_RenderFillRect(m_renderer.get(), &rect);

        SDL_SetRenderDrawColor(m_renderer.get(),
            enabled ? 160 : 70, enabled ? 160 : 70, enabled ? 180 : 80, 255);
        SDL_RenderRect(m_renderer.get(), &rect);

        drawText(btn.label, btn.x + 8.0f, btn.y + 12.0f,
                 enabled ? SDL_Color{220,220,220,255} : SDL_Color{90,90,100,255},
                 m_fontLarge.get());
    };

    std::ranges::for_each(m_algoButtons, [&](const Button& b){ drawBtn(b, !m_sorting); });

    drawBtn(m_startButton, !m_sorting);

    {
      SDL_FRect r{m_stopButton.x, m_stopButton.y, m_stopButton.w, m_stopButton.h};
      bool canInteract = m_sorting;
      bool isPaused = m_sorting && !m_liveMode;

      if (!canInteract) {
          SDL_SetRenderDrawColor(m_renderer.get(), 35, 35, 45, 255);
      } else if (isPaused) {
          SDL_SetRenderDrawColor(m_renderer.get(), 40, 180, 80, 255);
      } else {
          SDL_SetRenderDrawColor(m_renderer.get(), 200, 120, 30, 255);
      }
      SDL_RenderFillRect(m_renderer.get(), &r);
      SDL_SetRenderDrawColor(m_renderer.get(), canInteract ? 160 : 70, canInteract ? 160 : 70, canInteract ? 180 : 80, 255);
      SDL_RenderRect(m_renderer.get(), &r);

      std::string label = (m_sorting && !m_liveMode) ? "Weiter" : "Pause";
      drawText(label, m_stopButton.x + 20.0f, m_stopButton.y + 12.0f,
               canInteract ? SDL_Color{255,255,255,255} : SDL_Color{90,70,70,255},
               m_fontLarge.get());
    }

    {
      SDL_FRect r{m_cancelButton.x, m_cancelButton.y, m_cancelButton.w, m_cancelButton.h};
      bool canCancel = (m_sorting || m_historyIndex > 0);

      if (!canCancel) {
          SDL_SetRenderDrawColor(m_renderer.get(), 35, 35, 45, 255);
      } else {
          SDL_SetRenderDrawColor(m_renderer.get(), 180, 40, 40, 255);
      }
      SDL_RenderFillRect(m_renderer.get(), &r);
      SDL_SetRenderDrawColor(m_renderer.get(), canCancel ? 160 : 70, canCancel ? 160 : 70, canCancel ? 180 : 80, 255);
      SDL_RenderRect(m_renderer.get(), &r);

      drawText("Abbruch", m_cancelButton.x + 15.0f, m_cancelButton.y + 12.0f,
               canCancel ? SDL_Color{255,220,220,255} : SDL_Color{90,70,70,255},
               m_fontLarge.get());
    }

    const bool canFwd  = !m_liveMode && (m_historyIndex < static_cast<std::int32_t>(m_history.size()) - 1 || !m_sorting);
    const bool canBack = m_historyIndex > 0 && !m_liveMode;

    drawBtn(m_stepFwdButton,  canFwd);
    drawBtn(m_stepBackButton, canBack);
    drawBtn(m_randomButton,   !m_sorting);

    drawBtn(m_speedDownButton, true);
    drawBtn(m_speedUpButton,   true);

    drawText(std::format("{} ms", m_delayMs),
             m_speedUpButton.x + m_speedUpButton.w + 15.0f,
             m_stopButton.y + 12.0f,
             {200, 200, 200, 255}, m_fontLarge.get());

    drawBtn(m_sizeDownButton, !m_sorting);
    drawBtn(m_sizeUpButton,   !m_sorting);
    drawBtn(m_viewBarsButton);
    drawBtn(m_viewNumsButton);
    drawBtn(m_btnBackToMenu,  !m_sorting);
    drawBtn(m_btnBenchmark, true);

    drawText(std::format("n={}", m_arraySize),
             55.0f, WIN_H - UI_H + 122.0f,
             {200, 200, 200, 255}, m_fontLarge.get());
}

void Visualizer::draw()
{
    SDL_SetRenderDrawColor(m_renderer.get(), 22, 22, 32, 255);
    SDL_RenderClear(m_renderer.get());

    if (m_appState == AppState::MainMenu) {
        drawMainMenu();
    }
    else if (m_appState == AppState::Settings) {
        drawSettings();
    }
    else if (m_appState == AppState::Visualizer) {
        SDL_SetRenderDrawColor(m_renderer.get(), 70, 70, 100, 255);
        SDL_RenderLine(m_renderer.get(), 0, WIN_H - static_cast<std::int32_t>(UI_H),
                       WIN_W, WIN_H - static_cast<std::int32_t>(UI_H));

        if (m_viewMode == ViewMode::Bars) drawBarsView();
        else                              drawNumbersView();

        drawMetricsPanel();
        drawButtons();
    }

    SDL_RenderPresent(m_renderer.get());
}