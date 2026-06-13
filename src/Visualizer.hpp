// ============================================================
// Visualizer.hpp – Hauptklasse
// ============================================================
#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>
#include "SortAlgorithms.hpp"

// ── App Status ───────────────────────────────────────────────
enum class AppState  : uint8_t { MainMenu, Settings, Visualizer };
enum class ViewMode  : uint8_t { Bars, Numbers };
enum class Algorithm : uint8_t
{
    QuickSort, MergeSortRec, MergeSortIt,
    HeapSort,  RadixSort,    CountingSort, BubbleSort
};

struct Button
{
    float       x{}, y{};
    float       w{}, h{};
    std::string label;
    bool        active{false};
};

struct SdlWindowDeleter   { void operator()(SDL_Window* w) const noexcept { SDL_DestroyWindow(w);   } };
struct SdlRendererDeleter { void operator()(SDL_Renderer* r) const noexcept { SDL_DestroyRenderer(r); } };
struct TtfFontDeleter     { void operator()(TTF_Font* f) const noexcept { TTF_CloseFont(f);       } };

class Visualizer
{
public:
    Visualizer();
    ~Visualizer();

    Visualizer(const Visualizer&)            = delete;
    Visualizer& operator=(const Visualizer&) = delete;
    Visualizer(Visualizer&&)                 = default;
    Visualizer& operator=(Visualizer&&)      = default;

    void run();

private:
    std::unique_ptr<SDL_Window,   SdlWindowDeleter>   m_window;
    std::unique_ptr<SDL_Renderer, SdlRendererDeleter> m_renderer;
    std::unique_ptr<TTF_Font,     TtfFontDeleter>     m_fontTitle;
    std::unique_ptr<TTF_Font,     TtfFontDeleter>     m_fontLarge;
    std::unique_ptr<TTF_Font,     TtfFontDeleter>     m_fontSmall;
    std::unique_ptr<TTF_Font,     TtfFontDeleter>     m_fontTiny;

    SDL_AudioStream* m_audioStream{nullptr};

    // ── Globale Einstellungen ─────────────────────────────────
    AppState    m_appState  {AppState::MainMenu};
    bool        m_fullscreen{false};
    float       m_volume    {0.1f};
    bool        m_isDraggingVolume{false};
    SDL_FRect   m_volumeSliderBg{};

    // ── Array & Zustand ───────────────────────────────────────
    std::vector<int32_t> m_array;
    Algorithm   m_algorithm {Algorithm::QuickSort};
    ViewMode    m_viewMode  {ViewMode::Bars};
    int32_t     m_arraySize {20};
    bool        m_running   {true};
    bool        m_sorting   {false};
    int32_t     m_highlightA{-1};
    int32_t     m_highlightB{-1};
    uint32_t    m_delayMs   {10};
    std::string m_actionText;
    bool        m_liveMode  {false};

    // ── Threading & Playback ──────────────────────────────────
    std::thread             m_sortThread;
    mutable std::mutex      m_mutex;
    std::atomic<bool>       m_stopRequested {false};
    std::atomic<bool>       m_threadFinished{false};

    std::vector<SortStep>   m_history;
    int32_t                 m_historyIndex{-1};
    static constexpr int32_t MAX_HISTORY{10'000};

    std::vector<int32_t>    m_finalStepForIndex;

    LiveMetrics m_metrics;
    AlgoInfo    m_algoInfo;
    std::chrono::steady_clock::time_point m_sortStart;
    std::chrono::steady_clock::time_point m_lastStepTime;

    // ── Scrolling Status ──────────────────────────────────────
    int32_t m_explanationScrollY{0};
    float   m_explanationScrollX{0.0f};
    int32_t m_numbersScrollY{0};
    float   m_numbersScrollX{0.0f};
    bool    m_autoScrollNumbers{true};

    // ── Buttons ───────────────────────────────────────────────
    Button m_btnMenuStart, m_btnMenuSettings, m_btnMenuQuit;
    Button m_btnSettingsFullscreen, m_btnSettingsBack;

    std::vector<Button> m_algoButtons;
    Button m_startButton, m_stopButton, m_cancelButton, m_stepBackButton, m_stepFwdButton;
    Button m_randomButton, m_sizeUpButton, m_sizeDownButton;
    Button m_viewBarsButton, m_viewNumsButton;
    Button m_btnBackToMenu, m_btnBenchmark; // NEU: Benchmark Button

    void initSDL();
    void initButtons();
    void fillRandom();
    void startLive();
    void startStepping();
    void joinThread();
    void sortThreadFunc();
    void onSortStep(const std::vector<int32_t>& arr, int32_t a, int32_t b, std::string_view action);
    void playBeep(int32_t value, int32_t maxValue, int32_t durationMs);

    void pauseSort();
    void resumeSort();
    void cancelSort();
    void stepForward();
    void stepBackward();
    void applyHistoryStep(int32_t index);

    void handleEvents();
    void handleMainMenuClick(float mx, float my);
    void handleSettingsClick(float mx, float my);
    void handleButtonClick(float mx, float my);

    void draw();
    void drawMainMenu();
    void drawSettings();
    void drawBarsView();
    void drawNumbersView();
    void drawMetricsPanel();
    void drawExplanationPanel();
    void drawButtons();
    void drawText(std::string_view text, float x, float y, SDL_Color color, TTF_Font* font);
    void drawTextWrapped(std::string_view text, float x, float y, float maxWidth, SDL_Color color, TTF_Font* font, float& outHeight);

    [[nodiscard]] bool isInside(float mx, float my, const Button& btn) const;
};