#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

class BufferedReader {
private:
  std::istream &inputStream;
  int maxBufferSize = 1024;
  char buffer[1024];

public:
  BufferedReader(std::istream &inputStream);
  BufferedReader(std::istream &inputStream, int maxBufferSize);

  char read();
  std::vector<char> read(int n);
  std::string readLine();
  std::vector<std::string> lines();
};