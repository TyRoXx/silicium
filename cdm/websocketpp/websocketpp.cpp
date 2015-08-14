#include <cdm_description/all.hpp>
#include <silicium/expected.hpp>

extern "C"
#ifdef _WIN32
__declspec(dllexport)
#else
__attribute__((visibility("default")))
#endif
void cdm_describe(Si::expected<cdm::description> *result)
{
	cdm::description description;
	description.name = "websocketpp";
	*result = std::move(description);
}
