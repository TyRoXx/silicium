#include <reactive/bridge.hpp>
#include <reactive/ptr_observable.hpp>
#include <reactive/buffer.hpp>
#include <reactive/generate.hpp>
#include <reactive/transform.hpp>
#include <SDL2/SDL.h>
#include <boost/system/system_error.hpp>
#include <boost/scope_exit.hpp>
#include <boost/variant.hpp>

namespace rx
{
	template <class In, class State, class Step>
	struct variable
			: public observable<State>
			, private observer<typename In::element_type>
	{
		typedef State element_type;

		variable(In in, element_type initial_state, Step step)
			: in(std::move(in))
			, state(std::move(initial_state))
			, step(std::move(step))
		{
		}

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			check_fetch();
			receiver.got_element(state);
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
		}

	private:

		In in;
		element_type state;
		Step step;
		bool fetching = false;

		virtual void got_element(typename In::element_type value) SILICIUM_OVERRIDE
		{
			assert(fetching);
			fetching = false;
			check_fetch();
			state = step(std::move(state), std::move(value));
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			assert(fetching);
			fetching = false;
			check_fetch();
		}

		void check_fetch()
		{
			if (!fetching)
			{
				fetching = true;
				in.async_get_one(*this);
			}
		}
	};

	template <class In, class State, class Step>
	auto make_var(In &&in, State &&initial_state, Step &&step)
	{
		return variable<
				typename std::decay<In>::type,
				typename std::decay<State>::type,
				typename std::decay<Step>::type>(std::forward<In>(in), std::forward<State>(initial_state), std::forward<Step>(step));
	}

	template <class Element>
	auto ref(rx::observable<Element> &identity)
	{
		return rx::ptr_observable<Element, rx::observable<Element> *>(&identity);
	}
}

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

	struct game_state
	{
		int x = 100;
	};

	frame draw_game_state(game_state const &state)
	{
		return frame
		{
			{
				draw_filled_rect{make_color(0xff, 0x00, 0x00, 0xff), make_rect(state.x, 100, 200, 125)}
			}
		};
	}

	game_state step_game_state(game_state previous, SDL_Event event_)
	{
		switch (event_.type)
		{
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
				}
				break;
			}
		}
		return previous;
	}

	auto make_frames(rx::observable<SDL_Event> &input)
	{
		game_state initial_state;
		return rx::transform(rx::make_var(rx::ref(input), initial_state, step_game_state), draw_game_state);
	}

	void set_render_draw_color(SDL_Renderer &renderer, SDL_Color color)
	{
		SDL_SetRenderDrawColor(&renderer, color.r, color.g, color.b, color.a);
	}

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

	struct frame_renderer : rx::observer<frame>
	{
		explicit frame_renderer(SDL_Renderer &sdl)
			: sdl(&sdl)
		{
		}

		virtual void got_element(frame value) SILICIUM_OVERRIDE
		{
			set_render_draw_color(*sdl, make_color(0, 0, 0, 0xff));
			SDL_RenderClear(sdl);
			for (auto const &operation : value.operations)
			{
				boost::apply_visitor(draw_operation_renderer{*sdl}, operation);
			}
			SDL_RenderPresent(sdl);
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
		}

	private:

		SDL_Renderer *sdl;
	};
}

int main()
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

	rx::bridge<SDL_Event> input;
	auto frames = rx::wrap<frame>(make_frames(input));

	frame_renderer frame_renderer_(*renderer);
	for (;;)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				return 0;
			}
			if (input.is_waiting())
			{
				input.got_element(event);
			}
		}
		frames.async_get_one(frame_renderer_);
	}
}
