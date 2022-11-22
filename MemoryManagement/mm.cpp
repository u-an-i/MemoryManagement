#include "mm.hpp"


MemType::MemType(unsigned long long newType)
{
    type = newType;
}

MemType::~MemType()
{}

bool MemType::operator==(const MemType& another) const
{
    return type == another.type;
}

bool MemType::operator!=(const MemType& another) const
{
    return type != another.type;
}

bool MemType::isInvalid()
{
    return type == ULLONG_MAX;
}


MemTypeProvider::MemTypeProvider()
{}

MemTypeProvider::~MemTypeProvider()
{}

std::vector<unsigned long long> MemTypeProvider::returnedTypes;
unsigned long long MemTypeProvider::typePeak = 0;
bool MemTypeProvider::reuseReturnedMemTypes = true;
const MemType MemTypeProvider::defaultType = MemType(0);


MemRegistry::MemRegistry()
{}

MemRegistry::~MemRegistry()
{}

bool MemRegistry::early = true;
bool MemRegistry::overridden = false;
bool MemRegistry::typeChanged = false;
bool MemRegistry::autoDefaultType = true;
const unsigned long long MemRegistry::exportedDefaultType = MemTypeProvider::exportMemType(MemTypeProvider::defaultType);
unsigned long long MemRegistry::exportedTypeToRegisterFor = MemRegistry::exportedDefaultType;
memMap MemRegistry::registry = memMap(8);


MemObject::MemObject()
{
    MemRegistry::registerMemObject(this);
}

MemObject::MemObject(const MemType& type)
{
    MemRegistry::storeMemObjectPointer(this, type.type);
}

MemObject::~MemObject()
{
    MemRegistry::nullify(this);
}
