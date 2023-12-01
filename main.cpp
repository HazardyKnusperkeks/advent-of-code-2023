#include "challenge1.hpp"

#include <charconv>
#include <cstring>
#include <exception>
#include <filesystem>
#include <iostream>
#include <span>

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
        std::cerr << "Not enough parameters!";
        return -1;
    } //if ( argc < 3 )

    const std::filesystem::path dataDirectory{argv[1]};

    if ( !std::filesystem::exists(dataDirectory) ) {
        std::cerr << "Path " << dataDirectory << " does not exist!";
        return -2;
    } //if ( !std::filesystem::exists(dataDirectory) )

    const std::span inputs{argv + 2, argv + argc};

    for ( const auto& input : inputs ) {
        int        challenge;
        const auto result = std::from_chars(input, input + std::strlen(input), challenge);

        if ( result.ec != std::errc{} ) {
            std::cerr << input << " is not a valid challenge identifier!\n";
            continue;
        } //if ( result.ec != std::errc{} )

        try {
            switch ( challenge ) {
                case 1  : challenge1(dataDirectory); break;

                default : {
                    std::cerr << "Challenge " << challenge << " is not known!\n";
                    break;
                } //default
            } //switch ( challenge )
        } //try
        catch ( const std::exception& e ) {
            std::cerr << "Skipping Challenge " << challenge << ": " << e.what() << '\n';
        } //catch ( const std::exception& e)
    } //for ( const auto& input : inputs )

    return 0;
}
