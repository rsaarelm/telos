#ifndef XEV_NUM_HPP
#define XEV_NUM_HPP

#include <cstddef>
#include <iterator>

namespace xev {

/// Modulo function that handles negative numbers like you'd expect.
template<class Num>
Num mod(Num x, Num m) {
  return (x < 0 ? ((x % m) + m) : x % m);
}

template<class Num>
Num sign(Num x) {
  if (x < Num(0))
    return Num(-1);
  else if (x > Num(0))
    return Num(1);
  else
    return Num(0);
}

int rand_int(int max);

template<class Container>
typename Container::iterator rand_choice(const Container& container) {
  auto result = container.begin();
  std::advance(result, rand_int(container.size() - 1));
  return result;
}

float uniform_rand();

bool one_chance_in(int n);

void seed_rand(int seed);

void seed_rand(const char* seed);

}

#endif