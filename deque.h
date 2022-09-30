#include <vector>
#include <exception>
#include <iostream>
#include <stdio.h>

using std::cerr;
using std::vector;


template<typename T>
class Deque {
private:
  static const size_t kInternal_capacity_ = 16;
  static const size_t kMin_capacity_ = 8;
  size_t size_;
  vector<T*> pointers_;
  size_t external_start_;
  size_t external_end_;
  size_t internal_start_;
  size_t internal_end_;

  size_t find_external_index(size_t index) const {
    return external_start_ + index / kInternal_capacity_ +
           (internal_start_ + index % kInternal_capacity_) / kInternal_capacity_;
  }

  size_t find_internal_index(size_t index) const {
    return (internal_start_ + index % kInternal_capacity_) % kInternal_capacity_;
  }

  void expand() {
    size_t delta = external_end_ - external_start_ + 1;
    std::vector<T*> new_external(delta * 3, nullptr);
    for (size_t i = 0; i < delta; ++i) {
      new_external[delta + i] = pointers_[i + external_start_];
    }
    pointers_ = new_external;
    external_start_ = delta;
    external_end_ = 2 * delta - 1;
  }

public:
  template<bool constant>
  struct Iterator {
  private:
    std::conditional_t<constant, T* const*, T**> external_ptr_ = nullptr;
    size_t internal_index_ = 0;

  public:
    using pointer = typename std::conditional_t<constant, const T*, T*>;
    using reference = typename std::conditional_t<constant, const T&, T&>;
    using value_type = typename std::conditional_t<constant, const T, T>;

    Iterator() = default;

    Iterator(T** ptr, size_t index) : external_ptr_(ptr), internal_index_(index) {};

    Iterator(const Iterator<false>& other) : external_ptr_(other.external_ptr_),
                                             internal_index_(other.internal_index_) {}

    Iterator(T* const* ptr, size_t index) : external_ptr_(ptr), internal_index_(index) {};

    Iterator& operator=(const Iterator<false>& other) {
      external_ptr_ = other.external_ptr_;
      internal_index_ = other.internal_index_;
    }

    Iterator& operator++() {
      if (internal_index_ < kInternal_capacity_ - 1) {
        ++internal_index_;
        return *this;
      }
      ++external_ptr_;
      internal_index_ = 0;
      return *this;
    }

    Iterator operator++(int) {
      Iterator iter = *this;
      ++iter;
      return iter;
    }

    Iterator& operator--() {
      if (internal_index_ > 0) {
        --internal_index_;
        return *this;
      }
      --external_ptr_;
      internal_index_ = kInternal_capacity_ - 1;
      return *this;
    }

    Iterator operator--(int) {
      Iterator iter = *this;
      --iter;
      return iter;
    }

    Iterator operator+(int number) const {
      Iterator iter = *this;
      if (number > 0) {
        for (int i = 0; i < number; ++i) {
          ++iter;
        }
      } else {
        for (int i = 0; i < -number; ++i) {
          --iter;
        }
      }

      return iter;
    }

    Iterator operator-(int number) const {
      Iterator iter = *this;
      return iter + (-number);
    }

    bool operator==(const Iterator<constant>& other) const {
      return external_ptr_ == other.external_ptr_ && internal_index_ == other.internal_index_;
    }

    bool operator!=(const Iterator<constant>& other) const {
      return !(*this == other);
    }

    bool operator<(const Iterator<constant>& other) const {
      if (external_ptr_ == other.external_ptr_) {
        return internal_index_ < other.internal_index_;
      }
      return external_ptr_ < other.external_ptr_;
    }

    bool operator<=(const Iterator<constant>& other) const {
      return (*this < other) || (*this == other);
    }

    bool operator>=(const Iterator<constant>& other) const {
      return other <= *this;
    }

    bool operator>(const Iterator<constant>& other) const {
      return other < *this;
    }

    size_t operator-(const Iterator& other) const {
      Iterator iter = *this;
      size_t counter = 0;
      while (iter > other) {
        ++counter;
        --iter;
      }
      while (iter < other) {
        --counter;
        ++iter;
      }
      return counter;
    }

    pointer operator->() const {
      return *external_ptr_ + internal_index_;
    }

    reference operator*() const {
      return *(*external_ptr_ + internal_index_);
    }

    ~Iterator() = default;
  };


  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;


  Deque() : size_(0), external_start_(kMin_capacity_ / 2), external_end_(kMin_capacity_ / 2), internal_start_(0),
            internal_end_(0) {
    for (size_t i = 0; i <= kMin_capacity_; ++i) {
      pointers_.push_back(nullptr);
    }
  }


  Deque(const Deque<T>& other) : Deque() {
    for (auto iter = other.begin(); iter < other.end(); ++iter) {
      push_back(*iter);
    }
  }

  Deque(size_t size) : Deque() {
    for (size_t i = 0; i < size; ++i) {
      push_back(*reinterpret_cast<T*>(new uint8_t(sizeof(T))));
    }
  }

  Deque(size_t size, const T& value) : Deque() {
    for (size_t i = 0; i < size; ++i) {
      push_back(value);
    }
  }

  Deque& operator=(const Deque<T>& other) {
    size_t old_size = size_;
    size_t old_external_end = external_end_;
    for (auto iter = other.begin(); iter < other.end(); ++iter) {
      try {
        push_back(*iter);
      } catch (...) {
        while (size_ != old_size) {
          pop_back();
        }
        return *this;
      }
    }

    if (!pointers_.empty()) {
      for (auto it = begin(); it != begin() + old_size; ++it) {
        it->~T();
      }
      for (size_t i = external_start_; i < old_external_end; ++i) {
        delete[] reinterpret_cast<uint8_t*>(pointers_[i]);
      }
    }

    return *this;
  }

  [[nodiscard]] size_t size() const {
    return size_;
  }

  T& operator[](size_t index) {
    return *(pointers_[find_external_index(index)] + find_internal_index(index));
  }

  const T& operator[](size_t index) const {
    return *(pointers_[find_external_index(index)] + find_internal_index(index));
  }

  T& at(size_t index) {
    if (index < 0 || index >= size_) {
      throw std::out_of_range("index " + std::to_string(index)
                              + " is out of range, the length is " + std::to_string(size_));
    }
    return this->operator[](index);
  }

  const T& at(size_t index) const {
    if (index < 0 || index >= size_) {
      throw std::out_of_range("index " + std::to_string(index)
                              + " is out of range, the length is " + std::to_string(size_));
    }
    return this->operator[](index);
  }

  void push_back(const T& value) {
    if (size_ == 0) {
      pointers_[external_end_] = reinterpret_cast<T*>(new uint8_t[kInternal_capacity_ * sizeof(T)]);
      new(pointers_[external_end_] + internal_end_) T(value);
      ++size_;
      return;
    }
    if (internal_end_ < kInternal_capacity_ - 1) {
      new(pointers_[external_end_] + ++internal_end_) T(value);
    } else {
      if (external_end_ == pointers_.size() - 1) {
        expand();
      }
      pointers_[++external_end_] = reinterpret_cast<T*>(new uint8_t[kInternal_capacity_ * sizeof(T)]);
      internal_end_ = 0;
      try {
        new(pointers_[external_end_] + internal_end_) T(value);
      } catch (...) {
        delete[] reinterpret_cast<uint8_t*>(pointers_[external_end_--]);
        internal_end_ = kInternal_capacity_ - 1;
        throw;
      }
    }
    ++size_;
  }

  void pop_back() {
    (pointers_[external_end_] + internal_end_)->~T();
    if (internal_end_ > 0) {
      --internal_end_;
    } else {
      --external_end_;
      internal_end_ = kInternal_capacity_ - 1;
    }
    --size_;
  }

  void push_front(const T& value) {
    if (size_ == 0) {
      pointers_[external_start_] = reinterpret_cast<T*>(new uint8_t[kInternal_capacity_ * sizeof(T)]);
      new(pointers_[external_start_] + internal_start_) T(value);
      ++size_;
      return;
    }
    if (internal_start_ > 0) {
      new(pointers_[external_start_] + --internal_start_) T(value);
    } else {
      if (external_start_ == 0) {
        expand();
      }
      pointers_[--external_start_] = reinterpret_cast<T*>(new uint8_t[kInternal_capacity_ * sizeof(T)]);
      internal_start_ = kInternal_capacity_ - 1;
      try {
        new(pointers_[external_start_] + internal_start_) T(value);
      } catch (...) {
        delete[] reinterpret_cast<uint8_t*>(pointers_[external_start_++]);
        internal_start_ = 0;
        throw;
      }
    }
    ++size_;
  }

  void pop_front() {
    (pointers_[external_start_] + internal_start_)->~T();
    if (internal_start_ < kInternal_capacity_ - 1) {
      ++internal_start_;
    } else {
      internal_start_ = 0;
      ++external_start_;
    }
    --size_;
  }

  iterator begin() {
    return iterator(&pointers_[external_start_], internal_start_);
  }

  iterator end() {
    return iterator(&pointers_[external_end_], internal_end_ + 1);
  }

  const_iterator begin() const {
    return const_iterator(&pointers_[external_start_], internal_start_);
  }

  const_iterator end() const {
    return const_iterator(&pointers_[external_end_], internal_end_ + 1);
  }

  const_iterator cbegin() const {
    return const_iterator(&pointers_[external_start_], internal_start_);
  }

  const_iterator cend() const {
    return const_iterator(&pointers_[external_end_], internal_end_ + 1);
  }

  reverse_iterator rbegin() {
    return reverse_iterator(&pointers_[external_end_], internal_end_);
  }

  reverse_iterator rend() {
    return reverse_iterator(&pointers_[external_start_], internal_start_ - 1);
  }

  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(&pointers_[external_end_], internal_end_);
  }

  const_reverse_iterator rend() const {
    return const_reverse_iterator(&pointers_[external_start_], internal_start_ - 1);
  }

  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(&pointers_[external_end_], internal_end_);
  }

  const_reverse_iterator crend() const {
    return const_reverse_iterator(&pointers_[external_start_], internal_start_ - 1);
  }

  void insert(iterator iter, const T& value) {
    try {
      push_back(value);
    } catch (...) {
      return;
    }
    iterator current = end() - 1;
    try {
      while (current > iter) {
        std::swap(*current, *(current - 1));
        --current;
      }
    } catch (...) {
      ++current;
      iterator it(&pointers_[external_end_], internal_end_);
      while (current != it + 1) {
        std::swap(*current, *(current + 1));
        ++current;
      }
      pop_back();
      throw;
    }

  }

  void erase(iterator iter) {
    iterator current = iter;
    try {
      while (current < end() - 1) {
        std::swap(*current, *(current + 1));
        ++current;
      }
      pop_back();
    } catch (...) {
      --current;
      while (current != iter) {
        std::swap(*current, *(current - 1));
        --current;
      }
      throw;
    }
    pop_back();
  }

  ~Deque() {
    if (!pointers_.empty()) {
      for (auto it = begin(); it != end(); ++it) {
        it->~T();
      }
      for (size_t i = external_start_; i < external_end_ + 1; ++i) {
        delete[] reinterpret_cast<uint8_t*>(pointers_[i]);
      }
    }
  }
};