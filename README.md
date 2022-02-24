C++ "automatic" memory management
---

enables you to be able to be worry-free about memory leaks

---

Usage:

1 header include only.

**Derive your classes from MemObject.**

`MemRegistry::obliviate();    // before program exit`

Use other classes as `Derived<Other>* pointer = new Derived<Other>(); Other* pointerToOther = pointer->get();`; see Test Application.

---

Example:

```C++
#include "MemoryManagement/mm.hpp"

// derive your classes from MemObject

int main()
{
	// your code

	MemRegistry::obliviate();		// before program exit
}
```

---

You can still delete your objects before program exit when you like if you like, nothing extra to consider.

---

You can request a `MemType` by `MemTypeProvider::requestMemType()` and `MemRegistry::switchType(MemType& toSwitchTo, bool oneTimeOnly = true)` until possible `MemRegistry::revokeType()` (when `oneTimeOnly` = false) to bin your new objects into a group which you can `MemRegistry::forget(MemType&)` on a whim when you don't need this batch anymore or leave that up to `MemRegistry::obliviate()`. `MemRegistry::forget(memTypeObject)` does `MemTypeProvider::returnMemType(memTypeObject)` which puts `memTypeObject` back in the pool of possible provided `MemType`, do not reuse a returned `MemType` object; you may `MemTypeProvider::reuseReturnedMemTypes = false;` to prevent returned `memTypeObject` be put back in the pool which eliminates some computation and space used, see Note further below about amount of available `MemType`.

---

Overhead:  
- spatial:  
[sizeof(1 pointer), sizeof(size_t), sizeof(unsigned long long), compiler-dependant base class management bytes] per `new` during runtime + some fixed bytes + some bytes per many `new` (due to STL containers dynamic memory) until program exit  
- computational:  
few operations to register and few when you delete, some more when `forget`ting and `obliviate`ing

---

Note:
Requested `MemType`s might deplenish, check if `yourMemType.isInvalid()`, after 2^64 - 1 requests.

---

Thread-Safety:
To use `MemObject`-derived classes from threads, explicitly pass a `MemType` unique to a thread to the constructor of `MemObject` for derived classes instantiated on that thread; see Test Application.

---

Test Application tested with Visual Studio 2022 with C++ 20 on Windows 11.