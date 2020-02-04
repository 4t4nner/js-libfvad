#include <napi.h>

namespace test
{
std::string hello();
Napi::Object detectorWrapper(const Napi::CallbackInfo &info);
Napi::Object Init(Napi::Env env, Napi::Object exports);

struct detectorRet
{
    std::string error;
    std::string outFile;
    bool silence;
};
} // namespace test