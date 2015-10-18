#ifndef SILICIUM_ASIO_PROCESS_OUTPUT_HPP
#define SILICIUM_ASIO_PROCESS_OUTPUT_HPP

#include <silicium/asio/reading_observable.hpp>
#include <ventura/file_handle.hpp>

#ifdef _WIN32
#	include <boost/asio/windows/stream_handle.hpp>
#else
#	include <boost/asio/posix/stream_descriptor.hpp>
#endif

namespace Si
{
	struct process_output
	{
		typedef error_or<memory_range> element_type;
		typedef
#ifdef _WIN32
			boost::asio::windows::stream_handle
#else
			boost::asio::posix::stream_descriptor
#endif
			stream;

		process_output()
		{
		}

		process_output(process_output &&other) BOOST_NOEXCEPT
			: m_pipe_reader(std::move(other.m_pipe_reader))
			, m_buffer(std::move(other.m_buffer))
			, m_observable(std::move(other.m_observable))
		{
		}

		process_output &operator = (process_output &&other) BOOST_NOEXCEPT
		{
			m_pipe_reader = std::move(other.m_pipe_reader);
			m_buffer = std::move(other.m_buffer);
			m_observable = std::move(other.m_observable);
			return *this;
		}

		explicit process_output(std::unique_ptr<stream> pipe_reader)
			: m_pipe_reader(std::move(pipe_reader))
			, m_buffer(4096)
			, m_observable(*m_pipe_reader, make_memory_range(m_buffer))
		{
		}

		template <class Observer>
		void async_get_one(Observer &&observer)
		{
			return m_observable.async_get_one(std::forward<Observer>(observer));
		}

	private:

		std::unique_ptr<stream> m_pipe_reader;
		std::vector<char> m_buffer;
		asio::reading_observable<stream> m_observable;
		
		SILICIUM_DISABLE_COPY(process_output)
	};

	template <class AsioFileStream>
	SILICIUM_USE_RESULT
	AsioFileStream make_asio_file_stream(boost::asio::io_service &io, file_handle file)
	{
		return AsioFileStream(io, file.release());
	}
}

#endif
