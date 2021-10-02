// GameBoyCrossMacroAssembler.cpp : This file contains the 'main' function. Program execution begins and ends there.
// github.com/darkfoxtokoyami

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstddef>

#define DEBUG_MESSAGES
#define MSG(a) std::cout<<a<<"\n";
#ifndef DEBUG_MESSAGES
#undef MSG(a)
#define MSG(a)
#endif

#define WARN(a) if (g_WarningsEnabled) std::cout<<"[WARNING] " <<a<<"\n";

bool g_WarningsEnabled = true;
bool ends_with(std::string const& value, std::string const& ending);
std::vector<std::string> ParseArguments(int argc, const char** argv);
std::vector<std::string> ParseRawASMText(std::vector<std::string> ASMFiles);
std::vector<std::string> ParseASMLines(std::vector<std::string> lines);
std::vector<std::vector<std::string>> Tokenize(std::vector<std::string> lines);
std::vector<std::vector<std::string>> PreProcessor(std::vector<std::vector<std::string>> tokens);

struct SECTION
{
    unsigned short address;
    std::byte* data;
};

//std::vector<std::string> RawASMText;
int main(int argc, const char** argv)
{
    PreProcessor(Tokenize(ParseRawASMText(ParseArguments(argc, argv))));
    std::cin.get();
}

std::vector<std::string> ParseArguments(int argc, const char** argv)
{
    std::vector<std::string> asm_files;
    for (int i = 0; i < argc; i++)
    {
        if (ends_with(argv[i], ".asm"))
            //std::cout << argv[i]<<"\n";            
            asm_files.push_back(argv[i]);
    }

    //for (int i = 0; i < asm_files.size(); i++)
    //{
    //    std::cout << asm_files[i] << "\n";
    //}

    return asm_files;
}
/// <summary>
/// Open .asm files in ASMFiles; Parse files with parseASMLines
/// </summary>
std::vector<std::string> ParseRawASMText(std::vector<std::string> ASMFiles)
{
    std::vector<std::string> return_lines;
    for (int i = 0; i < ASMFiles.size(); i++)
    {
        std::ifstream file;
        std::string line;
        std::vector<std::string> lines;
        
        try
        {
            file.open(ASMFiles[i]);
            while (std::getline(file, line))
            {
                lines.push_back(line);
            }
            file.close();

            //std::cout << ASMFiles[i] << "\n";
            MSG(ASMFiles[i]);
            lines = ParseASMLines(lines);
            for (int j = 0; j < lines.size(); j++)
            {
                return_lines.push_back(lines[j]);
                MSG(lines[j]);
                //std::cout << lines[j] << "\n";
            }
        }
        catch (const std::ifstream::failure& e)
        {
            std::cout << "Error opening " << ASMFiles[i] << "\n";
        }
    }

    return return_lines;
}
/// <summary>
/// Strips out all the excess white space, removes comments and newline characters
/// </summary>
/// <param name="lines">Raw assembly file, with one string per line</param>
/// <returns>Parsed assembly file, with TLAs/Tokens separated by singular spaces</returns>
std::vector<std::string> ParseASMLines(std::vector<std::string> lines)
{
    std::vector<std::string> return_lines;
    for (int i = 0; i < lines.size(); i++)
    {
        bool contains_characters = false;
        bool semicolon = false;
        bool leading_whitespace_handled = false;
        bool whitespace = false;
        bool double_quote = false;
        bool single_quote = false;
        bool backslash = false;
        bool comment = false;
        std::string line = lines[i];
        std::string return_line;

        for (int j = 0; j < line.size(); j++)
        {
            if (comment)
                break;

            char c = line[j];

            switch (c)
            {
            case '\t':
            case ' ':
                if (leading_whitespace_handled)
                {
                    whitespace = true;
                }
                else
                {

                }
                break;
            
            case '\f':
            case '\r':
            case '\n':
                break;
            case '\'':
            case '"':
                if (c == '\'')
                {
                    if (single_quote)
                        single_quote = false;
                    else
                        single_quote = true;
                }
                else
                {
                    if (double_quote)
                        double_quote = false;
                    else
                        double_quote = true;
                }
            default:
                if (!leading_whitespace_handled)
                    leading_whitespace_handled = true;
                if (c == ';' && !double_quote && !single_quote)
                {
                    comment = true;
                    break;
                }
                if (c == ',' && !double_quote && !single_quote)
                    break;

                if (whitespace)
                {
                    return_line += ' ';
                    whitespace = false;
                }
                contains_characters = true;
                return_line += c;
                break;
            }
        }
        if (contains_characters)
            return_lines.push_back(return_line);
    }

    return return_lines;
}

std::vector<std::vector<std::string>> Tokenize(std::vector<std::string> lines)
{
// I know this is messy as fuck, but I use this in like 3 places and manually adding this in 3 times is a delicate snowflake thing to do. 
// I #undef this at the end of the function, so don't worry
#define PUSH_BACK_TOKEN     \
if (!token.empty())         \
{                           \
    pushed_back = true;     \
    tokens.push_back(token);\
    token = "";             \
}
    std::vector<std::vector<std::string>> return_tokens;
    for (int i = 0; i < lines.size(); i++)
    {
        bool contains_characters = false;
        bool semicolon = false;
        bool whitespace = false;
        bool double_quote = false;
        bool single_quote = false;
        bool backslash = false;
        bool comment = false;
        bool pushed_back = false;

        std::string line = lines[i];
        std::string token;
        std::vector<std::string> tokens;

        for (int j = 0; j < line.size(); j++)
        {
            pushed_back = false;
            char c = line[j];

            switch (c)
            {
            case '\t':
            case ' ':
                PUSH_BACK_TOKEN
                break;

            case '\f':
            case '\r':
            case '\n':
                break;
            case '\'':
            case '"':
                if (c == '\'')
                {
                    token += c;
                    if (single_quote)
                    {
                        single_quote = false;                        
                        PUSH_BACK_TOKEN
                    }
                    else
                        single_quote = true;
                }
                else
                {
                    token += c;
                    if (double_quote)
                    {
                        double_quote = false;
                        PUSH_BACK_TOKEN
                    }
                    else
                        double_quote = true;
                }
                break;
            default:
                contains_characters = true;
                token += c;
                break;
            }            
        }
        if (!pushed_back && !token.empty())
            tokens.push_back(token);
        if (contains_characters)
            return_tokens.push_back(tokens);
        //for (int i = 0; i < tokens.size(); i++)
            //MSG(tokens[i]);
            //return_lines.push_back(return_line);
    }
#ifdef DEBUG_MESSAGES
    for (int i = 0; i < return_tokens.size(); i++)
    {
        for (int j = 0; j < return_tokens[i].size(); j++)
        {
            std::cout<<return_tokens[i][j]<<" ";
        }

        std::cout << "\n";
    }
#endif
#undef PUSH_BACK_TOKEN
    return return_tokens;
}

std::vector<std::vector<std::string>> PreProcessor(std::vector<std::vector<std::string>> tokens)
{
    std::vector<std::vector<std::string>> return_tokens;
    bool restart = true;
    while (restart)
    {
        restart = false;
        //Handle Includes
        for (int i = 0; i < tokens.size(); i++)
        {
            std::vector<std::vector<std::string>> t_tokens;
            if (tokens[i].size() == 2)
            {
                if (!tokens[i][0].compare("INCLUDE") || !tokens[i][0].compare("#include"))
                {
                    MSG("Found an INCLUDE!");
                    std::vector<std::string> asm_files;

                    //asm_files.push_back(tokens[i][1]);
                    asm_files.push_back("hardware.inc");
                    std::vector<std::vector<std::string>> header = Tokenize(ParseRawASMText(asm_files));
                    if (i != 0)
                    {

                    }
                    else
                    {
                        t_tokens = header;
                        t_tokens.insert(t_tokens.end(), std::next(tokens.begin(), 1), tokens.end());
                    }
                    //for (int j = 0; j < header.size(); j++)
                    //    t_tokens.push_back(header[j]);

                    //for (int j = 0; j < tokens.size(); j++)
                    //    t_tokens.push_back(tokens[j]);
                    tokens = t_tokens;
#ifdef DEBUG_MESSAGES
                    MSG("\n\n PreProcessor HEADER");
                    for (int i = 0; i < tokens.size(); i++)
                    {
                        for (int j = 0; j < tokens[i].size(); j++)
                        {
                            std::cout << tokens[i][j] << " ";
                        }

                        std::cout << "\n";
                    }
#endif
                    restart = true;
                    break;
                }
            }
        }
    }

    return return_tokens;
}

/*
bool starts_with(std::string const& value, std::string const& starting)
{
    if (starting.size() > value.size()) return false;
        return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}*/

bool ends_with(std::string const& value, std::string const& ending)
{    
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
