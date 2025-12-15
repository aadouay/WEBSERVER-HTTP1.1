#include <server.hpp>
#include <iostream>

static void skipWhitespace(std::string& value) {
  std::size_t index = 0;
  while (value[index]) {
    if (
      value[index] == ' '  ||
      value[index] == '\n' ||
      value[index] == '\r' ||
      value[index] == '\t' ||
      value[index] == '\v' ||
      value[index] == '\f'
    )
      value.erase(index, 1);
    else
      index++;
  }
}

static void storeKeyValue(const std::string& key, const std::string& value, ctr& s) {
  (void)key;
  (void)value;
  (void)s;
  // 1. key should be unique
  // 2. check and parse type of values
}

static int extractKey(std::string& configFileContent, std::size_t& pos, ctr& s) {
  if (configFileContent[pos] != '\"')
    throw std::runtime_error("key must initiate with '\"'");

  pos++;
  std::size_t keyStart = pos;
  while (true) {
    if (configFileContent[pos] == '\0')
      throw std::runtime_error("unexpected end of file while parsing key");
    if (configFileContent[pos] == ':')
      throw std::runtime_error("expected '\"' at the end of key");
    if (configFileContent[pos] == '\"')
      break;
    pos++;
  }
  std::string key = configFileContent.substr(keyStart, pos - keyStart);
  pos += 2; // skip `":`

  if (configFileContent[pos - 1] != ':')
    throw std::runtime_error("expected ':' after key");

  std::size_t valueStart = pos;
  if (configFileContent[pos] != '[') {
    if (configFileContent[pos] == '-' || std::isdigit(configFileContent[pos])) {
      while (configFileContent[pos]) {
        if (configFileContent[pos] == ',' || configFileContent[pos] == '}' || configFileContent[pos] == '\"')
          break;
        pos++;
      }
    }
    else if (configFileContent[pos] == '\"') {
      int counter = -1;
      while (configFileContent[pos]) {
        if (configFileContent[pos] == '\"')
          counter++;
        if (configFileContent[pos] == ',')
          break;
        if (counter == 2)
          throw std::runtime_error("expected ',' or '}' after key-value pair");
        pos++;
      }
    }
    else {
      while (configFileContent[pos]) {
        if (configFileContent[pos] == ',' || configFileContent[pos] == '}')
          break;
        pos++;
      }
    }
  }
  else {
    int opn = 0;
    int cls = 0;
    while (configFileContent[pos]) {
      if (configFileContent[pos] == '[')
        opn++;
      if (configFileContent[pos] == ']')
        cls++;
      if (opn == cls)
        break;
      pos++;
    }
    pos++;
  }

  std::string value = configFileContent.substr(valueStart, pos - valueStart);

  if (value.empty())
    throw std::runtime_error("value cannot be empty");

  storeKeyValue(key, value, s);

  if (configFileContent[pos] != ',' && configFileContent[pos] != '}')
    throw std::runtime_error("expected ',' or '}' after key-value pair");
  if (configFileContent[pos] == ',' && configFileContent[pos + 1] != '\"')
    throw std::runtime_error("expected new key after ','");
  if (configFileContent[pos] == ',')
    pos++;
  if (configFileContent[pos] == '}')
    return 1;

  return 0;
}

static int extractObject(std::string& configFileContent, std::size_t& pos) {
  if (configFileContent[pos] != '{')
    throw std::runtime_error("expected '{'");
  ctr& s = server.create();
  pos++;
  while (true) {
    if (extractKey(configFileContent, pos, s))
      break;
  }
  if (configFileContent[pos] != '}')
    throw std::runtime_error("expected '}'");
  pos++;
  if (configFileContent[pos] == ',')
    pos++;
  else if (configFileContent[pos] == ']')
    return 1;
  else
    throw std::runtime_error("expected ',' or ']' after object");
  return 0;
}

void json(std::string& configFileContent) {
  skipWhitespace(configFileContent);
  std::size_t pos = configFileContent.find("{\"servers\":[");
  if (pos == std::string::npos)
    throw std::runtime_error("missing servers array");
  else if (pos != 0)
    throw std::runtime_error("unexpected token before servers array");
  pos += 12;
  while (true) {
    if (extractObject(configFileContent, pos))
      break;
  }
  if (configFileContent[pos] != ']')
    throw std::runtime_error("expected ']' at the end of servers array");
  pos++;
  if (configFileContent[pos] != '}')
    throw std::runtime_error("unexpected end of file");
}