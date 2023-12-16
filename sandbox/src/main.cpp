
#include <quartz.h>
#include <stdio.h>

class SandboxApp : public Quartz::Application
{
public:
  SandboxApp() {}
  ~SandboxApp() {}

  virtual QuartzResult Init() override { return Quartz_Success; }
  virtual QuartzResult Update() override { return Quartz_Success; }
  virtual void Shutdown() override {}
};

int main()
{
  SandboxApp app;

  QTZ_LOG_INFO("Something about the app");

  return 0;
}
