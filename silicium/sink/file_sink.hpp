#ifndef SILICIUM_FILE_SINK_HPP
#define SILICIUM_FILE_SINK_HPP

#include <silicium/sink/sink.hpp>
#include <silicium/fast_variant.hpp>
#include <silicium/file_handle.hpp>
#include <silicium/memory_range.hpp>
#include <silicium/flush.hpp>

#ifdef _WIN32
#	include <silicium/win32/win32.hpp>
#else
#	include <sys/uio.h>
#endif

namespace Si
{
	struct seek_set
	{
		boost::uint64_t from_beginning;
	};

	struct seek_add
	{
		boost::int64_t from_current;
	};

	typedef fast_variant<flush, memory_range, seek_set, seek_add> file_sink_element;

	struct file_sink
	{
		typedef file_sink_element element_type;
		typedef boost::system::error_code error_type;

		file_sink()
		{
		}

		explicit file_sink(native_file_descriptor destination)
			: m_destination(destination)
		{
		}

		error_type append(iterator_range<element_type const *> data)
		{
			return append_impl(data);
		}

	private:

		native_file_descriptor m_destination;

#ifdef _WIN32
		error_type append_impl(iterator_range<element_type const *> data)
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
			return visit<error_type>(
				element,
				[this](flush) -> error_type
			{
				if (FlushFileBuffers(m_destination))
				{
					return error_type();
				}
				return error_type(GetLastError(), boost::system::system_category());
			},
				[this](memory_range const &content) -> error_type
			{
				ptrdiff_t written = 0;
				while (written < content.size())
				{
					ptrdiff_t const remaining = content.size() - written;
					DWORD const max_write_size = std::min<DWORD>(
						(std::numeric_limits<DWORD>::max)(),
						static_cast<DWORD>((std::numeric_limits<ptrdiff_t>::max)())
						);
					DWORD const write_now = std::min<ptrdiff_t>(remaining, static_cast<ptrdiff_t>(max_write_size));
					DWORD written_now = 0;
					if (!WriteFile(m_destination, content.begin(), write_now, &written_now, nullptr))
					{
						return error_type(GetLastError(), boost::system::system_category());
					}
					assert(written_now > 0);
					written += static_cast<ptrdiff_t>(written_now);
				}
				return error_type();
			},
				[this](seek_set const request) -> error_type
			{
				LARGE_INTEGER distance;
				distance.QuadPart = request.from_beginning;
				if (SetFilePointerEx(m_destination, distance, NULL, FILE_BEGIN))
				{
					return error_type();
				}
				return error_type(GetLastError(), boost::system::system_category());
			},
				[this](seek_add const request) -> error_type
			{
				LARGE_INTEGER distance;
				distance.QuadPart = request.from_current;
				if (SetFilePointerEx(m_destination, distance, NULL, FILE_CURRENT))
				{
					return error_type();
				}
				return error_type(GetLastError(), boost::system::system_category());
			}
			);
		}
#else
		error_type append_impl(iterator_range<element_type const *> data)
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
					auto error = write_piece(*try_get_ptr<memory_range>(*(data.begin() + i - *write_streak_length)));
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
				if (try_get_ptr<memory_range>(element))
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
			return visit<error_type>(
				element,
				[this](flush) -> error_type
				{
					if (fdatasync(m_destination) == 0)
					{
						return error_type();
					}
					return error_type(errno, boost::system::system_category());
				},
				[this](memory_range const &content) -> error_type
				{
					return write_piece(content);
				},
				[this](seek_set const request) -> error_type
				{
					if (lseek64(m_destination, request.from_beginning, SEEK_SET) == static_cast<off_t>(-1))
					{
						return error_type(errno, boost::system::system_category());
					}
					return error_type();
				},
				[this](seek_add const request) -> error_type
				{
					if (lseek64(m_destination, request.from_current, SEEK_CUR) == static_cast<off_t>(-1))
					{
						return error_type(errno, boost::system::system_category());
					}
					return error_type();
				}
			);
		}

		error_type write_piece(memory_range const &content)
		{
			ptrdiff_t written = 0;
			while (written < content.size())
			{
				ssize_t rc = ::write(m_destination, content.begin() + written, content.size() - written);
				if (rc < 0)
				{
					return error_type(errno, boost::system::system_category());
				}
				written += static_cast<ptrdiff_t>(rc);
			}
			return error_type();
		}

		error_type write_vector(element_type const *begin, element_type const *end)
		{
			std::vector<iovec> vector(std::distance(begin, end));
			for (size_t i = 0; i < vector.size(); ++i, ++begin)
			{
				memory_range const &piece = *try_get_ptr<memory_range>(*begin);
				vector[i].iov_base = const_cast<void *>(static_cast<void const *>(piece.begin()));
				vector[i].iov_len = piece.size();
			}
			assert(vector.size() <= static_cast<size_t>(std::numeric_limits<int>::max()));
			ssize_t rc = ::writev(m_destination, vector.data(), static_cast<int>(vector.size()));
			if (rc < 0)
			{
				return error_type(errno, boost::system::system_category());
			}
			//assume that everything has been written
			return error_type();
		}
#endif
	};
}

#endif
