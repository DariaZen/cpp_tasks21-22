#include <cstring>
#include <iostream>


class String {
private:
  size_t size;
  size_t capacity;
  char* buffer;
public:
  String() : size(0), capacity(1), buffer(nullptr) {};

  String(const size_t n, char c) : size(n), capacity(2 * n), buffer(new char[n]) {
    memset(buffer, c, n);
  }

  String(const char* cstring) : size(strlen(cstring)), capacity(2 * size), buffer(new char[size * 2]) {
    memcpy(buffer, cstring, size);
  }

  String(const char c) : size(1), capacity(1), buffer(new char{c}) {};

  String(const String& other) : size(other.size), capacity(other.size * 2), buffer(new char[other.size * 2]) {
    memcpy(buffer, other.buffer, size);
  }

  bool operator==(const String& other) const {
    if (size != other.size) {
      return false;
    }
    for (size_t i = 0; i < size; ++i) {
      if (buffer[i] != other.buffer[i]) {
        return false;
      }
    }
    return true;
  }

  char operator[](size_t index) const {
    return buffer[index];
  }

  char& operator[](size_t index) {
    return buffer[index];
  }

  size_t length() const {
    return size;
  }

  void pop_back() {
    --size;
  }

  char& front() {
    return buffer[0];
  }

  char front() const {
    return buffer[0];
  }

  void push_back(const char next_element) {
    ++size;
    if (size > capacity) {
      capacity *= 2;
      char* copy = new char[capacity];
      memcpy(copy, buffer, size);
      delete[] buffer;
      buffer = copy;
    }
    buffer[size - 1] = next_element;
  }

  char& back() {
    return buffer[size - 1];
  }

  char back() const {
    return buffer[size - 1];
  }

  String& operator+=(const String& other) {
    capacity = 2 * (size + other.size) + 1;
    char* result = static_cast<char *> (malloc(capacity));
    memcpy(result, buffer, size) ;
    memcpy(result + size, other.buffer, other.size + 1);
    size += other.size;
    delete[] buffer;
    buffer = result;
    return *this;
  }

  size_t find(const String& substring, size_t begin = 0) const {
    if (size < substring.length()) {
      return size;
    }
    size_t sub_size = substring.length();
    for (size_t i = begin; i <= size - sub_size; ++i) {
      size_t current = 0;
      while (current < sub_size && buffer[i + current] == substring[current]) {
        ++current;
      }
      if (current == sub_size) {
        return i;
      }
    }
    return size;
  }

  size_t rfind(const String& substring) const {
    size_t ans = size;
    size_t begin = 0;
    while (find(substring, begin) != size) {
      ans = find(substring, begin);
      begin = ans + 1;
    }
    return ans;
  }

  String substr(size_t start, size_t count) const {
    String substring = String(count, 0);
    for (size_t i = 0; i < count; ++i) {
      substring.buffer[i] = buffer[start + i];
    }
    return substring;
  }

  bool empty() const {
    return (size == 0);
  }

  void clear() {
    delete[] buffer;
    buffer = new char[0];
    size = 0;
    capacity = 1;
  }

  String& operator=(const String& s) {
    String copy = s;
    exchange(copy);
    return *this;
  }

private:
  void exchange(String& s) {
    std::swap(size, s.size);
    std::swap(capacity, s.capacity);
    std::swap(buffer, s.buffer);
  }

public:
  ~String() {
    delete[] buffer;
  };
};


std::istream& operator>>(std::istream& in, String& string) {
  string.clear();
  char c;
  in.get(c);
  while (!isspace(c) && !in.eof()) {
    string += c;
    in.get(c);
  }
  return in;
}

std::ostream& operator<<(std::ostream& out, const String& string) {
  for (size_t i = 0; i < string.length(); ++i) {
    out << string[i];
  }
  return out;
}

String operator+(const String& first, const String& second) {
  String result(first);
  result += second;
  return result;
}