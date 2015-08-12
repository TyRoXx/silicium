#include <cdm_description/all.hpp>

extern "C"

#ifdef _WIN32
__declspec(dllexport)
#else
__attribute__((visibility("default")))
#endif

bool cdm_describe(cdm::description *result)
{
	result->name = "websocketpp";
	return true;
}
