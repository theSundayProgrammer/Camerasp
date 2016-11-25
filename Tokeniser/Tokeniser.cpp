// Tokeniser.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <algorithm>
#include <map>
#include <iostream>
#include <atomic>
std::map<std::string, std::string>
tokenize(std::string const& query)
{
  using namespace std;
  map<string, string> result;
  size_t offset = 0;
  size_t pos = query.find('=', offset);
  while (pos != string::npos)
  {
    string name = query.substr(offset, pos - offset);
    offset = pos + 1;
    pos = query.find('&', offset);
    if (pos != string::npos && pos > offset )
    {
      string val = query.substr(offset, pos - offset);
      result.insert(make_pair(name, val));
      offset = pos + 1;
    }
    else {
      string val = query.substr(offset);
      if (!val.empty())
        result.insert(make_pair(name, val));
      break;
    }
    pos = query.find('=', offset);
  }
  return result;
}
void test(std::string const& str)
{
  auto result = tokenize(str);
  for (auto iter : result)
  {
    std::cout << iter.first << ":" << iter.second << std::endl;
  }
  std::cout << "\n\n";
}
struct UrlParser {
  std::string command, query_key, query_value;
  UrlParser(std::string const& uri) {
    size_t pos = uri.find('/');
    size_t start = (pos == std::string::npos) ? 0 : pos + 1;
    pos = uri.find_first_of("?", start);
    auto fin = (pos == std::string::npos) ? uri.end() : uri.begin() + pos;
    command = std::string(uri.begin() + start, fin);
    if (fin != uri.end()) {
      pos = uri.find_first_of('&', ++pos);
      std::string query(++fin, (pos == std::string::npos) ? uri.end() : uri.begin() + pos);
      if (query.empty()) return;
      pos = query.find_first_of('=', 0);
      query_key = std::string(query.begin(), (pos == std::string::npos) ? query.end() : query.begin() + pos);
      if (pos != std::string::npos) {
        query_value = std::string(query.begin() + pos + 1, query.end());
      }
    }

  }
};
void testUrlParser(std::string const& url_)
{
  UrlParser url(url_);
  std::cout << url.command << "\n key: " << url.query_key << "\n value: " << url.query_value << std::endl;
}
void Test_itoa(int k) {
  char buffer[8];
  sprintf(buffer, "%04d", k);
  std::cout << std::string(buffer) << std::endl;
}

int main() {
  std::atomic<int> fileCount;
  int maxval = 10;
  int cmaxval = maxval;
  for (int i = 0; i < 25; ++i) {
    maxval = cmaxval;
    printf("%d, %d\n", ++fileCount,maxval);
    fileCount.compare_exchange_strong(maxval, 0);
  }
}

int main2() {
  for (int k = 0; k < 1000; k += 13)
    Test_itoa(k);
}
int main1()
{
  testUrlParser("/img?prev=1");
  testUrlParser("/img?prev=");
  testUrlParser("/img?prev");
  testUrlParser("/img?");
  testUrlParser("/img");
  testUrlParser("/");

  return 0;
}
int main0()
{
  test("h=15&w=67&th=89");
  test("h=15&w=67&th=");
  test("h=15w=67&th=89");
  return 0;
}

