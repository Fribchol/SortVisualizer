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
#include <cstdlib> // Fuer std::system

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
    float centerX = WIN_W / 2.0f - 125.0f;
    m_btnMenuStart    = {centerX, 300.0f, 250.0f, 60.0f, "Visualizer Starten", false};
    m_btnMenuSettings = {centerX, 400.0f, 250.0f, 60.0f, "Einstellungen", false};
    m_btnMenuQuit     = {centerX, 500.0f, 250.0f, 60.0f, "Beenden", false};

    m_btnSettingsFullscreen = {centerX, 300.0f, 250.0f, 50.0f, "Vollbild: AUS", false};
    m_volumeSliderBg        = {centerX, 450.0f, 250.0f, 20.0f};
    m_btnSettingsBack       = {centerX, 600.0f, 250.0f, 50.0f, "Zurueck zum Menue", false};

    const float row1Y = WIN_H - UI_H + 10.0f;
    const float row2Y = WIN_H - UI_H + 65.0f;
    const float row3Y = WIN_H - UI_H + 115.0f;

    static constexpr std::array<std::string_view, 7> labels
    {{ "QuickSort", "MergeSort (rek)", "MergeSort (it)",
       "HeapSort",  "RadixSort",       "CountingSort", "BubbleSort" }};

    float bx = VIS_X;
    for (uint8_t i = 0; i < labels.size(); ++i) {
        m_algoButtons.push_back({bx, row1Y, 170.0f, 45.0f, std::string(labels[i]), (i == 0)});
        bx += 175.0f;
    }

    m_startButton    = {10.0f,  row2Y, 100.0f, 40.0f, "Start",  false};
    m_stopButton     = {120.0f, row2Y, 100.0f, 40.0f, "Stop",   false};
    m_cancelButton   = {230.0f, row2Y, 110.0f, 40.0f, "Abbruch",false};
    m_stepBackButton = {350.0f, row2Y,  70.0f, 40.0f, "  < ",   false};
    m_stepFwdButton  = {430.0f, row2Y,  70.0f, 40.0f, "  > ",   false};
    m_randomButton   = {510.0f, row2Y, 110.0f, 40.0f, "Random", false};

    m_sizeDownButton = {10.0f,  row3Y,  40.0f, 35.0f, "-",      false};
    m_sizeUpButton   = {100.0f, row3Y,  40.0f, 35.0f, "+",      false};
    m_viewBarsButton = {160.0f, row3Y, 110.0f, 35.0f, "Balken", true };
    m_viewNumsButton = {280.0f, row3Y, 110.0f, 35.0f, "Zahlen", false};
    m_btnBackToMenu  = {400.0f, row3Y, 140.0f, 35.0f, "Hauptmenue", false};

    // NEU: Der Shell Benchmark Button
    m_btnBenchmark   = {550.0f, row3Y, 160.0f, 35.0f, "Shell Benchmark", false};
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

    m_explanationScrollY = 0;
    m_explanationScrollX = 0.0f;
    m_numbersScrollY = 0;
    m_numbersScrollX = 0.0f;
    m_autoScrollNumbers = true;

    m_finalStepForIndex.clear();
    m_history.push_back({m_array, -1, -1, "Anfangszustand"});
    m_historyIndex = 0;
}

// ============================================================
void Visualizer::joinThread()
{
    if (m_sortThread.joinable())
    {
        m_stopRequested = true;
        m_sortThread.join();
    }
    m_sorting = m_stopRequested = m_threadFinished = false;
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
            case Algorithm::BubbleSort:   SortAlgorithms::bubbleSort  (m_array, cb, m_metrics); break;
        }

        {
            std::lock_guard lock(m_mutex);
            if (!m_history.empty()) {
                const auto& finalArray = m_history.back().array;
                m_finalStepForIndex.assign(finalArray.size(), -1);

                for (size_t i = 0; i < finalArray.size(); ++i) {
                    int lastWrong = -1;
                    for (size_t s = 0; s < m_history.size(); ++s) {
                        if (i < m_history[s].array.size() && m_history[s].array[i] != finalArray[i]) {
                            lastWrong = static_cast<int>(s);
                        }
                    }
                    m_finalStepForIndex[i] = lastWrong;
                }
            }
        }

    }
    catch (const std::runtime_error& e)
    { if (std::string_view{e.what()} != "__STOP__") throw; }
    catch (...) {}

    m_threadFinished = true;
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
    std::lock_guard lock(m_mutex);
    if (m_history.size() < MAX_HISTORY) {
        m_history.push_back({arr, a, b, std::string(action)});
    }
}

// ============================================================
void Visualizer::startLive()
{
    if (m_sorting) return;
    joinThread();

    m_liveMode = true;
    m_sorting = true;
    m_stopRequested = false;
    m_metrics  = {};

    const SortStep anfang = m_history.empty() ? SortStep{m_array, -1, -1, "Anfangszustand"} : m_history.front();
    m_history.clear();
    m_history.push_back(anfang);
    m_array = anfang.array;
    m_historyIndex = 0;
    m_finalStepForIndex.clear();

    m_sortStart = std::chrono::steady_clock::now();
    m_lastStepTime = m_sortStart;

    m_sortThread = std::thread(&Visualizer::sortThreadFunc, this);
}

// ============================================================
void Visualizer::startStepping()
{
    if (m_sorting) return;
    joinThread();

    m_liveMode = false;
    m_sorting = true;
    m_stopRequested = false;
    m_metrics  = {};

    const SortStep anfang = m_history.empty() ? SortStep{m_array, -1, -1, "Anfangszustand"} : m_history.front();
    m_history.clear();
    m_history.push_back(anfang);
    m_array = anfang.array;
    m_historyIndex = 0;
    m_finalStepForIndex.clear();

    m_sortStart  = std::chrono::steady_clock::now();

    m_sortThread = std::thread(&Visualizer::sortThreadFunc, this);
}

void Visualizer::pauseSort() {
    m_liveMode = false;
}

void Visualizer::resumeSort() {
    m_liveMode = true;
    m_lastStepTime = std::chrono::steady_clock::now();
}

void Visualizer::cancelSort() {
    joinThread();
    m_liveMode = false;
    m_sorting = false;

    if (!m_history.empty()) {
        SortStep initial = m_history.front();
        m_history.clear();
        m_history.push_back(initial);
        m_array = initial.array;
    }

    m_historyIndex = 0;
    m_highlightA = -1;
    m_highlightB = -1;
    m_actionText = "Abgebrochen - Anfangszustand";
    m_finalStepForIndex.clear();
    m_metrics = {};
    m_autoScrollNumbers = true;
}

// ============================================================
void Visualizer::run()
{
    while (m_running)
    {
        handleEvents();

        auto now = std::chrono::steady_clock::now();

        if (m_sorting && m_liveMode) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastStepTime).count();
            if (elapsed >= m_delayMs) {
                m_lastStepTime = now;

                std::lock_guard lock(m_mutex);
                if (m_historyIndex < static_cast<int32_t>(m_history.size()) - 1) {
                    m_historyIndex++;
                    const auto& step = m_history[m_historyIndex];
                    m_array = step.array;
                    m_highlightA = step.indexA;
                    m_highlightB = step.indexB;
                    m_actionText = step.action;

                    int32_t maxVal = m_array.empty() ? 1 : *std::ranges::max_element(m_array);
                    int32_t valForPitch = maxVal / 2;
                    if (step.indexA >= 0 && step.indexA < m_array.size()) valForPitch = m_array[step.indexA];
                    else if (step.indexB >= 0 && step.indexB < m_array.size()) valForPitch = m_array[step.indexB];

                    playBeep(valForPitch, maxVal, std::max(1u, m_delayMs));
                } else if (m_threadFinished) {
                    m_sorting = false;
                    m_liveMode = false;
                    m_actionText = "Sortierung erfolgreich!";
                    m_highlightA = -1;
                    m_highlightB = -1;
                }
            }
        }

        draw();
        SDL_Delay(16);
    }
}