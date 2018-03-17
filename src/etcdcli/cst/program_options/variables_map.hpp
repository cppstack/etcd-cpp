#ifndef _CST_PROGRAM_OPTIONS_VARIABLES_MAP_HPP
#define _CST_PROGRAM_OPTIONS_VARIABLES_MAP_HPP

#include <cst/program_options/errors.hpp>
#include <cst/program_options/value_semantic.hpp>
#include <cst/program_options/options_description.hpp>

namespace cst {
namespace program_options {

struct parsed_options {
    typedef std::map<std::string, std::shared_ptr<variable_value>>
            recognized_type;
    typedef std::vector<std::string> unrecognized_type;

    recognized_type recognized_options;
    unrecognized_type unrecognized_options;
};

class variables_map {
    friend void store(const parsed_options& parsed, variables_map& vm);
    friend void notify(variables_map& vm);

public:
    std::size_t count(const std::string& opt) const
    {
        const auto& it = values_.recognized_options.find(strip_prefix(opt));
        if (it != values_.recognized_options.end())
            return it->second->count();

        if (std::find(values_.unrecognized_options.cbegin(),
                      values_.unrecognized_options.cend(),
                      with_prefix(opt))
            != values_.unrecognized_options.cend())
        {
            throw unrecognized_option(opt);
        }

        return 0;
    }

    const variable_value& operator[](const std::string& opt) const
    {
        const auto& it = values_.recognized_options.find(strip_prefix(opt));
        if (it != values_.recognized_options.end()) {
            if (it->second->count() == 0)
                throw option_not_present(opt);
            else
                return *(it->second);
        }

        if (std::find(values_.unrecognized_options.cbegin(),
                      values_.unrecognized_options.cend(),
                      with_prefix(opt))
            != values_.unrecognized_options.cend())
        {
            throw unrecognized_option(opt);
        }

        throw option_not_present(opt);
    }

private:
    parsed_options values_;
};

inline void store(const parsed_options& parsed, variables_map& vm)
{
    for (const auto& p : parsed.recognized_options)
        vm.values_.recognized_options[p.first] = p.second;

    vm.values_.unrecognized_options = parsed.unrecognized_options;
}

inline void notify(variables_map& vm)
{
    for (const auto& vp : vm.values_.recognized_options)
        if (vp.second->count() == 0 && vp.second->semantic()->is_required())
            throw missing_option(vp.first, vp.second->is_pos());

    for (const auto& vp : vm.values_.recognized_options)
        if (vp.second->count())
            vp.second->notify();
}

}
}

#endif
