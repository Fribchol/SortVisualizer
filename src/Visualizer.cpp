#include "Visualizer.hpp"
#include "SortAlgorithms.hpp"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <format>
#include <mutex>
#include <numbers>
#include <random>
#include <ranges>
#include <stdexcept>
#include <thread>

namespace {
    constexpr std::int32_t WIN_W     = 1400;
    constexpr std::int32_t WIN_H     = 860;
    constexpr float        UI_H      = 170.0f;
    constexpr float        VIS_X     = 10.0f;
}

constexpr std::string_view LABEL_FULLSCREEN_ON  = "Vollbild: AN";
constexpr std::string_view LABEL_FULLSCREEN_OFF = "Vollbild: AUS";

Visualizer::Visualizer()
{
    initSDL();
    initButtons();
    m_algoInfo = SortAlgorithms::getInfo(static_cast<std::uint8_t>(m_algorithm));
    fillRandom();
}

void Visualizer::initSDL()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
        throw std::runtime_error(std::format("SDL_Init: {}", SDL_GetError()));
    if (!TTF_Init())
        throw std::runtime_error(std::format("TTF_Init: {}", SDL_GetError()));

    m_window.reset(SDL_CreateWindow("Sort Visualizer", WIN_W, WIN_H, 0));
    if (!m_window) throw std::runtime_error(std::format("Window: {}", SDL_GetError()));

    m_renderer.reset(SDL_CreateRenderer(m_window.get(), nullptr));
    if (!m_renderer) throw std::runtime_error(std::format("Renderer: {}", SDL_GetError()));

    SDL_SetRenderLogicalPresentation(m_renderer.get(), WIN_W, WIN_H, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    m_fontTitle.reset(TTF_OpenFont("DejaVuSans.ttf", 36));
    m_fontLarge.reset(TTF_OpenFont("DejaVuSans.ttf", 15));
    m_fontSmall.reset(TTF_OpenFont("DejaVuSans.ttf", 12));
    m_fontTiny .reset(TTF_OpenFont("DejaVuSans.ttf", 11));
    if (!m_fontLarge || !m_fontSmall || !m_fontTiny || !m_fontTitle)
        throw std::runtime_error(std::format("Font: {}", SDL_GetError()));

    SDL_AudioSpec audioSpec = { SDL_AUDIO_F32, 1, 44100 };
    m_audioStream.reset(SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audioSpec, nullptr, nullptr));
    if (m_audioStream) SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(m_audioStream.get()));
}

void Visualizer::initButtons()
{
    constexpr float centerX = WIN_W / 2.0f - 125.0f;

    // Modernes C++20: designated initializers statt positionaler Aggregat-Init.
    // Selbstdokumentierend - man sieht am Call-Site sofort, welches Feld welchen
    // Wert bekommt, ohne die Struct-Definition danebenzulegen.
    m_btnMenuStart    = Button{.x = centerX, .y = 300.0f, .w = 250.0f, .h = 60.0f, .label = "Visualizer Starten"};
    m_btnMenuSettings = Button{.x = centerX, .y = 400.0f, .w = 250.0f, .h = 60.0f, .label = "Einstellungen"};
    m_btnMenuQuit     = Button{.x = centerX, .y = 500.0f, .w = 250.0f, .h = 60.0f, .label = "Beenden"};

    m_btnSettingsFullscreen = Button{.x = centerX, .y = 300.0f, .w = 250.0f, .h = 50.0f, .label = std::string(LABEL_FULLSCREEN_OFF)};
    m_volumeSliderBg        = SDL_FRect{.x = centerX, .y = 450.0f, .w = 250.0f, .h = 20.0f};
    m_btnSettingsBack       = Button{.x = centerX, .y = 600.0f, .w = 250.0f, .h = 50.0f, .label = "Zurück zum Menü"};

    constexpr float row1Y = WIN_H - UI_H + 10.0f;
    constexpr float row2Y = WIN_H - UI_H + 65.0f;
    constexpr float row3Y = WIN_H - UI_H + 115.0f;

    m_speedSliderBg = SDL_FRect{.x = 680.0f,  .y = row2Y, .w = 150.0f, .h = 20.0f};
    m_sizeSliderBg  = SDL_FRect{.x = 1050.0f, .y = row2Y, .w = 150.0f, .h = 20.0f};

    static constexpr std::array<std::string_view, 7> labels
    {{ "QuickSort", "MergeSort (rek)", "MergeSort (it)", "HeapSort", "RadixSort", "CountingSort", "BubbleSort" }};

    float bx = VIS_X;
    for (std::size_t i = 0; i < labels.size(); ++i) {
        m_algoButtons.push_back(Button{.x = bx, .y = row1Y, .w = 170.0f, .h = 45.0f, .label = std::string(labels[i]), .active = (i == 0)});
        bx += 175.0f;
    }

    m_startButton     = Button{.x = 10.0f,  .y = row2Y, .w = 90.0f, .h = 40.0f, .label = "Start"};
    m_stopButton      = Button{.x = 110.0f, .y = row2Y, .w = 90.0f, .h = 40.0f, .label = "Pause"};
    m_cancelButton    = Button{.x = 210.0f, .y = row2Y, .w = 90.0f, .h = 40.0f, .label = "Abbruch"};
    m_stepBackButton  = Button{.x = 310.0f, .y = row2Y, .w = 65.0f, .h = 40.0f, .label = "  < "};
    m_stepFwdButton   = Button{.x = 380.0f, .y = row2Y, .w = 65.0f, .h = 40.0f, .label = "  > "};

    m_caseRandomBtn   = Button{.x = 10.0f,  .y = row3Y, .w = 120.0f, .h = 35.0f, .label = "Zufall",      .active = true};
    m_caseSortedBtn   = Button{.x = 135.0f, .y = row3Y, .w = 120.0f, .h = 35.0f, .label = "Aufsteigend"};
    m_caseReverseBtn  = Button{.x = 260.0f, .y = row3Y, .w = 120.0f, .h = 35.0f, .label = "Absteigend"};
    m_caseEqualBtn    = Button{.x = 385.0f, .y = row3Y, .w = 120.0f, .h = 35.0f, .label = "Gleichgroß"};

    m_viewBarsButton  = Button{.x = 535.0f, .y = row3Y, .w = 100.0f, .h = 35.0f, .label = "Balken", .active = true};
    m_viewNumsButton  = Button{.x = 645.0f, .y = row3Y, .w = 100.0f, .h = 35.0f, .label = "Zahlen"};
    m_btnBackToMenu   = Button{.x = 755.0f, .y = row3Y, .w = 130.0f, .h = 35.0f, .label = "Hauptmenü"};
    m_btnBenchmark    = Button{.x = 895.0f, .y = row3Y, .w = 150.0f, .h = 35.0f, .label = "Shell Benchmark"};
}

void Visualizer::fillRandom()
{
    joinThread();
    std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<std::int32_t> dist(1, 99);
    m_array.resize(static_cast<std::size_t>(m_arraySize));
    std::ranges::generate(m_array, [&]{ return dist(rng); });

    m_history.reset(m_array.size());
    m_metrics = SortAlgorithms::LiveMetrics{};
    m_highlightA = -1; m_highlightB = -1;
    m_liveMode = false; m_sorting = false;
    m_explanationScrollY = 0; m_numbersScrollY = 0; m_autoScrollNumbers = true;
    m_finalStepForIndex.clear();
    m_history.push(m_array, -1, -1);
    m_historyIndex = 0;
    m_sortCase = SortCase::Random;
}

void Visualizer::fillSpecialCase(SortCase sc)
{
    joinThread();
    m_sortCase = sc;
    m_array.assign(static_cast<std::size_t>(m_arraySize), 0);

    switch (sc) {
        case SortCase::Sorted:
            for (std::int32_t i = 0; i < m_arraySize; ++i)
                m_array[static_cast<std::size_t>(i)] = 5 + (i * 20);
            break;
        case SortCase::Reverse:
            for (std::int32_t i = 0; i < m_arraySize; ++i)
                m_array[static_cast<std::size_t>(i)] = 5 + ((m_arraySize - 1 - i) * 20);
            break;
        case SortCase::Equal:
            std::ranges::fill(m_array, 250);
            break;
        case SortCase::Random:
            fillRandom();
            return;
    }

    m_history.reset(m_array.size());
    m_metrics = SortAlgorithms::LiveMetrics{};
    m_highlightA = -1; m_highlightB = -1;
    m_liveMode = false; m_sorting = false;
    m_autoScrollNumbers = true;
    m_finalStepForIndex.clear();
    m_history.push(m_array, -1, -1);
    m_historyIndex = 0;
}

void Visualizer::joinThread()
{
    // RAII: jthread bringt request_stop()/join() bereits von Haus aus mit,
    // wir müssen hier nur noch unseren eigenen Zustand zurücksetzen.
    if (m_sortThread.joinable()) {
        m_sortThread.request_stop();
        m_sortThread.join();
    }
    m_sorting = false;
    m_threadFinished = false;
}

void Visualizer::sortThreadFunc(std::stop_token stopToken)
{
    auto cb = [this, &stopToken](const std::vector<std::int32_t>& arr, std::int32_t a, std::int32_t b)
    {
        onSortStep(arr, a, b);
        if (stopToken.stop_requested()) throw std::runtime_error("__STOP__");
    };

    try {
        switch (m_algorithm) {
            case Algorithm::QuickSort:    SortAlgorithms::quickSort   (m_array, cb, m_metrics); break;
            case Algorithm::MergeSortRec: SortAlgorithms::mergeSort   (m_array, cb, m_metrics); break;
            case Algorithm::MergeSortIt:  SortAlgorithms::mergeSortIt (m_array, cb, m_metrics); break;
            case Algorithm::HeapSort:     SortAlgorithms::heapSort   (m_array, cb, m_metrics); break;
            case Algorithm::RadixSort:    SortAlgorithms::radixSort   (m_array, cb, m_metrics); break;
            case Algorithm::CountingSort: SortAlgorithms::countingSort(m_array, cb, m_metrics); break;
            case Algorithm::BubbleSort:   SortAlgorithms::bubbleSort  (m_array, cb, m_metrics); break;
        }

        std::scoped_lock lock(m_mutex);
        if (!m_history.empty()) {
            const auto finalArray = m_history.array(m_history.stepCount() - 1);
            m_finalStepForIndex.assign(finalArray.size(), -1);
            for (std::size_t i = 0; i < finalArray.size(); ++i) {
                std::int32_t lastWrong = -1;
                for (std::size_t s = 0; s < m_history.stepCount(); ++s) {
                    if (m_history.array(s)[i] != finalArray[i])
                        lastWrong = static_cast<std::int32_t>(s);
                }
                m_finalStepForIndex[i] = lastWrong;
            }
        }
    }
    catch (const std::runtime_error& e) { if (std::string_view{e.what()} != "__STOP__") throw; }
    catch (...) {}
    m_threadFinished = true;
}

void Visualizer::playBeep(std::int32_t value, std::int32_t maxValue, std::uint32_t durationMs) const {
    if (!m_audioStream || maxValue == 0 || durationMs == 0 || m_volume <= 0.01f) return;
    const float freq = 150.0f + (static_cast<float>(value) / static_cast<float>(maxValue)) * 1350.0f;
    constexpr std::int32_t sampleRate = 44100;
    auto numSamples = static_cast<std::int32_t>((static_cast<std::int64_t>(sampleRate) * durationMs) / 1000);
    std::vector<float> samples(static_cast<std::size_t>(numSamples));
    float phase = 0.0f;
    const float phaseIncrement = (2.0f * std::numbers::pi_v<float> * freq) / static_cast<float>(sampleRate);
    for (std::size_t i = 0; i < samples.size(); ++i) {
        float sample = std::sin(phase) * m_volume;
        if (i > samples.size() - 50 && samples.size() > 50) sample *= static_cast<float>(samples.size() - i) / 50.0f;
        samples[i] = sample;
        phase += phaseIncrement;
    }
    SDL_PutAudioStreamData(m_audioStream.get(), samples.data(), static_cast<std::int32_t>(samples.size() * sizeof(float)));
}

void Visualizer::onSortStep(const std::vector<std::int32_t>& arr, std::int32_t a, std::int32_t b)
{
    // Data-Oriented: push() kopiert direkt in den zusammenhängenden Puffer,
    // kein separates Heap-Objekt pro Schritt mehr nötig.
    std::scoped_lock lock(m_mutex);
    m_history.push(arr, a, b);
}

void Visualizer::startLive()
{
    if (m_sorting) return;
    joinThread();
    m_liveMode = m_sorting = true;
    m_metrics = SortAlgorithms::LiveMetrics{};

    std::vector<std::int32_t> firstArray = m_array;
    if (!m_history.empty()) {
        const auto span = m_history.array(0);
        firstArray.assign(span.begin(), span.end());
    }
    m_history.reset(firstArray.size());
    m_history.push(firstArray, -1, -1);
    m_array = std::move(firstArray);
    m_historyIndex = 0;
    m_finalStepForIndex.clear();
    m_sortStart = m_lastStepTime = std::chrono::steady_clock::now();

    // RAII: die Lambda-Hülle sorgt dafür, dass jthread den stop_token korrekt
    // an sortThreadFunc(std::stop_token) durchreicht, obwohl es eine
    // Member-Funktion ist (direktes &Visualizer::sortThreadFunc, this würde
    // hier NICHT zuverlässig den stop_token binden).
    m_sortThread = std::jthread([this](std::stop_token st) { sortThreadFunc(st); });
}

void Visualizer::startStepping()
{
    if (m_sorting) return;
    joinThread();
    m_liveMode = false;
    m_sorting = true;
    m_metrics = SortAlgorithms::LiveMetrics{};

    std::vector<std::int32_t> firstArray = m_array;
    if (!m_history.empty()) {
        const auto span = m_history.array(0);
        firstArray.assign(span.begin(), span.end());
    }
    m_history.reset(firstArray.size());
    m_history.push(firstArray, -1, -1);
    m_array = std::move(firstArray);
    m_historyIndex = 0;
    m_finalStepForIndex.clear();
    m_sortStart = std::chrono::steady_clock::now();

    m_sortThread = std::jthread([this](std::stop_token st) { sortThreadFunc(st); });
}

void Visualizer::pauseSort() { m_liveMode = false; }
void Visualizer::resumeSort() { m_liveMode = true; m_lastStepTime = std::chrono::steady_clock::now(); }

void Visualizer::cancelSort() {
    joinThread();
    std::scoped_lock lock(m_mutex);
    m_liveMode = m_sorting = false;
    if (!m_history.empty()) {
        const auto first = m_history.array(0);
        m_array.assign(first.begin(), first.end());
        m_history.keepFirstOnly();
    }
    m_historyIndex = m_highlightA = m_highlightB = -1;
    m_finalStepForIndex.clear();
    m_metrics = SortAlgorithms::LiveMetrics{};
    m_autoScrollNumbers = true;
}

void Visualizer::run()
{
    while (m_running) {
        handleEvents();
        const auto now = std::chrono::steady_clock::now();
        if (m_sorting && m_liveMode) {
            const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastStepTime).count();
            if (elapsed >= static_cast<std::int64_t>(m_delayMs)) {
                m_lastStepTime = now;

                std::vector<std::int32_t> stepArray;
                std::int32_t stepA = -1, stepB = -1;
                bool found = false;

                // Lock-Scope minimieren: nur die Daten kopieren, die wir brauchen
                {
                    std::scoped_lock lock(m_mutex);
                    if (m_historyIndex < static_cast<std::int32_t>(m_history.stepCount()) - 1) {
                        ++m_historyIndex;
                        const auto span = m_history.array(static_cast<std::size_t>(m_historyIndex));
                        stepArray.assign(span.begin(), span.end());
                        stepA = m_history.indexA(static_cast<std::size_t>(m_historyIndex));
                        stepB = m_history.indexB(static_cast<std::size_t>(m_historyIndex));
                        found = true;
                    }
                }

                if (found) {
                    m_array = std::move(stepArray);
                    m_highlightA = stepA;
                    m_highlightB = stepB;

                    const std::int32_t maxVal = m_array.empty() ? 1 : *std::ranges::max_element(m_array);
                    const std::int32_t v = (stepA >= 0 && stepA < static_cast<std::int32_t>(m_array.size())) ? m_array[static_cast<std::size_t>(stepA)] : maxVal / 2;
                    playBeep(v, maxVal, m_delayMs);
                } else if (m_threadFinished) {
                    m_sorting = m_liveMode = false;
                    m_highlightA = m_highlightB = -1;
                }
            }
        }
        draw();
        SDL_Delay(16);
    }
}