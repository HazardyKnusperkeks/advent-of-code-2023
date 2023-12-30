#include "challenge1.hpp"
#include "challenge10.hpp"
#include "challenge11.hpp"
#include "challenge12.hpp"
#include "challenge13.hpp"
#include "challenge14.hpp"
#include "challenge15.hpp"
#include "challenge16.hpp"
#include "challenge17.hpp"
#include "challenge18.hpp"
#include "challenge19.hpp"
#include "challenge2.hpp"
#include "challenge20.hpp"
#include "challenge21.hpp"
#include "challenge22.hpp"
#include "challenge23.hpp"
#include "challenge24.hpp"
#include "challenge25.hpp"
#include "challenge3.hpp"
#include "challenge4.hpp"
#include "challenge5.hpp"
#include "challenge6.hpp"
#include "challenge7.hpp"
#include "challenge8.hpp"
#include "challenge9.hpp"
#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <iterator>
#include <span>
#include <string_view>
#include <vector>

using namespace std::string_view_literals;

/**
 * @brief Hauptfunktion.
 * @author Björn Schäpers
 * @since 01.12.2023
 * @param[in] argc Die Anzahl der Arguments.
 * @param[in] argv Die Werte der Argumente.
 * @result 0 bei Erfolg.
 */
int main(int argc, const char* argv[]) {
    if ( argc < 3 ) {
        myErr("Not enough parameters!");
        return -1;
    } //if ( argc < 3 )

    const std::filesystem::path dataDirectory{argv[1]};

    if ( !std::filesystem::exists(dataDirectory) ) {
        myErr("Path {:s} does not exist!", dataDirectory.native());
        return -2;
    } //if ( !std::filesystem::exists(dataDirectory) )

    using Clock            = std::chrono::system_clock;

    const std::span inputs = [&argc, &argv](void) noexcept {
        std::span ret{argv + 2, argv + argc};
        if ( ret.size() == 1 && ret[0] == "0"sv ) {
            static std::array<const char*, 25> all{"1",  "2",  "3",  "4",  "5",  "6",  "7",  "8",  "9",
                                                   "10", "11", "12", "13", "14", "15", "16", "17", "18",
                                                   "19", "20", "21", "22", "23", "24", "25"};
            ret = std::span{all.data(), all.size()};
        } //if ( ret.size() == 1 && ret[0] == "0"sv )
        return ret;
    }();
    std::vector<std::string_view> challengeInput;
    const auto                    overallStart        = Clock::now();
    int                           challengesRun       = 0;
    int                           challengesSuccesful = 0;
    auto runAndAdd = [&challengeInput, &challengesSuccesful](bool (*func)(const std::vector<std::string_view>&)) {
        if ( func(challengeInput) ) {
            ++challengesSuccesful;
        } //if ( func(challengeInput) )
        else {
            myPrint("Failed\n");
        } //else -> if ( func(challengeInput ) )
        return;
    };

    for ( const auto& input : inputs ) {
        const auto challenge = [](std::string_view text) noexcept -> std::int64_t {
            try {
                return convert(text);
            } //try
            catch ( ... ) {
                return 0;
            }
        }({input, std::strlen(input)});

        if ( challenge == 0 ) {
            myErr("{:s} is not a valid challenge identifier!\n", input);
            continue;
        } //if ( challenge == 0 )

        const auto inputFilePath = dataDirectory / std::format("{:d}.txt", challenge);

        try {
            ++challengesRun;
            if ( !std::filesystem::exists(inputFilePath) ) {
                throw std::runtime_error{std::format("\"{:s}\" does not exist!", inputFilePath.c_str())};
            } //if ( !std::filesystem::exists(inputFilePath) )

            if ( !std::filesystem::is_regular_file(inputFilePath) ) {
                throw std::runtime_error{std::format("\"{:s}\" is not a file!", inputFilePath.c_str())};
            } //if ( !std::filesystem::is_regular_file(inputFilePath) )

            std::ifstream inputFile{inputFilePath};

            if ( !inputFile ) {
                throw std::runtime_error{std::format("Could not open \"{:s}\"!", inputFilePath.c_str())};
            } //if ( !inputFile )

            challengeInput.clear();
            inputFile.seekg(0, std::ios::end);
            const auto size = inputFile.tellg();
            inputFile.seekg(0, std::ios::beg);
            std::string fileContent(static_cast<std::size_t>(size), ' ');
            inputFile.read(fileContent.data(), size);
            std::ranges::copy(splitString<false>(fileContent, '\n'), std::back_inserter(challengeInput));
            auto lastNonEmpty = std::ranges::find_last_if_not(challengeInput, &std::string_view::empty);
            if ( lastNonEmpty.begin() != challengeInput.end() ) {
                challengeInput.erase(std::next(lastNonEmpty.begin()), lastNonEmpty.end());
            } //if ( lastNonEmpty.begin() != challengeInput.end() )

            myPrint(" == Starting Challenge {:d} ==\n", challenge);
            const auto start = Clock::now();

            switch ( challenge ) {
                case 1  : runAndAdd(challenge1); break;
                case 2  : runAndAdd(challenge2); break;
                case 3  : runAndAdd(challenge3); break;
                case 4  : runAndAdd(challenge4); break;
                case 5  : runAndAdd(challenge5); break;
                case 6  : runAndAdd(challenge6); break;
                case 7  : runAndAdd(challenge7); break;
                case 8  : runAndAdd(challenge8); break;
                case 9  : runAndAdd(challenge9); break;
                case 10 : runAndAdd(challenge10); break;
                case 11 : runAndAdd(challenge11); break;
                case 12 : runAndAdd(challenge12); break;
                case 13 : runAndAdd(challenge13); break;
                case 14 : runAndAdd(challenge14); break;
                case 15 : runAndAdd(challenge15); break;
                case 16 : runAndAdd(challenge16); break;
                case 17 : runAndAdd(challenge17); break;
                case 18 : runAndAdd(challenge18); break;
                case 19 : runAndAdd(challenge19); break;
                case 20 : runAndAdd(challenge20); break;
                case 21 : runAndAdd(challenge21); break;
                case 22 : runAndAdd(challenge22); break;
                case 23 : runAndAdd(challenge23); break;
                case 24 : runAndAdd(challenge24); break;
                case 25 : runAndAdd(challenge25); break;

                default : {
                    --challengesRun;
                    myErr("Challenge {:d} is not known!\n", challenge);
                    break;
                } //default
            } //switch ( challenge )

            const auto end      = Clock::now();
            const auto duration = end - start;
            myPrint(" == End of Challenge {:d} after {} ==\n\n", challenge,
                    std::chrono::duration_cast<std::chrono::milliseconds>(duration));
        } //try
        catch ( const std::exception& e ) {
            --challengesRun;
            myErr("Skipping Challenge {:d}: {:s}\n", challenge, e.what());
        } //catch ( const std::exception& e)
    } //for ( const auto& input : inputs )

    const auto overallEnd      = Clock::now();
    const auto overallDuration = overallEnd - overallStart;
    myPrint("After {} {:d} challenges correctly solved from {:d} ({:.2f}%)\n",
            std::chrono::duration_cast<std::chrono::milliseconds>(overallDuration), challengesSuccesful, challengesRun,
            challengesSuccesful * 100. / std::max(challengesRun, 1));

    return 0;
}
