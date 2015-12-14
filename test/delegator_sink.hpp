DELEGATOR_TEMPLATE(class Element, class Error)
DELEGATOR_NAME(Sink)
DELEGATOR_TYPEDEF(typedef Element element_type;)
DELEGATOR_TYPEDEF(typedef Error error_type;)
DELEGATOR_METHOD(append, error_type, Si::array_view<element_type>)
DELEGATOR_BASIC_METHOD(max_size, const, Si::optional<boost::uint64_t>, Si::nothing)