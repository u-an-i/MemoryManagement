C++ "automatic" memory management

enables you to be able to be worry-free about memory leaks

1 header include only.

Usage:

```C++
#include "MemoryManagement/mm.hpp"

// derive your classes from MemObject

int main()
{
	// your code

	MemRegistry::obliviate();		// before program exit
}
```

You can still delete your objects before program exit when you like if you like, nothing extra to consider.

You can request a `MemType` by `MemTypeProvider::requestMemType()` and `MemRegistry::switchType(MemType& toSwitchTo, bool oneTimeOnly = true)` until possible `MemRegistry::revokeType()` to bin your new objects into a group which you can `MemRegistry::forget(MemType&)` on a whim when you don't need this batch anymore or leave that up to `MemRegistry::obliviate()`.

Overhead:
spatial:
[sizeof(1 pointer), size_t, sizeof(unsigned long long), compiler-dependant base class management bytes] per `new` during runtime + some fixed bytes + some bytes per many new (due to STL containers dynamic memory) until program exit
computational:
few operations to register and few when you delete, some more when `forget`ting and `obliviate`ing

Note:
Requested `MemType`s might deplenish, check if `yourMemType.isInvalid()`, after 2^64 - 1 requests.


Example application tested with Visual Studio 2022 with C++ 20 on Windows 11.