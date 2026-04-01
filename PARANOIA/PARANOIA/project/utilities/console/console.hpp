#pragma once

class console
{
public:
    bool initialize(std::string_view title);

    template <typename... args_t>
    void print_impl(std::string_view fmt, args_t&&... args) const
    {
        this->emit(level::info, std::vformat(fmt, std::make_format_args(args...)));
    }

    template <typename... args_t>
    [[noreturn]] void error_impl(std::string_view fmt, args_t&&... args) const
    {
        this->emit(level::error, std::vformat(fmt, std::make_format_args(args...)));
        this->wait_and_exit();
    }

    template <typename... args_t>
    void warn_impl(std::string_view fmt, args_t&&... args) const
    {
        this->emit(level::warn, std::vformat(fmt, std::make_format_args(args...)));
    }

    template <typename... args_t>
    void success_impl(std::string_view fmt, args_t&&... args) const
    {
        this->emit(level::success, std::vformat(fmt, std::make_format_args(args...)));
    }

    void toggle_visibility() const;
private:
    void* m_handle{};
    void* m_input_handle{};

    enum class level { info, warn, error, success };

    void emit(level lvl, const std::string& message) const;
    [[noreturn]] void wait_and_exit() const;
};

// Fallback for legacy preprocessors that do not support C++20 __VA_OPT__
// The ## token pasting operator will swallow the preceding comma if no args are passed
#define print(fmt, ...)   print_impl(ecrypt(fmt), ##__VA_ARGS__)
#define error(fmt, ...)   error_impl(ecrypt(fmt), ##__VA_ARGS__)
#define warn(fmt, ...)    warn_impl(ecrypt(fmt), ##__VA_ARGS__)
#define success(fmt, ...) success_impl(ecrypt(fmt), ##__VA_ARGS__)

/*

class console
{
public:
    bool initialize(std::string_view title);

    // Changed std::format_string to std::string_view to allow runtime decrypted strings
    template <typename... args_t>
    void print(std::string_view fmt, args_t&&... args) const
    {
        // Using std::vformat and std::make_format_args for runtime formatting
        this->emit(level::info, std::vformat(fmt, std::make_format_args(args...)));
    }

    template <typename... args_t>
    [[noreturn]] void error(std::string_view fmt, args_t&&... args) const
    {
        this->emit(level::error, std::vformat(fmt, std::make_format_args(args...)));
        this->wait_and_exit();
    }

    template <typename... args_t>
    void warn(std::string_view fmt, args_t&&... args) const
    {
        this->emit(level::warn, std::vformat(fmt, std::make_format_args(args...)));
    }

    template <typename... args_t>
    void success(std::string_view fmt, args_t&&... args) const
    {
        this->emit(level::success, std::vformat(fmt, std::make_format_args(args...)));
    }

private:
    void* m_handle{};
    void* m_input_handle{};

    enum class level { info, warn, error, success };

    void emit(level lvl, const std::string& message) const;
    [[noreturn]] void wait_and_exit() const;
};

*/

/*

class console
{
public:
	bool initialize( std::string_view title );

	template <typename... args_t>
	void print( std::format_string<args_t...> fmt, args_t&&... args ) const
	{
		this->emit( level::info, std::format( fmt, std::forward<args_t>( args )... ) );
	}

	template <typename... args_t>
	[[noreturn]] void error( std::format_string<args_t...> fmt, args_t&&... args ) const
	{
		this->emit( level::error, std::format( fmt, std::forward<args_t>( args )... ) );
		this->wait_and_exit( );
	}

	template <typename... args_t>
	void warn( std::format_string<args_t...> fmt, args_t&&... args ) const
	{
		this->emit( level::warn, std::format( fmt, std::forward<args_t>( args )... ) );
	}

	template <typename... args_t>
	void success( std::format_string<args_t...> fmt, args_t&&... args ) const
	{
		this->emit( level::success, std::format( fmt, std::forward<args_t>( args )... ) );
	}

private:
	void* m_handle{};
	void* m_input_handle{};

	enum class level { info, warn, error, success };

	void emit( level lvl, const std::string& message ) const;
	[[noreturn]] void wait_and_exit( ) const;
};
*/