// ============================================================
// Visualizer.cpp – Konstruktor, SDL-Init, Thread-Logik
// ============================================================
#include "Visualizer.hpp"
#include "SortAlgorithms.hpp"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <array>
#include <chrono>
#include <format>
#include <random>
#include <ranges>
#include <stdexcept>
#include <cmath>
#include <numbers>
#include <algorithm>

static constexpr int32_t WIN_W     = 1400;
static constexpr int32_t WIN_H     = 860;
static constexpr float   METRICS_W = 380.0f;
static constexpr float   UI_H      = 170.0f;
static constexpr float   VIS_X     = 10.0f;
static constexpr float   VIS_H     = WIN_H - UI_H - 20.0f;

// ============================================================
Visualizer::Visualizer()
{
    initSDL();
    initButtons();
    m_algoInfo = SortAlgorithms::getInfo(static_cast<uint8_t>(m_algorithm));
    fillRandom();
}

Visualizer::~Visualizer()
{
    joinThread();
    if (m_audioStream) SDL_DestroyAudioStream(m_audioStream);
}

// ============================================================
void Visualizer::initSDL()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
        throw std::runtime_error(std::format("SDL_Init: {}", SDL_GetError()));
    if (!TTF_Init())
        throw std::runtime_error(std::format("TTF_Init: {}", SDL_GetError()));

    m_window.reset(SDL_CreateWindow("Sort Visualizer", WIN_W, WIN_H, 0));
    if (!m_window)
        throw std::runtime_error(std::format("Window: {}", SDL_GetError()));

    m_renderer.reset(SDL_CreateRenderer(m_window.get(), nullptr));
    if (!m_renderer)
        throw std::runtime_error(std::format("Renderer: {}", SDL_GetError()));

    SDL_SetRenderLogicalPresentation(m_renderer.get(), WIN_W, WIN_H, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    m_fontTitle.reset(TTF_OpenFont("DejaVuSans.ttf", 36));
    m_fontLarge.reset(TTF_OpenFont("DejaVuSans.ttf", 15));
    m_fontSmall.reset(TTF_OpenFont("DejaVuSans.ttf", 12));
    m_fontTiny .reset(TTF_OpenFont("DejaVuSans.ttf", 11));
    if (!m_fontLarge || !m_fontSmall || !m_fontTiny || !m_fontTitle)
        throw std::runtime_error(std::format("Font: {}", SDL_GetError()));

    SDL_AudioSpec audioSpec = { SDL_AUDIO_F32, 1, 44100 };
    m_audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audioSpec, nullptr, nullptr);
    if (m_audioStream) SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(m_audioStream));
}

// ============================================================
void Visualizer::initButtons()
{
    // ── Hauptmenü Buttons ──
    float centerX = WIN_W / 2.0f - 125.0f;
    m_btnMenuStart    = {centerX, 300.0f, 250.0f, 60.0f, "Visualizer Starten", false};
    m_btnMenuSettings = {centerX, 400.0f, 250.0f, 60.0f, "Einstellungen", false};
    m_btnMenuQuit     = {centerX, 500.0f, 250.0f, 60.0f, "Beenden", false};

    // ── Einstellungen Buttons & Slider ──
    m_btnSettingsFullscreen = {centerX, 300.0f, 250.0f, 50.0f, "Vollbild: AUS", false};
    m_volumeSliderBg        = {centerX, 450.0f, 250.0f, 20.0f};
    m_btnSettingsBack       = {centerX, 600.0f, 250.0f, 50.0f, "Zurueck zum Menue", false};

    // ── Visualizer UI Buttons ──
    const float row1Y = WIN_H - UI_H + 10.0f;
    const float row2Y = WIN_H - UI_H + 65.0f;
    const float row3Y = WIN_H - UI_H + 115.0f;

    static constexpr std::array<std::string_view, 6> labels
    {{ "QuickSort", "MergeSort (rek)", "MergeSort (it)",
       "HeapSort",  "RadixSort",       "CountingSort" }};

    float bx = VIS_X;
    for (uint8_t i = 0; i < labels.size(); ++i) {
        m_algoButtons.push_back({bx, row1Y, 170.0f, 45.0f, std::string(labels[i]), (i == 0)});
        bx += 175.0f;
    }

    m_startButton    = {10.0f,  row2Y, 110.0f, 40.0f, "Start",  false};
    m_stopButton     = {130.0f, row2Y, 110.0f, 40.0f, "Stop",   false};
    m_stepBackButton = {250.0f, row2Y,  80.0f, 40.0f, "  < ",   false};
    m_stepFwdButton  = {340.0f, row2Y,  80.0f, 40.0f, "  > ",   false};
    m_randomButton   = {430.0f, row2Y, 110.0f, 40.0f, "Random", false};
    m_sizeDownButton = {10.0f,  row3Y,  40.0f, 35.0f, "-",      false};
    m_sizeUpButton   = {100.0f, row3Y,  40.0f, 35.0f, "+",      false};
    m_viewBarsButton = {160.0f, row3Y, 110.0f, 35.0f, "Balken", true };
    m_viewNumsButton = {280.0f, row3Y, 110.0f, 35.0f, "Zahlen", false};
    m_btnBackToMenu  = {400.0f, row3Y, 140.0f, 35.0f, "Hauptmenue", false};
}

// ============================================================
void Visualizer::fillRandom()
{
    joinThread();

    std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<int32_t> dist(1, 99);

    m_array.resize(m_arraySize);
    std::ranges::generate(m_array, [&]{ return dist(rng); });

    m_history.clear();
    m_metrics        = {};
    m_highlightA     = -1;
    m_highlightB     = -1;
    m_actionText     = "Anfangszustand";
    m_stopRequested  = false;
    m_liveMode       = false;
    m_sorting        = false;

    // Alle Scroll-Positionen auf 0 setzen
    m_explanationScrollY = 0;
    m_explanationScrollX = 0.0f;
    m_numbersScrollY = 0;
    m_numbersScrollX = 0.0f;
    m_autoScrollNumbers = true;

    m_history.push_back({m_array, -1, -1, "Anfangszustand"});
    m_historyIndex = 0;
}

// ============================================================
void Visualizer::joinThread()
{
    if (m_sortThread.joinable())
    {
        m_stopRequested = true;
        { std::lock_guard lock(m_mutex); m_stepRequested = true; }
        m_stepCV.notify_all();
        m_sortThread.join();
    }
    m_sorting = m_stopRequested = m_stepRequested =
                m_stepDone      = m_threadFinished = false;
}

// ============================================================
void Visualizer::sortThreadFunc()
{
    auto cb = [this](const std::vector<int32_t>& arr, int32_t a, int32_t b, std::string_view action)
    {
        onSortStep(arr, a, b, action);
        if (m_stopRequested.load()) throw std::runtime_error("__STOP__");
    };

    try
    {
        switch (m_algorithm)
        {
            case Algorithm::QuickSort:    SortAlgorithms::quickSort   (m_array, cb, m_metrics); break;
            case Algorithm::MergeSortRec: SortAlgorithms::mergeSort   (m_array, cb, m_metrics); break;
            case Algorithm::MergeSortIt:  SortAlgorithms::mergeSortIt (m_array, cb, m_metrics); break;
            case Algorithm::HeapSort:     SortAlgorithms::heapSort    (m_array, cb, m_metrics); break;
            case Algorithm::RadixSort:    SortAlgorithms::radixSort   (m_array, cb, m_metrics); break;
            case Algorithm::CountingSort: SortAlgorithms::countingSort(m_array, cb, m_metrics); break;
        }
    }
    catch (const std::runtime_error& e)
    { if (std::string_view{e.what()} != "__STOP__") throw; }
    catch (...) {}

    m_threadFinished = true;
    m_sorting        = false;
    m_mainCV.notify_all();
}

// ============================================================
void Visualizer::playBeep(int32_t value, int32_t maxValue, int32_t durationMs)
{
    if (!m_audioStream || maxValue == 0 || durationMs <= 0 || m_volume <= 0.01f) return;

    const float minFreq = 150.0f;
    const float maxFreq = 1500.0f;
    const float freq = minFreq + (static_cast<float>(value) / static_cast<float>(maxValue)) * (maxFreq - minFreq);

    const int sampleRate = 44100;
    const int numSamples = std::max(1, (sampleRate * durationMs) / 1000);

    std::vector<float> samples(numSamples);
    float phase = 0.0f;
    const float phaseIncrement = (2.0f * std::numbers::pi_v<float> * freq) / static_cast<float>(sampleRate);

    for (int i = 0; i < numSamples; ++i)
    {
        float sample = std::sin(phase) * m_volume;
        if (i > numSamples - 50 && numSamples > 50) {
            sample *= static_cast<float>(numSamples - i) / 50.0f;
        }
        samples[i] = sample;
        phase += phaseIncrement;
    }

    SDL_PutAudioStreamData(m_audioStream, samples.data(), samples.size() * sizeof(float));
}

// ============================================================
void Visualizer::onSortStep(const std::vector<int32_t>& arr, int32_t a, int32_t b, std::string_view action)
{
    {
        std::lock_guard lock(m_mutex);
        if (static_cast<int32_t>(m_history.size()) < MAX_HISTORY)
            m_history.push_back({arr, a, b, std::string(action)});
    }

    {
        std::lock_guard lock(m_mutex);
        m_array = arr;
        m_highlightA = a;
        m_highlightB = b;
        m_actionText = action;
        m_metrics.elapsedMs = std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - m_sortStart).count();
    }

    if (m_liveMode)
    {
        int32_t maxVal = arr.empty() ? 1 : *std::ranges::max_element(arr);
        int32_t valForPitch = maxVal / 2;

        if (a >= 0 && a < static_cast<int32_t>(arr.size())) valForPitch = arr[a];
        else if (b >= 0 && b < static_cast<int32_t>(arr.size())) valForPitch = arr[b];

        playBeep(valForPitch, maxVal, std::max(1u, m_delayMs));

        if (m_delayMs > 0) std::this_thread::sleep_for(std::chrono::milliseconds(m_delayMs));
        if (m_stopRequested.load()) throw std::runtime_error("__STOP__");
    }
    else
    {
        m_stepDone = true;
        m_mainCV.notify_one();

        std::unique_lock lock(m_mutex);
        m_stepCV.wait(lock, [this]{ return m_stepRequested.load() || m_stopRequested.load(); });
        m_stepRequested = false;

        std::lock_guard lockHist(m_mutex);
        m_historyIndex = static_cast<int32_t>(m_history.size()) - 1;
    }
}

// ============================================================
void Visualizer::startLive()
{
    if (m_sorting) return;
    joinThread();

    m_liveMode = m_sorting = true;
    m_stopRequested = m_stepRequested = m_stepDone = m_threadFinished = false;
    m_metrics  = {};

    const SortStep anfang = m_history.empty() ? SortStep{m_array, -1, -1, "Anfangszustand"} : m_history.front();
    m_history.clear();
    m_history.push_back(anfang);
    m_array = anfang.array; m_historyIndex = 0;
    m_sortStart = std::chrono::steady_clock::now();

    m_sortThread = std::thread(&Visualizer::sortThreadFunc, this);
}

// ============================================================
void Visualizer::startStepping()
{
    if (m_sorting) return;
    joinThread();

    m_liveMode = false; m_sorting = true;
    m_stopRequested = m_stepRequested = m_stepDone = m_threadFinished = false;
    m_metrics  = {};

    const SortStep anfang = m_history.empty() ? SortStep{m_array, -1, -1, "Anfangszustand"} : m_history.front();
    m_history.clear();
    m_history.push_back(anfang);
    m_array = anfang.array; m_historyIndex = 0;
    m_sortStart  = std::chrono::steady_clock::now();
    m_sortThread = std::thread(&Visualizer::sortThreadFunc, this);

    {
        std::unique_lock lock(m_mutex);
        m_mainCV.wait_for(lock, std::chrono::milliseconds(500), [this]{ return m_stepDone.load() || m_threadFinished.load(); });
        m_stepDone = false;
    }
}

// ============================================================
void Visualizer::run()
{
    while (m_running)
    {
        handleEvents();
        draw();
        SDL_Delay(16);
    }
}