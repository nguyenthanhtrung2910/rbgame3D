#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

const std::unordered_map<std::string, unsigned int> COLOR2INT
{
    {"R", 0},
    {"B", 1},
    {"G", 2}
};

enum class Orientation
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
};

std::ostream& operator<<(std::ostream& os, Orientation o) 
{
    switch (o) {
        case Orientation::UP:   return os << "UP";
        case Orientation::DOWN: return os << "DOWN";
        case Orientation::LEFT:  return os << "LEFT";
        case Orientation::RIGHT:  return os << "RIGHT";
    }
    return os << "Unknown";
}

std::vector<std::pair<unsigned int, Orientation>> prossessLogFile(const std::string& logFile)
{
    std::vector<std::pair<unsigned int, Orientation>> robot_moves;
    std::ifstream file(logFile);
    if (!file.is_open()) {
        std::cerr << "Failed to open file.\n";
        exit(1);
    }
    std::regex pattern(R"(([RBG]) robot (\d) go ([DULR]))");
    std::string line;
    std::smatch match;
    std::getline(file, line);
    std::regex_search(line, match, std::regex{R"(game starts with (\d) number robots per player)"});
    int numRobotsPerPlayer = std::stoi(match[1]);
    while (std::getline(file, line)) {
        if (std::regex_search(line, match, pattern)) {
            unsigned int index = numRobotsPerPlayer*COLOR2INT.at(match[1]) + std::stoi(match[2]) - 1;
            if (match[3] == "L") robot_moves.emplace_back(index, Orientation::LEFT);
            if (match[3] == "R") robot_moves.emplace_back(index, Orientation::RIGHT);
            if (match[3] == "U") robot_moves.emplace_back(index, Orientation::UP);
            if (match[3] == "D") robot_moves.emplace_back(index, Orientation::DOWN);
        }
    }
    file.close();
    return robot_moves;
}


int main(int argc, char** argv)
{
    int time{0};
    std::vector<std::pair<unsigned int, Orientation>> commands{prossessLogFile(argv[1])};
    for (const auto& command: commands)
    {
        std::cout << "At t=" << time << " robot " << command.first << " : " << command.second << "\n";
        time += 1;
    }
}