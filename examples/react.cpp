#include <reactive/bridge.hpp>
#include <reactive/ptr_observable.hpp>
#include <reactive/buffer.hpp>
#include <reactive/generate.hpp>
#include <reactive/tuple.hpp>
#include <reactive/transform.hpp>
#include <reactive/while.hpp>
#include <SDL2/SDL.h>
#include <boost/system/system_error.hpp>
#include <boost/scope_exit.hpp>
#include <boost/variant.hpp>
#include <boost/asio.hpp>

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
			if (has_ended)
			{
				receiver.ended();
			}
			else
			{
				check_fetch();
				receiver.got_element(state);
			}
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
		}

	private:

		In in;
		element_type state;
		Step step;
		bool fetching = false;
		bool has_ended = false;

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
			assert(!has_ended);
			has_ended = true;
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

	template <class Input, class Predicate>
	struct filter_observable
			: public observable<typename Input::element_type>
			, private observer<typename Input::element_type>
	{
		typedef typename Input::element_type element_type;

		filter_observable(Input input, Predicate is_propagated)
			: input(std::move(input))
			, is_propagated(std::move(is_propagated))
		{
		}

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			assert(!receiver_);
			receiver_ = &receiver;
			input.async_get_one(*this);
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			assert(receiver_);
		}

	private:

		Input input;
		Predicate is_propagated;
		observer<element_type> *receiver_ = nullptr;

		virtual void got_element(element_type value) SILICIUM_OVERRIDE
		{
			assert(receiver_);
			if (!is_propagated(static_cast<typename std::add_const<element_type>::type>(value)))
			{
				input.async_get_one(*this);
				return;
			}
			exchange(receiver_, nullptr)->got_element(std::move(value));
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
			assert(receiver_);
			exchange(receiver_, nullptr)->ended();
		}
	};

	template <class Input, class Predicate>
	auto filter(Input &&input, Predicate &&is_propagated)
	{
		return filter_observable<
				typename std::decay<Input>::type,
				typename std::decay<Predicate>::type>(std::forward<Input>(input), std::forward<Predicate>(is_propagated));
	}

	struct timer_elapsed
	{
	};

	struct timer : observable<timer_elapsed>
	{
		typedef timer_elapsed element_type;

		explicit timer(boost::asio::io_service &io, boost::posix_time::time_duration delay)
			: impl(io)
			, delay(delay)
		{
		}

		virtual void async_get_one(observer<element_type> &receiver) SILICIUM_OVERRIDE
		{
			assert(!receiver_);
			receiver_ = &receiver;
			impl.expires_from_now(delay);
			impl.async_wait([this](boost::system::error_code error)
			{
				if (error)
				{
					assert(error == boost::asio::error::operation_aborted); //TODO: remove this assumption
					assert(!receiver_); //cancel() should have reset the receiver already
					return;
				}
				exchange(receiver_, nullptr)->got_element(timer_elapsed{});
			});
		}

		virtual void cancel() SILICIUM_OVERRIDE
		{
			assert(receiver_);
			impl.cancel();
			receiver_ = nullptr;
		}

	private:

		boost::asio::deadline_timer impl;
		boost::posix_time::time_duration delay;
		observer<element_type> *receiver_ = nullptr;
	};
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
				}
				break;
			}
		}
		return previous;
	}

	template <class Events>
	auto make_frames(Events &&input)
	{
		game_state initial_state;
		auto interesting_input = rx::filter(std::forward<Events>(input), [](SDL_Event const &event_)
		{
			return event_.type == SDL_KEYUP;
		});
		return rx::transform(rx::make_var(interesting_input, initial_state, step_game_state), draw_game_state);
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

	struct frame_renderer : rx::observer<frame>
	{
		explicit frame_renderer(SDL_Renderer &sdl)
			: sdl(&sdl)
		{
		}

		virtual void got_element(frame value) SILICIUM_OVERRIDE
		{
			render_frame(*sdl, value);
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
		}

	private:

		SDL_Renderer *sdl;
	};

	struct quitting
	{
	};

	struct frame_rendered
	{
	};

	template <class Input>
	struct total_consumer : private rx::observer<typename Input::element_type>
	{
		typedef typename Input::element_type element_type;

		explicit total_consumer(Input input)
			: input(std::move(input))
		{
		}

		void start()
		{
			input.async_get_one(*this);
		}

	private:

		Input input;

		virtual void got_element(element_type value) SILICIUM_OVERRIDE
		{
			(void)value; //ignore result
			return start();
		}

		virtual void ended() SILICIUM_OVERRIDE
		{
		}
	};

	template <class Input>
	auto make_total_consumer(Input &&input)
	{
		return total_consumer<typename std::decay<Input>::type>(std::forward<Input>(input));
	}
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

	rx::bridge<SDL_Event> frame_events;
	auto frames = rx::wrap<frame>(make_frames(while_(rx::ref(frame_events), [](SDL_Event const &event_)
	{
		bool continue_ = event_.type != SDL_QUIT;
		return continue_;
	})));

	boost::asio::io_service io;

	frame_renderer frame_renderer_(*renderer);
	rx::timer frame_rate_limiter(io, boost::posix_time::milliseconds(16));

	auto all_rendered = make_total_consumer(rx::transform(rx::make_tuple(rx::ref(frame_rate_limiter), frames), [&renderer, &frame_events](std::tuple<rx::timer_elapsed, frame> const &ready_frame)
	{
		SDL_Event event;
		while (frame_events.is_waiting() &&
			   SDL_PollEvent(&event))
		{
			frame_events.got_element(event);
		}

		render_frame(*renderer, std::get<1>(ready_frame));
		return frame_rendered{};
	}));
	all_rendered.start();

	io.run();
}
