
#include "stdafx.h"
#include <string>
#include <iostream>
#include <map>
#include <regex>
struct OldUrlParser {
  std::string command;
  std::map<std::string, std::string> queries;
  OldUrlParser(std::string const& uri) {
    size_t pos = uri.find('/');
    size_t start = (pos == std::string::npos) ? 0 : pos + 1;
    pos = uri.find_first_of("?", start);
    auto fin = (pos == std::string::npos) ? uri.end() : uri.begin() + pos;
    command = std::string(uri.begin() + start, fin);
    while (fin != uri.end()) {
      pos = uri.find_first_of('&', ++pos);
      auto next = (pos == std::string::npos) ? uri.end() : uri.begin() + pos;
      std::string query(++fin, next);
      fin = next;

      if (query.empty()) return;
      auto pos2 = query.find_first_of('=', 0);
      std::string query_key = std::string(query.begin(), (pos2 == std::string::npos) ? query.end() : query.begin() + pos2);
      std::string query_value;
      if (pos2 != std::string::npos) {
        query_value = std::string(query.begin() + pos2 + 1, query.end());

      }
      queries[query_key] = query_value;
    }
  }
};
/*
int main(int argc, char *argv[])
{
  UrlParser url("path?hue=br&width=320");
  std::cout << url.command;  
  for (auto& keyval : url.queries)  {
    std::cout << keyval.first << "=" << keyval.second << std::endl;
  }
 
  return EXIT_SUCCESS;
}*/
struct UrlParser {
  std::string command;
  std::map<std::string, std::string> queries;
  UrlParser(std::string const& s) {
    std::string uri = s;
    auto pos = uri.find('#');
    if (pos != std::string::npos) {
      uri = uri.erase(pos);
    }
    std::smatch m;
    std::regex e("([^?]*)(\\??)");
    if (std::regex_search(uri, m, e)) {
      command = m[1];
      uri = m.suffix().str();
      std::regex e2("([^&]+)(&?)");
      while (!uri.empty() && std::regex_search(uri, m, e2)) {
        std::regex e3("([^=]*)(=)");
        std::string keyval = m[1];
        std::smatch m3;
        if (std::regex_search(keyval, m3, e3)) {
          std::string key = m3[1];
          std::string val = m3.suffix().str();
          queries[key] = val;
        }
        uri = m.suffix().str();
      }
    }    else {
      command = uri;
    }
  }
};

int main()
{
  UrlParser uri("path?height=240&width=320#index");
  std::cout << "command=" << uri.command << std::endl;
  for (auto& kv : uri.queries)
    std::cout << kv.first << "=" << kv.second << std::endl;
  return 0;
}