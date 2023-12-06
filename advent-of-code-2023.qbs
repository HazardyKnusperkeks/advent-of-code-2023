import qbs

Project {
    name: "Advent of Code 2023"

    references: ["allWarnings.qbs"]

    CppApplication {
        consoleApplication: true
        files: [
            "challenge1.cpp",
            "challenge1.hpp",
            "challenge2.cpp",
            "challenge2.hpp",
            "challenge3.cpp",
            "challenge3.hpp",
            "challenge4.cpp",
            "challenge4.hpp",
            "challenge5.cpp",
            "challenge5.hpp",
            "main.cpp",
        ]

        Depends { name: "AllWarnings" }
        Depends { name: "cpp" }

        cpp.cxxLanguageVersion: "c++23"
    }

    Product {
        files: ["data/*.txt"]
        name: "Data"
    }
}
