#ifndef SOME_STRUCTS_H
#define SOME_STRUCTS_H

//#include <string>

namespace TestNamespace1 {

#define SOME_MACRO

struct Reflectable
{

};

struct Person : public Reflectable
{
    SOME_MACRO
    //std::string name;
    int age;
    bool alive;
};

}

namespace TestNamespace2 {

}

#endif // SOME_STRUCTS_H
