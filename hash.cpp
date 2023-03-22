#include <cassert>
#include <functional>
#include <iostream>
#include <unordered_map>

struct S {
  int m_i;
  float m_f;
  double m_d;
  std::string m_s;
};

/// output stream operator overloading so we can print objects of type S
std::ostream &operator<<(std::ostream &os, S const &s) {
  os << s.m_i << ' ' << s.m_f << ' ' << s.m_d << ' ' << s.m_s;
  return os;
}

/// we need the operator==() to resolve hash collisions
bool operator==(S const &lhs, S const &rhs) {
  return lhs.m_i == rhs.m_i
         && lhs.m_f == rhs.m_f
         && lhs.m_d == rhs.m_d
         && lhs.m_s == rhs.m_s;
}

/// we can use the hash_combine variadic-template function, to combine multiple
/// hashes into a single one
template <typename T, typename... Rest>
constexpr void hash_combine(std::size_t &seed, T const &val, Rest const &...rest) {
  constexpr size_t hash_mask{0x9e3779b9};
  constexpr size_t lsh{6};
  constexpr size_t rsh{2};
  seed ^= std::hash<T>{}(val) + hash_mask + (seed << lsh) + (seed >> rsh);
  (hash_combine(seed, rest), ...);
}

/// custom specialization of std::hash injected in namespace std
template<>
struct std::hash<S> {
  std::size_t operator()(S const &s) const noexcept {
    std::size_t h1 = std::hash<int>{}(s.m_i);
    std::size_t h2 = std::hash<float>{}(s.m_f);
    std::size_t h3 = std::hash<double>{}(s.m_d);
    std::size_t h4 = std::hash<std::string>{}(s.m_s);

    std::size_t ret_val = 0;
    hash_combine(ret_val, h1, h2, h3, h4);
    return ret_val;
  }
};

int main() {
  S s1{1, 2.0f, 3.0, "4"};
  S s2{1, 2.0f, 3.0, "4"};
  S s3{0, 2.0f, 3.0, "4"};

  std::size_t hash1 = std::hash<S>{}(s1);
  std::size_t hash2 = std::hash<S>{}(s2);
  std::size_t hash3 = std::hash<S>{}(s3);

  std::cout << hash1 << '\n';
  std::cout << hash2 << '\n';
  std::cout << hash3 << '\n';
  // Example output:
  // 8180152382189146914
  // 8180152382189146914
  // 8180152382189415317
  //
  // Notice that s1 and s2 have the same hash, which is different than the hash
  // of s3!

  assert(hash1 == hash2);
  assert(hash1 != hash3);

  std::unordered_map<S, int> m;
  m[s1] = 1;
  m[s2] = 2;
  m[s3] = 3;

  for (auto const &kv : m) {
    std::cout << "v:" << kv.second << " k:" << kv.first << '\n';
  }
  // Exaple output:
  // v:3 k:0 2 3 4
  // v:2 k:1 2 3 4
  //
  // Notice that s1 and s2 are the same key!
}
