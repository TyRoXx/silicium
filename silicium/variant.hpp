#ifndef SILICIUM_VARIANT_HPP
#define SILICIUM_VARIANT_HPP

#include <silicium/alignment_of.hpp>
#include <silicium/detail/argument_of.hpp>
#include <new>
#include <array>
#include <stdexcept>
#include <memory>
#include <boost/variant/static_visitor.hpp>
#include <boost/mpl/find.hpp>
#include <boost/mpl/push_front.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/optional.hpp>
#include <boost/type_traits.hpp>
#include <boost/throw_exception.hpp>

#define SILICIUM_HAS_VARIANT SILICIUM_COMPILER_HAS_VARIADIC_TEMPLATES

namespace Si
{
#if SILICIUM_HAS_VARIANT
    template <class Visitor, class Variant>
    auto apply_visitor(Visitor &&visitor, Variant &&variant) ->
        typename std::decay<Visitor>::type::result_type
    {
        return std::forward<Variant>(variant)
            .apply_visitor(std::forward<Visitor>(visitor));
    }

    template <class T>
    struct inplace
    {
    };

    struct bad_variant_access : std::exception
    {
        bad_variant_access()
        {
        }
    };

    namespace detail
    {
        template <class... T>
        union union_;

        template <>
        union union_<>
        {
        };

        template <class T0>
        union union_<T0>
        {
#if SILICIUM_COMPILER_HAS_CXX11_UNION
            T0 t0;

            union_()
            {
            }

            ~union_()
            {
            }
#else
            typename std::aligned_storage<sizeof(T0),
                                          alignment_of<T0>::value>::type t0;
#endif
        };

        template <class T0, class T1>
        union union_<T0, T1>
        {
#if SILICIUM_COMPILER_HAS_CXX11_UNION
            T0 t0;
            T1 t1;

            union_()
            {
            }

            ~union_()
            {
            }
#else
            typename std::aligned_storage<sizeof(T0),
                                          alignment_of<T0>::value>::type t0;
            typename std::aligned_storage<sizeof(T1),
                                          alignment_of<T1>::value>::type t1;
#endif
        };

        template <class T0, class T1, class T2>
        union union_<T0, T1, T2>
        {
#if SILICIUM_COMPILER_HAS_CXX11_UNION
            T0 t0;
            T1 t1;
            T2 t2;

            union_()
            {
            }

            ~union_()
            {
            }
#else
            typename std::aligned_storage<sizeof(T0),
                                          alignment_of<T0>::value>::type t0;
            typename std::aligned_storage<sizeof(T1),
                                          alignment_of<T1>::value>::type t1;
            typename std::aligned_storage<sizeof(T2),
                                          alignment_of<T2>::value>::type t2;
#endif
        };

        template <class T0, class T1, class T2, class T3, class... T>
        union union_<T0, T1, T2, T3, T...>
        {
#if SILICIUM_COMPILER_HAS_CXX11_UNION
            T0 t0;
            T1 t1;
            T2 t2;
            T3 t3;

            union_()
            {
            }

            ~union_()
            {
            }
#else
            typename std::aligned_storage<sizeof(T0),
                                          alignment_of<T0>::value>::type t0;
            typename std::aligned_storage<sizeof(T1),
                                          alignment_of<T1>::value>::type t1;
            typename std::aligned_storage<sizeof(T2),
                                          alignment_of<T2>::value>::type t2;
            typename std::aligned_storage<sizeof(T3),
                                          alignment_of<T3>::value>::type t3;
#endif
            union_<T...> t4;
        };

        template <class... T>
        struct combined_storage
        {
            typedef unsigned char index_type;

            combined_storage() BOOST_NOEXCEPT : m_index(0)
            {
            }

            index_type index() const BOOST_NOEXCEPT
            {
                return m_index;
            }

            void index(index_type value) BOOST_NOEXCEPT
            {
                m_index = value;
            }

            void set_valueless_by_exception() BOOST_NOEXCEPT
            {
                m_index = invalid_index;
            }

            bool valueless_by_exception() const BOOST_NOEXCEPT
            {
                return m_index == invalid_index;
            }

            char &storage() BOOST_NOEXCEPT
            {
                return reinterpret_cast<char &>(m_storage);
            }

            char const &storage() const BOOST_NOEXCEPT
            {
                return reinterpret_cast<char const &>(m_storage);
            }

        private:
            enum
            {
                invalid_index = 255
            };
            union_<T...> m_storage;
            index_type m_index;
        };

        template <class T>
        struct can_ever_be_invalid
            : std::integral_constant<bool, !std::is_pod<T>::value>
        {
        };

        struct never_invalid
        {
            void set_valueless_by_exception()
            {
                SILICIUM_UNREACHABLE();
            }

            bool valueless_by_exception() const
            {
                return false;
            }
        };

        struct sometimes_invalid
        {
            sometimes_invalid()
                : m_is_valid(true)
            {
            }

            void set_valueless_by_exception() BOOST_NOEXCEPT
            {
                m_is_valid = false;
            }

            bool valueless_by_exception() const BOOST_NOEXCEPT
            {
                return !m_is_valid;
            }

        private:
            bool m_is_valid;
        };

        template <class Single>
        struct combined_storage<Single>
            : public std::conditional<can_ever_be_invalid<Single>::value,
                                      sometimes_invalid, never_invalid>::type
        {
            typedef unsigned char index_type;

            combined_storage()
            {
            }

            index_type index() const BOOST_NOEXCEPT
            {
                return 0;
            }

            void index(index_type value) BOOST_NOEXCEPT
            {
                assert(value == 0);
                (void)value;
            }

            char &storage() BOOST_NOEXCEPT
            {
                return reinterpret_cast<char &>(m_storage);
            }

            char const &storage() const BOOST_NOEXCEPT
            {
                return reinterpret_cast<char const &>(m_storage);
            }

        private:
            union_<Single> m_storage;
        };

        template <class T>
        void destroy_storage(void *storage) BOOST_NOEXCEPT
        {
#ifdef _MSC_VER
            // VC++ 2013: Silence (wrong) warning about unreferenced parameter
            (void)storage;
#endif
            static_cast<T *>(storage)->~T();
        }

        template <class T>
        void move_construct_storage(void *destination, void *source)
        {
            auto &dest = *static_cast<T *>(destination);
            auto &src = *static_cast<T *>(source);
            new (&dest) T(std::move(src));
        }

        template <class T>
        void copy_construct_storage(void *destination, void const *source)
        {
            auto &dest = *static_cast<T *>(destination);
            auto &src = *static_cast<T const *>(source);
            new (&dest) T(src);
        }

        template <class T>
        void move_storage(void *destination, void *source) BOOST_NOEXCEPT
        {
            auto &dest = *static_cast<T *>(destination);
            auto &src = *static_cast<T *>(source);
            dest = std::move(src);
        }

        template <class T>
        void copy_storage(void *destination, void const *source)
        {
            auto &dest = *static_cast<T *>(destination);
            auto &src = *static_cast<T const *>(source);
            dest = src;
        }

        template <class T>
        bool equals(void const *left, void const *right)
        {
            auto &left_ = *static_cast<T const *>(left);
            auto &right_ = *static_cast<T const *>(right);
            return left_ == right_;
        }

        template <class T>
        bool less(void const *left, void const *right)
        {
            auto &left_ = *static_cast<T const *>(left);
            auto &right_ = *static_cast<T const *>(right);
            return left_ < right_;
        }

        template <class Visitor, class T, class Result>
        Result apply_visitor_impl(Visitor &&visitor, void *element)
        {
            return std::forward<Visitor>(visitor)(*static_cast<T *>(element));
        }

        template <class Visitor, class T, class Result>
        Result apply_visitor_const_impl(Visitor &&visitor, void const *element)
        {
            return std::forward<Visitor>(visitor)(
                *static_cast<T const *>(element));
        }

#if SILICIUM_COMPILER_HAS_VARIADIC_PACK_EXPANSION
        template <class First, class... Rest>
        struct first
        {
            typedef First type;
        };
#else
        template <class... Sequence>
        struct first;

        template <class First, class... Rest>
        struct first<First, Rest...>
        {
            typedef First type;
        };
#endif

#if SILICIUM_COMPILER_HAS_VARIADIC_PACK_EXPANSION
        template <class... All>
        struct make_mpl_vector
        {
            typedef boost::mpl::vector<All...> type;
        };
#else
        // workaround for GCC 4.6

        template <class... All>
        struct make_mpl_vector;

        template <class Head, class... Tail>
        struct make_mpl_vector<Head, Tail...>
        {
            typedef typename boost::mpl::push_front<
                typename make_mpl_vector<Tail...>::type, Head>::type type;
        };

        template <>
        struct make_mpl_vector<>
        {
            typedef boost::mpl::vector<> type;
        };
#endif

        template <class Element, class... All>
        struct index_of
            : boost::mpl::find<typename make_mpl_vector<All...>::type,
                               Element>::type::pos
        {
        };

        template <class... T>
        struct are_noexcept_movable;

        template <class First, class... T>
        struct are_noexcept_movable<First, T...>
            : boost::mpl::and_<boost::mpl::and_<
#if BOOST_VERSION >= 105400
                                   boost::is_nothrow_move_constructible<First>,
                                   boost::is_nothrow_move_assignable<First>
#else
                                   std::is_nothrow_move_constructible<First>,
                                   std::is_nothrow_move_assignable<First>
#endif
                                   >,
                               are_noexcept_movable<T...>>::type
        {
        };

        template <>
        struct are_noexcept_movable<> : std::true_type
        {
        };

        template <class Constructed>
        struct construction_visitor : boost::static_visitor<Constructed>
        {
            template <class T>
            Constructed operator()(T &&argument) const
            {
                return Constructed(std::forward<T>(argument));
            }
        };

        template <bool IsCopyable, class... T>
        struct variant_base;

        template <class... T>
        struct variant_base<false, T...>
        {
            enum
            {
                is_noexcept_movable =
#if SILICIUM_COMPILER_HAS_WORKING_NOEXCEPT
                    ::Si::detail::are_noexcept_movable<T...>::value
#else
                    1
#endif
            };

            typedef unsigned index_type;
            typedef boost::mpl::vector<T...> element_types;

            variant_base() BOOST_NOEXCEPT
            {
                typedef typename first<T...>::type constructed;
                new (reinterpret_cast<constructed *>(&this->storage()))
                    constructed();
            }

            ~variant_base() BOOST_NOEXCEPT
            {
                if (this->valueless_by_exception())
                {
                    return;
                }
                destroy_storage(this->index(), this->storage());
            }

            variant_base(variant_base &&other)
                BOOST_NOEXCEPT_IF(is_noexcept_movable)
            {
                this->set_index(other.index());
                move_construct_storage(
                    this->index(), this->storage(), other.storage());
            }

            variant_base &operator=(variant_base &&other)
                BOOST_NOEXCEPT_IF(is_noexcept_movable)
            {
                if (this->index() == other.index())
                {
                    move_storage(
                        this->index(), this->storage(), other.storage());
                }
                else
                {
                    destroy_storage(this->index(), this->storage());
                    move_construct_storage(
                        other.index(), this->storage(), other.storage());
                    this->set_index(other.index());
                }
                return *this;
            }

            template <
                class U, class CleanU = typename std::decay<U>::type,
                std::size_t Index = index_of<CleanU, T...>::value,
                class NoFastVariant = typename std::enable_if<
                    boost::mpl::and_<
                        boost::mpl::not_<std::is_base_of<variant_base, CleanU>>,
                        boost::mpl::bool_<(index_of<CleanU, T...>::value <
                                           sizeof...(T))>>::value,
                    void>::type>
            variant_base &operator=(U &&value)
            {
                destroy_storage(this->index(), this->storage());
#if SILICIUM_HAS_EXCEPTIONS
                try
#endif
                {
                    this->move_or_copy_construct_storage(
                        this->storage(), std::forward<U>(value),
                        std::is_const<
                            typename std::remove_reference<U>::type>());
                }
#if SILICIUM_HAS_EXCEPTIONS
                catch (...)
                {
                    this->set_valueless_by_exception();
                    throw;
                }
#endif
                this->set_index(Index);
                return *this;
            }

            template <
                class U, class CleanU = typename std::decay<U>::type,
                std::size_t Index = index_of<CleanU, T...>::value,
                class NoFastVariant = typename std::enable_if<
                    boost::mpl::and_<
                        boost::mpl::not_<std::is_base_of<variant_base, CleanU>>,
                        boost::mpl::bool_<(index_of<CleanU, T...>::value <
                                           sizeof...(T))>>::value,
                    void>::type>
            variant_base(U &&value)
            {
                this->set_index(Index);
                new (&this->storage()) CleanU(std::forward<U>(value));
            }

            template <class Requested, class... Args,
                      std::size_t Index = index_of<Requested, T...>::value>
            explicit variant_base(inplace<Requested>, Args &&... args)
            {
                this->set_index(Index);
                new (&this->storage()) Requested(std::forward<Args>(args)...);
            }

            index_type index() const BOOST_NOEXCEPT
            {
                if (valueless_by_exception())
                {
                    return static_cast<index_type>(-1);
                }
                return m_storage.index();
            }

            bool valueless_by_exception() const BOOST_NOEXCEPT
            {
                return m_storage.valueless_by_exception();
            }

            template <class Visitor>
            auto apply_visitor(Visitor &&visitor) ->
                typename std::decay<Visitor>::type::result_type
            {
                throw_if_invalid();
                typedef typename std::decay<Visitor>::type::result_type result;
                static std::array<result (*)(Visitor &&, void *),
                                  sizeof...(T)> const f{
                    {&apply_visitor_impl<Visitor, T, result>...}};
                return f[this->index()](
                    std::forward<Visitor>(visitor), &this->storage());
            }

            template <class Visitor>
            auto apply_visitor(Visitor &&visitor) const ->
                typename std::decay<Visitor>::type::result_type
            {
                throw_if_invalid();
                typedef typename std::decay<Visitor>::type::result_type result;
                static std::array<result (*)(Visitor &&, void const *),
                                  sizeof...(T)> const f{
                    {&apply_visitor_const_impl<Visitor, T, result>...}};
                return f[this->index()](
                    std::forward<Visitor>(visitor), &this->storage());
            }

            bool operator==(variant_base const &other) const
            {
                if (this->index() != other.index())
                {
                    return false;
                }
                static std::array<bool (*)(void const *, void const *),
                                  sizeof...(T)> const f = {{&equals<T>...}};
                return f[this->index()](&this->storage(), &other.storage());
            }

            bool operator<(variant_base const &other) const
            {
                if (this->index() > other.index())
                {
                    return false;
                }
                if (this->index() < other.index())
                {
                    return true;
                }
                static std::array<bool (*)(void const *, void const *),
                                  sizeof...(T)> const f = {{&less<T>...}};
                return f[this->index()](&this->storage(), &other.storage());
            }

            template <bool IsOtherCopyable, class... U>
            void assign(variant_base<IsOtherCopyable, U...> &&other)
            {
                *this = Si::apply_visitor(
                    construction_visitor<variant_base>(), std::move(other));
            }

        protected: // TODO: make private somehow
            typedef combined_storage<T...> storage_type;

            storage_type m_storage;

            char &storage() BOOST_NOEXCEPT
            {
                return m_storage.storage();
            }

            char const &storage() const BOOST_NOEXCEPT
            {
                return m_storage.storage();
            }

            void set_index(index_type w) BOOST_NOEXCEPT
            {
                m_storage.index(static_cast<unsigned char>(w));
            }

            void set_valueless_by_exception() BOOST_NOEXCEPT
            {
                m_storage.set_valueless_by_exception();
            }

            void throw_if_invalid() const
            {
                if (valueless_by_exception())
                {
                    boost::throw_exception(bad_variant_access());
                }
            }

            static void move_construct_storage(index_type index,
                                               char &destination, char &source)
            {
                static std::array<void (*)(void *, void *), sizeof...(T)> const
                    f = {{&::Si::detail::move_construct_storage<T>...}};
                f[index](&destination, &source);
            }

            template <class From>
            static void move_or_copy_construct_storage(char &destination,
                                                       From &&from,
                                                       std::false_type)
            {
                ::Si::detail::move_construct_storage<
                    typename std::decay<From>::type>(&destination, &from);
            }

            template <class From>
            static void move_or_copy_construct_storage(char &destination,
                                                       From const &from,
                                                       std::true_type)
            {
                ::Si::detail::copy_construct_storage<From>(&destination, &from);
            }

            static void destroy_storage(index_type index,
                                        char &destroyed) BOOST_NOEXCEPT
            {
                static std::array<void (*)(void *), sizeof...(T)> const f = {
                    {&::Si::detail::destroy_storage<T>...}};
                f[index](&destroyed);
            }

            static void move_storage(index_type index, char &destination,
                                     char &source) BOOST_NOEXCEPT
            {
                static std::array<void (*)(void *, void *), sizeof...(T)> const
                    f = {{&::Si::detail::move_storage<T>...}};
                f[index](&destination, &source);
            }

            SILICIUM_DELETED_FUNCTION(variant_base(variant_base const &))
            SILICIUM_DELETED_FUNCTION(
                variant_base &operator=(variant_base const &))
        };

        template <class... T>
        struct variant_base<true, T...> : variant_base<false, T...>
        {
            typedef variant_base<false, T...> base;
            typedef typename base::index_type index_type;

            variant_base() BOOST_NOEXCEPT
            {
            }

            template <
                class U, class CleanU = typename std::decay<U>::type,
                std::size_t Index = index_of<CleanU, T...>::value,
                class NoFastVariant = typename std::enable_if<
                    boost::mpl::and_<
                        boost::mpl::not_<std::is_base_of<variant_base, CleanU>>,
                        boost::mpl::bool_<(index_of<CleanU, T...>::value <
                                           sizeof...(T))>>::value,
                    void>::type>
            variant_base(U &&value)
                : base(std::forward<U>(value))
            {
            }

            template <class Requested, class... Args>
            explicit variant_base(inplace<Requested> i, Args &&... args)
                : base(i, std::forward<Args>(args)...)
            {
            }

            variant_base(variant_base &&other)
                BOOST_NOEXCEPT_IF(base::is_noexcept_movable)
                : base(std::move(other))
            {
            }

            variant_base(variant_base const &other)
                : base()
            {
                this->set_index(other.index());
                copy_construct_storage(
                    this->index(), this->storage(), other.storage());
            }

            variant_base &operator=(variant_base &&other)
                BOOST_NOEXCEPT_IF(base::is_noexcept_movable)
            {
                base::operator=(std::move(other));
                return *this;
            }

            template <
                class U, class CleanU = typename std::decay<U>::type,
                std::size_t Index = index_of<CleanU, T...>::value,
                class NoFastVariant = typename std::enable_if<
                    boost::mpl::and_<
                        boost::mpl::not_<std::is_base_of<variant_base, CleanU>>,
                        boost::mpl::bool_<(index_of<CleanU, T...>::value <
                                           sizeof...(T))>>::value,
                    void>::type>
            variant_base &operator=(U &&value)
            {
                base::operator=(std::forward<U>(value));
                return *this;
            }

            variant_base &operator=(variant_base const &other)
            {
                if (this->index() == other.index())
                {
                    copy_storage(
                        this->index(), this->storage(), other.storage());
                }
                else
                {
                    typename base::storage_type temporary;
                    copy_construct_storage(other.index(),
                                           reinterpret_cast<char &>(temporary),
                                           other.storage());
                    base::destroy_storage(this->index(), this->storage());
                    base::move_construct_storage(
                        other.index(), this->storage(),
                        reinterpret_cast<char &>(temporary));
                    this->set_index(other.index());
                }
                return *this;
            }

            using base::assign;

            template <bool IsOtherCopyable, class... U>
            void assign(variant_base<IsOtherCopyable, U...> const &other)
            {
                *this = Si::apply_visitor(
                    construction_visitor<variant_base>(), other);
            }

        private:
            static void copy_construct_storage(index_type index,
                                               char &destination,
                                               char const &source)
            {
                static std::array<void (*)(void *, void const *),
                                  sizeof...(T)> const f = {
                    {&::Si::detail::copy_construct_storage<T>...}};
                f[index](&destination, &source);
            }

            static void copy_storage(index_type index, char &destination,
                                     char const &source)
            {
                static std::array<void (*)(void *, void const *),
                                  sizeof...(T)> const f = {
                    {&::Si::detail::copy_storage<T>...}};
                f[index](&destination, &source);
            }
        };

#if SILICIUM_COMPILER_HAS_USING
        template <class... T>
        using select_variant_base = variant_base<true, T...>;
#else
        template <class... T>
        struct select_variant_base
        {
            typedef variant_base<true, T...> type;
        };
#endif

        struct ostream_visitor : boost::static_visitor<>
        {
            std::ostream *out;

            explicit ostream_visitor(std::ostream &out)
                : out(&out)
            {
            }

            template <class T>
            void operator()(T const &value) const
            {
                *out << value;
            }
        };

#if SILICIUM_COMPILER_HAS_USING
        template <bool IsCopyable, class... T>
        std::ostream &operator<<(std::ostream &out,
                                 variant_base<IsCopyable, T...> const &v)
        {
            Si::apply_visitor(::Si::detail::ostream_visitor(out), v);
            return out;
        }
#endif
    }

#if SILICIUM_COMPILER_HAS_USING
    template <class... T>
    using variant = ::Si::detail::select_variant_base<T...>;

    template <class... T>
    using non_copyable_variant = ::Si::detail::variant_base<false, T...>;
#else
    template <class... T>
    struct variant : ::Si::detail::select_variant_base<T...>::type
    {
        typedef typename ::Si::detail::select_variant_base<T...>::type base;

        variant() BOOST_NOEXCEPT
        {
        }

        template <
            class First, class... Initializer,
            class = typename boost::disable_if<
                boost::is_same<typename boost::decay<First>::type, variant>,
                void>::type>
        variant(First &&first, Initializer &&... init)
            : base(std::forward<First>(first),
                   std::forward<Initializer>(init)...)
        {
        }

        variant(variant &&other) BOOST_NOEXCEPT
            : base(std::move(static_cast<base &>(other)))
        {
        }

        variant(variant const &other)
            : base(static_cast<base const &>(other))
        {
        }

        template <
            class Content,
            class = typename boost::disable_if<
                boost::is_same<typename boost::decay<Content>::type, variant>,
                void>::type>
        variant &operator=(Content &&other)
        {
            static_cast<base &>(*this) = std::forward<Content>(other);
            return *this;
        }

        variant &operator=(variant &&other) BOOST_NOEXCEPT
        {
            static_cast<base &>(*this) = std::move(static_cast<base &>(other));
            return *this;
        }

        variant &operator=(variant const &other)
        {
            static_cast<base &>(*this) = static_cast<base const &>(other);
            return *this;
        }

        template <class Other>
        void assign(Other &&other)
        {
            base::assign(
                static_cast<typename std::decay<Other>::type::base const &>(
                    other));
        }
    };

    template <class... T>
    struct non_copyable_variant : ::Si::detail::variant_base<false, T...>
    {
        typedef typename ::Si::detail::variant_base<false, T...> base;

        non_copyable_variant() BOOST_NOEXCEPT
        {
        }

        template <class First, class... Initializer,
                  class = typename boost::disable_if<
                      boost::is_same<typename boost::decay<First>::type,
                                     non_copyable_variant>,
                      void>::type>
        non_copyable_variant(First &&first, Initializer &&... init)
            : base(std::forward<First>(first),
                   std::forward<Initializer>(init)...)
        {
        }

        non_copyable_variant(non_copyable_variant &&other) BOOST_NOEXCEPT
            : base(std::move(static_cast<base &>(other)))
        {
        }

        template <class Content,
                  class = typename boost::disable_if<
                      boost::is_same<typename boost::decay<Content>::type,
                                     non_copyable_variant>,
                      void>::type>
        non_copyable_variant &operator=(Content &&other)
        {
            static_cast<base &>(*this) = std::forward<Content>(other);
            return *this;
        }

        non_copyable_variant &
        operator=(non_copyable_variant &&other) BOOST_NOEXCEPT
        {
            static_cast<base &>(*this) = std::move(static_cast<base &>(other));
            return *this;
        }

        template <class Other>
        void assign(Other &&other)
        {
            base::assign(
                static_cast<typename std::decay<Other>::type::base const &>(
                    other));
        }

        SILICIUM_DISABLE_COPY(non_copyable_variant)
    };
#endif

#if !SILICIUM_COMPILER_HAS_USING
    template <class... T>
    std::ostream &operator<<(std::ostream &out, variant<T...> const &v)
    {
        Si::apply_visitor(::Si::detail::ostream_visitor(out), v);
        return out;
    }
#endif

    template <class... T>
    bool operator!=(variant<T...> const &left, variant<T...> const &right)
    {
        return !(left == right);
    }

    template <class... T>
    bool operator>(variant<T...> const &left, variant<T...> const &right)
    {
        return (right < left);
    }

    template <class... T>
    bool operator<=(variant<T...> const &left, variant<T...> const &right)
    {
        return !(left > right);
    }

    template <class... T>
    bool operator>=(variant<T...> const &left, variant<T...> const &right)
    {
        return !(left < right);
    }

    struct hash_visitor : boost::static_visitor<std::size_t>
    {
        template <class T>
        std::size_t operator()(T const &element) const
        {
            return std::hash<T>()(element);
        }
    };

    template <class Element>
    struct try_get_ptr_visitor : boost::static_visitor<Element *>
    {
        Element *operator()(Element &value) const BOOST_NOEXCEPT
        {
            return &value;
        }

        template <class Other>
        Element *operator()(Other const &) const BOOST_NOEXCEPT
        {
            return nullptr;
        }
    };

    template <class Element, bool IsCopyable, class... T>
    Element *try_get_ptr(::Si::detail::variant_base<IsCopyable, T...> &from)
        BOOST_NOEXCEPT
    {
        BOOST_STATIC_ASSERT(
            (boost::mpl::contains<boost::mpl::vector<T...>, Element>::value));
        return apply_visitor(try_get_ptr_visitor<Element>{}, from);
    }

    template <class Element, bool IsCopyable, class... T>
    typename std::add_const<Element>::type *try_get_ptr(
        ::Si::detail::variant_base<IsCopyable, T...> const &from) BOOST_NOEXCEPT
    {
        BOOST_STATIC_ASSERT(
            (boost::mpl::contains<boost::mpl::vector<T...>, Element>::value));
        return apply_visitor(
            try_get_ptr_visitor<typename std::add_const<Element>::type>{},
            from);
    }

    template <std::size_t Index, class Variant>
    typename boost::mpl::at<typename Variant::element_types,
                            boost::mpl::int_<Index>>::type *
    try_get(Variant &from)
    {
        return try_get_ptr<typename boost::mpl::at<
            typename Variant::element_types, boost::mpl::int_<Index>>::type>(
            from);
    }

    namespace detail
    {
        template <class Result, class... Functions>
        struct overloader;

        template <class Result, class Head>
        struct overloader<Result, Head>
        {
            // for apply_visitor
            typedef Result result_type;

            explicit overloader(Head &head)
                : m_head(&head)
            {
            }

            Result operator()(typename argument_of<Head>::type argument) const
            {
                return (*m_head)(
                    std::forward<typename argument_of<Head>::type>(argument));
            }

        private:
            typename std::remove_reference<Head>::type *m_head;
        };

        template <class Result, class Head, class... Tail>
        struct overloader<Result, Head, Tail...> : overloader<Result, Tail...>
        {
            using overloader<Result, Tail...>::operator();

            // for apply_visitor
            typedef Result result_type;

            explicit overloader(Head &head, Tail &... tail)
                : overloader<Result, Tail...>(tail...)
                , m_head(&head)
            {
            }

            Result operator()(typename argument_of<Head>::type argument) const
            {
                return (*m_head)(
                    std::forward<typename argument_of<Head>::type>(argument));
            }

        private:
            typename std::remove_reference<Head>::type *m_head;
        };
    }

    template <class Result, bool IsCopyable, class... T, class... Visitors>
    Result visit(::Si::detail::variant_base<IsCopyable, T...> &variant,
                 Visitors &&... visitors)
    {
        ::Si::detail::overloader<Result, Visitors...> ov(visitors...);
        return Si::apply_visitor(ov, variant);
    }

    template <class Result, bool IsCopyable, class... T, class... Visitors>
    Result visit(detail::variant_base<IsCopyable, T...> const &variant,
                 Visitors &&... visitors)
    {
        ::Si::detail::overloader<Result, Visitors...> ov(visitors...);
        return Si::apply_visitor(ov, variant);
    }
#endif
}

#if SILICIUM_HAS_VARIANT
namespace std
{
    template <class... T>
    struct hash<Si::variant<T...>>
    {
        std::size_t operator()(Si::variant<T...> const &v) const
        {
            return Si::apply_visitor(Si::hash_visitor(), v);
        }
    };
}
#endif

#endif
