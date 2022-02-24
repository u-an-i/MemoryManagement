#include <iostream>
#include <string>
#include <thread>
#include <atomic>

#include "MemoryManagement/mm.hpp"


class TestClass : MemObject
{
public:
    TestClass(int i)
    {
        value = i;
        threadNumber = -1;
        std::cout << "Constructing TestClass " << value << ".\n";
    }

    TestClass(int i, int j, const MemType& type) : MemObject(type)
    {
        value = i;
        threadNumber = j;
        std::cout << std::string("Constructing ").append(std::to_string(threadNumber)).append("s TestClass ").append(std::to_string(value)).append(".\n");
    }

    ~TestClass()
    {
        if (threadNumber < 0)
        {
            std::cout << "Destructing TestClass " << value << ".\n";
        }
        else
        {
            std::cout << "Destructing Thread " << threadNumber << "s TestClass " << value << ".\n";
        }
    }

private:
    int value;
    int threadNumber;
};


class DerivedDerivedString : public Derived<std::string>            // this derivation is only to add output
{
public:
    DerivedDerivedString(int i)
    {
        value = i;
        threadNumber = -1;
        std::cout << "Constructing DerivedDerivedString " << value << ":";
    }

    DerivedDerivedString(int i, int j, const MemType& type) : Derived<std::string>(type)
    {
        value = i;
        threadNumber = j;
        std::cout << std::string("Constructing ").append(std::to_string(threadNumber)).append("s DerivedDerivedString ").append(std::to_string(value)).append(".\n");
    }

    ~DerivedDerivedString()
    {
        if (threadNumber < 0)
        {
            std::cout << "Destructing DerivedDerivedString " << value << ".\n";
        }
        else
        {
            std::cout << "Destructing Thread " << threadNumber << "s DerivedDerivedString " << value << ".\n";
        }
    }

private:
    int value;
    int threadNumber;
};


class WrappedThread
{
public:
    WrappedThread()
    {
        maxThreads++;
    }

    ~WrappedThread()
    {
        if (threadPointer != nullptr)
        {
            threadPointer->detach();
            delete threadPointer;
        }
    }

    union returnValue
    {
    public:
        std::thread** ref;
        void* dummy;
    };

    union returnValue getThreadPointerReferenceForThreadCreation()
    {
        if (threadPointer == nullptr)
        {
            return returnValue{ .ref = &threadPointer };
        }
        else
        {
            return returnValue{ .dummy = nullptr };
        }
    }

    std::thread* getThreadPointer()
    {
        return threadPointer;
    }

    static void threadDone()                    // for more general use, `done` and `maxThreads` can be put in an unordered_map per id of a group where a group is meant for a batch of concurrent operations at a time
    {
        done++;
    }

    static bool threadsDone()
    {
        return done == maxThreads;
    }

    static void resetThreadCounter()
    {
        done = 0;
        maxThreads = 0;
    }

    static std::atomic<bool> passable;

private:
    std::thread* threadPointer = nullptr;
    static int done;
    static int maxThreads;
};

int WrappedThread::done = 0;
int WrappedThread::maxThreads = 0;
std::atomic<bool> WrappedThread::passable(true);


void threadCode(void (*doneCallback)(), int threadNumber, const MemType& memType)
{
    try
    {
        int i = 0;
        for (; i < 2; i++)
        {
            TestClass* pointer = new TestClass(i, threadNumber, memType);
        }
        for (; i < 4; i++)
        {
            DerivedDerivedString* pointer = new DerivedDerivedString(i-2, threadNumber, memType);
        }
    }
    catch (...)
    {
        std::cout << "\nlikely out of memory (thread" << threadNumber << ")\n";
    }
    bool gate = WrappedThread::passable.exchange(false);
    if (!gate)
    {
        WrappedThread::passable.wait(false);
    }
    doneCallback();
    WrappedThread::passable.store(true);
    WrappedThread::passable.notify_one();
}


int main()
{
    std::cout << "Hello World!\n";


    /*
    new MemObject();                        // should compiler error
    new MemType();                          // should compiler error
    new MemTypeProvider();                  // should compiler error
    new MemRegistry();                      // should compiler error
    */


    try
    {
        std::cout << "\n\nTests:\n\n";

        std::cout << "1. Own classes: Early `delete` and switching `MemType`:\n\n";

        for (int i = 0; i < 10; i++)
        {
            TestClass* pointer = new TestClass(i);
            if (i == 5)
            {
                std::cout << "pre-destruct: ";
                delete pointer;
                std::cout << "switching once: ";
                MemRegistry::switchType(MemTypeProvider::requestMemType());
            }
        }


        std::cout << "\n\n2. Other classes:\n\n";

        for (int i = 0; i < 5; i++)
        {
            DerivedDerivedString* pointer = new DerivedDerivedString(i);
            pointer->get()->append("string " + std::to_string(i));
            std::cout << " " << *pointer->get() << ".\n";
        }


        std::cout << "\n\n3. Threads:\n\n";

        for (int i = 0; i < 7; i++)
        {
            *((new Derived<WrappedThread>())->get()->getThreadPointerReferenceForThreadCreation().ref) = new std::thread(threadCode, &WrappedThread::threadDone, i, MemTypeProvider::requestMemType());            // a `Derived<std::thread>` uses the default constructor of thread which is no thread, hence the `WrappedThread`
        }

        std::thread thread([]() {                       // create a collector thread which checks the others for completion
            do
            {
                bool gate = WrappedThread::passable.exchange(false);
                if (!gate)
                {
                    WrappedThread::passable.wait(false);
                }
                if (WrappedThread::threadsDone())
                {
                    break;
                }
                WrappedThread::passable.store(true);
                WrappedThread::passable.notify_one();
            } while (true);
            });
        thread.join();                                  // instead of a collector thread, joining the individual `WrappedThread` after the loop (requiring storing them inside a container in the loop) works too: either the first thread took the longest already or it didn't and other threads still run after it completed and so on for the other threads joined after. Joining an already completed thread does nothing.


        std::cout << "\n\n4. Before Program Exit, expected order matching construction per `MemType` minus early `delete`d sequentially (but unordered across `MemType`s):\n\n";

        MemRegistry::obliviate();
    }
    catch (...)
    {
        std::cout << "\nlikely out of memory\n";
    }


    std::cout << "\n\nGood Bye World!\n\n";
}