#if defined(_WIN32)
#	define NO_RETURN __declspec(noreturn)
#else
#	define NO_RETURN __attribute__((noreturn)
#endif

#if defined(_MSC_VER)
// Fix MSVC compiler bug with - operator before an unsigned type
// https://stackoverflow.com/questions/36347748/error-c4146-unary-minus-operator-applied-to-unsigned-type-result-still-unsigne
#pragma warning (disable : 4146)

// Disable some warnings
#pragma warning (disable : 4244) // '=': conversion from 'int64_t' to 'int', possible loss of data
#pragma warning (disable : 4013) // 'usecheck' undefined; assuming extern returning int

#undef Ref
#endif
