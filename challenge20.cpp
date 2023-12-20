#include "challenge20.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <iterator>
#include <numeric>
#include <ranges>
#include <unordered_map>
#include <variant>

using namespace std::string_view_literals;

namespace {
using ModuleName  = std::string_view;
using Connections = std::vector<ModuleName>;

enum class Pulse : bool { Low, High };

struct FlipFlop {
    std::string_view Name;
    bool             IsOn = false;
};

struct Conjunction {
    std::string_view                             Name;
    std::unordered_map<ModuleName, Pulse>        PulseCache       = {};
    std::unordered_map<ModuleName, std::int64_t> PulseLoop        = {};
    int                                          HighPulses       = 0;
    int                                          HighPulsesNeeded = 0;
};

using Module = std::variant<FlipFlop, Conjunction>;

struct ModuleAndConnection {
    ::Module      Module;
    ::Connections Connections;
};

struct ModuleConfiguration {
    Connections                                         Broadcaster;
    std::unordered_map<ModuleName, ModuleAndConnection> Modules;
    ModuleName                                          RxPredecessor;
};

Connections parseConnections(std::string_view line) {
    constexpr auto prefix = "->"sv;
    throwIfInvalid(line.starts_with(prefix));
    line.remove_prefix(prefix.size());

    Connections ret;
    std::ranges::copy(splitString(line, ',') | std::views::transform([](std::string_view name) {
                          throwIfInvalid(name.starts_with(' '));
                          name.remove_prefix(1);
                          return name;
                      }),
                      std::back_inserter(ret));
    return ret;
}

constexpr auto BroadCasterString = "broadcaster"sv;
constexpr auto RxString          = "rx"sv;

auto parse(const std::vector<std::string_view>& input) {
    ModuleConfiguration ret;

    for ( auto line : input ) {
        throwIfInvalid(!line.empty());

        const auto nameEnd = line.find(' ');
        throwIfInvalid(nameEnd != std::string_view::npos);
        auto name              = line.substr(0, nameEnd);
        auto connectionsString = line.substr(nameEnd + 1);

        switch ( line.front() ) {
            case '%' : {
                name.remove_prefix(1);
                ret.Modules.emplace(name, ModuleAndConnection{FlipFlop{name}, parseConnections(connectionsString)});
                break;
            } //case '%'

            case '&' : {
                name.remove_prefix(1);
                ret.Modules.emplace(name,
                                    ModuleAndConnection{Conjunction{name}, parseConnections(connectionsString)});
                break;
            } //case '&'

            default : {
                throwIfInvalid(ret.Broadcaster.empty());
                throwIfInvalid(name == BroadCasterString);
                ret.Broadcaster = parseConnections(line.substr(nameEnd + 1));
                break;
            } //default
        } //switch ( line.front() )

        if ( connectionsString.contains(RxString) ) {
            throwIfInvalid(ret.RxPredecessor.empty());
            ret.RxPredecessor = name;
        } //if ( connectionsString.contains(RxString) )
    } //for ( auto line : input )

    auto addConjunctionPredeccesor = [&ret](ModuleName from, const Connections& connections) noexcept {
        for ( auto to : connections ) {
            auto iter = ret.Modules.find(to);
            if ( iter == ret.Modules.end() ) {
                continue;
            } //if ( iter == ret.Modules.end() )

            if ( std::holds_alternative<Conjunction>(iter->second.Module) ) {
                auto& conjunction = std::get<Conjunction>(iter->second.Module);
                conjunction.PulseCache.emplace(from, Pulse::Low);
                conjunction.PulseLoop.emplace(from, 0);
                ++conjunction.HighPulsesNeeded;
            } //if ( std::holds_alternative<Conjunction>(iter->second.Module) )
        } //for ( auto to : connections )

        return;
    };

    addConjunctionPredeccesor(BroadCasterString, ret.Broadcaster);

    for ( const auto& [moduleName, moduleAndConnection] : ret.Modules ) {
        addConjunctionPredeccesor(moduleName, moduleAndConnection.Connections);
    } //for ( const auto& [moduleName, moduleAndConnection] : ret.Modules )
    return ret;
}

struct ActivePulse {
    ModuleName Sender;
    ModuleName Receiver;
    ::Pulse    Pulse;

    ActivePulse(const std::tuple<const ModuleName&, const ModuleName&, const ::Pulse&>& stuff) noexcept :
            Sender{std::get<0>(stuff)}, Receiver{std::get<1>(stuff)}, Pulse{std::get<2>(stuff)} {
        return;
    }
};

struct Run {
    ModuleConfiguration      Configuration;
    std::vector<ActivePulse> CurrentPulses = {};
    std::vector<ActivePulse> NextPulses    = {};
    std::int64_t             LowPulses     = 0;
    std::int64_t             HighPulses    = 0;
    std::int64_t             ButtonPresses = 1;
    Conjunction*             RxPredeccesor = nullptr;

    Run(ModuleConfiguration configuration) noexcept : Configuration{std::move(configuration)} {
        if ( !Configuration.RxPredecessor.empty() ) {
            auto iter = Configuration.Modules.find(Configuration.RxPredecessor);
            if ( iter != Configuration.Modules.end() && std::holds_alternative<Conjunction>(iter->second.Module) ) {
                RxPredeccesor = std::addressof(std::get<Conjunction>(iter->second.Module));
            } //if ( iter != Configuration.Modules.end() )
        } //if ( !Configuration.RxPredecessor.empty() )
        return;
    }

    void applyPulse(FlipFlop& flip, ModuleName /*from*/, const Connections& receivers, Pulse pulse) noexcept {
        if ( pulse == Pulse::High ) {
            return;
        } //if ( pulse == Pulse::High )
        addPulses(flip.Name, receivers, flip.IsOn ? Pulse::Low : Pulse::High);
        flip.IsOn = !flip.IsOn;
        return;
    }

    void applyPulse(Conjunction& conjunction, ModuleName from, const Connections& receivers, Pulse pulse) noexcept {
        auto& cache = conjunction.PulseCache.find(from)->second;
        if ( cache != pulse ) {
            if ( pulse == Pulse::High ) {
                ++conjunction.HighPulses;

                auto& loopCache = conjunction.PulseLoop.find(from)->second;
                if ( loopCache == 0 ) {
                    loopCache = ButtonPresses;
                } //if ( loopCache == 0 )
            } //if ( pulse == Pulse::High )
            else {
                --conjunction.HighPulses;
            } //else -> if ( pulse == Pulse::High )
            cache = pulse;
        } //if ( cache != pulse )

        addPulses(conjunction.Name, receivers,
                  conjunction.HighPulses == conjunction.HighPulsesNeeded ? Pulse::Low : Pulse::High);
        return;
    }

    void addPulses(ModuleName sender, const Connections& receivers, Pulse pulse) noexcept {
        std::ranges::copy(std::views::zip(std::views::repeat(sender), receivers, std::views::repeat(pulse)),
                          std::back_inserter(NextPulses));
        return;
    }

    void press(int presses) noexcept {
        for ( ; presses > 0; --presses, ++ButtonPresses ) {
            //The Button:
            ++LowPulses;
            addPulses(BroadCasterString, Configuration.Broadcaster, Pulse::Low);

            do { //while ( !nextPulses.empty() )
                std::swap(CurrentPulses, NextPulses);
                for ( auto [sender, receiver, pulse] : CurrentPulses ) {
                    ++(pulse == Pulse::Low ? LowPulses : HighPulses);

                    const auto moduleIter = Configuration.Modules.find(receiver);
                    if ( moduleIter == Configuration.Modules.end() ) {
                        //Senke.
                        continue;
                    } //if ( moduleIter == Configuration.Modules.end() )

                    auto& [module, connections] = moduleIter->second;
                    std::visit(
                        [this, &connections, pulse, sender](auto& realModule) noexcept {
                            applyPulse(realModule, sender, connections, pulse);
                            return;
                        },
                        module);
                } //for ( auto [sender, receiver, pulse] : CurrentPulses )
                CurrentPulses.clear();
            } while ( !NextPulses.empty() );
        } //for ( ; presses > 0; --presses, ++ButtonPresses )
        return;
    }
};
} //namespace

bool challenge20(const std::vector<std::string_view>& input) {
    const auto moduleConfiguration = parse(input);

    Run run{moduleConfiguration};
    run.press(1000);
    auto result1 = run.LowPulses * run.HighPulses;
    myPrint(" == Result of Part 1: {:d} ==\n", result1);

    run = Run{moduleConfiguration};
    throwIfInvalid(run.RxPredeccesor);
    while ( std::ranges::any_of(run.RxPredeccesor->PulseLoop, [](auto pair) noexcept { return pair.second == 0; }) ) {
        run.press(1);
    } //while ( std::ranges::any_of(run.RxPredeccesor->PulseLoop, [](auto pair) noexcept { return pair.second == 0; }) )
    auto result = std::ranges::fold_left(run.RxPredeccesor->PulseLoop |
                                             std::views::transform(&std::pair<const ModuleName, std::int64_t>::second),
                                         1, std::lcm<std::int64_t, std::int64_t>);
    myPrint(" == Result of Part 2: {:d} ==\n", result);

    return result1 == 879'834'312 && result == 243037165713371;
}
