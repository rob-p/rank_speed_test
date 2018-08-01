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

size_t test_sel(std::vector<uint64_t>& p, sdsl::bit_vector::select_1_type& s, size_t nones) {
  ScopedTimer t;
  size_t res{0};
  for (size_t e = 1; e <= nones; ++e){
    res += s(e);
  }
  return res;
}

size_t test_sel(std::vector<uint64_t>& p, rank9sel& s, size_t nones) {
  ScopedTimer t;
  size_t res{0};
  for (size_t e = 1; e <= nones; ++e){
    res += s.select(e)-1;
  }
  return res;
}

int main(int argc, char* argv[]) {

  size_t vecLength{4000000000};
  std::cerr << "generating random queries ...";
  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<uint64_t> dis(1, vecLength);

  // values near the mean are the most likely
  // standard deviation affects the dispersion of generated values from the mean
  size_t nsamp = 50000000;
  std::vector<uint64_t> samps(nsamp);
  for (size_t i = 0; i < nsamp; ++i) {
    uint32_t start = static_cast<uint32_t>(dis(gen));
    samps[i] = start;
  }

  {
    std::cerr << "filling sdsl\n";
    sdsl::bit_vector bvs(vecLength);
    fill(samps, bvs);
    //std::cerr << "filling compact\n";
    compact::vector<uint64_t> bvc(1,vecLength);
    fill(samps, bvc);

    size_t t1 = sum(bvs);
    size_t t2 = t1;//sum(bvc);
    size_t nones = t1;
    /*
    std::cerr << "sum 1 = " << t1 << '\n';
    std::cerr << "sum 2 = " << t2 << '\n';
    */

    //auto r = memcmp(bvs.data(), bvc.get(), bvc.bytes());
    //std::cerr << "memcmp = " << r << '\n';

    {
      sdsl::bit_vector::select_1_type sels(&bvs);
      t1 = test_sel(samps, sels, nones);
      std::cerr << "sum 1 = " << t1 << '\n';
    }

    rank9sel sel9(bvc.get(), bvs.bit_size());

    t2 = test_sel(samps, sel9, nones);
    std::cerr << "sum 2 = " << t2 << '\n';
  }
  return 0;
}
