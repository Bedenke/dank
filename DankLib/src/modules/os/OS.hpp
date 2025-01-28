#include "URI.hpp"

namespace dank {

class OS {
public:
  virtual void getDataFromURI(URI &uri, ResourceData &output) = 0;
};

// Must be defined on the host application
extern OS *os;
} // namespace dank