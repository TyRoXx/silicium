#include <cdm_description/all.hpp>

extern "C"

#ifdef _WIN32
__declspec(dllexport)
#endif

bool cdm_describe(cdm::description *result)
{
	result->name = "websocketpp";
	return true;
}
