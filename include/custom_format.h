#include <iostream>
#include <iomanip>
#include <sstream>

struct CustomPunctuation : std::numpunct<char> {
    char do_thousands_sep() const override { return ','; }

    std::string do_grouping() const override { return "\03"; }
};

/**
 * @brief Formats and prints a given double value using a custom pattern.
 *
 * This function formats the given value according to the pattern and prints it to the console.
 *
 * @param pattern The desired pattern for the output.
 * @param value The double value to be formatted and printed.
 *
 * @note This function uses the iostream, sstream, iomanip, and string headers.
 */
void customFormat(const std::string &pattern, double value) {
    std::stringstream stream;
    stream.imbue(std::locale(stream.getloc(), new CustomPunctuation));

    int precision = static_cast<int>(pattern.substr(pattern.find('.')).size() - 1);
    stream << std::fixed << std::setprecision(precision) << value;
    std::cout << value << " " << pattern << " ";
    if (pattern.find('$') != std::string::npos)
        std::cout << "$";
    std::cout << stream.str() << std::endl;
}