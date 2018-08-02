#include "sdsl/int_vector.hpp"
#include "sdsl/rank_support.hpp"
#include "sdsl/select_support.hpp"
#include "compact_vector/compact_vector.hpp"
#include "ScopedTimer.hpp"
#include "rank9sel.h"
#include <random>

template <typename T>
void fill(std::vector<uint64_t>& p, T& v) {
  ScopedTimer t;
  for (auto s : p) {
    v[s] = 1;
  }
}

template <typename T>
size_t sum(T& v) {
  ScopedTimer t;
  size_t tot{0};
  for (const auto &s : v) {
    tot += s;
  }
  return tot;
}

size_t test_sel(sdsl::bit_vector::select_1_type& s, size_t nones) {
  size_t res{0};
  {
  ScopedTimer t;
  for (size_t e = 1; e <= nones; ++e){
    res += s(e);
  }
  }
  return res;
}

size_t test_sel(rank9sel& s, size_t nones) {
  size_t res{0};
  {
  ScopedTimer t;
  for (size_t e = 1; e <= nones; ++e){
    res += s.select(e-1);
  }
  }
  return res;
}

void first_diff(sdsl::bit_vector::select_1_type& s, rank9sel& s2, size_t nones) {
  size_t res{0};
  for (size_t e = 1; e <= nones; ++e){
    auto a = s(e);
    auto b = s2.select(e-1);
    if (a != b) {
      std::cerr << "e = " << e << '\n';
      std::cerr << "sdsl     says select(" << e << ") = " << a << '\n';
      std::cerr << "rank9sel says select(" << e << ") = " << b << '\n';
      return;
    }
  }
}

int main(int argc, char* argv[]) {

  size_t vecLength{4000000000};
  std::cerr << "generating random queries ...";
  //std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 gen(8675309); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<uint64_t> dis(1, vecLength);

  // values near the mean are the most likely
  // standard deviation affects the dispersion of generated values from the mean
  std::cerr << "filling sdsl\n";
  size_t nsamp = 50032258;
  //std::vector<uint64_t> samps(nsamp);
  sdsl::bit_vector bvs(vecLength);
  compact::vector<uint64_t> bvc(1,vecLength);
  for (size_t i = 0; i < nsamp; ++i) {
    uint32_t start = static_cast<uint64_t>(dis(gen));
    //samps[i] = start;
    bvs[start] = 1;
    bvc[start] = 1;
  }

  {
    std::cerr << "sum sdsl ";
    size_t t1 = sum(bvs);
    std::cerr << "\nsum compact ";
    size_t t2 = sum(bvc);
    std::cerr << "\n";
    std::cerr << "sum 1 = " << t1 << '\n';
    std::cerr << "sum 2 = " << t2 << '\n';

    //auto r = memcmp(bvs.data(), bvc.get(), bvc.bytes());
    //std::cerr << "memcmp = " << r << '\n';
    //auto bvs2 = bvs;

    sdsl::bit_vector::rank_1_type ranks(&bvs);
    sdsl::bit_vector::select_1_type sels(&bvs);
    size_t nones = ranks(bvs.size()-1);
    for (size_t i = 0; i < 10; ++i) {
      t1 = test_sel(sels, nones);
    }
    std::cerr << "sum 1 = " << t1 + ranks(100) << '\n';

    rank9sel sel9(bvc.get(), bvc.bytes()*8);
    for (size_t i = 0; i < 10; ++i) {
      t2 = test_sel(sel9, nones);
    }
    std::cerr << "sum 2 = " << t2 + sel9.rank(100) << '\n';
    //first_diff(sels, sel9, nones);
  }
  return 0;
}
