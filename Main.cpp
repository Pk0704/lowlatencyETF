#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

#include "RingBuffer.hpp"
#include "DataFetcher.hpp"
#include "WeatherAnalysis.hpp"
#include "DerivativePricer.hpp"

int main() {
    // Create a ring buffer to store incoming weather data
    RingBuffer<WeatherData> weatherBuffer(1024);

    // Create data fetcher (replace the URL with an actual endpoint)
    DataFetcher fetcher(weatherBuffer, "http://example.com/weatherapi");
    fetcher.start();

    WeatherAnalysis analyzer;
    DerivativePricer pricer;

    // Temporary storage for analysis
    std::vector<WeatherData> batch;
    batch.reserve(1024);

    // Main loop: periodically read from buffer, analyze, and price derivatives
    for (int i = 0; i < 100; ++i) {
        // Grab all available data
        batch.clear();
        while (true) {
            auto dataOpt = weatherBuffer.pop();
            if (!dataOpt.has_value()) {
                break;
            }
            batch.push_back(dataOpt.value());
        }

        if (!batch.empty()) {
            double avgTemp = analyzer.computeMovingAverageTemp(batch);
            // Just take the last data point for humidity/windSpeed as an example
            double humidity = batch.back().humidity;
            double windSpeed = batch.back().windSpeed;

            double rainRiskIndex = analyzer.computeRainRiskIndex(humidity, windSpeed);

            double rainfallOptionPrice = pricer.priceRainfallOption(rainRiskIndex);
            double tempSwapPrice       = pricer.priceTemperatureSwap(avgTemp);

            // Print results
            std::cout << "Latest Weather Data Count: " << batch.size()
                      << " | AvgTemp=" << avgTemp
                      << " | RainRisk=" << rainRiskIndex
                      << " | RainOptionPrice=" << rainfallOptionPrice
                      << " | TempSwapPrice=" << tempSwapPrice
                      << std::endl;
        }

        // Sleep briefly before next iteration
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Stop data fetcher and exit
    fetcher.stop();
    return 0;
}
