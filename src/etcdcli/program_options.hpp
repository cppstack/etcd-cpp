#ifndef _CST_PROGRAM_OPTIONS_HPP
#define _CST_PROGRAM_OPTIONS_HPP

#include <map>
#include <set>
#include <vector>
#include <memory>
#include <cstring>
#include <sstream>
#include <functional>
#include <regex>

namespace cst {
namespace program_options {

inline const std::string with_prefix(const std::string& opt)
{
    return (opt.length() > 1 && opt[0] != '-') ? "--" + opt : opt;
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

class variable_value;

class value_semantic {
public:
    virtual bool has_arg() const = 0;
    virtual bool has_default() const = 0;
    virtual bool has_implicit() const = 0;
    virtual bool is_vector() const = 0;
    virtual bool is_required() const = 0;

    virtual std::string default_str() const = 0;
    virtual std::string implicit_str() const = 0;

    virtual std::shared_ptr<variable_value> create_value() const = 0;

    virtual void parse(const std::shared_ptr<variable_value>& val) const = 0;
    virtual void parse(const std::string& opt,
                       const std::string& text,
                       const std::shared_ptr<variable_value>& val) const = 0;

    virtual bool try_parse(const std::string& opt,
                           const std::string& text) const = 0;

    virtual void notify(const variable_value&) const = 0;

    virtual ~value_semantic() {}
};

class variable_value {
public:
    variable_value(const std::shared_ptr<const value_semantic>& semantic) noexcept
        : typeid_(typeid(void)), semantic_(semantic)
    { }

    template<typename T>
    variable_value(const std::shared_ptr<const value_semantic>& semantic,
                   T* store)
        : typeid_(typeid(T)), semantic_(semantic)
    {
        if (store)
            value_ = store;
        else {
            holder_ = std::make_shared<T>();
            value_ = holder_.get();
        }
    }

    template <typename T>
    const T& as() const
    {
        if (typeid_ != typeid(T))
            throw std::bad_cast();
        return *(static_cast<const T*>(value_));
    }

    template <typename T>
    T& as()
    {
        if (typeid_ != typeid(T))
            throw std::bad_cast();
        return *(static_cast<T*>(value_));
    }

    void notify()
    {
        semantic_->notify(*this);
    }

    const std::shared_ptr<const value_semantic> semantic() const noexcept
    {
        return semantic_;
    }

    std::size_t& count() noexcept { return count_; }

    bool& is_pos() noexcept { return is_pos_; }

private:
    const std::type_info& typeid_;
    std::shared_ptr<const value_semantic> semantic_;
    std::shared_ptr<void> holder_;
    std::size_t count_ = 0;
    void*  value_ = nullptr;
    bool  is_pos_ = false;
};

template <typename T>
struct type_is_vector
{
    static constexpr bool value = false;
};

template <typename T>
struct type_is_vector<std::vector<T>>
{
    static constexpr bool value = true;
};

template <typename T>
void parse_value(const std::string& opt, const std::string& text, T& value)
{
    std::istringstream iss(text);

    if (! (iss >> value))
        throw invalid_argument(opt, text);

    if (iss.rdbuf()->in_avail() != 0)
        throw invalid_argument(opt, text);
}

inline void parse_value(const std::string& /* opt */, const std::string& text, std::string& value)
{
    value = text;
}

inline void parse_value(const std::string& opt, const std::string& text, bool& value)
{
    if (! strcasecmp(text.c_str(), "yes"))
        value = true;
    if (! strcasecmp(text.c_str(), "no"))
        value = false;
    else
        throw invalid_bool_value(opt, text);
}

template <typename T>
void parse_value(const std::string& opt, const std::string& text, std::vector<T>& value)
{
    T v;
    parse_value(opt, text, v);
    value.push_back(v);
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec)
{
    os << "[";
    if (vec.size() > 0) {
        os << vec[0];
        for (std::size_t i = 1; i < vec.size(); ++i)
            os << ", " << vec[i];
    }
    os << "]";
    return os;
}

template <typename T>
class option_value : public value_semantic,
                     public std::enable_shared_from_this<option_value<T>> {
public:
    option_value() = default;

    option_value(T* t)
        : store_(t)
    { }

    bool has_arg() const override      { return true; }
    bool has_default() const override  { return default_ ? true : false; }
    bool has_implicit() const override { return implicit_ ? true : false; }
    bool is_vector() const override    { return type_is_vector<T>::value; }
    bool is_required() const override  { return required_; }

    std::string default_str() const override
    {
        std::ostringstream oss;
        oss << *default_;
        return oss.str();
    }

    std::string implicit_str() const override
    {
        std::ostringstream oss;
        oss << *implicit_;
        return oss.str();
    }

    std::shared_ptr<variable_value> create_value() const override
    {
        return std::make_shared<variable_value>(this->shared_from_this(), store_);
    }

    void parse(const std::shared_ptr<variable_value>& val) const override
    {
        if (val->count() == 0 && default_) {
            val->as<T>() = *default_;
            val->count()++;
        }
    }

    void parse(const std::string& opt,
               const std::string& text,
               const std::shared_ptr<variable_value>& val) const override
    {
        if (text.empty() && implicit_)
            val->as<T>() = *implicit_;
        else
            parse_value(opt, text, val->as<T>());

        val->count()++;
    }

    bool try_parse(const std::string& opt,
                   const std::string& text) const override
    {
        try {
            T t;
            parse_value(opt, text, t);
        } catch (const option_parse_error&) {
            return false;
        }

        return true;
    }

    void notify(const variable_value& val) const override
    {
        if (notifier_)
            notifier_(val.as<T>());
    }

    std::shared_ptr<option_value<T>> default_value(const T& value)
    {
        default_ = std::make_shared<const T>(value);
        return this->shared_from_this();
    }

    std::shared_ptr<option_value<T>> implicit_value(const T& value)
    {
        implicit_ = std::make_shared<const T>(value);
        return this->shared_from_this();
    }

    std::shared_ptr<option_value<T>> notifier(const std::function<void(const T&)>& fn)
    {
        notifier_ = fn;
        return this->shared_from_this();
    }

    std::shared_ptr<option_value<T>> required()
    {
        required_ = true;
        return this->shared_from_this();
    }

private:
    T* const store_ = nullptr;
    std::shared_ptr<const T> default_;
    std::shared_ptr<const T> implicit_;
    std::function<void(const T&)> notifier_;
    bool required_ = false;
};

template <>
class option_value<void> : public value_semantic,
                           public std::enable_shared_from_this<option_value<void>> {
public:
    bool has_arg() const override      { return false; }
    bool has_default() const override  { return false; }
    bool has_implicit() const override { return false; }
    bool is_vector() const override    { return false; }
    bool is_required() const override  { return required_; }

    std::string default_str() const override  { return ""; }
    std::string implicit_str() const override { return ""; }

    std::shared_ptr<variable_value> create_value() const override
    {
        return std::make_shared<variable_value>(this->shared_from_this());
    }

    void parse(const std::shared_ptr<variable_value>& /*val*/) const override
    { }

    void parse(const std::string& /* opt  */,
               const std::string& /* text */,
               const std::shared_ptr<variable_value>& val) const override
    {
        val->count()++;
    }

    bool try_parse(const std::string& /* opt  */,
                   const std::string& /* text */) const override
    {
        return false;
    }

    void notify(const variable_value& /*val*/) const override
    {
        if (notifier_)
            notifier_();
    }

    std::shared_ptr<option_value<void>> notifier(const std::function<void(void)>& fn)
    {
        notifier_ = fn;
        return this->shared_from_this();
    }

    std::shared_ptr<option_value<void>> required()
    {
        required_ = true;
        return this->shared_from_this();
    }

private:
    std::function<void(void)> notifier_;
    bool required_ = false;
};

template <typename T>
std::shared_ptr<option_value<T>> value()
{
    return std::make_shared<option_value<T>>();
}

template <typename T>
std::shared_ptr<option_value<T>> value(T* t)
{
    return std::make_shared<option_value<T>>(t);
}

struct option_description
{
    option_description()
        : semantic(std::make_shared<option_value<std::vector<std::string>>>()),
          hide(true)
    { }

    option_description(const std::string& opts_,
                       const std::shared_ptr<const value_semantic>& sm_,
                       const std::string& desc_,
                       const std::string& arg_,
                       bool hide_);

    std::string s;
    std::string l;
    std::shared_ptr<const value_semantic> semantic;
    std::string desc;
    std::string arg;
    bool hide;
};

class options_description;

class option_adder {
public:
    option_adder(options_description* owner) noexcept
        : owner_(owner)
    { }

    option_adder& operator()(const std::string& opts,
                             const std::string& desc)
    {
        return (*this)(opts, value<void>(), desc);
    }

    option_adder& operator()(const std::string& opts,
                             const std::shared_ptr<const value_semantic>& semantic,
                             const std::string& desc = "",
                             const std::string& arg = "", bool hide = false);

private:
    options_description* owner_;
};

class positional_options_description {
public:
    positional_options_description& add(const std::string& name, int max_count);

    const std::string& name_for_position(unsigned pos) const;

    bool contains(const std::string& name) const
    {
        return set_.count(name);
    }

private:
    const std::string empty_name_ = "";
    std::vector<std::pair<std::string, unsigned>> positionals_;
    std::set<std::string> set_;
    unsigned max_total_count_ = 0;
};

using parsed_options = std::map<std::string, std::shared_ptr<variable_value>>;

class options_description {
    friend parsed_options parse_command_line(int argc, const char* const argv[],
                          const options_description& desc,
                          const positional_options_description& pos_desc);

    friend std::ostream& operator<<(std::ostream& os, const options_description& desc);

public:
    options_description(const std::string& caption);

    option_adder add_options() { return option_adder(this); }

    void add_option(const std::shared_ptr<const option_description>& desc);

private:
    std::string help() const;

    std::string format_opt(const std::shared_ptr<const option_description>& o) const;

    std::string format_desc(const std::shared_ptr<const option_description>& o, std::size_t indent, std::size_t width) const;

    const std::size_t OPTION_LINE_LENGTH = 76;
    const std::size_t OPTION_LONGEST = 30;
    const std::size_t OPTION_DESC_GAP = 2;

    std::string caption_;
    std::map<std::string, std::shared_ptr<const option_description>> optmap_;
    std::set<std::string> optset_;
    std::vector<std::shared_ptr<const option_description>> options_;
};

inline std::ostream& operator<<(std::ostream& os, const options_description& desc)
{
    return os << desc.help();
}

class variables_map {
    friend void store(parsed_options&& options, variables_map& vm) noexcept;
    friend void notify(variables_map& vm);

public:
    std::size_t count(const std::string& opt) const;

    const variable_value& operator[](const std::string& opt) const;

private:
    parsed_options values_;
};



const std::regex option_specifier("([[:alnum:]][-_[:alnum:]]+)?(?:,([[:alnum:]]))?");
const std::regex long_option_matcher("--([[:alnum:]][-_[:alnum:]]+)(?:=(.*))?");

inline const std::string strip_prefix(const std::string& opt)
{
    std::size_t i = 0;
    while (i < 2 && i < opt.length() && opt[i] == '-')
        ++i;
    return i > 0 ? opt.substr(i) : opt;
}

inline option_description::option_description(const std::string& opts_,
                 const std::shared_ptr<const value_semantic>& sm_,
                 const std::string& desc_,
                 const std::string& arg_,
                 bool hide_)
    : semantic(sm_), desc(desc_), arg(arg_), hide(hide_)
{
    std::smatch matches;
    regex_match(opts_, matches, option_specifier);

    l = std::move(matches[1]);
    s = std::move(matches[2]);

    if (s.empty() && l.empty())
        throw bad_option_spec(opts_);
}

inline positional_options_description& positional_options_description::add(const std::string& name, int max_count)
{
    if (max_count < 0)
        max_count = (~0U);

    if (max_total_count_ +  max_count > max_total_count_)
        max_total_count_ += max_count;

    set_.insert(name);

    positionals_.emplace_back(strip_prefix(name), (unsigned)max_count);

    return *this;
}

inline const std::string& positional_options_description::name_for_position(unsigned pos) const
{
    if (pos >= max_total_count_)
        return empty_name_;

    for (const auto& pe : positionals_) {
        if (pos < pe.second)
            return pe.first;
        pos -= pe.second;
    }

    return empty_name_;
}

inline options_description::options_description(const std::string& caption)
    : caption_(caption)
{
    std::shared_ptr<const option_description> unknown_argvs_desc
        = std::make_shared<const option_description>();

    options_.push_back(unknown_argvs_desc);
}

inline void options_description::add_option(const std::shared_ptr<const option_description>& desc)
{
    const auto& s = desc->s;
    const auto& l = desc->l;

    if (! s.empty() && optset_.count(s))
        throw duplicate_option(s);

    if (! l.empty() && optset_.count(l))
        throw duplicate_option(l);

    options_.push_back(desc);
}

inline std::string options_description::help() const
{
    std::string desc = caption_;
    desc += "\nOptions:\n";

    std::size_t longest = 0;

    std::vector<std::string> opts;

    for (const auto& o : options_) {
        if (o->hide)
            continue;

        const auto& s = format_opt(o);
        longest = std::max(longest, s.length());
        opts.push_back(s);
    }

    longest = std::min(longest, OPTION_LONGEST);

    // widest allowed description
    std::size_t allowed = OPTION_LINE_LENGTH - longest - OPTION_DESC_GAP;

    auto it = opts.cbegin();

    for (const auto& o : options_) {
        if (o->hide)
            continue;

        desc += *it;

        if (it->length() > longest) {
            desc += '\n';
            desc.append(longest + OPTION_DESC_GAP, ' ');
        } else {
            desc.append(longest + OPTION_DESC_GAP - it->length(), ' ');
        }

        desc += format_desc(o, longest + OPTION_DESC_GAP, allowed);
        desc += '\n';

        ++it;
    }

    return desc;
}

inline std::string options_description::format_opt(const std::shared_ptr<const option_description>& o) const
{
    std::string result = "  ";

    if (! o->s.empty())
        result += "-" + o->s + ",";
    else
        result += "   ";

    if (! o->l.empty())
        result += " --" + o->l;

    const auto& sm = o->semantic;

    if (sm->has_arg()) {
        const auto& arg = o->arg.empty() ? "arg" : o->arg;

        if (sm->has_implicit())
            result += " [" + arg + "(=" + sm->implicit_str() + ")]";
        else
            result += " <" + arg + ">";
    }

    return result;
}

inline std::string options_description::format_desc(const std::shared_ptr<const option_description>& o, std::size_t indent, std::size_t width) const
{
    std::string desc = o->desc;

    const auto& sm = o->semantic;

    if (sm->is_required())
        desc += " (required)";

    if (sm->has_default())
        desc += " (default: " + sm->default_str() + ")";

    std::string result;

    auto cur = std::begin(desc);
    auto stp = cur;
    auto lsp = cur;

    for (std::size_t size = 0; cur != std::end(desc); ++cur) {
        if (*cur == ' ')
            lsp = cur;

        if (++size > width) {
            if (lsp > stp) {
                result.append(stp, lsp);
                result.push_back('\n');
                result.append(indent, ' ');
                stp = lsp + 1;
            } else {
                result.append(stp, cur + 1);
                result.push_back('\n');
                result.append(indent, ' ');
                stp = cur + 1;
                lsp = stp;
            }
            size = 0;
        }
    }

    // append whatever is left
    result.append(stp, cur);

    return result;
}

inline option_adder& option_adder::operator()(const std::string& opts,
                 const std::shared_ptr<const value_semantic>& semantic,
                 const std::string& desc,
                 const std::string& arg, bool hide)
{
    owner_->add_option(std::make_shared<const option_description>(opts, semantic, desc, arg, hide));

    return *this;
}

inline std::size_t variables_map::count(const std::string& opt) const
{
    const auto& it = values_.find(strip_prefix(opt));
    if (it == values_.end())
        return 0;
    return it->second->count();
}

inline const variable_value& variables_map::operator[](const std::string& opt) const
{
    const auto& it = values_.find(strip_prefix(opt));
    if (it == values_.end())
        throw option_not_present(opt);
    if (it->second->count() == 0)
        throw option_not_present(opt);
    return *(it->second);
}

inline parsed_options parse_command_line(int argc, const char* const argv[],
               const options_description& desc,
               const positional_options_description& pos_desc
                   = positional_options_description())
{
    parsed_options results;

    for (const auto& opt : desc.options_) {
        std::shared_ptr<variable_value> value = opt->semantic->create_value();
        bool pos = false;

        if (opt->s.empty() && opt->l.empty()) {
            results.emplace(opt->s, value);
            pos = true;
        } else {
            if (! opt->s.empty()) {
                results.emplace(opt->s, value);
                pos |= pos_desc.contains(opt->s);
            }
            if (! opt->l.empty()) {
                results.emplace(opt->l, value);
                pos |= pos_desc.contains(opt->l);
            }
        }

        value->is_pos() = pos;
    }

    auto is_option = [](const char* arg) -> bool {
        /* starts  with '-' (and is not exactly "-" or "--") */
        if (arg[0] == '-')
            return (arg[1] && (arg[1] != '-' || arg[2]));

        return false;
    };

    auto find_variable_value = [&results](const std::string& opt) -> std::shared_ptr<variable_value> {
        const auto& it = results.find(opt);
        if (it == results.end())
            throw unknown_option(opt);

        return it->second;
    };

    auto get_next_arg = [&argc, &argv](const std::string& opt,
         const std::shared_ptr<const value_semantic>& semantic,
         int& idx) -> const char*
    {
        if (idx + 1 < argc && std::strcmp(argv[idx+1], "--") != 0) {
            if (!semantic->has_implicit())
                return argv[++idx];
            else if (semantic->try_parse(opt, argv[idx+1]))
                return argv[++idx];
            else
                return "";
        } else {
            if (semantic->has_implicit())
                return "";
            else
                throw missing_argument(opt);
        }
    };

    std::size_t pos_n = 0;

    auto consume_positional = [&](const char* arg) {
        const auto& opt = pos_desc.name_for_position(pos_n);
        const auto& value = find_variable_value(opt);
        const auto& semantic = value->semantic();

        if (! semantic->has_arg())
            throw superfluous_argument(opt, arg);

        semantic->parse(opt, arg, value);
        ++pos_n;
    };

    for (int ai = 1; ai < argc; ++ai) {
        if (strcmp(argv[ai], "--") == 0) {
            while (++ai < argc)
                consume_positional(argv[ai]);
            break;
        }

        /* is it an option? */
        if (is_option(argv[ai])) {
            if (argv[ai][1] == '-') { /* a long option */
                std::cmatch matches;
                regex_match(argv[ai], matches, long_option_matcher);

                if (matches.empty())
                    throw invalid_option_name(argv[ai]);

                std::string opt = std::move(matches[1]);
                std::string arg = std::move(matches[2]);

                const auto& value = find_variable_value(opt);
                const auto& semantic = value->semantic();

                if (! arg.empty() && ! semantic->has_arg())
                    throw superfluous_argument(opt, arg);

                if (arg.empty() && semantic->has_arg())
                    arg = std::string(get_next_arg(opt, semantic, ai));

                semantic->parse(opt, arg, value);
            } else { /* short option(s) */
                for (int i = 1; argv[ai][i]; ++i) {
                    const auto& opt = std::string(1, argv[ai][i]);
                    const auto& value = find_variable_value(opt);
                    const auto& semantic = value->semantic();

                    if (semantic->has_arg()) {
                        std::string arg;

                        if (argv[ai][i+1])
                            arg = std::string(&argv[ai][i+1]);
                        else
                            arg = std::string(get_next_arg(opt, semantic, ai));

                        semantic->parse(opt, arg, value);
                        break;
                    }

                    semantic->parse(opt, "", value);
                }
            }
        } else { /* not an option flag */
            consume_positional(argv[ai]);
        }
    }

    for (const auto& res : results)
        res.second->semantic()->parse(res.second);

    return results;
}

inline void store(parsed_options&& options, variables_map& vm) noexcept
{
    vm.values_ = std::move(options);
}

inline void notify(variables_map& vm)
{
    for (const auto& vp : vm.values_)
        if (vp.second->count() == 0 && vp.second->semantic()->is_required())
            throw missing_option(vp.first, vp.second->is_pos());

    for (const auto& vp : vm.values_)
        if (vp.second->count())
            vp.second->notify();
}

/*
 * The library doesn't throw this error. It leaves the user to decide.
 */
class leftover_arguments : public option_parse_error {
public:
    explicit leftover_arguments(const variable_value& argv)
        : option_parse_error("leftover positional arguments:")
    {
        for (const auto& a : argv.as<std::vector<std::string>>())
            message_ += " " + a;
    }
};

}
}

#endif
