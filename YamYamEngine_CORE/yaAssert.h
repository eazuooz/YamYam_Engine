#pragma once
#ifdef _DEBUG
	#define YA_ENABLE_ASSERTS
#else
	//
#endif

#ifdef YA_ENABLE_ASSERTS

	// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
	// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
	#define YA_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { YA##type##ERROR(msg, __VA_ARGS__); YA_DEBUGBREAK(); } }
	#define YA_INTERNAL_ASSERT_WITH_MSG(type, check, ...) YA_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
	#define YA_INTERNAL_ASSERT_NO_MSG(type, check) YA_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", YA_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

	#define YA_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
	#define YA_INTERNAL_ASSERT_GET_MACRO(...) YA_EXPAND_MACRO( YA_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, YA_INTERNAL_ASSERT_WITH_MSG, YA_INTERNAL_ASSERT_NO_MSG) )

	// Currently accepts at least the condition and one additional parameter (the message) being optional
	#define YA_ASSERT(...) YA_EXPAND_MACRO( YA_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
	#define YA_CORE_ASSERT(...) YA_EXPAND_MACRO( YA_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
	#define YA_ASSERT(...)
	#define YA_CORE_ASSERT(...)
#endif