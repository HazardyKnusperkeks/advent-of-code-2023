#include "challenge25.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <map>
#include <memory>
#include <ranges>
#include <unordered_map>
#include <unordered_set>

namespace {
struct Node {
    std::string_view   Name;
    std::vector<Node*> Connected;

    Node(std::string_view name) noexcept : Name{name} {
        return;
    }
};

struct Graph {
    std::vector<std::unique_ptr<Node>>          Nodes;
    std::unordered_map<std::string_view, Node*> NodeMap;
};

Graph parse(const std::vector<std::string_view>& input) {
    Graph ret;
    auto  getNode = [&ret](std::string_view name) noexcept {
        auto iter = ret.NodeMap.find(name);
        if ( iter != ret.NodeMap.end() ) {
            return iter->second;
        } //if ( iter != ret.NodeMap.end() )
        auto node = ret.Nodes.emplace_back(std::make_unique<Node>(name)).get();
        ret.NodeMap.emplace(name, node);
        return node;
    };

    for ( auto line : input ) {
        auto colon = line.find(':');
        throwIfInvalid(colon != std::string_view::npos);
        auto lhs = getNode(line.substr(0, colon));
        line.remove_prefix(colon + 2);

        for ( auto rhsName : splitString(line, ' ') ) {
            auto rhs = getNode(rhsName);
            lhs->Connected.push_back(rhs);
            rhs->Connected.push_back(lhs);
        } //for ( auto rhsName : splitString(line, ' ') )
    } //for ( auto line : input )
    return ret;
}

std::pair<std::vector<Node*>, std::vector<Node*>> calculateCut(const Graph& graph,
                                                               std::size_t  numberOfCutEdges) noexcept {
    std::pair<std::vector<Node*>, std::vector<Node*>> ret;
    auto& [group1, group2] = ret;

    group1.reserve(graph.Nodes.size());
    group1.push_back(graph.Nodes.front().get());
    group2.resize(graph.Nodes.size() - 1);
    std::ranges::transform(graph.Nodes | std::views::drop(1), group2.begin(), &std::unique_ptr<Node>::get);
    std::ranges::sort(group2);

    struct ConnectedCounter {
        Node*       NodeInGroup2;
        std::size_t Counter;

        ConnectedCounter(void) noexcept : ConnectedCounter{nullptr} {
            return;
        }

        ConnectedCounter(Node* node) noexcept : NodeInGroup2{node}, Counter{1} {
            return;
        }
    };

    std::vector<ConnectedCounter> directlyReachedFromGroup1;
    const auto&                   connectedFromFirstNode = group1.front()->Connected;
    directlyReachedFromGroup1.resize(connectedFromFirstNode.size());
    std::ranges::copy(connectedFromFirstNode, directlyReachedFromGroup1.begin());
    std::ranges::sort(directlyReachedFromGroup1, {}, &ConnectedCounter::NodeInGroup2);

    while ( directlyReachedFromGroup1.size() != numberOfCutEdges && !group2.empty() ) {
        auto maxIter    = std::ranges::max_element(directlyReachedFromGroup1, {}, &ConnectedCounter::Counter);
        auto nodeToMove = maxIter->NodeInGroup2;
        directlyReachedFromGroup1.erase(maxIter);
        group1.push_back(nodeToMove);
        group2.erase(std::ranges::lower_bound(group2, nodeToMove));

        for ( auto connectedNode : nodeToMove->Connected ) {
            if ( std::ranges::binary_search(group2, connectedNode) ) {
                auto iter = std::ranges::lower_bound(directlyReachedFromGroup1, connectedNode, {},
                                                     &ConnectedCounter::NodeInGroup2);
                if ( iter->NodeInGroup2 == connectedNode ) {
                    ++iter->Counter;
                } //if ( iter->NodeInGroup2 == connectedNode )
                else {
                    directlyReachedFromGroup1.emplace(iter, connectedNode);
                } //else -> if ( iter->NodeInGroup2 == connectedNode )
            } //if ( std::ranges::binary_search(group2, connectedNode) )
        } //for ( auto connectedNode : nodeToMove->Connected )
    } //while ( directlyReachedFromGroup1.size() != numberOfCutEdges && !group2.empty() )

    return ret;
}
} //namespace

bool challenge25(const std::vector<std::string_view>& input) {
    const auto graph      = parse(input);

    auto [group1, group2] = calculateCut(graph, 3);
    auto groupSizeProduct = group1.size() * group2.size();
    myPrint(" == Result of Part 1: {:d} ==\n", groupSizeProduct);

    return groupSizeProduct == 612945;
}
