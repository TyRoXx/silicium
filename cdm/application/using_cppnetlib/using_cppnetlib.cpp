#include <network/http/client.hpp>
#include "gtest/gtest.h"
#include <iostream>

TEST(UsingCppNetLib, Example)
{
	client::request request_("https://google.com/");
	request_ << header("Connection", "close");
	client client_;
	client::response response_ = client_.get(request_);
	std::string body_ = body(response_);
	std::cerr << body_ << '\n';
}
