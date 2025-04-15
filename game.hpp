#ifndef GAME_H
#define GAME_H
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <string>
#include <unistd.h> 

#include "shader.hpp"
#include "model.hpp"

inline constexpr unsigned int SCR_WIDTH{800};
inline constexpr unsigned int SCR_HEIGHT{600};
inline constexpr unsigned int DELAY{100000};

// std::vector<Cell> generateCells();

std::vector<std::pair<unsigned int, Orientation>> prossessLogFile(const std::string& logFile);

struct PairHash {
    template <typename T1, typename T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ (h2 << 1);
    }
};

const std::unordered_set<std::pair<int, int>, PairHash> yellowCells
{
    {2, 0}, {4, 0}, {6, 0}, 
    {0, 2}, {0, 4}, {0, 6},
    {8, 2}, {8, 4}, {8, 6}
};

const std::unordered_map<std::string, unsigned int> COLOR2INT
{
    {"R", 0},
    {"B", 1},
    {"G", 2}
};

class Game
{
public:
    Game(const Board& board, const std::string& logFile);
    void render(GLFWwindow* window);
    void moveForward(unsigned int forkliftIndex, GLFWwindow* window);
    void turnLeft(unsigned int forkliftIndex, GLFWwindow* window);
    void turnRight(unsigned int forkliftIndex, GLFWwindow* window);
    void turnBack(unsigned int forkliftIndex, GLFWwindow* window);
    void pickup(unsigned int forkliftIndex, GLFWwindow* window);
    void dropoff(unsigned int forkliftIndex);
    void generateBox(unsigned int forkliftIndex);
    void up(unsigned int forkliftIndex, GLFWwindow* window);
    void down(unsigned int forkliftIndex, GLFWwindow* window);
    void left(unsigned int forkliftIndex, GLFWwindow* window);
    void right(unsigned int forkliftIndex, GLFWwindow* window);
    void run(const std::string& logFile, GLFWwindow* window);
private:
    int _time{0};
    Board _board;
    Shader _notexture;
    Shader _withtexture;
    std::vector<Forklift> _forklifts;
    std::unique_ptr<Box> _boxLeft{nullptr};
    std::unique_ptr<Box> _boxCenter{nullptr};
    std::unique_ptr<Box> _boxRight{nullptr};
    void _setupForklifts(const std::string& logFile);
};

#endif