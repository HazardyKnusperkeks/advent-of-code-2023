#include "challenge1.hpp"
#include "challenge2.hpp"
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
#include <charconv>
#include <cstring>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <iterator>
#include <span>
#include <string_view>
#include <vector>

/**
 * @brief Hauptfunktion.
 * @author Björn Schäpers
 * @since 01.12.2023
 * @param[in] argc Die Anzahl der Arguments.
 * @param[in] argv Die Werte der Argumente.
 * @result 0 bei Erfolg.
 */
int main(int argc, char* argv[]) {
    if ( argc < 3 ) {
        myErr("Not enough parameters!");
        return -1;
    } //if ( argc < 3 )

    const std::filesystem::path dataDirectory{argv[1]};

    if ( !std::filesystem::exists(dataDirectory) ) {
        myErr("Path {:s} does not exist!", dataDirectory.native());
        return -2;
    } //if ( !std::filesystem::exists(dataDirectory) )

    const std::span               inputs{argv + 2, argv + argc};
    std::vector<std::string_view> challengeInput;

    for ( const auto& input : inputs ) {
        int        challenge;
        const auto result = std::from_chars(input, input + std::strlen(input), challenge);

        if ( result.ec != std::errc{} ) {
            myErr("{:s} is not a valid challenge identifier!\n", input);
            continue;
        } //if ( result.ec != std::errc{} )

        const auto inputFilePath = dataDirectory / std::format("{:d}.txt", challenge);

        try {
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
            std::ranges::copy(splitString(fileContent, '\n'), std::back_inserter(challengeInput));

            switch ( challenge ) {
                // case 1  : challenge1(challengeInput); break;
                // case 2  : challenge2(challengeInput); break;
                // case 3  : challenge3(challengeInput); break;
                // case 4  : challenge4(challengeInput); break;
                // case 5  : challenge5(challengeInput); break;
                // case 6  : challenge6(challengeInput); break;
                // case 7  : challenge7(challengeInput); break;
                // case 8  : challenge8(challengeInput); break;
                case 9  : challenge9(challengeInput); break;

                default : {
                    myErr("Challenge {:d} is not known!\n", challenge);
                    break;
                } //default
            } //switch ( challenge )
        } //try
        catch ( const std::exception& e ) {
            myErr("Skipping Challenge {:d}: {:s}\n", challenge, e.what());
        } //catch ( const std::exception& e)
    } //for ( const auto& input : inputs )

    return 0;
}
