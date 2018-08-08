#include "sdsl/int_vector.hpp"
#include "sdsl/rank_support.hpp"
#include "sdsl/select_support.hpp"
#include "compact_vector/compact_vector.hpp"
#include "ScopedTimer.hpp"
#include "rs9/rank9sel.h"
//#include "packed_vector.hpp"
#include <random>

template <typename T>
void fill(std::vector<uint64_t>& p, T& v) {
  ScopedTimer t;
  for (auto s : p) {
    v[s] = 1;
  }
}


template <typename T>
__attribute__((flatten)) size_t sum_new(T& v, size_t nel) {
  ScopedTimer t;
  size_t tot{0};
  for (auto s : v) {
    tot += s;
  }
  return tot;
}

template <typename T>
__attribute__((flatten)) size_t sum_old(T& v, size_t nel)  {
  ScopedTimer t;
  size_t tot{0};
  for (size_t i = 0; i < nel; ++i){
    tot += v[i];
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

template <typename Vec1T, typename Vec2T>
void testVecs(Vec1T& bvs, Vec2T& bvc, size_t vecLength) {
  std::cerr << "generating random queries ...";
  std::mt19937 gen(8675309); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<uint64_t> dis(1, vecLength);
  size_t nsamp = static_cast<size_t>(vecLength * 0.1);
  std::cerr << "filling vectors\n";
  for (size_t i = 0; i < nsamp; ++i) {
    uint32_t start = static_cast<uint64_t>(dis(gen));
    //samps[i] = start;
    bvs[start] = 1;
    bvc[start] = 1;
  }

  {
    size_t t1{0};
    size_t t2{0};
    std::cerr << "\nsum sdsl (new)\n";
    for (size_t i = 0; i < 3; ++i) {
      t1 = sum_new(bvs, vecLength);
    }
    std::cerr << "\nsum sdsl (old)\n";
    for (size_t i = 0; i < 3; ++i) {
      t1 = sum_old(bvs, vecLength);
    }
    std::cerr << "sum 1 = " << t1 << '\n';
    std::cerr << "\nsum compact (new)\n";
    for (size_t i = 0; i < 3; ++i) {
      t2 = sum_new(bvc, vecLength);
    }
    std::cerr << "\nsum compact (old)\n";
    for (size_t i = 0; i < 3; ++i) {
      t2 = sum_old(bvc, vecLength);
    }
    std::cerr << "\n";
    std::cerr << "sum 2 = " << t2 << '\n';
  }
}

int main(int argc, char* argv[]) {

  size_t vecLength = std::stoll(argv[1]);

  std::cerr << "\n\n Dynamically Sized \n\n";
  {
    sdsl::int_vector<> bvs(vecLength, 0, 1);
    compact::vector<uint64_t> bvc(1, vecLength);
    testVecs(bvs, bvc, vecLength);
  }
  std::cerr << "\n\n Statically Sized \n\n";
  {
    sdsl::bit_vector bvs(vecLength);
    compact::vector<uint64_t, 1> bvc(vecLength);
    testVecs(bvs, bvc, vecLength);
  }

  {
    //auto r = memcmp(bvs.data(), bvc.get(), bvc.bytes());
    //std::cerr << "memcmp = " << r << '\n';
    //auto bvs2 = bvs;
    /*
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
    */
    //first_diff(sels, sel9, nones);
  }
  return 0;
}
