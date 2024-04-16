#include <bit>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

using namespace std;

template <typename T> void write_file(fstream &file, const T &x) {
  file.write((char *)(x), sizeof(x));
}
void read_file(fstream &file, void *data) {
  file.read((char *)(&data), sizeof(data));
}

int main() {
  printf("%ld\n", sizeof(uint8_t));
  fstream in("word.bf", ios::out | ios::binary);

  uint16_t version = 1;
  in.write((char *)&version, sizeof(version));

  uint16_t hash = 2;
  in.write((char *)&hash, sizeof(hash));

  uint16_t bits = 32;
  in.write((char *)&bits, sizeof(bits));

  vector<uint8_t> bitarray(bits);
  for (int i = 0; i < bits; ++i) {
    bitarray[i] = i % 2;
    in.write((char *)&bitarray[i], sizeof(bitarray[0]));
  }
  in.close();

  bitarray.clear();
  fstream out("word.bf", ios::in | ios::binary);
  version = 245;
  out.read((char *)&version, sizeof(version));
  printf("version: %d\n", version);

  out.read((char *)&hash, sizeof(hash));
  printf("hash: %d\n", hash);

  out.read((char *)&bits, sizeof(bits));
  printf("bits: %d\n", bits);

  for (int i = 0; i < bits; ++i) {
    out.read((char *)&bitarray[i], sizeof(bitarray[0]));
    printf("%i: %d\n", i, bitarray[i]);
  }
  out.close();
}
