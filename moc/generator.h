#ifndef REFLECTIVE_RAPIDJSON_GENERATOR_H
#define REFLECTIVE_RAPIDJSON_GENERATOR_H

#include <iosfwd>
#include <vector>

namespace ReflectiveRapidJSON {

bool generateReflectionCode(const std::vector<const char *> &sourceFiles, std::ostream &os);
}

#endif // REFLECTIVE_RAPIDJSON_GENERATOR_H
