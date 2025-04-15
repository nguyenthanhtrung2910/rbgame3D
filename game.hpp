#ifndef GAME_H
#define GAME_H
#include <iostream>
#include <memory>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <glm/glm.hpp>


#include "shader.hpp"
#include "model.hpp"

inline constexpr unsigned int SCR_WIDTH{800};
inline constexpr unsigned int SCR_HEIGHT{600};
inline const glm::mat4 PROJECTION{glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f)};
inline const glm::mat4 VIEW{glm::lookAt(glm::vec3{0.0f, -3.0f, 4.0f}, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f,-1.0f, 0.0f})};
inline const glm::mat4 MODEL{glm::scale(glm::mat4(1.0f), glm::vec3(0.45f, 0.45f, 0.45f))};

struct PairHash {
    template <typename T1, typename T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ (h2 << 1);
    }
};

inline const std::unordered_set<std::pair<int, int>, PairHash> yellowCells
{
    {2, 0}, {4, 0}, {6, 0}, 
    {0, 2}, {0, 4}, {0, 6},
    {8, 2}, {8, 4}, {8, 6}
};

inline const std::unordered_map<std::string, unsigned int> COLOR2INT
{
    {"R", 0},
    {"B", 1},
    {"G", 2}
};

std::vector<std::pair<unsigned int, Orientation>> prossessLogFile(const std::string& logFile);

class Game
{
public:
    Game(const std::string& logFile);
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