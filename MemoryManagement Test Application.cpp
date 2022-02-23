#include <iostream>
#include <string>
#include "MemoryManagement/mm.hpp"

class TestClass : MemObject
{
public:
    TestClass(int i)
    {
        value = i;
        std::cout << "Constructing TestClass " << value << ".\n";
    }

    ~TestClass()
    {
        std::cout << "Destructing TestClass " << value << ".\n";
    }

private:
    int value;
};

class DerivedDerivedString : public Derived<std::string>            // this derivation is only to add output
{
public:
    DerivedDerivedString()
    {
        std::cout << "Constructing DerivedDerivedString";
    }

    ~DerivedDerivedString()
    {
        std::cout << "Destructing DerivedDerivedString.\n";
    }
};

int main()
{
    std::cout << "Hello World!\n";

    /*
    new MemObject();                        // should compiler error
    new MemType();                          // should compiler error
    new MemTypeProvider();                  // should compiler error
    new MemRegistry();                      // should compiler error
    */

    for (int i = 0; i < 10; i++)
    {
        TestClass* pointer = new TestClass(i);
        if (i == 5)
        {
            std::cout << "pre-destruct: ";
            delete pointer;
            MemRegistry::switchType(MemTypeProvider::requestMemType());
        }
    }

    for (int i = 0; i < 5; i++)
    {
        DerivedDerivedString* pointer = new DerivedDerivedString();
        pointer->get()->append("string " + std::to_string(i));
        std::cout << " " << *pointer->get() << ".\n";
    }

    MemRegistry::obliviate();

    std::cout << "Good Bye World!\n";
}