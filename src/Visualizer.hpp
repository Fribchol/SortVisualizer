#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <span>
#include <stop_token>
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

// ─────────────────────────────────────────────────────────────────────────
// Data-Oriented Design: Statt pro Sortier-Schritt einen eigenen, unabhängig
// geheapten std::vector<int32_t> zu halten (klassisches "Array of Structs",
// viele kleine Allokationen, schlechte Cache-Lokalität), liegen alle
// Array-Schnappschüsse in EINEM zusammenhängenden Puffer (row-major, feste
// Schrittweite = stride). Die Tausch-Indizes (indexA/indexB) werden separat
// als "Struct of Arrays" gehalten, weil sie beim Zeichnen viel häufiger und
// unabhängig von den vollen Arrays durchsucht werden (siehe "touched"-Scans
// in VisualizerDraw.cpp). Ein reserve() am Anfang vermeidet zusätzlich
// tausende Re-Allokationen während einer Sortierung.
// ─────────────────────────────────────────────────────────────────────────
class HistoryBuffer
{
public:
    void reset(std::size_t elementsPerStep, std::size_t expectedSteps = 256)
    {
        m_stride = elementsPerStep;
        m_data.clear();
        m_indexA.clear();
        m_indexB.clear();
        m_data.reserve(elementsPerStep * expectedSteps);
        m_indexA.reserve(expectedSteps);
        m_indexB.reserve(expectedSteps);
    }

    void push(std::span<const std::int32_t> arr, std::int32_t a, std::int32_t b)
    {
        if (m_indexA.size() >= MAX_STEPS) return;
        m_data.insert(m_data.end(), arr.begin(), arr.end());
        m_indexA.push_back(a);
        m_indexB.push_back(b);
    }

    // Wirft bei Aufruf auf einem leeren Puffer bewusst nicht, sondern liefert
    // einen leeren Span zurück - der Aufrufer prüft ohnehin stets empty()/stepCount() zuerst.
    [[nodiscard]] std::span<const std::int32_t> array(std::size_t step) const noexcept
    {
        return {m_data.data() + step * m_stride, m_stride};
    }
    [[nodiscard]] std::int32_t indexA(std::size_t step) const noexcept { return m_indexA[step]; }
    [[nodiscard]] std::int32_t indexB(std::size_t step) const noexcept { return m_indexB[step]; }

    [[nodiscard]] std::size_t stepCount() const noexcept { return m_indexA.size(); }
    [[nodiscard]] bool        empty()     const noexcept { return m_indexA.empty(); }

    // Behält nur den allerersten Schritt (z. B. beim Abbrechen einer Sortierung)
    void keepFirstOnly()
    {
        if (empty()) return;
        m_data.resize(m_stride);
        m_indexA.resize(1);
        m_indexB.resize(1);
    }

private:
    static constexpr std::size_t MAX_STEPS = 100'000;

    std::vector<std::int32_t> m_data;    // alle Schritte, hintereinander (row-major)
    std::vector<std::int32_t> m_indexA;  // ein Eintrag pro Schritt
    std::vector<std::int32_t> m_indexB;  // ein Eintrag pro Schritt
    std::size_t m_stride{0};             // Elemente pro Schritt == Array-Größe
};

class Visualizer
{
public:
    Visualizer();

    // RAII: kein manueller Destruktor mehr nötig! std::jthread (siehe m_sortThread
    // weiter unten) ruft in seinem eigenen Destruktor automatisch request_stop()
    // und join() auf. Da m_sortThread in der Deklarationsreihenfolge NACH den
    // SDL-Handles (Fenster/Renderer/Fonts/Audio) steht, wird er beim Zerstören
    // des Visualizer-Objekts auch VOR diesen SDL-Handles zerstört (C++ zerstört
    // Member stets in umgekehrter Deklarationsreihenfolge). Der Sortier-Thread
    // ist also garantiert beendet, bevor irgendeine SDL-Ressource verschwindet -
    // exakt das Verhalten, das wir vorher manuell nachbauen mussten.
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

    // RAII: std::jthread statt std::thread + manuellem atomic<bool> "stopRequested".
    // Der eingebaute std::stop_token/std::stop_source-Mechanismus übernimmt das
    // Abbruch-Signal; das Objekt joint sich beim Zerstören von selbst.
    std::jthread             m_sortThread;
    mutable std::mutex       m_mutex;
    std::atomic<bool>        m_threadFinished{false};

    // Data-Oriented: zusammenhängender Puffer statt vector<SortStep> mit
    // tausenden Einzel-Allokationen (siehe HistoryBuffer oben).
    HistoryBuffer            m_history;
    std::int32_t             m_historyIndex{-1};

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
    void sortThreadFunc(std::stop_token stopToken);

    void onSortStep(const std::vector<std::int32_t>& arr, std::int32_t a, std::int32_t b);
    void playBeep(std::int32_t value, std::int32_t maxValue, std::uint32_t durationMs) const;

    void pauseSort();
    void resumeSort();
    void cancelSort();

    void stepForward();
    void stepBackward();
    void applyHistoryStep(std::int32_t index);

    void handleEvents();
    void handleMainMenuClick(float mx, float my);
    void handleSettingsClick(float mx, float my);
    void handleButtonClick(float mx, float my);

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