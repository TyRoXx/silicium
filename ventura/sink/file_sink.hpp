#ifndef VENTURA_FILE_SINK_HPP
#define VENTURA_FILE_SINK_HPP

#include <silicium/write.hpp>
#include <silicium/sink/sink.hpp>
#include <silicium/variant.hpp>
#include <silicium/file_handle.hpp>
#include <silicium/memory_range.hpp>
#include <ventura/flush.hpp>

#ifdef _WIN32
#	include <silicium/win32/win32.hpp>
#else
#	include <sys/uio.h>
#endif

#ifdef __APPLE__
#	define SILICIUM_HAS_FILE_SINK 0
#else
#	define SILICIUM_HAS_FILE_SINK SILICIUM_HAS_VARIANT
#endif

namespace ventura
{
#if SILICIUM_HAS_FILE_SINK
	struct seek_set
	{
		boost::uint64_t from_beginning;
	};

	struct seek_add
	{
		boost::int64_t from_current;
	};

	typedef Si::variant<flush, Si::memory_range, seek_set, seek_add> file_sink_element;

	struct file_sink
	{
		typedef file_sink_element element_type;
		typedef boost::system::error_code error_type;

		file_sink()
		{
		}

		explicit file_sink(Si::native_file_descriptor destination)
			: m_destination(destination)
		{
		}

		error_type append(Si::iterator_range<element_type const *> data)
		{
			return append_impl(data);
		}

	private:

		Si::native_file_descriptor m_destination;

#ifdef _WIN32
		error_type append_impl(Si::iterator_range<element_type const *> data)
		{
			for (auto const &element : data)
			{
				error_type error = append_one(element);
				if (error)
				{
					return error;
				}
			}
			return error_type();
		}

		error_type append_one(element_type const &element)
		{
			return Si::visit<error_type>(
				element,
				[this](flush) -> error_type
			{
				if (FlushFileBuffers(m_destination))
				{
					return error_type();
				}
				return Si::get_last_error();
			},
				[this](Si::memory_range const &content) -> error_type
			{
				Si::error_or<std::size_t> written = write(m_destination, content);
				if (!written.is_error())
				{
					assert(written.get() == static_cast<std::size_t>(content.size()));
				}
				return written.error();
			},
				[this](seek_set const request) -> error_type
			{
				LARGE_INTEGER distance;
				distance.QuadPart = request.from_beginning;
				if (SetFilePointerEx(m_destination, distance, NULL, FILE_BEGIN))
				{
					return error_type();
				}
				return Si::get_last_error();
			},
				[this](seek_add const request) -> error_type
			{
				LARGE_INTEGER distance;
				distance.QuadPart = request.from_current;
				if (SetFilePointerEx(m_destination, distance, NULL, FILE_CURRENT))
				{
					return error_type();
				}
				return Si::get_last_error();
			}
			);
		}
#else
		error_type append_impl(Si::iterator_range<element_type const *> data)
		{
			boost::optional<size_t> write_streak_length;
			ptrdiff_t i = 0;
			auto flush_writes = [&, this]() -> error_type
			{
				if (!write_streak_length)
				{
					return error_type();
				}
				if (*write_streak_length == 1)
				{
					auto error = write_piece(*Si::try_get_ptr<Si::memory_range>(*(data.begin() + i - *write_streak_length)));
					write_streak_length = boost::none;
					return error;
				}
				else
				{
					auto error = write_vector(data.begin() + i - *write_streak_length, data.begin() + i);
					write_streak_length = boost::none;
					return error;
				}
			};
			for (; i < data.size(); ++i)
			{
				auto &element = data[i];
				size_t const max_streak_length = 1024;
				if (Si::try_get_ptr<Si::memory_range>(element))
				{
					if (write_streak_length)
					{
						++(*write_streak_length);
						if (*write_streak_length < max_streak_length)
						{
							continue;
						}
					}
					else
					{
						write_streak_length = 1;
						continue;
					}
				}

				error_type err = flush_writes();
				if (err)
				{
					return err;
				}

				err = append_one(element);
				if (err)
				{
					return err;
				}
			}
			return flush_writes();
		}

		error_type append_one(element_type const &element)
		{
			return Si::visit<error_type>(
				element,
				[this](flush) -> error_type
				{
					if (fdatasync(m_destination) == 0)
					{
						return error_type();
					}
					return Si::get_last_error();
				},
				[this](Si::memory_range const &content) -> error_type
				{
					return write_piece(content);
				},
				[this](seek_set const request) -> error_type
				{
					if (lseek64(m_destination, request.from_beginning, SEEK_SET) == static_cast<off_t>(-1))
					{
						return Si::get_last_error();
					}
					return error_type();
				},
				[this](seek_add const request) -> error_type
				{
					if (lseek64(m_destination, request.from_current, SEEK_CUR) == static_cast<off_t>(-1))
					{
						return Si::get_last_error();
					}
					return error_type();
				}
			);
		}

		error_type write_piece(Si::memory_range const &content)
		{
			Si::error_or<std::size_t> written = Si::write(m_destination, content);
			if (!written.is_error())
			{
				assert(written.get() == static_cast<size_t>(content.size()));
			}
			return written.error();
		}

		error_type write_vector(element_type const *begin, element_type const *end)
		{
			std::vector<iovec> vector(std::distance(begin, end));
			for (size_t i = 0; i < vector.size(); ++i, ++begin)
			{
				Si::memory_range const &piece = *Si::try_get_ptr<Si::memory_range>(*begin);
				vector[i].iov_base = const_cast<void *>(static_cast<void const *>(piece.begin()));
				vector[i].iov_len = piece.size();
			}
			assert(vector.size() <= static_cast<size_t>((std::numeric_limits<int>::max)()));
			ssize_t rc = ::writev(m_destination, vector.data(), static_cast<int>(vector.size()));
			if (rc < 0)
			{
				return Si::get_last_error();
			}
			//assume that everything has been written
			return error_type();
		}
#endif
	};

#endif
}

#endif
