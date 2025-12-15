#include <server.hpp>
#include <iostream>
#include <cstdlib>

static void skipWhitespace(std::string &value)
{
  std::size_t index = 0;
  while (value[index])
  {
    if (value[index] == ' ' || value[index] == '\n' || value[index] == '\r' ||
        value[index] == '\t' || value[index] == '\v' || value[index] == '\f')
      value.erase(index, 1);
    else
      index++;
  }
}

static int duplicateKey(std::vector<std::string> &temp, const std::string &key)
{
  for (std::size_t i = 0; i < temp.size(); i++)
  {
    if (temp[i] == key)
      throw std::runtime_error("duplicate key: " + key);
  }
  temp.push_back(key);
  return 0;
}

static int isDigit(std::string const &value)
{
  std::size_t z = 0;
  if (value[z] == '-')
    z++; // json don't accept '+'
  for (std::size_t i = z; i < value.length(); i++)
  {
    if (!std::isdigit(value[i]))
      return 1;
  }
  return 0;
}

static int isString(std::string const &value)
{
    if (value.length() < 2)
    return 1;  // Too short to be a valid string
  
  if (value[0] != '\"')
    return 1;  // Doesn't start with "
  
  if (value[value.length() - 1] != '\"')
    return 1;  // Doesn't end with "
  
  // Valid string format
  return 0;
}

// -----------------------------------------------------------------------
static void storeRouteKeyValue(std::string const &key, std::string const &value, rt &route)
{
  static std::vector<std::string> temp;
  duplicateKey(temp, key);
  if (key == "path")
  {
    if (isString(value))
      throw std::runtime_error("path value must be a string");
    route.path() = value.substr(1, value.length() - 2);
  }
  else if (key == "source")
  {
    if (isString(value))
      throw std::runtime_error("source value must be a string");
    route.source() = value.substr(1, value.length() - 2);
  }
  else if (key == "method")
  {

    if (value[0] != '[')
      throw std::runtime_error("method value must be an array");

    std::size_t pos = 1; // Skip opening [

    // Check for empty array
    if (value[pos] == ']')
    {
      throw std::runtime_error("method array cannot be empty");
    }

    while (pos < value.length() && value[pos] != ']')
    {
      if (value[pos] != '"')
        throw std::runtime_error("method array must contain strings");

      pos++; // Skip opening "
      std::size_t methodStart = pos;

      // Find closing "
      while (pos < value.length() && value[pos] != '"')
      {
        pos++;
      }

      if (pos >= value.length())
        throw std::runtime_error("unterminated string in method array");

      // Extract method
      std::string method = value.substr(methodStart, pos - methodStart);

      // Validate HTTP method
      if (method != "GET" && method != "POST" && method != "PUT" &&
          method != "DELETE" && method != "PATCH" && method != "HEAD" &&
          method != "OPTIONS")
      {
        throw std::runtime_error("invalid HTTP method: " + method);
      }

      route.add(method);
      pos++; // Skip closing "

      // Check for comma or end of array
      if (value[pos] == ',')
      {
        pos++; // Skip comma
      }
      else if (value[pos] == ']')
      {
        break; // End of array
      }
      else
      {
        throw std::runtime_error("expected ',' or ']' in method array");
      }
    }

    if (value[pos] != ']')
      throw std::runtime_error("expected ']' at end of method array");
  }
  else
    throw std::runtime_error("unknown key: " + key);
}

static int extractRouteKey(std::string &configFileContent, std::size_t &pos, rt &route)
{
  if (configFileContent[pos] != '\"')
    throw std::runtime_error("key must initiate with '\"'");

  pos++;
  std::size_t keyStart = pos;
  while (true)
  {
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
  if (configFileContent[pos] != '[')
  {
    if (configFileContent[pos] == '-' || std::isdigit(configFileContent[pos]))
    {
      while (configFileContent[pos])
      {
        if (configFileContent[pos] == ',' || configFileContent[pos] == '}' || configFileContent[pos] == '\"')
          break;
        pos++;
      }
    }
    else if (configFileContent[pos] == '\"')
    {
      int counter = -1;
      while (configFileContent[pos])
      {
        if (configFileContent[pos] == '\"')
          counter++;
        if (configFileContent[pos] == ',' || configFileContent[pos] == ']' || configFileContent[pos] == '}')
          break;
        if (counter == 2)
          throw std::runtime_error("expected ',' or '}' after key-value pair");
        pos++;
      }
    }
    else
    {
      while (configFileContent[pos])
      {
        if (configFileContent[pos] == ',' || configFileContent[pos] == '}')
          break;
        pos++;
      }
    }
  }
  else
  {
    int opn = 0;
    int cls = 0;
    while (configFileContent[pos])
    {
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

  std::string valueByResult = configFileContent.substr(valueStart, pos - valueStart);

  if (valueByResult.empty())
    throw std::runtime_error("value cannot be empty");

  storeRouteKeyValue(key, valueByResult, route);

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
// -----------------------------------------------------------------------

static int extractRoutes(std::string &value, std::size_t &pos, ctr &s)
{
  if (value[pos] != '{')
    throw std::runtime_error("expected '{'");
  rt &route = s.create();
  pos++;
  while (true)
  {
    if (extractRouteKey(value, pos, route))
      break;
  }
  if (value[pos] != '}')
    throw std::runtime_error("expected '}'");
  pos++;
  if (value[pos] == ',')
    pos++;
  else if (value[pos] == ']')
    return 1;
  else
    throw std::runtime_error("expected ',' or ']' after object");
  return 0;
}

static void storeKeyValue(std::string &key, std::string &value, ctr &s)
{
  static std::vector<std::string> temp;
  duplicateKey(temp, key);
  if (key == "port")
  {
    if (isDigit(value))
      throw std::runtime_error("port value must be a number");
    int port = std::atoi(value.c_str());
    if (port < 1024 || port > 65535)
      throw std::runtime_error("port must be between 1024 and 65535");
    s.port() = static_cast<std::size_t>(port);
  }
  else if (key == "name")
  {
    std::cout << "hello: " << value << std::endl;
    if (isString(value))
      throw std::runtime_error("name value must be a string");
    s.name() = value.substr(1, value.length() - 2);
  }
  else if (key == "version")
  {
    if (isString(value))
      throw std::runtime_error("version value must be a string");
    s.version() = value.substr(1, value.length() - 2);
  }
  else if (key == "notfound")
  {
    if (isString(value))
      throw std::runtime_error("notfound value must be a string");
    s.notfound() = value.substr(1, value.length() - 2);
  }
  else if (key == "servererror")
  {
    if (isString(value))
      throw std::runtime_error("servererror value must be a string");
    s.servererror() = value.substr(1, value.length() - 2);
  }
  else if (key == "log")
  {
    if (isString(value))
      throw std::runtime_error("log value must be a string");
    s.log() = value.substr(1, value.length() - 2);
  }
  else if (key == "bodylimit")
  {
    if (isDigit(value))
      throw std::runtime_error("bodylimit value must be a number");
    int bodylimit = std::atoi(value.c_str());
    if (bodylimit < 0)
      throw std::runtime_error("bodylimit must be a non-negative number");
    s.bodylimit() = static_cast<std::size_t>(bodylimit);
  }
  else if (key == "timeout")
  {
    if (isDigit(value))
      throw std::runtime_error("timeout value must be a number");
    int timeout = std::atoi(value.c_str());
    if (timeout < 0)
      throw std::runtime_error("timeout must be a non-negative number");
    s.timeout() = static_cast<std::size_t>(timeout);
  }
  else if (key == "uploaddir")
  {
    if (isString(value))
      throw std::runtime_error("uploaddir value must be a string");
    s.uploaddir() = value.substr(1, value.length() - 2);
  }
  else if (key == "index")
  {
    if (isString(value))
      throw std::runtime_error("index value must be a string");
    s.index() = value.substr(1, value.length() - 2);
  }
  else if (key == "root")
  {
    std::cout << "hello: " << value << std::endl;
    if (isString(value))
      throw std::runtime_error("root value must be a string");
    s.root() = value.substr(1, value.length() - 2);
  }
  else if (key == "routes")
  {
    std::size_t pos = value.find("[");
    if (pos == std::string::npos || pos != 0)
      throw std::runtime_error("invalid routes format");
    pos++;
    while (true)
    {
      if (extractRoutes(value, pos, s))
        break;
    }
    if (value[pos] != ']')
      throw std::runtime_error("expected ']' at the end of servers array");
  }
  else
    throw std::runtime_error("unknown key: " + key);
}

static int extractKey(std::string const &configFileContent, std::size_t &pos, ctr &s)
{
  if (configFileContent[pos] != '\"')
    throw std::runtime_error("key must initiate with '\"'");

  pos++;
  std::size_t keyStart = pos;
  while (true)
  {
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
  if (configFileContent[pos] != '[')
  {
    if (configFileContent[pos] == '-' || std::isdigit(configFileContent[pos]))
    {
      while (configFileContent[pos])
      {
        if (configFileContent[pos] == ',' || configFileContent[pos] == '}' || configFileContent[pos] == '\"')
          break;
        pos++;
      }
    }
    else if (configFileContent[pos] == '\"')
    {
      int counter = -1;
      while (configFileContent[pos])
      {
        if (configFileContent[pos] == '\"')
          counter++;
        if (configFileContent[pos] == ',' || configFileContent[pos] == ']' || configFileContent[pos] == '}')
          break;
        if (counter == 2)
          throw std::runtime_error("expected ',' or '}' after key-value pair");
        pos++;
      }
    }
    else
    {
      while (configFileContent[pos])
      {
        if (configFileContent[pos] == ',' || configFileContent[pos] == '}')
          break;
        pos++;
      }
    }
  }
  else
  {
    int opn = 0;
    int cls = 0;
    while (configFileContent[pos])
    {
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

static int extractObject(std::string const &configFileContent, std::size_t &pos)
{
  if (configFileContent[pos] != '{')
    throw std::runtime_error("expected '{'");
  ctr &s = server.create();
  pos++;
  while (true)
  {
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

void json(std::string &configFileContent)
{
  skipWhitespace(configFileContent);
  std::size_t pos = configFileContent.find("{\"servers\":[");
  if (pos == std::string::npos)
    throw std::runtime_error("missing servers array");
  else if (pos != 0)
    throw std::runtime_error("unexpected token before servers array");
  pos += 12;
  while (true)
  {
    if (extractObject(configFileContent, pos))
      break;
  }
  if (configFileContent[pos] != ']')
    throw std::runtime_error("expected ']' at the end of servers array");
  pos++;
  if (configFileContent[pos] != '}')
    throw std::runtime_error("unexpected end of file");
}