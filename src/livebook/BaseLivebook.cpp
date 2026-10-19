#ifdef USE_LIVEBOOK
    #include "BaseLivebook.h"
    #include <algorithm>
    #include <iostream>
    #include "../misc.h"
using namespace Alexander::Livebook;

namespace {

// Identify the client: some services (e.g. Lichess) reject or throttle anonymous agents
constexpr const char* USER_AGENT = "Alexander-LiveBook (+https://github.com/amchess/Alexander)";

// How long a live book stays silent after a network error / server error / rate limit
constexpr std::chrono::seconds RETRY_DELAY{30};
constexpr std::chrono::seconds RATE_LIMIT_DELAY{60};  // Lichess: "wait one minute"

// Never print the query string (it contains the position)
std::string endpoint_of(const std::string& uri) { return uri.substr(0, uri.find('?')); }

}

static size_t curl_write(void* contents, const size_t size, const size_t nmemb, void* userp) {
    const size_t new_size = size * nmemb;
    const auto   buffer   = static_cast<std::string*>(userp);

    buffer->append(static_cast<char*>(contents), new_size);

    return new_size;
}

void BaseLivebook::set_timeout(const long timeout_ms_) { timeoutMs = std::max(timeout_ms_, 1L); }

long BaseLivebook::get_timeout() { return timeoutMs; }

void BaseLivebook::clean_buffer_from_terminator() {
    if (readBuffer.empty())
    {
        return;
    }

    if (const auto last_char = readBuffer.back();
        last_char == '\n' || last_char == '\r' || last_char == '\0')
    {
        readBuffer.pop_back();
    }
}

// Perform an HTTP request to the given URI and store the response in readBuffer
CURLcode BaseLivebook::do_request(const std::string&              uri,
                                  const std::vector<std::string>& headers_) {
    readBuffer.clear();
    httpStatus = 0;

    CURL* curl = curl_easy_init();
    if (!curl)
    {
        report(uri, "failed to initialize cURL");
        return CURLE_FAILED_INIT;
    }

    curl_slist* headers = nullptr;
    for (const auto& header : headers_)
    {
        headers = curl_slist_append(headers, header.c_str());
    }

    const long timeout = get_timeout();

    curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    // Hosts move (e.g. explorer.lichess.ovh -> explorer.lichess.org): follow the
    // redirects. cURL does not forward the Authorization header to another host.
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
    // Never block the engine for longer than the configured timeout. NOSIGNAL is
    // required for the timeouts to be thread safe.
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, timeout);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout);

    const CURLcode res = curl_easy_perform(curl);

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpStatus);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return res;
}

bool BaseLivebook::fetch(const std::string&              uri,
                         const std::vector<std::string>& headers_,
                         const bool                      partial_ok_,
                         const std::string&              auth_hint_) {
    if (is_suspended())
    {
        return false;
    }

    if (const CURLcode res = do_request(uri, headers_); res != CURLE_OK)
    {
        // A streamed answer cut by our timeout: what arrived so far is still usable
        if (partial_ok_ && res == CURLE_OPERATION_TIMEDOUT && httpStatus == 200)
        {
            return true;
        }

        report(uri, std::string("request failed: ") + curl_easy_strerror(res) + ". Paused for "
                      + std::to_string(RETRY_DELAY.count()) + "s");
        suspend_for(RETRY_DELAY);
        return false;
    }

    if (httpStatus >= 200 && httpStatus < 300)
    {
        return true;
    }

    const std::string status = "HTTP " + std::to_string(httpStatus);

    if (httpStatus == 401 || httpStatus == 403)
    {
        report(uri, "refused the request (" + status + ")" + auth_hint_
                      + ". Disabled until the LiveBook options are changed");
        suspend_until_reconfigured();
    }
    else if (httpStatus == 429)
    {
        report(uri, "rate limit reached (" + status + "). Paused for "
                      + std::to_string(RATE_LIMIT_DELAY.count()) + "s");
        suspend_for(RATE_LIMIT_DELAY);
    }
    else if (httpStatus >= 500)
    {
        report(uri, "server error (" + status + "). Paused for "
                      + std::to_string(RETRY_DELAY.count()) + "s");
        suspend_for(RETRY_DELAY);
    }
    else
    {
        report(uri, "unexpected answer (" + status + ")");
    }

    return false;
}

void BaseLivebook::report(const std::string& uri, const std::string& message) {
    sync_cout << "info string LiveBook " << endpoint_of(uri) << ": " << message << sync_endl;
}

bool BaseLivebook::is_suspended() const {
    return std::chrono::steady_clock::now() < suspendedUntil;
}

void BaseLivebook::suspend_for(const std::chrono::seconds duration_) {
    suspendedUntil = std::chrono::steady_clock::now() + duration_;
}

void BaseLivebook::suspend_until_reconfigured() {
    suspendedUntil = std::chrono::steady_clock::time_point::max();
}
#endif
