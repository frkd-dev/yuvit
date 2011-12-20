/*
GetOpt_pp:  Yet another C++ version of getopt.
    Copyright (C) 2007, 2008, 2009, 2010  Daniel Gutson, FuDePAN

    This file is part of GetOpt_pp.

    GetOpt_pp is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    GetOpt_pp is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#if __APPLE__
extern char** environ;
#elif _WIN32
#include <Stdio.h>
#define environ _environ
#else
#include <unistd.h>
#endif

#include "getopt_pp.h"

namespace GetOpt
{

GETOPT_INLINE Token* GetOpt_pp::_add_token(const std::string& value, Token::Type type)
{
    Token* const ret = new Token(value, type);
    if (_first_token == NULL)
        _first_token = ret;
    else
        _last_token->link_to(ret);
    _last_token = ret;
    return ret;
}

GETOPT_INLINE void GetOpt_pp::_init_flags()
{
    std::stringstream ss;
    _flags = ss.flags();
}

GETOPT_INLINE void GetOpt_pp::_parse(int argc, const char* const* const argv)
{
    _app_name = argv[0];
    bool any_option_processed = false;

    // parse arguments by their '-' or '--':
    //   (this will be a state machine soon)
    for (int i = 1; i < argc; i++)
    {
        const char current = argv[i][0];
        const char next = argv[i][1];

        if (current == '-' && (isalnum(next) || next == '-'))
        {
            // see what's next, differentiate whether it's short or long:
            if (next == '-' && argv[i][2] != 0)
            {
                // long option
                _longOps[&argv[i][2]].token = _add_token(&argv[i][2], Token::LongOption);
                any_option_processed = true;
            }
            else
            {
                // short option
                // iterate over all of them, keeping the last one in currentData
                // (so the intermediates will generate 'existent' arguments, as of '-abc')
                size_t j = 1;
                do
                {
                    _shortOps[argv[i][j]].token = _add_token(std::string(&argv[i][j], 1), Token::ShortOption);
                    j++;
                }
                while (argv[i][j] != 0);

                any_option_processed = true;
            }
        }
        else
        {
            _add_token(argv[i], any_option_processed ? Token::UnknownYet : Token::GlobalArgument);
        }
    }

    _last = _Option::OK;    // TODO: IMPROVE!!
}

GETOPT_INLINE void GetOpt_pp::_parse_env()
{
    // this will be optimized in version 3
    std::string var_name;
    std::string var_value;
    size_t var = 0;
    std::string::size_type pos;
    OptionData* data;

    while (environ[var] != NULL)
    {
        var_name = environ[var];
        pos = var_name.find('=');

        if (pos != std::string::npos)
        {
            var_value = var_name.substr(pos + 1);
            var_name = var_name.substr(0, pos);

            if (_longOps.find(var_name) == _longOps.end())
            {
                data = &_longOps[var_name];
                data->token = _add_token(var_name, Token::LongOption);
                data->flags = OptionData::Envir;
                _add_token(var_value, Token::OptionArgument);
            }
        }
        else
            (data = &_longOps[var_name])->flags = OptionData::Envir;

        var++;
    }
}

GETOPT_INLINE GetOpt_pp::GetOpt_pp(int argc, const char* const* const argv)
    : _exc(std::ios_base::goodbit), _first_token(NULL), _last_token(NULL)
{
    _init_flags();
    _parse(argc, argv);
}

GETOPT_INLINE GetOpt_pp::GetOpt_pp(int argc, const char* const* const argv, _EnvTag)
    : _first_token(NULL), _last_token(NULL)
{
    _init_flags();
    _parse(argc, argv);
    _parse_env();
}

GETOPT_INLINE GetOpt_pp::~GetOpt_pp()
{
    Token* next;
    Token* current(_first_token);
    while (current != NULL)
    {
        next = current->next;
        delete current;
        current = next;
    }
}

GETOPT_INLINE GetOpt_pp& GetOpt_pp::operator >> (const _Option& opt) throw(GetOptEx)
{
    if (_last != _Option::ParsingError)
    {
        _last = opt(_shortOps, _longOps, _first_token, _flags);

        switch (_last)
        {
            case _Option::OK:
                break;

            case _Option::OptionNotFound:
                if (_exc & std::ios_base::eofbit)
                    throw OptionNotFoundEx();
                break;

            case _Option::BadType:
                if (_exc & std::ios_base::failbit)
                    throw InvalidFormatEx();
                break;

            case _Option::NoArgs:
                if (_exc & std::ios_base::eofbit)
                    throw ArgumentNotFoundEx();
                break;

            case _Option::TooManyArgs:
                if (_exc & std::ios_base::failbit)
                    throw TooManyArgumentsEx();
                break;

            case _Option::OptionNotFound_NoEx:
                break;  // Ok, it will be read by casting to bool

            case _Option::ParsingError:
                break;  // just to disable warning
        }
    }
    else if (_exc & std::ios_base::failbit)
        throw ParsingErrorEx();

    return *this;
}

GETOPT_INLINE GetOpt_pp& GetOpt_pp::operator >> (std::ios_base & (*iomanip)(std::ios_base&))
{
    std::stringstream ss;
    ss.flags(_flags);
    _flags = (ss << iomanip).flags();
    return *this;
}

GETOPT_INLINE bool GetOpt_pp::options_remain() const
{
    bool remain = false;
    ShortOptions::const_iterator it = _shortOps.begin();
    while (it != _shortOps.end() && !remain)
    {
        remain = (it->second.flags == OptionData::CmdLine_NotExtracted);
        ++it;
    }

    if (!remain)
    {
        LongOptions::const_iterator it = _longOps.begin();
        while (it != _longOps.end() && !remain)
        {
            remain = (it->second.flags == OptionData::CmdLine_NotExtracted);
            ++it;
        }
    }

    if (!remain)
    {
        // check for the global arguments:
        Token* token = _first_token;
        while (!remain && token != NULL)
        {
            remain = (token->type == Token::GlobalArgument || token->type == Token::UnknownYet);
            token = token->next;
        }
    }

    return remain;
}

}
