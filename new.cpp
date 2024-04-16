#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
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

struct BloomFilter {
  uint16_t version;
  string header;
  vector<HashFunction> hash_fns;
  vector<uint8_t> mem;
};

uint32_t hash_value(BloomFilter &bf, int i, const string &s) {
  uint64_t hash = bf.hash_fns[i](s);
  uint32_t hash_23bit = static_cast<uint32_t>((hash >> 32) ^ (hash & 0xFFFF));
  uint64_t boundry = bf.mem.size() - 1;
  hash_23bit &= boundry;
  return hash_23bit;
}

void set_value(BloomFilter &bf, const string &s) {
  for (int i = 0; i < bf.hash_fns.size(); ++i) {
    bf.mem[hash_value(bf, i, s)] = 1;
  }
}

bool check_value(BloomFilter &bf, const string &s) {
  for (int i = 0; i < bf.hash_fns.size(); ++i) {
    if (!bf.mem[hash_value(bf, i, s)])
      return false;
  }
  return true;
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

bool dump(BloomFilter &bf, const string &file_name) {
  fstream file;
  file.open(file_name, ios::out | ios::binary);

  if (!file) {
    return false;
  }

  uint16_t version = 1;
  uint16_t hash_functions = bf.hash_fns.size();
  int filter_bits = bf.mem.size();

  file.write(bf.header.c_str(), bf.header.length());
  file.write((char *)&version, sizeof(version));
  file.write((char *)&hash_functions, sizeof(hash_functions));
  file.write((char *)&filter_bits, sizeof(filter_bits));

  for (uint8_t it : bf.mem)
    file.write((char *)&it, sizeof(it));
  file.close();
  return true;
}

bool load(BloomFilter &bf, const string &file_name) {
  fstream file;
  file.open(file_name, ios::in | ios::binary);
  char header[5];
  file.read(header, 4);
  header[4] = '\0';

  if (strcmp(header, bf.header.c_str())) {
    cout << "Invalid file format \n";
    return false;
  }

  uint16_t version = 0;
  file.read((char *)(&version), sizeof(version));
  bf.version = version;

  uint16_t num_hash_functions = 0;
  file.read(reinterpret_cast<char *>(&num_hash_functions),
            sizeof(num_hash_functions));

  int num_bits_filter = 0;
  file.read(reinterpret_cast<char *>(&num_bits_filter),
            sizeof(num_bits_filter));
  bf.mem.resize(num_bits_filter);

  for (uint32_t i = 0; i < num_bits_filter; ++i) {
    uint8_t byte_value;
    file.read((char *)(&byte_value), sizeof(byte_value));
    bf.mem[i] = (byte_value);
  }

  file.close();
  return true;
}
void build(const string &input, const string &output) {
  BloomFilter bf;
  bf.header = "CCBF";
  bf.hash_fns.push_back(fnv_hash_0);
  bf.hash_fns.push_back(fnv_hash_1);
  bf.hash_fns.push_back(fnv_hash_1a);
  bf.mem.resize(1 << 20, 0);
  bf.version = 1;

  ifstream file(input);
  string temp;
  while (getline(file, temp)) {
    set_value(bf, temp);
  }
  file.close();

  string output_filepath = output;
  if (!dump(bf, output_filepath)) {
    cout << "ERROR: Dumping not successful\n";
    exit(1);
  }
}

void use(const string &input, char **values) {
  BloomFilter nbf;
  nbf.header = "CCBF";
  nbf.hash_fns.clear();
  nbf.hash_fns.push_back(fnv_hash_0);
  nbf.hash_fns.push_back(fnv_hash_1);
  nbf.hash_fns.push_back(fnv_hash_1a);
  if (!load(nbf, input)) {
    cout << "ERROR: Loading Bloom Filter\n";
    exit(1);
  }
  cout << nbf.version << endl;
  cout << nbf.hash_fns.size() << endl;
  cout << nbf.mem.size() << endl;
  for (int i = 0; values[i] != nullptr; ++i) {
    if (check_value(nbf, values[i])) {
      cout << values[i] << " exist" << endl;
    } else {
      cout << values[i] << " does not exis" << endl;
    }
  }
}

int main(int argc, char **argv) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0]
              << " [-build filename.txt output.bf | -use filename.bf word]"
              << std::endl;
    return 1;
  }

  std::string command = argv[1];
  if (command == "-build" && argc == 4) {
    build(argv[2], argv[3]);
  } else if (command == "-use" && argc > 3) {
    use(argv[2], argv + 3);
  } else {
    std::cerr << "Invalid command or arguments." << std::endl;
    return 1;
  }
  cout << "Using: " << get_mem_usage() << " memory\n";

  return 0;
}
