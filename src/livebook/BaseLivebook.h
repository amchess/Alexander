#ifndef BASE_LIVEBOOK_H
#define BASE_LIVEBOOK_H
#ifdef USE_LIVEBOOK

    #include <atomic>
    #include <chrono>
    #include <map>
    #include <string>
    #include <vector>
    #include "analysis/Analysis.h"

    #include "../position.h"

    #define CURL_STATICLIB
extern "C" {
    #include <curl/curl.h>
}
    #undef min
    #undef max

namespace Alexander::Livebook {
class BaseLivebook {
   public:
    BaseLivebook()                               = default;
    BaseLivebook(const BaseLivebook&)            = delete;
    BaseLivebook& operator=(const BaseLivebook&) = delete;

    // Virtual destructor for proper cleanup of derived classes
    virtual ~BaseLivebook() = default;

    // Pure virtual function to lookup analysis for a given UCI position
    virtual std::vector<std::pair<std::string, Analysis>> lookup(const Position& position) = 0;

    // Maximum duration of an HTTP request in milliseconds. The lookup is done
    // while our clock is running, so a request must never be allowed to hang.
    static void set_timeout(long timeout_ms_);
    static long get_timeout();

   protected:
    // CURL* curl = nullptr; // cURL handle
    std::string readBuffer;      // Buffer to store the response data
    long        httpStatus = 0;  // HTTP status of the last request (0 = no response)

    void clean_buffer_from_terminator();

    // Function to perform an HTTP request
    CURLcode do_request(const std::string& uri, const std::vector<std::string>& headers_ = {});

    // Perform the request and validate the answer. Returns true only if a usable
    // body was received. On failure an "info string" is printed and, depending on
    // the error, the live book is paused for a while (rate limit, server down) or
    // until it is reconfigured (credentials refused). If partial_ok_ is set, a
    // streamed answer cut by the timeout is accepted (see LichessPlayer).
    bool fetch(const std::string&              uri,
               const std::vector<std::string>& headers_    = {},
               bool                            partial_ok_ = false,
               const std::string&              auth_hint_  = "");

    static void report(const std::string& uri, const std::string& message);

   private:
    static inline std::atomic<long> timeoutMs{5000};

    std::chrono::steady_clock::time_point suspendedUntil{};

    [[nodiscard]] bool is_suspended() const;
    void               suspend_for(std::chrono::seconds duration_);
    void               suspend_until_reconfigured();
};
}

#endif
#endif  //BASE_LIVEBOOK_H
