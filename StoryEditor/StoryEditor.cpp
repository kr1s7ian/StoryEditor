#include <iostream>
#include <windows.h>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <fstream>
#include <ctype.h>


struct choice;
struct scenario;
std::map<int, scenario> scenarios;
std::vector<std::string> lines;
std::map<int, scenario> cachedScenarios;

std::vector<std::string> Tokenize(const std::string& str) {
    std::cout << str << "\n";
    std::vector<std::string> tokens;
    int count = 0;
    for (const auto c : str) {
        if (c == '"') {
            count++;
        }
    }

    // if count is not even
    if (count % 2 != 0) {
        std::cout << "input is not even!" << "\n";
        return tokens;
    }

    std::string token;
    int count2 = 0;
    for (const auto& c : str) {
        if (c != '"') {
            if (count2 % 2 != 0) {
                token += c;
            }
        }
        else {
            // if " amount is even then the line is correctly formatted and can be pushed into tokens, after that we clear the token string
            if (count2 % 2 != 0) {
                std::cout << "loading: " << token << "\n";
                tokens.push_back(token);
                token = "";
            }
            count2++;
        }
    }

    return tokens;
}

struct choice {
    std::string name = "null";
    int consequence = -1;

    choice(std::string name_, int consequence_) {
        name = name_;
        consequence = consequence_;
    }
};

struct scenario {
    int id = -1;
    std::string description = "null";
    std::vector<choice> choices;


    scenario(int id_, std::string desc_, std::initializer_list<choice> choice_) {
        id = id_;
        description = desc_;
        for (auto& choice : choice_) {
            choices.push_back(choice);
        }
    }
};

scenario stringToScenario(const std::string& str) {
    std::vector<std::string> tokens = Tokenize(str);
    scenario scn(stoi(tokens[0]), tokens[1], {});
    // only if tokens size is greater than 2 then it has choices so if its less than 2 we dont need to load them.
    if (tokens.size() > 2) {
        for (int i = 2; i < tokens.size(); i += 2) {
            scn.choices.push_back(choice(tokens[i], stoi(tokens[i + 1])));
        }
    }

    return scn;
}

std::string scenarioToString(const scenario& scn) {
    std::string str;
    str += "\"" + std::to_string(scn.id) + "\" ";
    str += "\"" + scn.description + "\"";

    for (const auto& choice : scn.choices) {
        str += " \"" + choice.name + "\" " + "\"" + std::to_string(choice.consequence) + "\"";
    }

    str += ";";

    return str;
}


std::vector<std::string> loadFile() {
    std::vector<std::string> lines;
    std::string line;

    std::fstream file("mystory.txt");
    while (std::getline(file, line, ';')) {
        lines.push_back(line);
        //std::cout << line << "\n";
    }
    file.close();
    return lines;
}

void saveFile() {
    std::ofstream file("mystory.txt");
    std::string line;

    for (const auto& scn : cachedScenarios) {
        line = scenarioToString(scn.second);
        file << "\n" << line;
    }
    file.close();
}

std::map<int, scenario> cacheScenarios(const std::vector<std::string>& lines) {
    std::map<int, scenario> cached_scenarios;

    for (const auto& line : lines) {
        scenario scn = stringToScenario(line);
        cached_scenarios.insert(std::pair<int, scenario>(scn.id, scn));
    }

    return cached_scenarios;
}

void runScenario(int id);

int generateRandomUniqueId();

void liveEditor(const std::string& str, const int id) {
    std::vector<std::string> tokens = Tokenize(str);
    scenario& scn = cachedScenarios.at(id);

    int r = generateRandomUniqueId();

    if (stoi(tokens[0]) == 0) {
        scn.description = tokens[1];
    }
    else if (stoi(tokens[0]) > scn.choices.size()) {
        std::cout << "creating new dialogue";
        scenario newScn(r, "New Dialogue", { choice("(Go back)", scn.id) });
        cachedScenarios.insert(std::pair<int, scenario>(r, newScn));
        scn.choices.push_back(choice(tokens[1], r));
        scn.choices[stoi(tokens[0]) - 1].consequence = r;
    }
    else {
        scn.choices[stoi(tokens[0]) - 1].name = tokens[1];
        // scn.choices[stoi(tokens[0])-1].consequence = r;
    }

    saveFile();
    runScenario(id);
}

bool isNumber(const std::string& str) {
    for (const char& c : str) {
        if (isalpha(c)) {
            return false;
        }
    }

    return true;
}

bool validateInput(const std::string& str, int max) {
    bool result = true;

    if (str[0] == '-') {
        return false;
    }

    //check if input contains only spaces
    if (str.find_first_not_of(' ') == std::string::npos)
    {
        return false;
    }


    if (isNumber(str)) {
        int n = stoi(str);
        std::cout << n << "\n";
        std::cout << max << "\n";
        if (n > max || n < 0) {
            std::cout << "invalid input, choice overflow" << "\n";
            result = false;
        }
    }
    else if (str[0] != '"') {
        result = false;
    }

    return result;
}

void runScenario(int id) {
    scenario& scn = cachedScenarios.at(id);
    system("cls");
    std::cout << scn.description << "\n";
    std::cout << "\n";
    int n = 1;
    for (const auto& choice : scn.choices) {
        std::cout << n++ << "] ";
        std::cout << choice.name << "\n";
    }
    // remember to -1 index because its one based indexing and not zero based
    std::string input;
    std::getline(std::cin, input);
    if (validateInput(input, scn.choices.size())) {
        if (input[0] == '"') {
            liveEditor(input, id);
        }
        else {
            runScenario(scn.choices[stoi(input) - 1].consequence);
        }
    }
    else {
        //if input was not valid re run the current scenario
        runScenario(id);
    }
}

int generateRandomUniqueId() {
    int r;
    r = rand() % 99999;
    if (cachedScenarios.find(r) != cachedScenarios.end()) {
        return generateRandomUniqueId();
    }
    else {
        return r;
    }
}


int main()
{
    lines = loadFile();
    cachedScenarios = cacheScenarios(lines);
    runScenario(0);
}