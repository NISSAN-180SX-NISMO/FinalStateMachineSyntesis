#include <set>
#include <unordered_set>
#include <set>
#include <cstdint>
#include <map>
#include <fstream>
#include "iostream"
#include "vector"
#include "string"

std::ofstream fout;
std::string readRegexIn()
{
    std::ifstream inputFile("../regex.in");
    if (!inputFile) {
        return "";
    }

    std::string line;
    while (std::getline(inputFile, line)) {

    }

    inputFile.close();
    fout.open("../regex.out");
    return line;
}

class chuset : public std::unordered_set<char>
{
public:
    bool have (char _)
    {
        return this->find(_) != this->end();
    }

    chuset()
    {}
    chuset(const std::string& chs)
    {
        for (auto const& ch : chs)
            this->insert(ch);
    }
};

struct place
{
    enum type_t
    {
        master,
        slave
    };
    unsigned index;
    type_t type;
    char ch = '!';
    place set_type(type_t type)
    {
        place newplace = *this;
        newplace.type = type;
        return newplace;
    }
};

void title(const std::string& _, bool __ = true )
{
    if (__) fout << "\n\n\n";
    fout << " ==[ " << _  << " ]=" << "\n\n";
}


int main()
{
//    std::string regex = "n<a|b|c>(k|m)<z><x>f";
    std::string regex = readRegexIn();

    title("Source regex", false);
    fout << regex << std:: endl;

    chuset alphabet = [](const std::string& input)
    {
        chuset output;
        static chuset sysch {"<>()|"};
        for (int i = input.size(); i >= 0; --i)
            if (!sysch.have(input[i]))
                output.insert(input[i]);
        return output;
    }(regex);

    std::string spaced_regex = [](const std::string& input)
    {
        std::string output;
        for (const auto& ch : input)
            output += std::string(" ") + ch;
        return output + std::string(" ");
    }(regex);
#define ch(x) spaced_regex[x]

    std::vector<std::vector<place>> places = [&alphabet](const std::string& input) // set main places
    {
        unsigned i = 0;
        std::vector<std::vector<place>> output;
        output.push_back(std::vector<place> { { i, place::type_t::slave } } );
        for (const auto& ch : input)
            if (alphabet.have(ch))
                output.push_back(std::vector<place> { { ++i, place::type_t::master, ch } } );
            else output.push_back({});
        return output;
    }(spaced_regex);


    for (size_t i = 0; i < spaced_regex.size();)
    {
        if (ch(i) == '(') // rule 1 or 2
        {
            // rule 1:
            auto main_place = places[i - 1];
            if (! main_place.empty()) main_place[0].set_type(place::type_t::slave);
            std::vector<place> after_term;
            for (; ch(i)  != ')';)
                if (alphabet.have(ch(++i))) {
                    places[i - 1] = main_place;
                    after_term.push_back(places[i + 1][0].set_type(place::type_t::slave));
                }
            // rule 2
            for (auto& place : after_term) place.type = place::type_t::slave;
            places[i + 1] = after_term;
        }
        else if (ch(++i)  == '<') // rule 2 or 3
        {
            // rule 1:
            auto main_place = places[i - 1];
            if (! main_place.empty()) main_place[0].set_type(place::type_t::slave);
            std::vector<place> after_term = {main_place};
            for (; ch(i)  != '>';)
                if (alphabet.have(ch(++i))) {
                    places[i - 1] = main_place;
                    after_term.push_back(places[i + 1][0].set_type(place::type_t::slave));
                }

            // rule 2
            for (auto& place : after_term) place.type = place::type_t::slave;
            places[i + 1] = after_term;

            // rule 3:
            for (size_t bi = i; ch(bi)  != '<';)
                if (alphabet.have(ch(--bi))) {
                    places[bi - 1] = after_term;
                }
        }
    }


    auto empty = [](const decltype(places)& vec)
    {
        for (auto subvec : vec)
            if (subvec.size() > 0)
                return false;
        return true;
    };

    auto pcopy = places;

    title("Places indecies");
    fout << spaced_regex << std::endl;


    auto print_places = [&](){
        while (!empty(pcopy)) {
            for (auto &vec : pcopy) {
                if (vec.empty())
                    fout << " ";
                else {
                    fout << vec.front().index;
                    vec.erase(vec.begin(), vec.begin() + 1);
                }

            }
            fout << std::endl;
        }
    };

    print_places();

    // matrix X / Q
    std::map<char, std::vector<signed>> XQ = [&alphabet](){
        std::map<char, std::vector<signed>> xq;
        for (auto ch : alphabet)
            xq.insert({ch, std::vector<signed>(alphabet.size(), -1)});
        return xq;
    }();

    title("Preparatory matrix");
    auto printXQ = [&alphabet, &XQ] (){
        fout << "   ";
        for (size_t i = 0; i < alphabet.size(); ++i)
            fout << i << "  ";
        fout << std::endl;


        for (auto ch : alphabet) {
            fout << ch << "  ";
            for (auto place : XQ[ch]) {
                if (place == -1) fout << "-";
                else fout << place;
                fout << "  ";
            }
            fout << std::endl;
        }
    };

    for (auto ch : alphabet)
        for (size_t i = 0; i < places.size(); ++i)
            if (!places[i].empty())
                if (places[i][0].type == place::type_t::master)
                    if (places[i][0].ch == ch && ch != ' ')
                        for (auto place : places[i - 2])
                            XQ[ch][place.index] = places[i][0].index;


    printXQ();

#define q(x) "q"  + std::to_string(x)
    std::map<std::string, std::map<char, std::string>> transition_matrix = [&alphabet]()
    {
        std::map<std::string, std::map<char, std::string>> transition_matrix;
        for (int i = 0; i < alphabet.size(); ++i)
            transition_matrix[q(i)] = [&]()
            {
                std::map<char, std::string> map;
                for (auto ch : alphabet)
                    map[ch] = " -- ";
                return map;
            }();
        return transition_matrix;
    }();

    title("Transition matrix");
    auto print_transition_matrix = [&transition_matrix, &alphabet]()
    {
        fout << "     ";
        for (auto ch : alphabet)
            fout << ch << "     ";
        fout << std::endl;


        for (auto pair : transition_matrix) {
            fout << pair.first << "  ";
            for (auto ch : alphabet) {
                fout << pair.second[ch] << "  ";
            }
            fout << std::endl;
        }
    };

    for (auto ch : alphabet)
        for (int i = 0; i < XQ[ch].size(); ++i)
            if (XQ[ch][i] != -1)
                transition_matrix[q(i)][ch] = q(XQ[ch][i]) + "/" + (XQ[ch][i] == XQ[ch].size() - 1 ? "1" : "0");

    print_transition_matrix();



    std::map<std::string, std::map<std::string, std::string>> automaton_matrix = [&alphabet]()
    {
        std::map<std::string, std::map<std::string, std::string>> automaton_matrix;
        for (int i = 0; i < alphabet.size(); ++i)
            automaton_matrix[q(i)] = [&]()
            {
                std::map<std::string, std::string> map;
                for (int i = 0; i < alphabet.size(); ++i)
                    map[q(i)] = " - ";
                return map;
            }();
        return automaton_matrix;
    }();

    title("Automaton matrix");
    auto print_automaton_matrix = [&automaton_matrix, &alphabet]()
    {
        fout << "     ";
        for (auto pair : automaton_matrix)
            fout << pair.first << "   ";
        fout << std::endl;


        for (auto pair : automaton_matrix) {
            fout << pair.first << "  ";
            for (auto subpair : pair.second) {
                fout << subpair.second << "  ";
            }
            fout << std::endl;
        }
    };

    for (auto ch : alphabet)
        for (int i = 0; i < XQ[ch].size(); ++i)
            if (XQ[ch][i] != -1)
                automaton_matrix[q(i)][q(XQ[ch][i]) ] = std::string() + ch + "/" + (XQ[ch][i] == XQ[ch].size() - 1 ? "1" : "0");

    print_automaton_matrix();

    fout.close();
    return 0;
}