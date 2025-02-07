#include "modules/os/OS.hpp"

namespace dank {

class WindowsOS : public dank::OS {
public:
  void getCaptureSharableContent(CaptureSharableContent &output) override {}

  void setCaptureConfig(CaptureConfig &config) override {}

  void getDataFromURI(URI &uri, ResourceData &output) override {}
};

} // namespace dank

dank::OS *dank::os = new WindowsOS();
