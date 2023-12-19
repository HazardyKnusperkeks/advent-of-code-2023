#include "challenge19.hpp"

#include "helper.hpp"
#include "print.hpp"
#include "3rdParty/ctre/include/ctre.hpp"

#include <algorithm>
#include <functional>
#include <iterator>
#include <ranges>
#include <unordered_map>

using namespace std::string_view_literals;

namespace {
constexpr auto Accepted = "A"sv;
constexpr auto Rejected = "R"sv;

struct MetalPiece {
    std::int64_t Extremely;
    std::int64_t Musical;
    std::int64_t Aerodynamic;
    std::int64_t Shiny;

    std::int64_t rating(void) const noexcept {
        return Extremely + Musical + Aerodynamic + Shiny;
    }
};

struct Rule {
    std::string_view Target;
    std::int64_t MetalPiece::*Attribute;
    bool                      Less;
    std::int64_t              Threshold;
};

struct Workflow {
    std::vector<Rule> Rules;
    std::string_view  Fallback;
};

struct Instance {
    std::vector<MetalPiece>                        MetalPieces;
    std::unordered_map<std::string_view, Workflow> Workflows;
};

std::pair<std::string_view, Workflow> parseWorkflow(std::string_view line) {
    std::pair<std::string_view, Workflow> ret;

    const auto bracePos = line.find('{');
    throwIfInvalid(bracePos != std::string_view::npos);

    ret.first = line.substr(0, bracePos);

    line.remove_prefix(bracePos + 1);
    throwIfInvalid(line.ends_with('}'));
    line.remove_suffix(1);

    for ( auto ruleText : splitString(line, ',') ) {
        throwIfInvalid(ret.second.Fallback.empty());
        if ( ruleText.size() == 1 ) {
            ret.second.Fallback = ruleText;
            throwIfInvalid(ruleText == Accepted || ruleText == Rejected);
            continue;
        } //if ( ruleText.size() == 1 )

        if ( ruleText.size() >= 2 && ruleText[1] != '>' && ruleText[1] != '<' ) {
            ret.second.Fallback = ruleText;
            continue;
        } //if ( ruleText.size() >= 2 && ruleText[1] != '>' && ruleText[1] != '<' )

        auto match = ctre::match<"([amsx])([<>])(\\d+):([a-zAR]+)">(ruleText);
        throwIfInvalid(match);

        auto& rule = ret.second.Rules.emplace_back();

        switch ( *match.get<1>().data() ) {
            case 'a' : rule.Attribute = &MetalPiece::Aerodynamic; break;
            case 'm' : rule.Attribute = &MetalPiece::Musical; break;
            case 's' : rule.Attribute = &MetalPiece::Shiny; break;
            case 'x' : rule.Attribute = &MetalPiece::Extremely; break;
        } //switch ( *match.get<1>().data() )

        rule.Less      = *match.get<2>().data() == '<';
        rule.Threshold = convert(match.get<3>());
        rule.Target    = match.get<4>();
    } //for ( auto ruleText : splitString(line, ',') )

    return ret;
}

MetalPiece parseMetalPiece(std::string_view line) {
    auto match = ctre::match<"\\{x=(\\d+),m=(\\d+),a=(\\d+),s=(\\d+)\\}">(line);
    throwIfInvalid(match);
    return {convert(match.get<1>()), convert(match.get<2>()), convert(match.get<3>()), convert(match.get<4>())};
}

Instance parse(const std::vector<std::string_view>& input) {
    const auto predicate = [](std::string_view line) noexcept { return !line.starts_with('{'); };
    Instance   ret;
    std::ranges::transform(input | std::views::take_while(predicate) |
                               std::views::take_while([](std::string_view line) noexcept { return !line.empty(); }),
                           std::inserter(ret.Workflows, ret.Workflows.end()), parseWorkflow);
    std::ranges::transform(input | std::views::drop_while(predicate) | std::views::drop_while(&std::string_view::empty),
                           std::back_inserter(ret.MetalPieces), parseMetalPiece);
    return ret;
}

std::int64_t calcCombinations(MetalPiece min, MetalPiece max, std::string_view workflowName,
                              const std::unordered_map<std::string_view, Workflow>& workflows) noexcept {
    if ( min.Extremely > max.Extremely ) {
        return 0;
    } //if ( min.Extremely > max.Extremely )

    if ( min.Aerodynamic > max.Aerodynamic ) {
        return 0;
    } //if ( min.Aerodynamic > max.Aerodynamic )

    if ( min.Musical > max.Musical ) {
        return 0;
    } //if ( min.Musical > max.Musical )

    if ( min.Shiny > max.Shiny ) {
        return 0;
    } //if ( min.Shiny > max.Shiny )

    if ( workflowName == Rejected ) {
        return 0;
    } //if ( workflowName == Rejected )

    if ( workflowName == Accepted ) {
        return (max.Extremely - min.Extremely + 1) * (max.Aerodynamic - min.Aerodynamic + 1) *
               (max.Musical - min.Musical + 1) * (max.Shiny - min.Shiny + 1);
    } //if ( workflowName == Accepted )

    const auto&  workflow = workflows.find(workflowName)->second;
    std::int64_t ret      = 0;

    for ( const auto& rule : workflow.Rules ) {
        auto  middle   = rule.Less ? max : min;
        auto& toModify = std::invoke(rule.Attribute, middle);
        toModify       = rule.Threshold;

        if ( rule.Less ) {
            --toModify;
            ret                              += calcCombinations(min, middle, rule.Target, workflows);
            std::invoke(rule.Attribute, min)  = rule.Threshold;
        } //if ( rule.Less )
        else {
            ++toModify;
            ret                              += calcCombinations(middle, max, rule.Target, workflows);
            std::invoke(rule.Attribute, max)  = rule.Threshold;
        } //else -> if ( rule.Less )
    } //for ( const auto& rule : workflow.Rules )

    return ret + calcCombinations(min, max, workflow.Fallback, workflows);
}
} //namespace

bool challenge19(const std::vector<std::string_view>& input) {
    const auto instance = parse(input);

    const auto accepted = [&workflows = instance.Workflows](const MetalPiece& piece) noexcept {
        auto workflowName = "in"sv;
        while ( workflowName != Accepted && workflowName != Rejected ) {
            auto workflowIter = workflows.find(workflowName);
            throwIfInvalid(workflowIter != workflows.end());
            const auto& workflow    = workflowIter->second;
            bool        ruleApplied = false;

            for ( const auto& rule : workflow.Rules ) {
                auto applyRule = [&workflowName, &ruleApplied, &rule](void) noexcept {
                    workflowName = rule.Target;
                    ruleApplied  = true;
                    return;
                };

                auto value = std::invoke(rule.Attribute, piece);
                if ( rule.Less && value < rule.Threshold ) {
                    applyRule();
                    break;
                } //if ( rule.Less && value < rule.Threshold )

                if ( !rule.Less && value > rule.Threshold ) {
                    applyRule();
                    break;
                } //else if ( !rule.Less && value > rule.Threshold )
            } //for ( const auto& rule : workflow.Rules )

            if ( !ruleApplied ) {
                workflowName = workflow.Fallback;
            } //if ( !ruleApplied )
        } //while ( workflowName != Accepted && workflowName != Rejected )
        return workflowName == Accepted;
    };

    auto sum1 = std::ranges::fold_left(instance.MetalPieces | std::views::filter(accepted) |
                                           std::views::transform(&MetalPiece::rating),
                                       0, std::plus<>{});
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    auto sum2 = calcCombinations({1, 1, 1, 1}, {4000, 4000, 4000, 4000}, "in"sv, instance.Workflows);
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 377025 && sum2 == 135'506'683'246'673;
}
