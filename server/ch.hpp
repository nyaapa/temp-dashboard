#pragma once

#include <stdexcept>
#include <curl/curl.h>
#include <string>

namespace temp::ch {
  constexpr char CH_HOST_N_PORT[] = "localhost:8123";

  struct curl_global_guard {
    curl_global_guard() {
      curl_global_init(CURL_GLOBAL_ALL);    
    }

    ~curl_global_guard() {
      curl_global_cleanup();
    }
  };

  class curl_guard {
  public:
    curl_guard() {
      curl = curl_easy_init();
      if (!curl)
        throw std::runtime_error(std::string("Failed to initialize curl"));
    }

    ~curl_guard() {
      curl_easy_cleanup(curl);
    }

    CURL* operator->() {
      return curl;
    }

    operator CURL*() {
      return curl;
    } 
  private:
    CURL* curl;
  };

  void post(const char* query, long timeout) {
    curl_global_guard global;
    curl_guard curl;
    curl_easy_setopt(curl, CURLOPT_URL, CH_HOST_N_PORT);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, query);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeout);

    auto res = curl_easy_perform(curl);
    if(res != CURLE_OK)
      throw std::runtime_error(std::string("curl_easy_perform() failed: ") + curl_easy_strerror(res));
  }

  void post(const std::string& query, long timeout) {
    post(query.c_str(), timeout);
  }
};