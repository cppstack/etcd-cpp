#ifndef _CST_PROGRAM_OPTIONS_ERRORS_HPP
#define _CST_PROGRAM_OPTIONS_ERRORS_HPP

#include <vector>

namespace cst {
namespace program_options {

inline const std::string with_prefix(const std::string& opt)
{
    const std::size_t len = opt.length();
    return (len == 0 || opt[0] == '-') ? opt
         : (len == 1 ? "-" + opt : "--" + opt);
}

class option_error : public std::exception {
public:
    option_error(const std::string& message)
        : message_(message)
    { }

    option_error(const std::string& name,
                 const std::string& value,
                 const std::string& message)
        : name_(name), value_(value), message_(message)
    { }

    const std::string& name()  const noexcept { return name_; }
    const std::string& value() const noexcept { return value_; }

    virtual const char* what() const noexcept { return message_.c_str(); }

protected:
    const std::string name_;
    const std::string value_;
    std::string message_;
};

class option_desc_error : public option_error {
public:
    using option_error::option_error;
};

class duplicate_option : public option_desc_error {
public:
    explicit duplicate_option(const std::string& name)
        : option_desc_error(name, "", "duplicate option '" + name + "'")
    { }
};

class bad_option_spec : public option_desc_error {
public:
    explicit bad_option_spec(const std::string& spec)
        : option_desc_error(spec, "", "bad option specifier '" + spec + "'")
    { }
};

class option_parse_error : public option_error {
public:
    using option_error::option_error;
};

class invalid_option_name : public option_parse_error {
public:
    explicit invalid_option_name(const std::string& name)
        : option_parse_error(name, "", "invalid option '" + name + "'")
    { }
};

class unknown_option : public option_parse_error {
public:
    explicit unknown_option(const std::string& opt)
        : option_parse_error(opt, "", "unknown option '" + with_prefix(opt) + "'")
    { }
};

class superfluous_argument : public option_parse_error {
public:
    superfluous_argument(const std::string& opt, const std::string& arg)
        : option_parse_error(opt, arg, "unnecessary argument '" + arg + "' for option '" + with_prefix(opt) + "'")
    { }
};

class missing_argument : public option_parse_error {
public:
    explicit missing_argument(const std::string& opt)
        : option_parse_error(opt, "", "missing argument for option '" + with_prefix(opt) + "'")
    { }
};

class invalid_argument : public option_parse_error {
public:
    invalid_argument(const std::string& opt, const std::string& arg)
        : option_parse_error(opt, arg, "invalid argument '" + arg + "'")
    {
        if (! opt.empty())
            message_ += " for option '" + with_prefix(opt) + "'";
    }
};

class invalid_value : public option_parse_error {
public:
    /* usually thrown by users in notifier */
    invalid_value(const std::string& message)
        : option_parse_error(message)
    { }

    invalid_value(const std::string& opt, const std::string& value, const std::string& reason)
        : option_parse_error(opt, value, "invalid value '" + value + "' for option '" + with_prefix(opt) + "', reason: " + reason)
    { }
};

class invalid_bool_value : public option_parse_error {
public:
    invalid_bool_value(const std::string& opt, const std::string& value)
        : option_parse_error(opt, value, "invalid bool value '" + value + "' for option '" + with_prefix(opt) + "', accepts: (yes|no)")
    { }
};

class missing_option : public option_parse_error {
public:
    explicit missing_option(const std::string& opt, bool is_pos = false)
        : option_parse_error(opt, "", "")
    {
        if (is_pos)
            message_ = "missing " + opt;
        else
            message_ = "missing option '" + with_prefix(opt) + "'";
    }
};

class option_map_error : public option_error {
public:
    using option_error::option_error;
};

class option_not_present : public option_map_error {
public:
    explicit option_not_present(const std::string& opt)
        : option_map_error(opt, "", "option '" + with_prefix(opt) + "' is not present")
    { }
};

class unrecognized_option : public option_map_error {
public:
    explicit unrecognized_option(const std::string& opt)
        : option_map_error(opt, "", "option '" + with_prefix(opt) + "' is unrecognized")
    { }
};

/*
 * The library doesn't throw this error. It leaves the user to decide.
 */
class leftover_arguments : public option_parse_error {
public:
    /* The argv comes from vm[""].as<std::vector<std::string>>() */
    explicit leftover_arguments(const std::vector<std::string>& argv)
        : option_parse_error("leftover positional arguments:")
    {
        for (const auto& a : argv)
            message_.append(1, ' ').append(a);
    }
};

}
}

#endif
