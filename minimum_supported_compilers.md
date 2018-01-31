**Goal:** Bump up the absolute minimum supported compilers versions of CppMicroServices for Release 4.0

**Rationale:** 
 * The current absolute minimum supported compilers make it hard to write idiomatic modern C++ code
 * Keep up with the rapid evolution of the C++ standard (every 3 years now - C++11, 14, 17, 20)
 
**Decision:** Upgrade compilers to support core **C++14** standard (and in Visual Studio, most of the C++14 features)

**Proposal:**

Compiler | Release date | C++14 support Status | URL 
---------|--------------|----------------------|------
VS 2015 RTM |  20 Jul 2015 | Mostly complete |  https://blogs.msdn.microsoft.com/vcblog/2015/06/19/c111417-features-in-vs-2015-rtm/
GCC 5.1 | 22 Apr 2015 | Complete | http://en.cppreference.com/w/cpp/compiler_support
Xcode 7.0 (Clang 3.7) | 20 Sep 2015 | Complete | https://en.wikipedia.org/wiki/Xcode#Latest_versions
Clang 3.4 | 02 Jan 2014	| Complete | http://releases.llvm.org/

**Current absolute minimum supported compilers:**
 * GCC 4.6
 * Clang 3.1
 * Clang from Xcode 6.4
 * Visual Studio 2013

