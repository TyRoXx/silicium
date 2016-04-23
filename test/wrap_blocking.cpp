#include <silicium/observable/coroutine_generator.hpp>
#include <silicium/observable/consume.hpp>
#include <algorithm>
#include <boost/test/unit_test.hpp>
#include <boost/thread/future.hpp>
#include <boost/asio/io_service.hpp>

#ifndef BOOST_NO_EXCEPTIONS
namespace Si
{
    template <class Observable>
    struct has_blocking_get_one : std::false_type
    {
    };

    template <class Action>
    struct blocking_observable
    {
        typedef decltype(std::declval<Action>()()) element_type;

        explicit blocking_observable(Action act)
            : act(std::move(act))
        {
        }

#if !SILICIUM_COMPILER_GENERATES_MOVES
        blocking_observable(blocking_observable &&other)
            : act(std::move(other.act))
            , done(std::move(other.done))
        {
        }

        blocking_observable &operator=(blocking_observable &&other)
        {
            act = std::move(other.act);
            done = std::move(other.done);
            return *this;
        }
#endif

        void async_get_one(ptr_observer<observer<element_type>> receiver)
        {
#if BOOST_VERSION >= 105200
            using boost::async;
            using boost::launch;
#else
            using std::async;
            using std::launch;
#endif
            done = async(launch::async, [this, receiver]()
                         {
                             element_type result = act();
                             receiver.got_element(std::move(result));
                         });
        }

        element_type get_one()
        {
            return act();
        }

    private:
        Action act;
#if BOOST_VERSION >= 105200
        boost::unique_future<void>
#else
        std::future<void>
#endif
            done;
    };

    template <class Action>
    struct has_blocking_get_one<blocking_observable<Action>> : std::true_type
    {
    };

    template <class Action>
    auto wrap_blocking(Action &&act)
        -> blocking_observable<typename std::decay<Action>::type>
    {
        return blocking_observable<typename std::decay<Action>::type>(act);
    }
}
#endif

#if SILICIUM_HAS_COROUTINE_GENERATOR
namespace
{
    int blocking_stuff()
    {
        return 2;
    }
}

BOOST_AUTO_TEST_CASE(wrap_blocking_coroutine)
{
    auto coro = Si::make_coroutine_generator<int>(
        [](Si::push_context<int> &yield)
        {
            auto blocking = Si::wrap_blocking(blocking_stuff);
            auto const intermediate_result = yield.get_one(blocking);
            BOOST_REQUIRE(intermediate_result);
            yield(*intermediate_result + 3);
        });
    boost::asio::io_service io;
    std::unique_ptr<boost::asio::io_service::work> work(
        new boost::asio::io_service::work(io));
    auto consumer = Si::consume<int>([&work](int element)
                                     {
                                         BOOST_CHECK_EQUAL(5, element);
                                         work.reset();
                                     });
    coro.async_get_one(Si::observe_by_ref(consumer));
    io.run();
}
#endif
