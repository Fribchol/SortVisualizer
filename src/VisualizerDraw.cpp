// ============================================================
// VisualizerDraw.cpp – Rendern der Visualisierung
// ============================================================
#include "Visualizer.hpp"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>
#include <format>
#include <ranges>
#include <sstream>

static constexpr int32_t WIN_W     = 1400;
static constexpr int32_t WIN_H     = 860;
static constexpr float   METRICS_W = 380.0f;
static constexpr float   UI_H      = 170.0f;
static constexpr float   VIS_X     = 10.0f;
static constexpr float   VIS_Y     = 10.0f;
static constexpr float   VIS_W     = WIN_W - METRICS_W - 20.0f;
static constexpr float   VIS_H     = WIN_H - UI_H - 20.0f;
static constexpr float   MET_X     = WIN_W - METRICS_W;
static constexpr float   MET_H     = WIN_H - UI_H;

struct SdlSurfaceDeleter { void operator()(SDL_Surface* s) const noexcept { SDL_DestroySurface(s); } };
struct SdlTextureDeleter { void operator()(SDL_Texture* t) const noexcept { SDL_DestroyTexture(t); } };
using SurfacePtr = std::unique_ptr<SDL_Surface, SdlSurfaceDeleter>;
using TexturePtr = std::unique_ptr<SDL_Texture, SdlTextureDeleter>;

void Visualizer::drawText(std::string_view text, float x, float y, SDL_Color color, TTF_Font* font)
{
    if (text.empty() || !font) return;
    SurfacePtr surf{ TTF_RenderText_Blended(font, std::string(text).c_str(), 0, color) };
    if (!surf) return;
    TexturePtr tex{ SDL_CreateTextureFromSurface(m_renderer.get(), surf.get()) };
    surf.reset();
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
        currentY += lineH; outHeight += lineH;
    };

    std::istringstream iss{std::string(text)};
    std::string word;
    while (iss >> word)
    {
        const std::string test = currentLine.empty() ? word : currentLine + " " + word;
        int tw{}, th{};
        TTF_GetStringSize(font, test.c_str(), 0, &tw, &th);
        if (static_cast<float>(tw) > maxWidth && !currentLine.empty())
        { flushLine(currentLine); currentLine = word; }
        else
            currentLine = test;
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

    drawText(std::format("Lautstaerke: {}%", static_cast<int>(m_volume * 100)), m_volumeSliderBg.x, m_volumeSliderBg.y - 30.0f, {200, 200, 200, 255}, m_fontLarge.get());

    SDL_SetRenderDrawColor(m_renderer.get(), 30, 30, 45, 255);
    SDL_RenderFillRect(m_renderer.get(), &m_volumeSliderBg);

    SDL_FRect thumb = {m_volumeSliderBg.x + m_volume * m_volumeSliderBg.w - 10.0f, m_volumeSliderBg.y - 10.0f, 20.0f, m_volumeSliderBg.h + 20.0f};
    SDL_SetRenderDrawColor(m_renderer.get(), 100, 200, 100, 255);
    SDL_RenderFillRect(m_renderer.get(), &thumb);
}

void Visualizer::drawBarsView()
{
    std::lock_guard lock(m_mutex);
    if (m_array.empty()) return;

    const int32_t maxVal = std::ranges::max(m_array);
    const float   barW   = VIS_W / static_cast<float>(m_array.size());

    for (int32_t i = 0; i < static_cast<int32_t>(m_array.size()); ++i)
    {
        const float barH = (static_cast<float>(m_array[i]) / static_cast<float>(maxVal)) * VIS_H;
        const float bx   = VIS_X + i * barW;
        const float by   = VIS_Y + VIS_H - barH;

        if      (i == m_highlightA) SDL_SetRenderDrawColor(m_renderer.get(), 255,  80,  80, 255);
        else if (i == m_highlightB) SDL_SetRenderDrawColor(m_renderer.get(), 255, 220,  50, 255);
        else                        SDL_SetRenderDrawColor(m_renderer.get(),  80, 160, 255, 255);

        SDL_FRect barRect{bx + 1.0f, by, barW - 2.0f, barH};
        SDL_RenderFillRect(m_renderer.get(), &barRect);

        if (barW > 28.0f)
            drawText(std::format("{}", m_array[i]), bx + 2.0f, by - 15.0f,
                     {220, 220, 220, 255}, m_fontTiny.get());
    }

    drawText(m_actionText, VIS_X, VIS_Y + VIS_H + 4.0f,
             {200, 200, 200, 255}, m_fontTiny.get());

    if (!m_history.empty() && m_historyIndex >= 0)
    {
        drawText(std::format("Schritt {}/{}",
                             m_historyIndex + 1,
                             static_cast<int32_t>(m_history.size())),
                 VIS_X + 400.0f, VIS_Y + VIS_H + 4.0f,
                 {150, 200, 255, 255}, m_fontTiny.get());
    }
}

// ── DIE ZAHLEN-ANSICHT MIT SICHTBAREN SCROLLBALKEN ───────────
void Visualizer::drawNumbersView()
{
    constexpr float lineH = 26.0f;
    constexpr float numW  = 40.0f;
    const float startArrayX = VIS_X + 45.0f;

    int32_t histSize{};
    int32_t currentArraySize = 0;
    {
        std::lock_guard lock(m_mutex);
        histSize = static_cast<int32_t>(m_history.size());
        currentArraySize = m_array.size();
    }
    if (histSize == 0) return;

    const int32_t maxLines = static_cast<int32_t>(VIS_H / lineH);

    // Vertikales Scrolling (Einheitlich Top-Down)
    int32_t maxScrollY = std::max(0, histSize - maxLines);

    if (m_autoScrollNumbers) {
        if (m_liveMode && m_sorting) {
            m_numbersScrollY = maxScrollY;
        } else if (m_historyIndex >= 0) {
            m_numbersScrollY = std::clamp(m_historyIndex - maxLines / 2, 0, maxScrollY);
        }
    }
    m_numbersScrollY = std::clamp(m_numbersScrollY, 0, maxScrollY);

    // Horizontales Scrolling
    float maxItemWidth = currentArraySize * numW + 600.0f;
    float maxScrollX = std::max(0.0f, maxItemWidth - VIS_W);
    m_numbersScrollX = std::clamp(m_numbersScrollX, 0.0f, maxScrollX);

    const int32_t startIdx = m_numbersScrollY;
    const float offsetX = -m_numbersScrollX;

    SDL_Rect clipRect = { static_cast<int>(VIS_X), static_cast<int>(VIS_Y), static_cast<int>(VIS_W), static_cast<int>(VIS_H) };
    SDL_SetRenderClipRect(m_renderer.get(), &clipRect);

    // Zahlen zeichnen
    for (int32_t s = startIdx; s < std::min(histSize, startIdx + maxLines); ++s)
    {
        SortStep step;
        {
            std::lock_guard lock(m_mutex);
            if (s >= static_cast<int32_t>(m_history.size())) break;
            step = m_history[s];
        }

        const auto& [array, indexA, indexB, action] = step;
        const float   fy = VIS_Y + (s - startIdx) * lineH;
        const int32_t n  = static_cast<int32_t>(array.size());

        const bool isCur = m_liveMode ? (s == histSize - 1) : (s == m_historyIndex && m_historyIndex >= 0);

        if (isCur) {
            SDL_SetRenderDrawColor(m_renderer.get(), 40, 40, 70, 255);
            SDL_FRect curRect{VIS_X, fy, VIS_W - 10.0f, lineH};
            SDL_RenderFillRect(m_renderer.get(), &curRect);
        }

        drawText(std::format("{}:", s + 1), VIS_X, fy + 4.0f,
                 isCur ? SDL_Color{255,220,60,255} : SDL_Color{130,130,130,255}, m_fontTiny.get());

        for (int32_t i = 0; i < n; ++i)
        {
            const float fx = startArrayX + i * numW + offsetX;
            if (fx > VIS_X + VIS_W) break;
            if (fx + numW < startArrayX - 10.0f) continue;

            if (i == indexA || i == indexB) {
                SDL_SetRenderDrawColor(m_renderer.get(),
                    i == indexA ? 160 : 160, i == indexA ?  30 : 140, i == indexA ?  30 :  20, 255);
                SDL_FRect highlightRect{fx - 2.0f, fy + 2.0f, numW - 2.0f, lineH - 4.0f};
                SDL_RenderFillRect(m_renderer.get(), &highlightRect);
            }
            drawText(std::format("{}", array[i]), fx, fy + 4.0f,
                     (i == indexA || i == indexB) ? SDL_Color{255,255,100,255} : SDL_Color{200,200,200,255},
                     m_fontTiny.get());
        }

        if (indexA >= 0 && indexB >= 0 && indexA != indexB)
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

        const float actionX = startArrayX + n * numW + 8.0f + offsetX;
        if (actionX < VIS_X + VIS_W) {
            drawText(action, actionX, fy + 4.0f, {100, 190, 100, 255}, m_fontTiny.get());
        }
    }

    SDL_SetRenderClipRect(m_renderer.get(), nullptr);

    // ── SICHTBARER VERTIKALER SCROLLBALKEN (LINKS) ──
    if (histSize > maxLines) {
        const float sbX = VIS_X + VIS_W - 6.0f;
        const float sbH = VIS_H - 10.0f; // Etwas Platz unten für den horizontalen
        SDL_SetRenderDrawColor(m_renderer.get(), 35, 35, 55, 255);
        SDL_FRect sbRect{sbX, VIS_Y, 4.0f, sbH};
        SDL_RenderFillRect(m_renderer.get(), &sbRect);

        const float thumbH = std::max(sbH * static_cast<float>(maxLines) / histSize, 8.0f);
        const float thumbPos = sbH * static_cast<float>(m_numbersScrollY) / histSize;
        SDL_SetRenderDrawColor(m_renderer.get(), 100, 140, 200, 255);
        SDL_FRect thumbRect{sbX, VIS_Y + thumbPos, 4.0f, thumbH};
        SDL_RenderFillRect(m_renderer.get(), &thumbRect);
    }

    // ── SICHTBARER HORIZONTALER SCROLLBALKEN (LINKS) ──
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

    drawText("Schritt-Erklaerung:", panelX, panelY, colTitle, m_fontSmall.get());

    int32_t histSize{};
    { std::lock_guard lock(m_mutex);
      histSize = static_cast<int32_t>(m_history.size()); }
    if (histSize == 0) return;

    const int32_t maxLines = static_cast<int32_t>((panelEnd - panelY - 20.0f) / lineH);
    const int32_t curIdx   = m_liveMode ? histSize - 1 : std::max(0, m_historyIndex);

    // Vertikales Scrollen (Einheitlich Top-Down)
    const int32_t maxScrollY = std::max(0, histSize - maxLines);
    if (m_autoScrollNumbers) m_explanationScrollY = maxScrollY;
    m_explanationScrollY = std::clamp(m_explanationScrollY, 0, maxScrollY);

    // Horizontales Scrollen
    float maxExpScrollX = std::max(0.0f, 1500.0f - METRICS_W); // Geschätzte max. Textbreite
    m_explanationScrollX = std::clamp(m_explanationScrollX, 0.0f, maxExpScrollX);

    const int32_t startIdx = m_explanationScrollY;
    const int32_t endIdx   = std::min(histSize, startIdx + maxLines);

    SDL_Rect clipRect = { static_cast<int>(MET_X), static_cast<int>(panelY + 18.0f), static_cast<int>(METRICS_W), static_cast<int>(MET_H - panelY - 18.0f) };
    SDL_SetRenderClipRect(m_renderer.get(), &clipRect);

    float cy = panelY + 20.0f;
    for (int32_t s = startIdx; s < endIdx && cy < panelEnd; ++s)
    {
        SortStep step;
        { std::lock_guard lock(m_mutex);
          if (s >= static_cast<int32_t>(m_history.size())) break;
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

    // ── SICHTBARER VERTIKALER SCROLLBALKEN (RECHTS) ──
    if (histSize > maxLines)
    {
        const float sbX = static_cast<float>(WIN_W) - 6.0f;
        const float sbH = panelEnd - panelY - 20.0f;
        SDL_SetRenderDrawColor(m_renderer.get(), 35, 35, 55, 255);
        SDL_FRect sbRect{sbX, panelY + 20.0f, 4.0f, sbH};
        SDL_RenderFillRect(m_renderer.get(), &sbRect);

        const float thumbH   = std::max(sbH * static_cast<float>(maxLines) / histSize, 8.0f);
        const float thumbPos = sbH * static_cast<float>(m_explanationScrollY) / histSize;
        SDL_SetRenderDrawColor(m_renderer.get(), 100, 140, 200, 255);
        SDL_FRect thumbRect{sbX, panelY + 20.0f + thumbPos, 4.0f, thumbH};
        SDL_RenderFillRect(m_renderer.get(), &thumbRect);
    }

    // ── SICHTBARER HORIZONTALER SCROLLBALKEN (RECHTS) ──
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
                   static_cast<int32_t>(MET_X), 0,
                   static_cast<int32_t>(MET_X), static_cast<int32_t>(MET_H));

    drawText(m_algoInfo.name, MET_X + 12.0f, 12.0f, {120, 200, 255, 255}, m_fontLarge.get());

    SDL_SetRenderDrawColor(m_renderer.get(), 60, 60, 90, 255);
    SDL_RenderLine(m_renderer.get(), static_cast<int32_t>(MET_X + 6), 36, WIN_W - 4, 36);

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
      SDL_SetRenderDrawColor(m_renderer.get(), m_sorting ? 180 : 35,
                                               m_sorting ?  40 : 35,
                                               m_sorting ?  40 : 45, 255);
      SDL_RenderFillRect(m_renderer.get(), &r);
      SDL_SetRenderDrawColor(m_renderer.get(), 160, 80, 80, 255);
      SDL_RenderRect(m_renderer.get(), &r);
      drawText(m_stopButton.label, m_stopButton.x + 8.0f, m_stopButton.y + 12.0f,
               m_sorting ? SDL_Color{255,200,200,255} : SDL_Color{90,70,70,255},
               m_fontLarge.get());
    }

    const bool canFwd  = !m_liveMode && (!m_sorting || !m_threadFinished ||
                          m_historyIndex < static_cast<int32_t>(m_history.size()) - 1);
    const bool canBack = m_historyIndex > 0 && !m_liveMode;

    drawBtn(m_stepFwdButton,  canFwd);
    drawBtn(m_stepBackButton, canBack);
    drawBtn(m_randomButton,   !m_sorting);
    drawBtn(m_sizeDownButton, !m_sorting);
    drawBtn(m_sizeUpButton,   !m_sorting);
    drawBtn(m_viewBarsButton);
    drawBtn(m_viewNumsButton);
    drawBtn(m_btnBackToMenu,  !m_sorting);

    drawText(std::format("n={}", m_arraySize),
             55.0f, WIN_H - UI_H + 122.0f,
             {200, 200, 200, 255}, m_fontLarge.get());
}

// ── HAUPT-ZEICHENFUNKTION (ROUTING) ──────────────────────────
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
        SDL_RenderLine(m_renderer.get(), 0, WIN_H - static_cast<int32_t>(UI_H),
                       WIN_W, WIN_H - static_cast<int32_t>(UI_H));

        if (m_viewMode == ViewMode::Bars) drawBarsView();
        else                              drawNumbersView();

        drawMetricsPanel();
        drawButtons();
    }

    SDL_RenderPresent(m_renderer.get());
}