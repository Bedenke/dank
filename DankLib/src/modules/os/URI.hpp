#pragma once

#include <cstddef>
#include <string>

namespace dank {
struct URI {
  std::string protocol;
  std::string host;
  std::string path;
  std::string port;
  std::string query;

  URI() {}

  URI(std::string protocol, std::string path)
      : protocol(protocol), path(path) {}

  URI(const std::string uri) {
    typedef std::string::const_iterator iterator_t;

    if (uri.length() == 0)
      return;

    iterator_t uriEnd = uri.end();

    // get query start
    iterator_t queryStart = std::find(uri.begin(), uriEnd, '?');

    // protocol
    iterator_t protocolStart = uri.begin();
    iterator_t protocolEnd = std::find(protocolStart, uriEnd, ':'); //"://");

    if (protocolEnd != uriEnd) {
      std::string prot = &*(protocolEnd);
      if ((prot.length() > 3) && (prot.substr(0, 3) == "://")) {
        this->protocol = std::string(protocolStart, protocolEnd);
        protocolEnd += 3; //      ://
      } else
        protocolEnd = uri.begin(); // no protocol
    } else
      protocolEnd = uri.begin(); // no protocol

    // host
    iterator_t hostStart = protocolEnd;
    iterator_t pathStart = std::find(hostStart, uriEnd, '/'); // get pathStart

    iterator_t hostEnd =
        std::find(protocolEnd, (pathStart != uriEnd) ? pathStart : queryStart,
                  ':'); // check for port

    this->host = std::string(hostStart, hostEnd);

    // port
    if ((hostEnd != uriEnd) && ((&*(hostEnd))[0] == ':')) // we have a port
    {
      hostEnd++;
      iterator_t portEnd = (pathStart != uriEnd) ? pathStart : queryStart;
      this->port = std::string(hostEnd, portEnd);
    }

    // path
    if (pathStart != uriEnd)
      this->path = std::string(pathStart, queryStart);

    // query
    if (queryStart != uriEnd)
      this->query = std::string(queryStart, uri.end());
  }

  inline URI operator=(URI a) {
    protocol = a.protocol;
    host = a.host;
    port = a.port;
    path = a.path;
    query = a.query;
    return a;
  }
};

struct ResourceData {
  size_t size;
  void *data;
};

} // namespace dank