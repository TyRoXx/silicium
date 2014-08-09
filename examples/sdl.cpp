#include <reactive/bridge.hpp>
#include <reactive/ref.hpp>
#include <reactive/transform.hpp>
#include <reactive/while.hpp>
#include <reactive/finite_state_machine.hpp>
#include <reactive/filter.hpp>
#include <reactive/generate.hpp>
#include <reactive/ptr_observable.hpp>
#include <reactive/cache.hpp>
#include <reactive/get.hpp>
#include <boost/scope_exit.hpp>
#include <boost/variant.hpp>
#include <vector>

#ifdef _MSC_VER
#	include <SDL.h>
#else
#	include <SDL2/SDL.h>
#endif

namespace
{
	void throw_error()
	{
		throw std::runtime_error(std::string("SDL error :") + SDL_GetError());
	}

	void check_sdl(int rc)
	{
		if (rc < 0)
		{
			throw_error();
		}
	}

	struct window_destructor
	{
		void operator()(SDL_Window *w) const
		{
			SDL_DestroyWindow(w);
		}
	};

	struct renderer_destructor
	{
		void operator()(SDL_Renderer *r) const
		{
			SDL_DestroyRenderer(r);
		}
	};

	void set_render_draw_color(SDL_Renderer &renderer, SDL_Color color)
	{
		SDL_SetRenderDrawColor(&renderer, color.r, color.g, color.b, color.a);
	}

	SDL_Color make_color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
	{
		SDL_Color result;
		result.r = red;
		result.g = green;
		result.b = blue;
		result.a = alpha;
		return result;
	}

	SDL_Rect make_rect(int x, int y, int w, int h)
	{
		SDL_Rect result;
		result.x = x;
		result.y = y;
		result.w = w;
		result.h = h;
		return result;
	}

	struct draw_filled_rect
	{
		SDL_Color color;
		SDL_Rect where;
	};

	typedef boost::variant<
		draw_filled_rect
	> draw_operation;

	struct frame
	{
		std::vector<draw_operation> operations;
	};

	struct draw_operation_renderer : boost::static_visitor<>
	{
		explicit draw_operation_renderer(SDL_Renderer &sdl)
			: sdl(&sdl)
		{
		}

		void operator()(draw_filled_rect const &operation) const
		{
			set_render_draw_color(*sdl, operation.color);
			SDL_RenderDrawRect(sdl, &operation.where);
		}

	private:

		SDL_Renderer *sdl;
	};

	void render_frame(SDL_Renderer &sdl, frame const &frame_)
	{
		set_render_draw_color(sdl, make_color(0, 0, 0, 0xff));
		SDL_RenderClear(&sdl);
		for (auto const &operation : frame_.operations)
		{
			boost::apply_visitor(draw_operation_renderer{sdl}, operation);
		}
		SDL_RenderPresent(&sdl);
	}

	struct game_state
	{
		int x = 100;
		int y = 100;
		int count = 1;
	};

	int const max_rect_count = 5;

	frame draw_game_state(game_state const &state)
	{
		std::vector<draw_operation> operations;
		for (int i = 0; i < state.count; ++i)
		{
			int const max_width = 80;
			int const single_offset = (max_width / 2) / max_rect_count;
			int const total_offset = single_offset * i;
			int const width = max_width - total_offset * 2;
			operations.emplace_back(draw_filled_rect{make_color(0xff, 0x00, 0x00, 0xff), make_rect(state.x + total_offset, state.y + total_offset, width, width)});
		}
		return frame
		{
			std::move(operations)
		};
	}

	boost::optional<game_state> step_game_state(game_state previous, SDL_Event event_)
	{
		switch (event_.type)
		{
		case SDL_QUIT:
			return boost::none;

		case SDL_KEYUP:
			{
				switch (event_.key.keysym.sym)
				{
				case SDLK_LEFT:
					previous.x -= 10;
					break;

				case SDLK_RIGHT:
					previous.x += 10;
					break;

				case SDLK_UP:
					previous.y -= 10;
					break;

				case SDLK_DOWN:
					previous.y += 10;
					break;

				case SDLK_PLUS:
					previous.count = std::min(max_rect_count, previous.count + 1);
					break;

				case SDLK_MINUS:
					previous.count = std::max(1, previous.count - 1);
					break;

				case SDLK_ESCAPE:
					return boost::none;
				}
				break;
			}
		}
		return previous;
	}

	template <class Events>
	auto make_frames(Events &&input)
#ifdef _MSC_VER
		-> rx::unique_observable<frame>
#endif
	{
		game_state initial_state;
		auto model = rx::make_finite_state_machine(std::forward<Events>(input), initial_state, step_game_state);
		return
#ifdef _MSC_VER
			rx::box<frame>
#endif
			(rx::transform(std::move(model), draw_game_state));
	}
}

int main(int, char* []) //SDL2 requires the parameters on Windows
{
	check_sdl(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS));
	BOOST_SCOPE_EXIT(void)
	{
		SDL_Quit();
	}
	BOOST_SCOPE_EXIT_END;

	std::unique_ptr<SDL_Window, window_destructor> window(SDL_CreateWindow("Silicium react.cpp", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, 0));
	if (!window)
	{
		throw_error();
		return 1;
	}

	std::unique_ptr<SDL_Renderer, renderer_destructor> renderer(SDL_CreateRenderer(window.get(), -1, 0));

	rx::bridge<SDL_Event> frame_events;
	auto frames = rx::cache(make_frames(rx::ref(frame_events)), frame());

	for (;;)
	{
		SDL_Event event{};
		while (frame_events.is_waiting() && SDL_PollEvent(&event))
		{
			frame_events.got_element(event);
		}

		auto f = rx::get(frames);
		if (!f)
		{
			break;
		}

		render_frame(*renderer, *f);

		SDL_Delay(16);
	}
	return 0;
}
