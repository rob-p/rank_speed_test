#include "bar.h"
#include "rank9b.h"
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

class GCRank {
public:
  GCRank() {}

  GCRank(BIT_ARRAY* ba) {
    r_.reset(new Poppy(ba->words, ba->num_of_bits));
  }

  GCRank(GCRank&& other) {
    r_.swap(other.r_);
  }

  GCRank& operator=(GCRank&& other) {
    r_.swap(other.r_);
    return *this;
  }

  inline uint64_t operator[](size_t i) { return r_->rank(i); }
private:
  std::unique_ptr<Poppy> r_{nullptr};
};

GCRank getGCCountRank(std::string& txp) {
  BIT_ARRAY* ba = bit_array_create(txp.length());

  for (size_t i = 0; i < txp.length(); ++i) {
    if (txp[i] == 'G' or txp[i] == 'C') {
      bit_array_set(ba, i);
    }
  }
  return GCRank(ba);
}

int main(int argc, char* argv[]) {
  std::vector<std::string> txpfile{argv[1]};
  fastx_parser::FastxParser<fastx_parser::ReadSeq> parser(txpfile, 1, 1);
  parser.start();


  std::string txpString;
  // Get the read group by which this thread will
  // communicate with the parser (*once per-thread*)
  auto rg = parser.getReadGroup();

  while (parser.refill(rg)) {
    // Here, rg will contain a chunk of read pairs
    // we can process.
    for (auto& rp : rg) {
      auto& r1 = rp.seq;
      transform(r1.begin(), r1.end(), r1.begin(), ::toupper);
      txpString += r1;
    }
  }


  std::cerr << "txpString length = " << txpString.length() << "\n";
  std::vector<uint32_t> gcVec;
  std::cerr << "creating GC vector ...";
  {
    ScopedTimer t;
    gcVec = getGCCount(txpString);
  }
  std::cerr << "done\n";

  GCRank gcR;
  std::cerr << "creating GC Rank vector ..";
  {
    ScopedTimer t;
    gcR = getGCCountRank(txpString);
  }
  std::cerr << "done\n";


  std::cerr << "generating random queries ...";
  std::random_device rd;  //Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<> dis(1, txpString.length() - 500);
  // values near the mean are the most likely
  // standard deviation affects the dispersion of generated values from the mean
  std::normal_distribution<> d(5,2);
  size_t nsamp = 100000000;
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

  std::vector<uint32_t> statsVecRank(starts.size());
  std::cerr << "querying with vector ...";
  {
    ScopedTimer t;
    for (size_t i = 0; i < starts.size(); ++i) {
      statsVecRank[i] = gcR[starts[i] + lens[i]] - gcR[starts[i]];
    }
  }
  std::cerr << "done\n";


  bool same = (statsVec == statsVecRank);
  std::cerr << "are they the same? ... " << std::boolalpha << same << "\n";
  
  if (!same) {
    for (size_t i = 0; i < starts.size(); ++i) {
      auto x = statsVec[i] ;
      auto y = statsVecRank[i] ;
      if (x != y) {
        std::cerr << statsVec[i] << "\t" << statsVecRank[i] << "\t" << txpString.substr(starts[i], lens[i]) << "\t(" << starts[i] << ", " << lens[i] << ")\t" ;
        for(size_t j = starts[i]-1; j < starts[i] + lens[i] + 1; ++j) {
          std::cerr << gcVec[j] << ", ";
        }
        std::cerr << "\n";
      }
    }
  }
  
}
