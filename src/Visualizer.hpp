#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>
#include "SortAlgorithms.hpp"

// App Status & Spezialszenarien
enum class AppState  : std::uint8_t { MainMenu, Settings, Visualizer };
enum class ViewMode  : std::uint8_t { Bars, Numbers };
enum class SortCase  : std::uint8_t { Random, Sorted, Reverse, Equal };
enum class Algorithm : std::uint8_t
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

// ── RAII Deleter für Smart Pointer
struct SdlWindowDeleter   { void operator()(SDL_Window* w) const noexcept { SDL_DestroyWindow(w);   } };
struct SdlRendererDeleter { void operator()(SDL_Renderer* r) const noexcept { SDL_DestroyRenderer(r); } };
struct TtfFontDeleter     { void operator()(TTF_Font* f) const noexcept { TTF_CloseFont(f);       } };
struct SdlAudioStreamDeleter { void operator()(SDL_AudioStream* s) const noexcept { SDL_DestroyAudioStream(s); } };

class Visualizer
{
public:
    Visualizer();
    ~Visualizer() = default;

    Visualizer(const Visualizer&)            = delete;
    Visualizer& operator=(const Visualizer&) = delete;
    Visualizer(Visualizer&&)                 = delete;
    Visualizer& operator=(Visualizer&&)      = delete;

    void run();

private:
    std::unique_ptr<SDL_Window,    SdlWindowDeleter>      m_window;
    std::unique_ptr<SDL_Renderer,  SdlRendererDeleter>    m_renderer;
    std::unique_ptr<TTF_Font,      TtfFontDeleter>        m_fontTitle;
    std::unique_ptr<TTF_Font,      TtfFontDeleter>        m_fontLarge;
    std::unique_ptr<TTF_Font,      TtfFontDeleter>        m_fontSmall;
    std::unique_ptr<TTF_Font,      TtfFontDeleter>        m_fontTiny;

    std::unique_ptr<SDL_AudioStream, SdlAudioStreamDeleter> m_audioStream;

    AppState    m_appState  {AppState::MainMenu};
    bool        m_fullscreen{false};
    float       m_volume    {0.1f};
    bool        m_isDraggingVolume{false};
    SDL_FRect   m_volumeSliderBg{};

    bool        m_isDraggingSpeed{false};
    SDL_FRect   m_speedSliderBg{};
    bool        m_isDraggingSize{false};
    SDL_FRect   m_sizeSliderBg{};

    std::vector<std::int32_t> m_array;
    Algorithm     m_algorithm {Algorithm::QuickSort};
    ViewMode      m_viewMode  {ViewMode::Bars};
    SortCase      m_sortCase  {SortCase::Random};
    std::int32_t  m_arraySize {20};
    bool          m_running   {true};
    bool          m_sorting   {false};
    std::int32_t  m_highlightA{-1};
    std::int32_t  m_highlightB{-1};
    std::uint32_t m_delayMs   {10};
    bool          m_liveMode  {false};

    std::thread             m_sortThread;
    mutable std::mutex      m_mutex;
    std::atomic<bool>       m_stopRequested {false};
    std::atomic<bool>       m_threadFinished{false};

    std::vector<SortStep>   m_history;
    std::int32_t            m_historyIndex{-1};
    static constexpr std::int32_t MAX_HISTORY{10'000};

    std::vector<std::int32_t> m_finalStepForIndex;

    SortAlgorithms::LiveMetrics m_metrics;
    SortAlgorithms::AlgoInfo    m_algoInfo;

    std::chrono::steady_clock::time_point m_sortStart;
    std::chrono::steady_clock::time_point m_lastStepTime;

    std::int32_t m_explanationScrollY{0};
    float        m_explanationScrollX{0.0f};
    std::int32_t m_numbersScrollY{0};
    float        m_numbersScrollX{0.0f};
    bool         m_autoScrollNumbers{true};

    Button m_btnMenuStart, m_btnMenuSettings, m_btnMenuQuit;
    Button m_btnSettingsFullscreen, m_btnSettingsBack;

    std::vector<Button> m_algoButtons;
    Button m_startButton, m_stopButton, m_cancelButton, m_stepBackButton, m_stepFwdButton;
    Button m_caseRandomBtn, m_caseSortedBtn, m_caseReverseBtn, m_caseEqualBtn;
    Button m_viewBarsButton, m_viewNumsButton;
    Button m_btnBackToMenu, m_btnBenchmark;

    void initSDL();
    void initButtons();
    void fillRandom();
    void fillSpecialCase(SortCase sc);
    void startLive();
    void startStepping();
    void joinThread();
    void sortThreadFunc();

    void onSortStep(const std::vector<std::int32_t>& arr, std::int32_t a, std::int32_t b);
    void playBeep(std::int32_t value, std::int32_t maxValue, std::uint32_t durationMs) const;

    void pauseSort();
    void resumeSort();
    void cancelSort();

    // HIER DIE ERGÄNZTEN METHODEN
    void stepForward();
    void stepBackward();
    void applyHistoryStep(std::int32_t index);

    void handleEvents();
    void handleMainMenuClick(float mx, float my);
    void handleSettingsClick(float mx, float my);
    void handleButtonClick(float mx, float my);

    // Setzt genau einen der vier Case-Buttons (Zufall/Sortiert/Umgekehrt/Gleich) als aktiv (grün)
    void setActiveCaseButton(Button& target);

    void draw();
    void drawMainMenu() const;
    void drawSettings() const;
    void drawBarsView();
    void drawNumbersView();
    void drawMetricsPanel();
    void drawButtons();

    void drawText(std::string_view text, float x, float y, SDL_Color color, TTF_Font* font) const;
    void drawTextWrapped(std::string_view text, float x, float y, float maxWidth, SDL_Color color, TTF_Font* font, float& outHeight) const;

    [[nodiscard]] static bool isInside(float mx, float my, const Button& btn);
};