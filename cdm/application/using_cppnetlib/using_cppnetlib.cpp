#include <boost/network/protocol/http.hpp>
#include "gtest/gtest.h"
#include <iostream>

TEST(UsingCppNetLib, Example)
{
	boost::network::http::client::request request_("http://google.com/");
	request_ << boost::network::header("Connection", "close");
	boost::network::http::client client_;
	boost::network::http::client::response response_ = client_.get(request_);
	std::string body_ = body(response_);
	std::cerr << body_ << '\n';
}
