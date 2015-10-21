file(GLOB sources
	"*.h" "*.cpp"
)

set(formatted ${formatted} ${sources} PARENT_SCOPE)
add_library(siliciumc ${sources})
