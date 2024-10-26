#include <buffered_reader.h>

BufferedReader::BufferedReader(std::istream &inputStream)
    : inputStream(inputStream) {}

BufferedReader::BufferedReader(std::istream &inputStream, int maxBufferSize)
    : inputStream(inputStream), maxBufferSize(maxBufferSize) {}

char BufferedReader::read() {
  // Not implemented yet
  throw std::runtime_error("Not implemented yet");
}

std::string BufferedReader::readLine() {
  // Not implemented yet
  throw std::runtime_error("Not implemented yet");
}

std::vector<std::string> BufferedReader::lines() {
  // Not implemented yet
  throw std::runtime_error("Not implemented yet");
}