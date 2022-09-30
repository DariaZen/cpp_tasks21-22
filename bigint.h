#include <iostream>
#include <vector>
#include <string>

using std::vector;
using std::string;


class BigInteger {
private:
  static const int base = 1000000000;
  static const int digits_number = 9;
  vector<int64_t> buffer;
  size_t size = 0;
  int sign = 1;

public:

  BigInteger() = default;

  BigInteger(const BigInteger& other) = default;

  explicit BigInteger(const string& str) {
    buffer = {};
    size = 0;
    sign = 1;
    string current;
    int count = 0;
    for (int64_t i = str.size() - 1; i >= 0; --i) {
      if (str[i] == '-') {
        sign = -1;
      } else {
        if (count < digits_number) {
          current = str[i] + current;
          ++count;
        } else {
          buffer.push_back(std::stoi(current));
          ++size;
          count = 1;
          current = str[i];
        }
      }
    }
    if (count != 0) {
      buffer.push_back(std::stoi(current));
      ++size;
    }
    if (size == 1 && buffer[0] == 0) {
      sign = 1;
    }
  }

  BigInteger(int64_t number) {
    sign = number > 0 ? -1 : 1;
    if (number < 0) {
      sign = -1;
    } else {
      sign = 1;
    }
    if (number == 0) {
      buffer = {0};
      size = 1;
    } else {
      size = 0;
      int64_t number1 = abs(number);
      while (number1 > 0) {
        buffer.push_back(number1 % base);
        number1 /= base;
        ++size;
      }
    }
  }

  BigInteger& operator/=(const BigInteger& other);

  BigInteger& operator%=(const BigInteger& other);

  BigInteger& operator++();

  BigInteger operator++(int);

  BigInteger& operator--();

  BigInteger operator--(int);
  
  int get_sign() const {
    return sign;
  }

  string toString() const {
    if (size == 1 && buffer[0] == 0) {
      return "0";
    }
    string number;
    if (sign == -1) {
      number += "-";
    }
    for (size_t i = size; i >= 1; --i) {
      string s = std::to_string(buffer[i - 1]);
      while (s.size() < digits_number && (i - 1) != size - 1) {
        s = '0' + s;
      }
      number += s;
    }
    return number;
  }

  bool operator==(const BigInteger& other) const {
    if (sign != other.sign) {
      return false;
    }
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

  bool operator!=(const BigInteger& other) const {
    return !(*this == other);
  }

  bool operator<(const BigInteger& other) const {
    if (sign != other.sign) {
      return sign < other.sign;
    }
    if (size != other.size) {
      if (sign == 1) {
        return size < other.size;
      }
      return size > other.size;
    }
    for (int64_t i = size - 1; i >= 0; --i) {
      if (buffer[i] != other.buffer[i]) {
        if (sign == 1) {
          return buffer[i] < other.buffer[i];
        }
        return buffer[i] > other.buffer[i];
      }
    }
    return false;
  }

  bool operator<=(const BigInteger& other) const {
    return (*this < other || *this == other);
  }

  bool operator>(const BigInteger& other) const {
    return other < *this;
  }

  bool operator>=(const BigInteger& other) const {
    return other <= *this;
  }

  BigInteger& operator+=(const BigInteger& other) {
    int64_t current = 0;
    buffer.resize(std::max(size, other.size) + 1, 0);
    if (sign == other.sign) {
      for (size_t i = 0; i < buffer.size(); ++i) {
        current += i < other.size ? other.buffer[i] + buffer[i]: buffer[i];
        buffer[i] = current % base;
        current /= base;
      }
    } else {
      sign = other.sign;
      if (*this >= other) {
        sign = -1;
        for (size_t i = 0; i < buffer.size(); ++i) {
          current += i < other.size ? other.sign * (-other.buffer[i] + buffer[i]): other.sign * buffer[i];
          buffer[i] = current >= 0 ? current: base + current;
          current = current >= 0 ? 0: -1;
        }
      } else {
        sign = 1;
        for (size_t i = 0; i < buffer.size(); ++i) {
          current += i < other.size ? other.sign * (other.buffer[i] - buffer[i]): -buffer[i] * other.sign;
          buffer[i] = current >= 0 ? current: base + current;
          current = current >= 0 ? 0: -1;
        }
      }
    }
    size = buffer.size();
    size_t current_size = size;
    for (size_t i = current_size - 1; i >= 1; --i) {
      if (buffer[i] == 0) {
        --size;
        buffer.pop_back();
      } else {
        break;
      }
    }
    if (size == 1 && buffer[0] == 0) {
      sign = 1;
    }
    return *this;
  }

  BigInteger& operator-=(const BigInteger& other) {
    BigInteger bigint = other;
    bigint.sign *= -1;
    return *this += bigint;
  }

  BigInteger& operator*=(const BigInteger& other) {
    sign *= other.sign;
    BigInteger answer;
    answer.sign = sign;
    size_t current_size = size + other.size + 1;
    answer.buffer.resize(current_size, 0);
    answer.size = current_size;
    for (size_t i = 0; i < size; ++i) {
      for (size_t j = 0; j < other.size; ++j) {
        int64_t result = buffer[i] * other.buffer[j] + answer.buffer[i + j];
        answer.buffer[i + j + 1] += result / base;
        answer.buffer[i + j] = result % base;
      }
    }
    for (size_t i = current_size - 1; i >= 1; --i) {
      if (answer.buffer[i] != 0) {
        break;
      }
      --answer.size;
      answer.buffer.pop_back();
    }
    return *this = answer;
  }

  explicit operator bool() const {
    if (*this == 0) {
      return false;
    }
    return true;
  }

  BigInteger operator-() const {
    BigInteger answer(*this);
    if (answer == 0) {
      return answer;
    }
    answer.sign = -sign;
    return answer;
  }

  ~BigInteger() {
    buffer.clear();
    buffer.shrink_to_fit();
  }
};


BigInteger operator+(const BigInteger& first, const BigInteger& second) {
  BigInteger answer(first);
  answer += second;
  return answer;
}

BigInteger operator-(const BigInteger& first, const BigInteger& second) {
  BigInteger answer(first);
  answer -= second;
  return answer;
}

BigInteger& BigInteger::operator++() {
  return *this += 1;
}

BigInteger BigInteger::operator++(int) {
  return (*this += 1) - 1;
}

BigInteger& BigInteger::operator--() {
  return *this -= 1;
}

BigInteger BigInteger::operator--(int) {
  return (*this -= 1) + 1;
}

BigInteger operator*(const BigInteger& first, const BigInteger& second) {
  BigInteger answer(first);
  answer *= second;
  return answer;
}

std::istream& operator>>(std::istream& in, BigInteger& bigInteger) {
  std::string str;
  in >> str;
  bigInteger = BigInteger(str);
  return in;
}

std::ostream& operator<<(std::ostream& out, const BigInteger& bigInteger) {
  std::string string = bigInteger.toString();
  for (char ch: string) {
    out << ch;
  }
  return out;
}

BigInteger& BigInteger::operator/=(const BigInteger& other) {
  sign *= other.sign;
  BigInteger abs_other = other;
  abs_other.sign = 1;
  BigInteger answer;
  BigInteger slice;
  answer.sign = sign;
  for (int64_t i = size - 1; i >= 0; --i) {
    slice.buffer.insert(slice.buffer.begin(), buffer[i]);
    ++slice.size;
    size_t current = slice.size;
    for (size_t i = current - 1; current >= 1; --i) {
      if (slice.buffer[i] == 0) {
        --slice.size;
      } else {
        break;
      }
    }
    int64_t left = 0;
    int64_t right = base;
    int64_t divider = 0;
    while (left <= right) {
      int64_t middle = (left + right) / 2;
      if (abs_other * middle <= slice) {
        divider = middle;
        left = middle + 1;
      } else {
        right = middle - 1;
      }
    }
    answer.buffer.push_back(divider);
    ++answer.size;
    slice -= abs_other * divider;
  }
  std::reverse(answer.buffer.begin(), answer.buffer.end());
  size_t current_size = answer.buffer.size();
  for (size_t i = current_size - 1; i >= 1; --i) {
    if (answer.buffer[i] == 0) {
      --answer.size;
      answer.buffer.pop_back();
    } else {
      break;
    }
  }
  return *this = answer;
} // Ğ¿Ğ¾Ñ Ğ¼Ğ¾Ñ Ñ ĞµÑ Ñ  ĞµÑ Ğµ

BigInteger operator/(const BigInteger& first, const BigInteger& second) {
  BigInteger answer(first);
  answer /= second;
  return answer;
}

BigInteger& BigInteger::operator%=(const BigInteger& other) {
  *this -= (*this / other) * other;
  return *this;
}

BigInteger operator%(const BigInteger& first, const BigInteger& second) {
  BigInteger answer(first);
  answer %= second;
  return answer;
}


class Rational {
private:
  BigInteger numerator;
  BigInteger denominator = 1;
public:
  Rational() = default;

  Rational(const Rational& other) : numerator(other.numerator), denominator(other.denominator) {
    shorten();
  };

  Rational(const BigInteger& first, const BigInteger& second = 1) : numerator(first), denominator(second) {};

  Rational(const int64_t first, const int64_t second = 1) : numerator(first), denominator(second) {};

  void transform() {
    numerator *= denominator.get_sign();
    if (numerator == 0 || -numerator == 0) {
      numerator *= numerator.get_sign();
    }
    denominator *= denominator.get_sign();
  }

  void shorten() {
    transform();
    int s1 = numerator.get_sign();
    int s2 = denominator.get_sign();
    BigInteger a = numerator;
    a *= a.get_sign();
    BigInteger b = denominator;
    b *= b.get_sign();
    while (a && b) {
      if (a > b) {
        a %= b;
      } else {
        b %= a;
      }
    }
    numerator /= a + b;
    denominator /= a + b;
    numerator *= s1 * s2;
    denominator *= denominator.get_sign();
  }

  Rational& operator+=(const Rational& other) {
    numerator = numerator * other.denominator + other.numerator * denominator;
    denominator = denominator * other.denominator;
    shorten();
    return *this;
  }

  Rational& operator-=(const Rational& other) {
    numerator = numerator * other.denominator - other.numerator * denominator;
    denominator = denominator * other.denominator;
    shorten();
    return *this;
  }

  Rational& operator*=(const Rational& other) {
    numerator = numerator * other.numerator;
    denominator = denominator * other.denominator;
    shorten();
    return *this;
  }

  Rational& operator/=(const Rational& other) {
    numerator = numerator * other.denominator;
    denominator = other.numerator * denominator;
    shorten();
    return *this;
  }

  Rational operator-() const {
    Rational answer(*this);
    if (numerator == 0) {
      return answer;
    }
    answer.numerator *= -1;
    return answer;
  }

  bool operator==(const Rational& other) const {
    return (numerator * other.denominator == denominator * other.numerator);
  }

  bool operator!=(const Rational& other) const {
    return !(*this == other);
  }

  bool operator<(const Rational& other) const {
    Rational first(*this);
    Rational second(other);
    first.transform();
    second.transform();
    return first.numerator * second.denominator < first.denominator * second.numerator;
  }

  bool operator<=(const Rational& other) const {
    return (*this < other || *this == other);
  }

  bool operator>(const Rational& other) const {
    return other < *this;
  }

  bool operator>=(const Rational& other) const {
    return other <= *this;
  }

  string toString() {
    shorten();
    if (denominator == 1) {
      return numerator.toString();
    }
    return numerator.toString() + '/' + denominator.toString();
  }

  string asDecimal(size_t precision = 0) const {
    BigInteger pow = 1;
    for (size_t i = 0; i < precision; ++i) {
      pow *= 10;
    }
    string answer;
    if (numerator == 0) {
      return "0." + string(precision, '0');
    }
    if (numerator.get_sign() != denominator.get_sign()) {
      answer = "-";
    }
    BigInteger a = numerator;
    BigInteger b = denominator;
    a *= a.get_sign();
    b *= b.get_sign();
    BigInteger qoutient = a / b;
    answer += qoutient.toString();
    if (precision == 0) {
      return answer;
    }
    answer += '.';
    qoutient *= qoutient.get_sign();
    string afterPoint = (((a * pow) / b) - qoutient * pow).toString();
    while (afterPoint.size() != precision) {
      afterPoint = '0' + afterPoint;;
    }
    answer += afterPoint;
    return answer;
  }

  explicit operator double() const {
    return strtod(asDecimal(10).c_str(), nullptr);
  }
};

Rational operator+(const Rational& first, const Rational& second) {
  Rational answer(first);
  answer += second;
  return answer;
}

Rational operator-(const Rational& first, const Rational& second) {
  Rational answer(first);
  answer -= second;
  return answer;
}

Rational operator*(const Rational& first, const Rational& second) {
  Rational answer(first);
  answer *= second;
  return answer;
}

Rational operator/(const Rational& first, const Rational& second) {
  Rational answer(first);
  answer /= second;
  return answer;
}