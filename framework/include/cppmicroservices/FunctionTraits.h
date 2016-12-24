/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/

#ifndef CPPMICROSERVICES_FUNCTIONTRAITS_H
#define CPPMICROSERVICES_FUNCTIONTRAITS_H

#include <type_traits>

namespace cppmicroservices
{
  // build R (*)(Args...) from R (Args...)
  // compile error if signature is not a valid function signature
  template <typename, typename>
  struct build_free_function;

  template <typename F, typename R, typename ... Args>
  struct build_free_function < F, R(Args...) >
  {
    using type = R(*)(Args...);
  };

  // build R (C::*)(Args...) from R (Args...)
  // compile error if signature is not a valid member function signature
  template <typename, typename>
  struct build_class_function;

  template <typename C, typename R, typename ... Args>
  struct build_class_function < C, R(Args...) >
  {
    using type = R(C::*)(Args...);
  };

  // determine whether a class C has an operator() with signature S
  template <typename C, typename S>
  struct is_functor_with_signature
  {
    typedef char(&yes)[1];
    typedef char(&no)[2];

    // helper struct to determine that C::operator() does indeed have
    // the desired signature; &C::operator() is only of type 
    // R (C::*)(Args...) if this is true
    template <typename T, T> struct check;

    // T is needed to enable SFINAE
    template <typename T> static yes deduce(check <
      typename build_class_function<C, S>::type, &T::operator() > *);
    // fallback if check helper could not be built
    template <typename> static no deduce(...);

    //not VC 2013 compatible static bool constexpr value = sizeof(deduce<C>(0)) == sizeof(yes);
    enum { value = sizeof(deduce<C>(0)) == sizeof(yes) };
  };

  // determine whether a free function pointer F has signature S
  template <typename F, typename S>
  struct is_function_with_signature
  {
  // check whether F and the function pointer of S are of the same
  // type
    enum {
      value = std::is_same <
      F, typename build_free_function<F, S>::type
      > ::value
    };
  };

  // C is a class, delegate to is_functor_with_signature
  template <typename C, typename S, bool>
  struct is_functor_or_free_function_impl
  : std::integral_constant <
  bool, is_functor_with_signature<C, S>::value
  >
  {};

  // F is not a class, delegate to is_function_with_signature
  template <typename F, typename S>
  struct is_functor_or_free_function_impl<F, S, false>
  : std::integral_constant <
  bool, is_function_with_signature<F, S>::value
  >
  {};

  // Determine whether type Callable is callable with signature Signature.
  // Compliant with functors, i.e. classes that declare operator(); and free
  // function pointers: R (*)(Args...), but not R (Args...)!
  template <typename Callable, typename Signature>
  struct is_functor_or_free_function
  : is_functor_or_free_function_impl <
  Callable, Signature,
  std::is_class<Callable>::value
  >
  {};
}

#endif // CPPMICROSERVICES_FUNCTION_TRAITS_H
