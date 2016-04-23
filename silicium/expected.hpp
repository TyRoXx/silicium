#ifndef SILICIUM_EXPECTED_HPP
#define SILICIUM_EXPECTED_HPP

#include <silicium/variant.hpp> //for inplace
#include <boost/type_traits/alignment_of.hpp>
#include <type_traits>

#define SILICIUM_HAS_EXPECTED SILICIUM_HAS_EXCEPTIONS

#if SILICIUM_HAS_EXPECTED
#include <boost/exception_ptr.hpp>
namespace Si
{
    template <class T, class ExceptionPtr = boost::exception_ptr>
    struct expected
    {
        typedef T value_type;
        typedef ExceptionPtr exception_ptr;

        expected()
            : m_state(has_value)
        {
            new (&value_address()) value_type();
        }

        expected(T &&value)
            : m_state(has_value)
        {
            new (&value_address()) value_type(std::move(value));
        }

        expected(T const &value)
            : m_state(has_value)
        {
            new (&value_address()) value_type(value);
        }

#if SILICIUM_COMPILER_HAS_VARIADIC_TEMPLATES
        template <class A0, class... Args>
        explicit expected(A0 &&a0, Args &&... args)
            : m_state(has_value)
        {
            new (&value_address())
                value_type(std::forward<A0>(a0), std::forward<Args>(args)...);
        }
#else
        template <class A0, class A1>
        expected(A0 &&a0, A1 &&a1)
            : m_state(has_value)
        {
            new (&value_address())
                value_type(std::forward<A0>(a0), std::forward<A1>(a1));
        }
#endif

        expected(exception_ptr exception)
            : m_state(has_exception)
        {
            new (&exception_address()) exception_ptr(std::move(exception));
        }

        expected(expected &&other)
            : m_state(other.m_state)
        {
            switch (m_state)
            {
            case has_value:
                new (&value_address())
                    value_type(std::move(other.value_address()));
                break;

            case has_exception:
                new (&exception_address())
                    exception_ptr(std::move(other.exception_address()));
                break;
            }
        }

        expected(expected const &other)
            : m_state(other.m_state)
        {
            switch (m_state)
            {
            case has_value:
                new (&value_address()) value_type(other.value_address());
                break;

            case has_exception:
                new (&exception_address())
                    exception_ptr(other.exception_address());
                break;
            }
        }

        expected &operator=(T &&value)
        {
            switch (m_state)
            {
            case has_value:
                value_address() = std::move(value);
                break;

            case has_exception:
                exception_address().~exception_ptr();
                new (&value_address()) value_type(std::move(value));
                m_state = has_value;
                break;
            }
            return *this;
        }

        expected &operator=(T const &value)
        {
            switch (m_state)
            {
            case has_value:
                value_address() = value;
                break;

            case has_exception:
            {
                value_type copy(value);
                exception_address().~exception_ptr();
                new (&value_address()) value_type(std::move(copy));
                m_value = has_value;
                break;
            }
            }
            return *this;
        }

        expected &operator=(expected &&other)
        {
            switch (m_state)
            {
            case has_value:
                switch (other.m_state)
                {
                case has_value:
                    value_address() = std::move(other.value_address());
                    break;

                case has_exception:
                    value_address().~value_type();
                    new (&exception_address())
                        exception_ptr(std::move(other.exception_address()));
                    break;
                }
                break;

            case has_exception:
                switch (other.m_state)
                {
                case has_value:
                    exception_address().~exception_ptr();
                    new (&value_address())
                        value_type(std::move(other.value_address()));
                    break;

                case has_exception:
                    exception_address() = std::move(other.exception_address());
                    break;
                }
                break;
            }
            m_state = other.m_state;
            return *this;
        }

        expected &operator=(expected const &other)
        {
            switch (m_state)
            {
            case has_value:
                switch (other.m_state)
                {
                case has_value:
                    value_address() = other.value_address();
                    break;

                case has_exception:
                    value_address().~value_type();
                    new (&exception_address())
                        exception_ptr(other.exception_address());
                    break;
                }
                break;

            case has_exception:
                switch (other.m_state)
                {
                case has_value:
                    exception_address().~exception_ptr();
                    new (&value_address()) value_type(other.value_address());
                    break;

                case has_exception:
                    exception_address() = other.exception_address();
                    break;
                }
                break;
            }
            m_state = other.m_state;
            return *this;
        }

        ~expected()
        {
            switch (m_state)
            {
            case has_value:
                value_address().~value_type();
                break;

            case has_exception:
                exception_address().~exception_ptr();
                break;
            }
        }

        T &value()
#if SILICIUM_COMPILER_HAS_RVALUE_THIS_QUALIFIER
            &
#endif
        {
            switch (m_state)
            {
            case has_value:
                return value_address();

            case has_exception:
                rethrow_exception(exception_address());
            }
            SILICIUM_UNREACHABLE();
        }

        T const &value() const
#if SILICIUM_COMPILER_HAS_RVALUE_THIS_QUALIFIER
            &
#endif
        {
            switch (m_state)
            {
            case has_value:
                return value_address();

            case has_exception:
                rethrow_exception(exception_address());
            }
            SILICIUM_UNREACHABLE();
        }

#if SILICIUM_COMPILER_HAS_RVALUE_THIS_QUALIFIER
        T &&value() &&
        {
            switch (m_state)
            {
            case has_value:
                return std::move(value_address());

            case has_exception:
                rethrow_exception(exception_address());
            }
            SILICIUM_UNREACHABLE();
        }
#endif

#if SILICIUM_COMPILER_HAS_VARIADIC_TEMPLATES
        template <class... Args>
        void emplace(Args &&... args)
        {
            switch (m_state)
            {
            case has_value:
                value_address().~value_type();
                break;

            case has_exception:
                exception_address().~exception_ptr();
                break;
            }
            new (&value_address()) value_type(std::forward<Args>(args)...);
        }
#else
        void emplace()
        {
            switch (m_state)
            {
            case has_value:
                value_address().~value_type();
                break;

            case has_exception:
                exception_address().~exception_ptr();
                break;
            }
            new (&value_address()) value_type();
        }

        template <class A0>
        void emplace(A0 &&a0)
        {
            switch (m_state)
            {
            case has_value:
                value_address().~value_type();
                break;

            case has_exception:
                exception_address().~exception_ptr();
                break;
            }
            new (&value_address()) value_type(std::forward<A0>(a0));
        }

        template <class A0, class A1>
        void emplace(A0 &&a0, A1 &&a1)
        {
            switch (m_state)
            {
            case has_value:
                value_address().~value_type();
                break;

            case has_exception:
                exception_address().~exception_ptr();
                break;
            }
            new (&value_address())
                value_type(std::forward<A0>(a0), std::forward<A1>(a1));
        }
#endif

        bool valid() const BOOST_NOEXCEPT
        {
            switch (m_state)
            {
            case has_value:
                return true;
            case has_exception:
                return false;
            }
            SILICIUM_UNREACHABLE();
        }

    private:
        enum state
        {
            has_value,
            has_exception
        };

        union
        {
            typename std::aligned_storage<
                sizeof(T), boost::alignment_of<T>::value>::type m_value;
            typename std::aligned_storage<
                sizeof(exception_ptr),
                boost::alignment_of<exception_ptr>::value>::type m_exception;
        };
        state m_state;

        T &value_address()
        {
            return *reinterpret_cast<T *>(&m_value);
        }

        T const &value_address() const
        {
            return *reinterpret_cast<T const *>(&m_value);
        }

        exception_ptr &exception_address()
        {
            return *reinterpret_cast<exception_ptr *>(&m_exception);
        }

        exception_ptr const &exception_address() const
        {
            return *reinterpret_cast<exception_ptr const *>(&m_exception);
        }
    };

    BOOST_STATIC_ASSERT(sizeof(expected<void *>) <=
                        (sizeof(boost::exception_ptr) + sizeof(void *)));
}
#endif

#endif
