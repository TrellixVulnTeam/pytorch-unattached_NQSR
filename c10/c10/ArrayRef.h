//===--- ArrayRef.h - Array Reference Wrapper -------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// ATen: modified from llvm::ArrayRef.
// removed llvm-specific functionality
// removed some implicit const -> non-const conversions that rely on
// complicated std::enable_if meta-programming
// removed a bunch of slice variants for simplicity...

#pragma once

#include "Assert.h"

#include <array>
#include <iterator>
#include <vector>

namespace c10 {

/// ArrayRef - Represent a constant reference to an array (0 or more elements
/// consecutively in memory), i.e. a start pointer and a length.  It allows
/// various APIs to take consecutive elements easily and conveniently.
///
/// This class does not own the underlying data, it is expected to be used in
/// situations where the data resides in some other buffer, whose lifetime
/// extends past that of the ArrayRef. For this reason, it is not in general
/// safe to store an ArrayRef.
///
/// This is intended to be trivially copyable, so it should be passed by
/// value.
template<typename T>
class ArrayRef {
public:
  using iterator = const T *;
  using const_iterator = const T *;
  using size_type = size_t;

  using reverse_iterator = std::reverse_iterator<iterator>;

private:
  /// The start of the array, in an external buffer.
  const T *Data;

  /// The number of elements.
  size_type Length;

public:
  /// @name Constructors
  /// @{

  /// Construct an empty ArrayRef.
  /*implicit*/ constexpr ArrayRef() : Data(nullptr), Length(0) {}

  /// Construct an ArrayRef from a single element.
  /*implicit*/ constexpr ArrayRef(const T &OneElt)
    : Data(&OneElt), Length(1) {}

  /// Construct an ArrayRef from a pointer and length.
  constexpr ArrayRef(const T *data, size_t length)
    : Data(data), Length(length) {}

  /// Construct an ArrayRef from a range.
  constexpr ArrayRef(const T *begin, const T *end)
    : Data(begin), Length(end - begin) {}

  /// Construct an ArrayRef from a std::vector.
  template<typename A>
  /*implicit*/ ArrayRef(const std::vector<T, A> &Vec)
    : Data(Vec.data()), Length(Vec.size()) {}

  /// Construct an ArrayRef from a std::array
  template <size_t N>
  /*implicit*/ constexpr ArrayRef(const std::array<T, N> &Arr)
      : Data(Arr.data()), Length(N) {}

  /// Construct an ArrayRef from a C array.
  template <size_t N>
  /*implicit*/ constexpr ArrayRef(const T (&Arr)[N]) : Data(Arr), Length(N) {}

  /// Construct an ArrayRef from a std::initializer_list.
  /*implicit*/ constexpr ArrayRef(const std::initializer_list<T> &Vec)
  : Data(Vec.begin() == Vec.end() ? (T*)nullptr : Vec.begin()),
    Length(Vec.size()) {}

  /// @}
  /// @name Simple Operations
  /// @{

  constexpr iterator begin() const { return Data; }
  constexpr iterator end() const { return Data + Length; }

  constexpr reverse_iterator rbegin() const { return reverse_iterator(end()); }
  constexpr reverse_iterator rend() const { return reverse_iterator(begin()); }

  /// empty - Check if the array is empty.
  constexpr bool empty() const { return Length == 0; }

  constexpr const T *data() const { return Data; }

  /// size - Get the array size.
  constexpr size_t size() const { return Length; }

  /// front - Get the first element.
  constexpr const T &front() const {
    C10_ASSERT(!empty(), "Empty list!");
    return Data[0];
  }

  /// back - Get the last element.
  constexpr const T &back() const {
    C10_ASSERT(!empty(), "Empty list!");
    return Data[Length-1];
  }

  /// equals - Check for element-wise equality.
  /// NB: constexpr in C++14
  /*constexpr*/ bool equals(ArrayRef RHS) const {
    if (Length != RHS.Length)
      return false;
    return std::equal(begin(), end(), RHS.begin());
  }

  /// slice(n, m) - Chop off the first N elements of the array, and keep M
  /// elements in the array.
  constexpr ArrayRef<T> slice(size_t N, size_t M) const {
    C10_ASSERT(N+M <= size(), "Invalid specifier");
    return ArrayRef<T>(data()+N, M);
  }

  /// slice(n) - Chop off the first N elements of the array.
  constexpr ArrayRef<T> slice(size_t N) const { return slice(N, size() - N); }

  /// @}
  /// @name Operator Overloads
  /// @{
  constexpr const T &operator[](size_t Index) const {
    return Data[Index];
  }

  /// Vector compatibility
  constexpr const T &at(size_t Index) const {
    C10_ASSERT(Index < Length, "Invalid index!");
    return Data[Index];
  }

  /// Disallow accidental assignment from a temporary.
  ///
  /// The declaration here is extra complicated so that "arrayRef = {}"
  /// continues to select the move assignment operator.
  template <typename U>
  typename std::enable_if<std::is_same<U, T>::value, ArrayRef<T>>::type &
  operator=(U &&Temporary) = delete;

  /// Disallow accidental assignment from a temporary.
  ///
  /// The declaration here is extra complicated so that "arrayRef = {}"
  /// continues to select the move assignment operator.
  template <typename U>
  typename std::enable_if<std::is_same<U, T>::value, ArrayRef<T>>::type &
  operator=(std::initializer_list<U>) = delete;

  /// @}
  /// @name Expensive Operations
  /// @{
  std::vector<T> vec() const {
    return std::vector<T>(Data, Data+Length);
  }

  /// @}
  /// @name Conversion operators
  /// @{
  operator std::vector<T>() const {
    return std::vector<T>(Data, Data+Length);
  }

  /// @}
};

} // end namespace c10
