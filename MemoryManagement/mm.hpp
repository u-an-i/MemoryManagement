#ifndef MM_HPP
#define MM_HPP


#include <unordered_map>
#include <vector>
#include <climits>


class MemType;


class MemObject
{
    friend class MemRegistry;

protected:
    MemObject();
    MemObject(const MemType& type);
    virtual ~MemObject();

private:
    size_t index;
    unsigned long long type;
};


class MemType
{
    friend MemObject;
    friend class MemTypeProvider;

    MemType(unsigned long long newType);

public:
    ~MemType();

    bool operator==(const MemType& another) const;
    bool operator!=(const MemType& another) const;

    bool isInvalid();

private:
    unsigned long long type;
};


class MemTypeProvider
{
    friend class MemRegistry;

    MemTypeProvider();
    ~MemTypeProvider();

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
        MemTypeProvider::returnMemType(MemTypeProvider::exportMemType(memType));
    }

    const static MemType defaultType;
    static bool reuseReturnedMemTypes;

private:
    static unsigned long long typePeak;
    static std::vector<unsigned long long> returnedTypes;

    static void returnMemType(const unsigned long long exportedType)
    {
        if (reuseReturnedMemTypes)
        {
            // at 2^64 - 2 available MemType the question is whether the overhead of trying to reuse memType ever has a roi
            if (exportedType == typePeak)
            {
                typePeak--;
            }
            else {
                try
                {
                    returnedTypes.push_back(exportedType);
                }
                catch (...)
                {
                    // loss of reusability of memType taken, the memory will not leak however!
                }
            }
            if (returnedTypes.size() > 0)
            {
                bool pass;
                do
                {
                    pass = false;
                    for (auto it = returnedTypes.begin(); it != returnedTypes.end(); ++it)
                    {
                        if (*it == typePeak)
                        {
                            returnedTypes.erase(it);
                            pass = true;
                            break;
                        }
                    }
                }
                while (pass);
            }
        }
    }

    static unsigned long long exportMemType(const MemType& memType)
    {
        return memType.type;
    }
};


typedef std::unordered_map<unsigned long long, std::vector<MemObject*>> memMap;

class MemRegistry
{
    friend MemObject;

    MemRegistry();
    ~MemRegistry();

public:
    static void forget(const MemType& type)
    {
        MemRegistry::forget(MemTypeProvider::exportMemType(type));
    }

    static void switchType(const MemType& type, bool oneTimeOnly = true)
    {
        exportedTypeToRegisterFor = MemTypeProvider::exportMemType(type);
        autoDefaultType = oneTimeOnly;
        typeChanged = true;
    }

    static void revokeType()
    {
        exportedTypeToRegisterFor = MemTypeProvider::exportMemType(MemTypeProvider::defaultType);
        typeChanged = false;
    }

    static void obliviate()
    {
        overridden = true;
        MemTypeProvider::reuseReturnedMemTypes = false;
        for (auto it = registry.begin(); it != registry.end();)
        {
            auto next = std::next(it, 1);
            forget(it->first);
            it = next;
        }
    }

private:
    static void forget(const unsigned long long exportedType)
    {
        if (exportedType != exportedDefaultType || overridden)
        {
            early = false;
            for (auto it = registry[exportedType].begin(); it != registry[exportedType].end(); ++it)
            {
                if (*it != nullptr)
                {
                    delete* it;
                }
            }
            registry.erase(exportedType);
            MemTypeProvider::returnMemType(exportedType);
            early = true;
        }
    }

    static void registerMemObject(MemObject* memObject)
    {
        storeMemObjectPointer(memObject, exportedTypeToRegisterFor);
        if (typeChanged && autoDefaultType)
        {
            exportedTypeToRegisterFor = exportedDefaultType;
        }
    }

    static void storeMemObjectPointer(MemObject* memObject, unsigned long long type)
    {
        memObject->type = type;
        memObject->index = registry[type].size();
        registry[type].push_back(memObject);
    }

    static void nullify(MemObject* memObject)
    {
        if (early)
        {
            registry[memObject->type][memObject->index] = nullptr;
        }
    }

    static memMap registry;
    static unsigned long long exportedTypeToRegisterFor;
    const static unsigned long long exportedDefaultType;
    static bool autoDefaultType;
    static bool typeChanged;
    static bool overridden;
    static bool early;
};


template<class T>
class Derived : T, MemObject
{
public:
    Derived()
    {}

    Derived(const MemType& type) : MemObject(type)
    {}

    ~Derived()
    {}

    T* get()
    {
        return (T*)this;
    }
};


#endif
