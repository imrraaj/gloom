#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <sys/resource.h>
#include <vector>

using namespace std;

typedef uint64_t (*HashFunction)(const string &);

long get_mem_usage() {
  struct rusage myusage;
  getrusage(RUSAGE_SELF, &myusage);
  return myusage.ru_maxrss;
}

std::vector<std::string> split(const std::string &s,
                               const std::string &delimiter) {
  size_t pos_start = 0, pos_end;
  std::string token;
  std::vector<std::string> res;

  while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
    token = s.substr(pos_start, pos_end - pos_start);
    pos_start = pos_end + delimiter.length();
    res.push_back(token);
  }
  res.push_back(s.substr(pos_start));
  return res;
}

const static uint64_t FNV_offset_basis = 14695981039346656037ULL;
const static uint64_t FNV_prime = 1099511628211ULL;

uint64_t fnv_hash_1(const string &s) {
  uint64_t hash = FNV_offset_basis;
  for (uint8_t byte_of_data : s) {
    hash *= FNV_prime;
    hash ^= byte_of_data;
  }
  return hash;
}

uint64_t fnv_hash_1a(const string &s) {
  uint64_t hash = FNV_offset_basis;
  for (uint8_t byte_of_data : s) {
    hash ^= byte_of_data;
    hash *= FNV_prime;
  }
  return hash;
}

uint64_t fnv_hash_0(const string &s) {
  uint64_t hash = 0;
  for (uint8_t byte_of_data : s) {
    hash *= FNV_prime;
    hash ^= byte_of_data;
  }
  return hash;
}

class BloomFilter {
private:
  size_t max_memory_limit;
  vector<HashFunction> hash_fns;
  vector<bool> mem;

public:
  BloomFilter(size_t max_memory, vector<HashFunction> fns) {
    this->max_memory_limit = max_memory;
    this->hash_fns = fns;
    this->mem.resize(max_memory, false);
  }

  void set_value(const string &s) {
    for (HashFunction fn : this->hash_fns) {
      uint64_t hash_value = fn(s);
      uint32_t hash_value_23bit =
          static_cast<uint32_t>((hash_value >> 32) ^ (hash_value & 0xFFFFFFFF));
      hash_value_23bit &= 0x7FFFFF; // Keep only lower 23 bits
      set_bit(hash_value_23bit);
    }
  }

  bool check_value(const string &s) {
    for (HashFunction fn : this->hash_fns) {
      uint64_t hash_value = fn(s);
      uint32_t hash_value_23bit =
          static_cast<uint32_t>((hash_value >> 32) ^ (hash_value & 0xFFFFFFFF));
      hash_value_23bit &= 0x7FFFFF; // Keep only lower 23 bits
      if (!check_bit(hash_value_23bit))
        return false;
    }
    return true;
  }

  void set_bit(uint32_t value) { mem[value] = true; }

  bool check_bit(uint32_t value) { return mem[value]; }
};

int main(int argc, char **argv) {

  map<string, bool> dict_map;
  if (argc < 2) {
    cout << "USAGE: " << argv[0] << " <filename.txt>" << endl;
    return 0;
  }
  ifstream dict(argv[1]);

  string temp;
  while (getline(dict, temp)) {
    dict_map[temp] = true;
  }
  dict.close();

  string text = "hi hello word concurrency coding challenges imadethis up";
  vector<string> items = split(text, " ");
  for (const auto &it : items) {
    bool ans = dict_map[it];
    if (!ans)
      cout << it << " is not present\n";
  }

  cout << "Using: " << get_mem_usage() << " memory\n";

  return 0;
}
