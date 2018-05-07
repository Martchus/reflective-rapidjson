#ifndef REFLECTIVE_RAPIDJSON_TESTS_HELPER_H
#define REFLECTIVE_RAPIDJSON_TESTS_HELPER_H

#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/misc/traits.h>
#include <c++utilities/tests/testutils.h>

// ensure "operator<<" from TestUtilities is visible prior to the call site
using TestUtilities::operator<<;

#include <cppunit/extensions/HelperMacros.h>

/*!
 * \brief Asserts equality of two iteratables printing the differing indices.
 */
template <typename Iteratable, Traits::EnableIf<Traits::IsIteratable<Iteratable>, Traits::Not<Traits::IsString<Iteratable>>>* = nullptr>
inline void assertEqualityLinewise(const Iteratable &iteratable1, const Iteratable &iteratable2)
{
    std::vector<std::string> differentLines;
    std::size_t currentLine = 0;

    for (auto i1 = iteratable1.cbegin(), i2 = iteratable2.cbegin(); i1 != iteratable1.cend() || i2 != iteratable2.cend(); ++currentLine) {
        if (i1 != iteratable1.cend() && i2 != iteratable2.cend()) {
            if (*i1 != *i2) {
                differentLines.push_back(ConversionUtilities::numberToString(currentLine));
            }
            ++i1, ++i2;
        } else if (i1 != iteratable1.cend()) {
            differentLines.push_back(ConversionUtilities::numberToString(currentLine));
            ++i1;
        } else if (i2 != iteratable1.cend()) {
            differentLines.push_back(ConversionUtilities::numberToString(currentLine));
            ++i2;
        }
    }
    if (!differentLines.empty()) {
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "the following lines differ: " + ConversionUtilities::joinStrings(differentLines, ", "), iteratable1, iteratable2);
    }
}

#endif // REFLECTIVE_RAPIDJSON_TESTS_HELPER_H
