#include "URI.hpp"
#include "modules/os/Capture.hpp"

namespace dank {

class OS {
public:
  virtual void getDataFromURI(URI &uri, ResourceData &output) = 0;
  virtual void getCaptureSharableContent(CaptureSharableContent &output) = 0;
  virtual void setCaptureConfig(CaptureConfig &config) = 0;
};

// Must be defined on the host application
extern OS *os;
} // namespace dank