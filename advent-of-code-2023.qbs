import qbs

Project {
    name: "Advent of Code 2023"

    references: ["allWarnings.qbs"]

    Product {
        name: "Boost.Multiprecision"

        Export {
            Depends { name: "cpp" }
            cpp.systemIncludePaths: [
                "3rdParty/boost/libs/config/include",
                "3rdParty/boost/libs/multiprecision/include",
            ]
        }
    }

    Product {
        name: "Boost.Integer"

        Export {
            Depends { name: "cpp" }
            cpp.systemIncludePaths: [
                "3rdParty/boost/libs/assert/include",
                "3rdParty/boost/libs/core/include",
                "3rdParty/boost/libs/integer/include",
            ]
        }
    }

    CppApplication {
        consoleApplication: true
        files: [
            "3rdParty/ctre/include/**/*.hpp",
            "challenge1.cpp",
            "challenge1.hpp",
            "challenge10.cpp",
            "challenge10.hpp",
            "challenge11.cpp",
            "challenge11.hpp",
            "challenge12.cpp",
            "challenge12.hpp",
            "challenge13.cpp",
            "challenge13.hpp",
            "challenge14.cpp",
            "challenge14.hpp",
            "challenge15.cpp",
            "challenge15.hpp",
            "challenge16.cpp",
            "challenge16.hpp",
            "challenge17.cpp",
            "challenge17.hpp",
            "challenge18.cpp",
            "challenge18.hpp",
            "challenge19.cpp",
            "challenge19.hpp",
            "challenge2.cpp",
            "challenge2.hpp",
            "challenge20.cpp",
            "challenge20.hpp",
            "challenge21.cpp",
            "challenge21.hpp",
            "challenge22.cpp",
            "challenge22.hpp",
            "challenge23.cpp",
            "challenge23.hpp",
            "challenge24.cpp",
            "challenge24.hpp",
            "challenge25.cpp",
            "challenge25.hpp",
            "challenge3.cpp",
            "challenge3.hpp",
            "challenge4.cpp",
            "challenge4.hpp",
            "challenge5.cpp",
            "challenge5.hpp",
            "challenge6.cpp",
            "challenge6.hpp",
            "challenge7.cpp",
            "challenge7.hpp",
            "challenge8.cpp",
            "challenge8.hpp",
            "challenge9.cpp",
            "challenge9.hpp",
            "coordinate3d.hpp",
            "helper.cpp",
            "helper.hpp",
            "main.cpp",
            "print.cpp",
            "print.hpp",
        ]

        Depends { name: "AllWarnings" }
        Depends { name: "cpp" }
        Depends { name: "Boost.Integer" }
        Depends { name: "Boost.Multiprecision" }

        cpp.cxxLanguageVersion: "c++23"
        cpp.cxxFlags: ["-fconcepts-diagnostics-depth=10"]
    }

    Product {
        files: ["data/*.txt"]
        name: "Data"
    }
}
