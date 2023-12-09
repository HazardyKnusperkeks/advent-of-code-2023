#include "helper.hpp"

#include <stdexcept>

void throwIfInvalid(bool valid, const char* msg) {
    if ( !valid ) {
        throw std::runtime_error{msg};
    } //if ( !valid )
    return;
}
