#include "devtools/TimeCommands.h"
#include "devtools/DebugConsole.h"
#include "gameplay/TimeManager.h"

#include <algorithm>
#include <sstream>

namespace fresh
{
namespace devtools
{

void TimeCommands::registerCommands(DebugConsole* console, TimeManager* timeManager)
{
    if (!console || !timeManager) {
        return;
    }

    // time - Show current time
    console->registerCommand(
        "time",
        [console, timeManager](const std::vector<std::string>& args) {
            if (args.empty()) {
                // Show current time
                std::ostringstream oss;
                oss << "Current time: " << timeManager->getFormattedTime()
                    << " (Day " << timeManager->getCurrentDay() << ")";
                if (timeManager->isPaused()) {
                    oss << " [PAUSED]";
                }
                oss << " Rate: " << timeManager->getTimeRate() << "x";
                console->print(oss.str());

                // Show additional info
                if (timeManager->isDaytime()) {
                    console->print("  Status: Daytime");
                } else {
                    console->print("  Status: Nighttime");
                }
                if (timeManager->isSunrise()) {
                    console->print("  (Sunrise in progress)");
                }
                if (timeManager->isSunset()) {
                    console->print("  (Sunset in progress)");
                }
            } else {
                // Subcommand
                std::string subCmd = args[0];
                std::transform(subCmd.begin(), subCmd.end(), subCmd.begin(), ::tolower);

                if (subCmd == "set" && args.size() >= 2) {
                    std::string timeValue = args[1];
                    std::transform(timeValue.begin(), timeValue.end(), timeValue.begin(), ::tolower);

                    if (timeValue == "dawn") {
                        timeManager->setTimeOfDay(TimeManager::TimeOfDay::Dawn);
                        console->print("Time set to dawn (6:00 AM)");
                    } else if (timeValue == "noon") {
                        timeManager->setTimeOfDay(TimeManager::TimeOfDay::Noon);
                        console->print("Time set to noon (12:00 PM)");
                    } else if (timeValue == "dusk") {
                        timeManager->setTimeOfDay(TimeManager::TimeOfDay::Dusk);
                        console->print("Time set to dusk (6:00 PM)");
                    } else if (timeValue == "midnight") {
                        timeManager->setTimeOfDay(TimeManager::TimeOfDay::Midnight);
                        console->print("Time set to midnight (12:00 AM)");
                    } else {
                        // Try to parse as ticks
                        try {
                            int ticks = std::stoi(timeValue);
                            if (ticks < 0 || ticks >= 24000) {
                                console->printError("Ticks must be between 0 and 23999");
                                return;
                            }
                            timeManager->setTime(ticks);
                            std::ostringstream oss;
                            oss << "Time set to " << ticks << " ticks ("
                                << timeManager->getFormattedTime() << ")";
                            console->print(oss.str());
                        } catch (const std::exception&) {
                            console->printError("Invalid time value. Use dawn/noon/dusk/midnight or 0-23999");
                        }
                    }
                } else if (subCmd == "rate" && args.size() >= 2) {
                    try {
                        float rate = std::stof(args[1]);
                        if (rate < 0.0f) {
                            console->printError("Rate must be >= 0");
                            return;
                        }
                        if (rate > 100.0f) {
                            console->printWarning("Very high rate may cause issues");
                        }
                        timeManager->setTimeRate(rate);
                        std::ostringstream oss;
                        oss << "Time rate set to " << rate << "x";
                        console->print(oss.str());
                    } catch (const std::exception&) {
                        console->printError("Invalid rate. Use a number (e.g., 1.0, 2.5)");
                    }
                } else if (subCmd == "pause") {
                    if (timeManager->isPaused()) {
                        console->print("Time is already paused");
                    } else {
                        timeManager->pause();
                        console->print("Time paused");
                    }
                } else if (subCmd == "resume") {
                    if (!timeManager->isPaused()) {
                        console->print("Time is not paused");
                    } else {
                        timeManager->resume();
                        console->print("Time resumed");
                    }
                } else if (subCmd == "toggle") {
                    timeManager->togglePause();
                    console->print(timeManager->isPaused() ? "Time paused" : "Time resumed");
                } else {
                    console->printError("Unknown subcommand. Use: set, rate, pause, resume, toggle");
                }
            }
        },
        "Time controls. Usage: time [set <dawn/noon/dusk/midnight/ticks>|rate <speed>|pause|resume|toggle]");
}

void TimeCommands::unregisterCommands(DebugConsole* console)
{
    if (!console) {
        return;
    }

    console->unregisterCommand("time");
}

} // namespace devtools
} // namespace fresh
