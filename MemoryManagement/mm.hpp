#ifndef MM_HPP
#define MM_HPP

#include <unordered_map>
#include <vector>
#include <limits>

class MemObject
{
    friend class MemRegistry;

protected:
    MemObject();
    virtual ~MemObject();

private:
    size_t index;
    unsigned long long type;
};

class MemType
{
    friend class MemTypeProvider;

    MemType(unsigned long long newType)
    {
        type = newType;
    }

public:
    ~MemType()
    {}

    size_t operator()(const MemType& toHash) const
    {
        return toHash.type;
    }

    bool operator==(const MemType& another) const
    {
        return type == another.type;
    }

    bool isInvalid()
    {
        return type == ULLONG_MAX;
    }

private:
    unsigned long long type;
};

class MemTypeProvider
{
    friend class MemRegistry;

    MemTypeProvider()
    {}

    ~MemTypeProvider()
    {}

public:
    static MemType requestMemType()
    {
        if (typePeak != ULLONG_MAX)
        {
            typePeak++;
        }
        return MemType(typePeak);
    }

    static void returnMemType(const MemType& memType)
    {
        // at 2^64 - 2 available MemType the question is whether the overhead of trying to reuse memType ever has a roi
        if (memType.type == typePeak)
        {
            typePeak--;
        }
        else {
            try
            {
                returnedMemTypes.push_back(memType);
            }
            catch (...)
            {
                // loss of reusability of memType taken, the memory will not leak however!
            }
        }
        if (returnedMemTypes.size() > 0)
        {
            bool pass;
            do
            {
                pass = false;
                for (auto it = returnedMemTypes.begin(); it != returnedMemTypes.end(); ++it)
                {
                    if (it->type == typePeak)
                    {
                        returnedMemTypes.erase(it);
                        pass = true;
                        break;
                    }
                }
            } while (pass);
        }
    }

    const static MemType defaultType;

private:
    static unsigned long long typePeak;
    static std::vector<MemType> returnedMemTypes;

    static unsigned long long exportMemType(MemType& memType)
    {
        return memType.type;
    }

    static MemType importMemType(unsigned long long exportedMemType)
    {
        return MemType(exportedMemType);
    }
};

std::vector<MemType> MemTypeProvider::returnedMemTypes;
unsigned long long MemTypeProvider::typePeak = 0;
const MemType MemTypeProvider::defaultType = MemType(0);

typedef std::unordered_map<MemType, std::vector<MemObject*>, MemType> memMap;

class MemRegistry
{
    friend MemObject;

    MemRegistry()
    {}

    ~MemRegistry()
    {}

public:
    static void forget(const MemType& type)
    {
        for (auto it = registry[type].begin(); it != registry[type].end(); ++it)
        {
            if (*it != nullptr)
            {
                delete* it;
            }
        }
        registry.erase(type);
        MemTypeProvider::returnMemType(type);
    }

    static void switchType(const MemType& type, bool oneTimeOnly = true)
    {
        typeToRegisterFor = type;
        autoDefaultType = oneTimeOnly;
        typeChanged = true;
    }

    static void revokeType()
    {
        typeToRegisterFor = MemTypeProvider::defaultType;
        typeChanged = false;
    }

    static void obliviate()
    {
        for (auto it = registry.begin(); it != registry.end(); ++it)
        {
            forget(it->first);
        }
    }

private:
    static void registerMemObject(MemObject* memObject)
    {
        registry[typeToRegisterFor].push_back(memObject);
        memObject->index = registry[typeToRegisterFor].size();
        memObject->type = MemTypeProvider::exportMemType(typeToRegisterFor);
        if (typeChanged && autoDefaultType)
        {
            typeToRegisterFor = MemTypeProvider::defaultType;
        }
    }

    static void nullify(MemObject* memObject)
    {
        registry[MemTypeProvider::importMemType(memObject->type)][memObject->index - 1] = nullptr;
    }

    static memMap registry;
    static MemType typeToRegisterFor;
    static bool autoDefaultType;
    static bool typeChanged;
};

bool MemRegistry::typeChanged = false;
bool MemRegistry::autoDefaultType = true;
MemType MemRegistry::typeToRegisterFor = MemTypeProvider::defaultType;
memMap MemRegistry::registry = memMap(8, MemTypeProvider::defaultType);

MemObject::MemObject()
{
    MemRegistry::registerMemObject(this);
}

MemObject::~MemObject()
{
    MemRegistry::nullify(this);
}

#endif