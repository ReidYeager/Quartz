
#include <quartz.h>
#include <stdio.h>

class SandboxApp : public Quartz::Application
{
public:
  SandboxApp() {}
  ~SandboxApp() {}

  virtual void Init() override {}
  virtual void Shutdown() override {}
};

int main()
{
  SandboxApp app;

  QTZ_LOG_INFO("Something about the app");

  return 0;
}
