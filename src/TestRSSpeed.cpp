#include "bar.h"
#include "rank9b.h"
#include "rank9.h"
#include "FastxParser.hpp"
#include "ScopedTimer.hpp"
#include "poppy.h"
#include <vector>
#include <random>
#include <cmath>
#include <iostream>

std::vector<uint32_t> getGCCount(std::string& txp) {
  std::vector<uint32_t> gc(txp.length(), 0);
  uint32_t sum = 0;//txp[0] == 'G' or txp[0] == 'C');
  for (size_t i = 0; i < txp.length(); ++i) {
    if(txp[i] == 'G' or txp[i] == 'C') { ++sum; }
    gc[i] = sum;
  }
  return gc;
}

template <typename T>
class GCRank {
public:
  GCRank() {}

  GCRank(BIT_ARRAY* ba) : ba_(ba){
    r_.reset(new T(ba->words, ba->num_of_bits));
  }

  GCRank(GCRank&& other) {
    std::swap(ba_, other.ba_);
    r_.swap(other.r_);
  }

  GCRank& operator=(GCRank&& other) {
    std::swap(ba_, other.ba_);
    r_.swap(other.r_);
    return *this;
  }

  inline uint64_t operator[](size_t i) { return r_->rank(i); }
  BIT_ARRAY* getBA() { return ba_; }
private:
  BIT_ARRAY* ba_;
  std::unique_ptr<T> r_{nullptr};
};

template <typename T>
GCRank<T> getGCCountRank(std::string& txp, bool rev=false) {
  BIT_ARRAY* ba = bit_array_create(txp.length());

  for (size_t i = 0; i < txp.length(); ++i) {
    if (txp[i] == 'G' or txp[i] == 'C') {
      bit_array_set(ba, i);
    }
  }
  if (rev) {
    //bit_array_reverse(ba);
  
  size_t i;
  for (i = 0; i + 64 < txp.length(); i += 64) {
    bit_array_reverse_region(ba, i, 64);
  }
  if (i < txp.length()) {
    bit_array_reverse_region(ba, i, txp.length() - i);
  }
  
  }
  return GCRank<T>(ba);
}

int main(int argc, char* argv[]) {
  std::vector<std::string> txpfile{argv[1]};
  fastx_parser::FastxParser<fastx_parser::ReadSeq> parser(txpfile, 1, 1);
  parser.start();


  std::string txpString;
  // Get the read group by which this thread will
  // communicate with the parser (*once per-thread*)
  auto rg = parser.getReadGroup();

  size_t tnum{0};
  while (parser.refill(rg)) {
    // Here, rg will contain a chunk of read pairs
    // we can process.
    for (auto& rp : rg) {
      auto& r1 = rp.seq;
      transform(r1.begin(), r1.end(), r1.begin(), ::toupper);
      txpString += r1;
    }
    //++tnum;
    //if(tnum > 1) { break; }
  }


  std::cerr << "txpString length = " << txpString.length() << "\n";
  std::vector<uint32_t> gcVec;
  std::cerr << "creating GC vector ...";
  {
    ScopedTimer t;
    gcVec = getGCCount(txpString);
  }
  std::cerr << "done\n";

  GCRank<rank9b> gcR9b;
  std::cerr << "creating GC Rank vector ..";
  {
    ScopedTimer t;
    gcR9b = getGCCountRank<rank9b>(txpString);
  }
  std::cerr << "done\n";

  GCRank<rank9> gcR9;
  std::cerr << "creating GC Rank vector ..";
  {
    ScopedTimer t;
    gcR9 = getGCCountRank<rank9>(txpString);
  }
  std::cerr << "done\n";

  GCRank<Poppy> gcPoppy;
  std::cerr << "creating GC Rank vector ..";
  {
    ScopedTimer t;
    gcPoppy = getGCCountRank<Poppy>(txpString, true);
  }
  std::cerr << "done\n";

  /*
  std::cerr << txpString.substr(0, 64);
  std::cerr << "\n";
  char* bitstr = new char[65];
  bitstr[64] = '\0';
  bit_array_to_substr(gcPoppy.getBA(), 0, 64, bitstr, '1', '0', 1);
  std::cerr << bitstr << "\n";
  for (size_t i = 0; i < 64; ++i) {
    std::cerr << gcPoppy[i+1] << ", ";
  }
  std::cerr << "\n";
  for (size_t i = 0; i < 64; ++i) {
    std::cerr << gcR9[i+1] << ", ";
  }
  std::cerr << "\n";
  return 0;
  */

  std::cerr << "generating random queries ...";
  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<> dis(1, txpString.length() - 5000);
  // values near the mean are the most likely
  // standard deviation affects the dispersion of generated values from the mean
  std::normal_distribution<> d(200,20);
  size_t nsamp = 10000000;
  std::vector<uint32_t> starts(nsamp);
  std::vector<uint32_t> lens(nsamp);
  for (size_t i = 0; i < nsamp; ++i) {
    uint32_t start = static_cast<uint32_t>(dis(gen));
    int32_t len = static_cast<int32_t>(std::round(d(gen)));
    if (len < 1) { len = 1; }
    if (start + len >= txpString.length()) { len = txpString.length() - start; }
    starts[i] = start;
    lens[i] = len;
  }
  std::cerr << "done\n";

  std::vector<uint32_t> statsVec(starts.size());
  std::cerr << "querying with vector ...";
  {
    ScopedTimer t;
    for (size_t i = 0; i < starts.size(); ++i) {
      statsVec[i] = gcVec[starts[i] - 1 + lens[i]] - gcVec[starts[i]-1];
    }
  }
  std::cerr << "done\n";

  std::vector<uint32_t> statsVecRank9(starts.size());
  std::cerr << "querying with rank9 ...";
  {
    ScopedTimer t;
    for (size_t i = 0; i < starts.size(); ++i) {
      statsVecRank9[i] = gcR9[starts[i] + lens[i]] - gcR9[starts[i]];
    }
  }
  std::cerr << "done\n";


  std::vector<uint32_t> statsVecRank9b(starts.size());
  std::cerr << "querying with rank9b ...";
  {
    ScopedTimer t;
    for (size_t i = 0; i < starts.size(); ++i) {
      statsVecRank9b[i] = gcR9b[starts[i] + lens[i]] - gcR9b[starts[i]];
    }
  }
  std::cerr << "done\n";

  std::vector<uint32_t> statsVecRankPoppy(starts.size());
  std::cerr << "querying with rank poppy ...";
  {
    ScopedTimer t;
    for (size_t i = 0; i < starts.size(); ++i) {
      statsVecRankPoppy[i] = gcPoppy[starts[i] + lens[i]] - gcPoppy[starts[i]];
    }
  }
  std::cerr << "done\n";



  bool same = (statsVec == statsVecRank9);
  std::cerr << "rank9 is same ? ... " << std::boolalpha << same << "\n";
  same = (statsVec == statsVecRank9b);
  std::cerr << "rank9b is same ? ... " << std::boolalpha << same << "\n";
  same = (statsVec == statsVecRankPoppy);
  std::cerr << "rank poppy is same ? ... " << std::boolalpha << same << "\n";
  if (!same) {
    for (size_t i = 0; i < starts.size(); ++i) {
      auto x = statsVec[i] ;
      auto y = statsVecRankPoppy[i] ;
      if (x != y) {
        std::cerr << statsVec[i] << "\t" << statsVecRankPoppy[i] << "\t" << txpString.substr(starts[i], lens[i]) << "\t(" << starts[i] << ", " << lens[i] << ")\n\t" ;
        for(size_t j = starts[i]-1; j < starts[i] + lens[i] + 1; ++j) {
          std::cerr << gcVec[j] << ", ";
        }
        std::cerr << "\n\t";
        for(size_t j = starts[i]-1; j < starts[i] + lens[i] + 1; ++j) {
          std::cerr << gcPoppy[j] << ", ";
        }
        std::cerr << "\n";
      }
    }
  }

  return EXIT_SUCCESS;
}
