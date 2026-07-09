#include "Visualizer.hpp"
#include "SortAlgorithms.hpp"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <format>
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

const std::string LABEL_FULLSCREEN_ON  = "Vollbild: AN";
const std::string LABEL_FULLSCREEN_OFF = "Vollbild: AUS";

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
    m_btnMenuStart    = {centerX, 300.0f, 250.0f, 60.0f, "Visualizer Starten", false};
    m_btnMenuSettings = {centerX, 400.0f, 250.0f, 60.0f, "Einstellungen", false};
    m_btnMenuQuit     = {centerX, 500.0f, 250.0f, 60.0f, "Beenden", false};
    m_btnSettingsFullscreen = {centerX, 300.0f, 250.0f, 50.0f, std::string(LABEL_FULLSCREEN_OFF), false};
    m_volumeSliderBg        = {centerX, 450.0f, 250.0f, 20.0f};
    m_btnSettingsBack       = {centerX, 600.0f, 250.0f, 50.0f, "Zurück zum Menü", false};

    constexpr float row1Y = WIN_H - UI_H + 10.0f;
    constexpr float row2Y = WIN_H - UI_H + 65.0f;
    constexpr float row3Y = WIN_H - UI_H + 115.0f;

    m_speedSliderBg = { 680.0f, row2Y, 150.0f, 20.0f };
    m_sizeSliderBg  = { 1050.0f, row2Y, 150.0f, 20.0f };

    static constexpr std::array<std::string_view, 7> labels
    {{ "QuickSort", "MergeSort (rek)", "MergeSort (it)", "HeapSort", "RadixSort", "CountingSort", "BubbleSort" }};

    float bx = VIS_X;
    for (std::size_t i = 0; i < labels.size(); ++i) {
        m_algoButtons.push_back({bx, row1Y, 170.0f, 45.0f, std::string(labels[i]), (i == 0)});
        bx += 175.0f;
    }

    m_startButton = {10.0f, row2Y, 90.0f, 40.0f, "Start", false};
    m_stopButton = {110.0f, row2Y, 90.0f, 40.0f, "Pause", false};
    m_cancelButton = {210.0f, row2Y, 90.0f, 40.0f, "Abbruch", false};
    m_stepBackButton = {310.0f, row2Y, 65.0f, 40.0f, "  < ", false};
    m_stepFwdButton = {380.0f, row2Y, 65.0f, 40.0f, "  > ", false};
    m_caseRandomBtn = { 10.0f, row3Y, 120.0f, 35.0f, "Zufall", true };
    m_caseSortedBtn = {135.0f, row3Y, 120.0f, 35.0f, "Aufsteigend", false};
    m_caseReverseBtn = {260.0f, row3Y, 120.0f, 35.0f, "Absteigend", false};
    m_caseEqualBtn = {385.0f, row3Y, 120.0f, 35.0f, "Gleichgroß", false};
    m_viewBarsButton = {535.0f, row3Y, 100.0f, 35.0f, "Balken", true };
    m_viewNumsButton = {645.0f, row3Y, 100.0f, 35.0f, "Zahlen", false};
    m_btnBackToMenu = {755.0f, row3Y, 130.0f, 35.0f, "Hauptmenü", false};
    m_btnBenchmark = {895.0f, row3Y, 150.0f, 35.0f, "Shell Benchmark", false};
}

void Visualizer::fillRandom()
{
    joinThread();
    std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<std::int32_t> dist(1, 99);
    m_array.resize(static_cast<std::size_t>(m_arraySize));
    std::ranges::generate(m_array, [&]{ return dist(rng); });
    m_history.clear();
    m_metrics = SortAlgorithms::LiveMetrics{};
    m_highlightA = -1; m_highlightB = -1;
    m_stopRequested = false; m_liveMode = false; m_sorting = false;
    m_explanationScrollY = 0; m_numbersScrollY = 0; m_autoScrollNumbers = true;
    m_finalStepForIndex.clear();
    m_history.push_back({m_array, -1, -1});
    m_historyIndex = 0;
    m_sortCase = SortCase::Random;
}

void Visualizer::fillSpecialCase(SortCase sc)
{
    joinThread();
    m_sortCase = sc;
    m_array.clear();
    m_array.resize(static_cast<std::size_t>(m_arraySize));
    if (sc == SortCase::Sorted) {
        for (std::int32_t i = 0; i < m_arraySize; ++i) m_array[static_cast<std::size_t>(i)] = 5 + (i * 20);
    } else if (sc == SortCase::Reverse) {
        for (std::int32_t i = 0; i < m_arraySize; ++i) m_array[static_cast<std::size_t>(i)] = 5 + ((m_arraySize - 1 - i) * 20);
    } else if (sc == SortCase::Equal) {
        for (auto& val : m_array) val = 250;
    } else { fillRandom(); return; }
    m_history.clear();
    m_metrics = SortAlgorithms::LiveMetrics{};
    m_highlightA = -1; m_highlightB = -1;
    m_stopRequested = false; m_liveMode = false; m_sorting = false;
    m_autoScrollNumbers = true;
    m_finalStepForIndex.clear();
    m_history.push_back({m_array, -1, -1});
    m_historyIndex = 0;
}

void Visualizer::joinThread()
{
    if (m_sortThread.joinable()) {
        m_stopRequested = true;
        m_sortThread.join();
    }
    m_sorting = m_stopRequested = m_threadFinished = false;
}

void Visualizer::sortThreadFunc()
{
    auto cb = [this](const std::vector<std::int32_t>& arr, std::int32_t a, std::int32_t b)
    {
        onSortStep(arr, a, b);
        if (m_stopRequested.load()) throw std::runtime_error("__STOP__");
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
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_history.empty()) {
            const auto& finalArray = m_history.back().array;
            m_finalStepForIndex.assign(finalArray.size(), -1);
            for (std::size_t i = 0; i < finalArray.size(); ++i) {
                std::int32_t lastWrong = -1;
                for (std::size_t s = 0; s < m_history.size(); ++s) {
                    if (i < m_history[s].array.size() && m_history[s].array[i] != finalArray[i])
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
    // Kopie erstellen, solange wir noch NICHT im Lock sind
    std::vector<std::int32_t> arrCopy = arr;

    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_history.size() < 100000) {
        // Jetzt übergeben wir die Kopie sicher
        m_history.push_back({std::move(arrCopy), a, b});
    }
}

void Visualizer::startLive()
{
    if (m_sorting) return;
    joinThread();
    m_liveMode = m_sorting = true;
    m_stopRequested = false;
    m_metrics = SortAlgorithms::LiveMetrics{};
    const SortStep anfang = m_history.empty() ? SortStep{m_array, -1, -1} : m_history.front();
    m_history.clear();
    m_history.push_back(anfang);
    m_array = anfang.array;
    m_historyIndex = 0;
    m_finalStepForIndex.clear();
    m_sortStart = m_lastStepTime = std::chrono::steady_clock::now();
    m_sortThread = std::thread(&Visualizer::sortThreadFunc, this);
}

void Visualizer::startStepping()
{
    if (m_sorting) return;
    joinThread();
    m_liveMode = false;
    m_sorting = true;
    m_stopRequested = false;
    m_metrics = SortAlgorithms::LiveMetrics{};
    const SortStep anfang = m_history.empty() ? SortStep{m_array, -1, -1} : m_history.front();
    m_history.clear();
    m_history.push_back(anfang);
    m_array = anfang.array;
    m_historyIndex = 0;
    m_finalStepForIndex.clear();
    m_sortStart = std::chrono::steady_clock::now();
    m_sortThread = std::thread(&Visualizer::sortThreadFunc, this);
}

void Visualizer::pauseSort() { m_liveMode = false; }
void Visualizer::resumeSort() { m_liveMode = true; m_lastStepTime = std::chrono::steady_clock::now(); }

void Visualizer::cancelSort() {
    joinThread();
    std::lock_guard<std::mutex> lock(m_mutex);
    m_liveMode = m_sorting = false;
    if (!m_history.empty()) {
        // WICHTIG: Erst eine echte Kopie ziehen! m_history.front() ist eine Referenz
        // IN den Vector hinein. Würde man diese Referenz direkt an assign() übergeben,
        // wird sie ungültig, sobald assign() den Container-Inhalt verändert/löscht -
        // genau das meldet der MSVC Debug-Assert "assignment value cannot be a
        // reference into the container".
        const SortStep firstStep = m_history.front();
        m_array = firstStep.array;
        m_history.assign(1, firstStep);
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

                SortStep nextStep;
                bool found = false;

                // Lock-Scope minimieren: Wir kopieren nur die Daten, die wir brauchen
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    if (m_historyIndex < static_cast<std::int32_t>(m_history.size()) - 1) {
                        m_historyIndex++;
                        nextStep = m_history[static_cast<std::size_t>(m_historyIndex)];
                        found = true;
                    }
                }

                if (found) {
                         // Sicherstellen, dass der Sortierthread nicht gleichzeitig schreibt
                         // Eigentlich ist das m_mutex schon korrekt, aber kopiere VOR der Zuweisung!
                     std::vector<std::int32_t> safeCopy = nextStep.array;
                     m_array = std::move(safeCopy); // Zuweisung ist jetzt sicher

                        m_highlightA = nextStep.indexA;
                        m_highlightB = nextStep.indexB;

                    const std::int32_t maxVal = m_array.empty() ? 1 : *std::ranges::max_element(m_array);
                    const std::int32_t v = (nextStep.indexA >= 0 && nextStep.indexA < static_cast<std::int32_t>(m_array.size())) ? m_array[static_cast<std::size_t>(nextStep.indexA)] : maxVal/2;
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