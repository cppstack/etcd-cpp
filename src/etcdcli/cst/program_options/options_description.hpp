#ifndef _CST_PROGRAM_OPTIONS_OPTIONS_DESCRIPTION_HPP
#define _CST_PROGRAM_OPTIONS_OPTIONS_DESCRIPTION_HPP

#include <map>
#include <set>
#include <vector>
#include <sstream>
#include <regex>

#include <cst/program_options/errors.hpp>
#include <cst/program_options/value_semantic.hpp>

namespace cst {
namespace program_options {

inline const std::string strip_prefix(const std::string& opt)
{
    std::size_t i = 0;
    while (i < 2 && i < opt.length() && opt[i] == '-')
        ++i;
    return i > 0 ? opt.substr(i) : opt;
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
                       bool hide_)
        : semantic(sm_), desc(desc_), arg(arg_), hide(hide_)
    {
        std::smatch matches;
        regex_match(opts_, matches, specifier_);

        l = std::move(matches[1]);
        s = std::move(matches[2]);

        if (s.empty() && l.empty())
            throw bad_option_spec(opts_);
    }

    const std::regex specifier_{
        "([[:alnum:]][-_[:alnum:]]+)?(?:,([[:alnum:]]))?"};

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
    explicit option_adder(options_description* owner) noexcept
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
    positional_options_description& add(const std::string& name, int max_count)
    {
        if (max_count < 0)
            max_count = (~0U);

        if (max_total_count_ +  max_count > max_total_count_)
            max_total_count_ += max_count;

        set_.insert(name);

        positionals_.emplace_back(strip_prefix(name), (unsigned) max_count);

        return *this;
    }

    const std::string& name_for_position(unsigned pos) const
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

    bool contains(const std::string& name) const
    { return set_.count(name); }

private:
    const std::string empty_name_ = "";
    std::vector<std::pair<std::string, unsigned>> positionals_;
    std::set<std::string> set_;
    unsigned max_total_count_ = 0;
};

struct parsed_options;

class options_description {
    friend class command_line_parser;
    friend parsed_options parse_command_line(int argc, const char* const argv[],
                          const options_description& desc,
                          const positional_options_description& pos_desc);

    friend std::ostream& operator<<(std::ostream& os, const options_description& desc);

public:
    explicit options_description(const std::string& caption = "")
        : caption_(caption)
    {
        std::shared_ptr<const option_description> unknown_argvs_desc
            = std::make_shared<const option_description>();

        options_.push_back(unknown_argvs_desc);
    }

    option_adder add_options()
    { return option_adder(this); }

    void add_option(const std::shared_ptr<const option_description>& desc)
    {
        const auto& s = desc->s;
        const auto& l = desc->l;

        if (!s.empty() && optset_.count(s))
            throw duplicate_option(s);

        if (!l.empty() && optset_.count(l))
            throw duplicate_option(l);

        options_.push_back(desc);
    }

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
    owner_->add_option(std::make_shared<const option_description>(
        opts, semantic, desc, arg, hide));

    return *this;
}

}
}

#endif
