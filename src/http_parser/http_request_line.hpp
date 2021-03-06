#include <unordered_map>
#include <string>

/** @file http_request_line.hpp
 * \ingroup http
 * @brief HttpRequestLine class declaration
 */

namespace http {
class HttpRequestLine {
  public:
    std::string action;
    std::string uri;
    std::unordered_map<std::string, std::string> parameters;
    std::string protocolVersion;

    HttpRequestLine(std::string action, std::string uri, std::string protocolVersion, std::unordered_map<std::string, std::string> parameters);
    HttpRequestLine(std::string action, std::string uri, std::string protocolVersion);

};
}

