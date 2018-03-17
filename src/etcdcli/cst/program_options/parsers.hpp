#ifndef _CST_PROGRAM_OPTIONS_PARSERS_HPP
#define _CST_PROGRAM_OPTIONS_PARSERS_HPP

#include <regex>

#include <cst/program_options/errors.hpp>
#include <cst/program_options/value_semantic.hpp>
#include <cst/program_options/options_description.hpp>
#include <cst/program_options/variables_map.hpp>

namespace cst {
namespace program_options {

class command_line_parser {
public:
    command_line_parser(int argc, const char *const *argv)
    {
        for (int i = 1; i < argc; ++i)
            args_.emplace_back(argv[i]);
    }

    explicit command_line_parser(const std::vector<std::string>& args)
        : args_(args)
    { }

    command_line_parser& options(const options_description& desc) noexcept
    {
        desc_ = &desc;
        return *this;
    }

    command_line_parser& positional(
        const positional_options_description& desc) noexcept
    {
        pos_desc_ = &desc;
        return *this;
    }

    command_line_parser& allow_unregistered() noexcept
    {
        allow_unregistered_ = true;
        return *this;
    }

    parsed_options run();

private:
    const std::regex long_option_matcher_{
        "--([[:alnum:]][-_[:alnum:]]+)(?:=(.*))?"};

    std::vector<std::string> args_;
    options_description dummy_desc_;
    positional_options_description dummy_pos_desc_;
    const options_description* desc_ = &dummy_desc_;
    const positional_options_description* pos_desc_ = &dummy_pos_desc_;
    bool allow_unregistered_ = false;
};

inline parsed_options command_line_parser::run()
{
    parsed_options results;

    for (const auto& opt : desc_->options_) {
        std::shared_ptr<variable_value> value = opt->semantic->create_value();
        bool pos = false;

        if (opt->s.empty() && opt->l.empty()) {
            results.recognized_options.emplace(opt->s, value);
            pos = true;
        } else {
            if (!opt->s.empty()) {
                results.recognized_options.emplace(opt->s, value);
                pos |= pos_desc_->contains(opt->s);
            }
            if (!opt->l.empty()) {
                results.recognized_options.emplace(opt->l, value);
                pos |= pos_desc_->contains(opt->l);
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

    auto find_variable_value = [&results](const std::string& opt)
        -> std::shared_ptr<variable_value>
    {
        const auto& it = results.recognized_options.find(opt);
        return it != results.recognized_options.end() ? it->second : nullptr;
    };

    auto get_next_arg = [this](const std::string& opt,
         const std::shared_ptr<const value_semantic>& semantic,
         std::vector<std::string>::const_iterator& it) -> std::string
    {
        if (it + 1 != args_.end() && *(it+1) != "--") {
            if (!semantic->has_implicit())
                return *++it;
            else if (semantic->try_parse(opt, *(it+1)))
                return *++it;
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

    auto consume_positional = [&](const std::string& arg) {
        const auto& opt = pos_desc_->name_for_position(pos_n++);
        if (opt.empty() && allow_unregistered_) {
            results.unrecognized_options.push_back(arg);
            return;
        }

        const auto& value = find_variable_value(opt);
        const auto& semantic = value->semantic();

        if (!semantic->has_arg())
            throw superfluous_argument(opt, arg);

        semantic->parse(opt, arg, value);
    };

    for (auto it = args_.cbegin(); it != args_.cend(); ++it) {
        if (*it == "--") {
            while (++it != args_.cend())
                consume_positional(*it);
            break;
        }

        const char* argv = it->c_str();

        if (is_option(argv)) {
            if (argv[1] == '-') { /* a long option */
                std::cmatch matches;
                regex_match(argv, matches, long_option_matcher_);

                if (matches.empty())
                    throw invalid_option_name(*it);

                std::string opt = std::move(matches[1]);
                std::string arg = std::move(matches[2]);

                const auto& value = find_variable_value(opt);
                if (!value) {
                    if (!allow_unregistered_)
                        throw unknown_option(opt);
                    results.unrecognized_options.emplace_back(argv);
                    continue;
                }

                const auto& semantic = value->semantic();

                if (!arg.empty() && !semantic->has_arg())
                    throw superfluous_argument(opt, arg);

                if (arg.empty() && semantic->has_arg())
                    arg = get_next_arg(opt, semantic, it);

                semantic->parse(opt, arg, value);

            } else { /* short option(s) */
                for (int i = 1; argv[i]; ++i) {
                    const auto& opt = std::string(1, argv[i]);
                    const auto& value = find_variable_value(opt);
                    if (!value) {
                        if (!allow_unregistered_)
                            throw unknown_option(opt);
                        results.unrecognized_options.push_back("-" + opt);
                        continue;
                    }

                    const auto& semantic = value->semantic();

                    if (semantic->has_arg()) {
                        std::string arg = argv[i+1] ? std::string(&argv[i+1])
                                        : get_next_arg(opt, semantic, it);

                        semantic->parse(opt, arg, value);
                        break;
                    }

                    semantic->parse(opt, "", value);
                }
            }
        } else /* not an option flag */
            consume_positional(*it);
    }

    for (const auto& res : results.recognized_options)
        res.second->semantic()->parse(res.second);

    return results;
}

inline parsed_options parse_command_line(int argc, const char* const argv[],
                                         const options_description& desc)
{
    return command_line_parser(argc, argv).options(desc).run();
}

inline parsed_options parse_command_line(int argc, const char* const argv[],
                                         const options_description& desc,
                              const positional_options_description& pos_desc)
{
    return command_line_parser(argc, argv).options(desc)
                                          .positional(pos_desc)
                                          .run();
}

}
}

#endif
