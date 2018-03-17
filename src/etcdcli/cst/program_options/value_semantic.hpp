#ifndef _CST_PROGRAM_OPTIONS_VALUE_SEMANTIC_HPP
#define _CST_PROGRAM_OPTIONS_VALUE_SEMANTIC_HPP

#include <vector>
#include <sstream>
#include <functional>

#include <cst/program_options/errors.hpp>

namespace cst {
namespace program_options {

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

    virtual ~value_semantic() = default;
};

class variable_value {
public:
    explicit variable_value(const std::shared_ptr<const value_semantic>& semantic) noexcept
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
    void* value_ = nullptr;
    bool is_pos_ = false;
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

}
}

#endif
