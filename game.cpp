#include <iostream>
#include <fstream>
#include <regex>
#include <memory>
#include <vector>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "game.hpp"

std::vector<std::pair<unsigned int, Orientation>> prossessLogFile(const std::string& logFile)
{
    std::vector<std::pair<unsigned int, Orientation>> robot_moves;
    std::ifstream file(logFile);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file.\n";
        exit(1);
    }
    std::regex pattern(R"(([RBG]) robot (\d) go (down|right|left|up))");
    std::string line;
    std::smatch match;
    std::getline(file, line);
    std::regex_search(line, match, std::regex{R"(game starts with (\d) number robots per player)"});
    int numRobotsPerPlayer{std::stoi(match[1])};
    while (std::getline(file, line))
    {
        if (std::regex_search(line, match, pattern))
        {
            unsigned int index = numRobotsPerPlayer*COLOR2INT.at(match[1]) + std::stoi(match[2]) - 1;
            if (match[3] == "left") robot_moves.emplace_back(index, Orientation::LEFT);
            if (match[3] == "right") robot_moves.emplace_back(index, Orientation::RIGHT);
            if (match[3] == "up") robot_moves.emplace_back(index, Orientation::UP);
            if (match[3] == "down") robot_moves.emplace_back(index, Orientation::DOWN);
        }
    }
    file.close();
    return robot_moves;
}


Game::Game(const std::string& logFile)
: _board{"assets/board/board.obj", PROJECTION, VIEW, MODEL}
, _notexture{setupShader("shaders/notexture.vs", "shaders/notexture.fs")}
, _withtexture{setupShader("shaders/withtexture.vs", "shaders/withtexture.fs")}
{
    this->_setupForklifts(logFile);
    this->_boxLeft = std::make_unique<Box>("assets/box/box.obj", PROJECTION, VIEW, glm::translate(MODEL, glm::vec3{2.0f, 0.0f, 3.0f}));
    this->_boxCenter = std::make_unique<Box>("assets/box/box.obj", PROJECTION, VIEW,glm::translate(MODEL, glm::vec3{0.0f, 0.0f, 3.0f}));
    this->_boxRight = std::make_unique<Box>("assets/box/box.obj", PROJECTION, VIEW, glm::translate(MODEL, glm::vec3{-2.0f, 0.0f, 3.0f}));
}

void Game::_setupForklifts(const std::string& logFile)
{
    std::ifstream file(logFile);
    if (!file.is_open()) 
    {
        std::cerr << "Failed to open file.\n";
        exit(1);
    }
    std::regex pattern(R"(([RBG]) robot \d in position \[(\d),(\d)\])");
    std::string line;
    glm::vec3 color;
    while (std::getline(file, line))
    {
        std::smatch match;
        if (std::regex_search(line, match, pattern))
        {
            int x = std::stoi(match[2]);
            int y = std::stoi(match[3]); 
            if (match[1] == "R") color = glm::vec3{0.8f, 0.2f, 0.2f};
            else if (match[1] == "G") color = glm::vec3{0.2f, 0.8f, 0.2f};
            else if (match[1] == "B") color = glm::vec3{0.2f, 0.2f, 0.8f};
            this->_forklifts.emplace_back("assets/forklift/forklift.obj", PROJECTION, VIEW, glm::translate(MODEL, glm::vec3(4-x, 0.0f, y-4)), x, y, color);
        }
    }
    file.close();
}

void Game::render(GLFWwindow* window)
{
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    this->_board.draw(this->_withtexture);
    for (const auto& forklift : this->_forklifts) 
    {
        forklift.draw(this->_notexture, this->_withtexture);
    }
    if (this->_boxCenter)
        this->_boxCenter->draw(this->_withtexture);
    if (this->_boxLeft)
        this->_boxLeft->draw(this->_withtexture);
    if (this->_boxRight)
        this->_boxRight->draw(this->_withtexture);
    glfwSwapBuffers(window);
}


void Game::moveForward(unsigned int forkliftIndex, GLFWwindow* window)
{
    for (unsigned int i=0; i<10; ++i)
    {
        this->_forklifts[forkliftIndex].translate(glm::vec3(0.0f, 0.0f, 0.1f));
        this->render(window);
    }
}

void Game::turnLeft(unsigned int forkliftIndex, GLFWwindow* window)
{
    for (unsigned int i=0; i<9; ++i)
    {
        this->_forklifts[forkliftIndex].rotate(glm::radians(-10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        this->render(window);
    };
}

void Game::turnRight(unsigned int forkliftIndex, GLFWwindow* window)
{
    for (unsigned int i=0; i<9; ++i)
    {
        this->_forklifts[forkliftIndex].rotate(glm::radians(10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        this->render(window);
    };
}

void Game::turnBack(unsigned int forkliftIndex, GLFWwindow* window)
{
    for (unsigned int i=0; i<9; ++i)
    {
        this->_forklifts[forkliftIndex].rotate(glm::radians(20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        this->render(window);
    };
}

void Game::pickup(unsigned int forkliftIndex, GLFWwindow* window)
{
    Forklift* forklift{&this->_forklifts[forkliftIndex]};
    if (forklift->y() == 7)
    {
        if (forklift->x() == 2)
        {
            switch (forklift->orientation()) 
            {
                case Orientation::UP:
                    this->turnBack(forkliftIndex, window);
                    forklift->setOrientation(Orientation::DOWN);
                    break;
                case Orientation::DOWN:
                    break;
                case Orientation::LEFT:
                    this->turnLeft(forkliftIndex, window);
                    forklift->setOrientation(Orientation::DOWN);
                    break;
                case Orientation::RIGHT:
                    this->turnRight(forkliftIndex, window);
                    forklift->setOrientation(Orientation::DOWN);
                    break;
            }
            forklift->setBox(std::move(this->_boxLeft));
        }
        else if (forklift->x() == 4)
        {
            switch (forklift->orientation()) 
            {
                case Orientation::UP:
                    this->turnBack(forkliftIndex, window);
                    forklift->setOrientation(Orientation::DOWN);
                    break;
                case Orientation::DOWN:
                    break;
                case Orientation::LEFT:
                    this->turnLeft(forkliftIndex, window);
                    forklift->setOrientation(Orientation::DOWN);
                    break;
                case Orientation::RIGHT:
                    this->turnRight(forkliftIndex, window);
                    forklift->setOrientation(Orientation::DOWN);
                    break;
            }
            forklift->setBox(std::move(this->_boxCenter));
        } 
        else if (forklift->x() == 6)
        {
            switch (forklift->orientation()) 
            {
                case Orientation::UP:
                    this->turnBack(forkliftIndex, window);
                    forklift->setOrientation(Orientation::DOWN);
                    break;
                case Orientation::DOWN:
                    break;
                case Orientation::LEFT:
                    this->turnLeft(forkliftIndex, window);
                    forklift->setOrientation(Orientation::DOWN);
                    break;
                case Orientation::RIGHT:
                    this->turnRight(forkliftIndex, window);
                    forklift->setOrientation(Orientation::DOWN);
                    break;
            }
            forklift->setBox(std::move(this->_boxRight));
        }
    }
}

void Game::generateBox(unsigned int forkliftIndex)
{
    Forklift* forklift{&this->_forklifts[forkliftIndex]};
    if (forklift->y() == 7)
    {
        if (forklift->x() == 2)
            this->_boxLeft = std::make_unique<Box>("assets/box/box.obj", PROJECTION, VIEW, glm::translate(MODEL, glm::vec3{2.0f, 0.0f, 3.0f}));
        else if (forklift->x() == 4)
            this->_boxCenter = std::make_unique<Box>("assets/box/box.obj", PROJECTION, VIEW,glm::translate(MODEL, glm::vec3{0.0f, 0.0f, 3.0f}));
        else if (forklift->x() == 6)
            this->_boxRight = std::make_unique<Box>("assets/box/box.obj", PROJECTION, VIEW, glm::translate(MODEL, glm::vec3{-2.0f, 0.0f, 3.0f}));
    }
}

void Game::dropoff(unsigned int forkliftIndex)
{
    Forklift* forklift{&this->_forklifts[forkliftIndex]};
    if (yellowCells.find({forklift->x(), forklift->y()}) != yellowCells.end())
    {
        forklift->setBox(nullptr);
    }
}

void Game::up(unsigned int forkliftIndex, GLFWwindow* window)
{
    Forklift* forklift{&this->_forklifts[forkliftIndex]};
    switch (forklift->orientation()) 
    {
        case Orientation::UP:
            this->moveForward(forkliftIndex, window);
            break;
        case Orientation::DOWN:
            this->turnBack(forkliftIndex, window);
            this->moveForward(forkliftIndex, window);
            forklift->setOrientation(Orientation::UP);
            break;
        case Orientation::LEFT:
            this->turnRight(forkliftIndex, window);
            this->moveForward(forkliftIndex, window);
            forklift->setOrientation(Orientation::UP);
            break;
        case Orientation::RIGHT:
            this->turnLeft(forkliftIndex, window);
            this->moveForward(forkliftIndex, window);
            forklift->setOrientation(Orientation::UP);
            break;
    }
    this->generateBox(forkliftIndex);
    forklift->setY(forklift->y()-1);
    this->pickup(forkliftIndex, window);
    this->dropoff(forkliftIndex);
}

void Game::down(unsigned int forkliftIndex, GLFWwindow* window)
{
    Forklift* forklift{&this->_forklifts[forkliftIndex]};
    switch (forklift->orientation())
    {
        case Orientation::UP:
            this->turnBack(forkliftIndex, window);
            this->moveForward(forkliftIndex, window);
            forklift->setOrientation(Orientation::DOWN);
            break;
        case Orientation::DOWN:
            this->moveForward(forkliftIndex, window);
            break;
        case Orientation::LEFT:
            this->turnLeft(forkliftIndex, window);
            this->moveForward(forkliftIndex, window);
            forklift->setOrientation(Orientation::DOWN);
            break;
        case Orientation::RIGHT:
            this->turnRight(forkliftIndex, window);
            this->moveForward(forkliftIndex, window);
            forklift->setOrientation(Orientation::DOWN);
            break;
    }
    this->generateBox(forkliftIndex);
    forklift->setY(forklift->y()+1);
    this->pickup(forkliftIndex, window);
    this->dropoff(forkliftIndex);
}

void Game::left(unsigned int forkliftIndex, GLFWwindow* window)
{
    Forklift* forklift{&this->_forklifts[forkliftIndex]};
    switch (forklift->orientation()) 
    {
        case Orientation::UP:
            this->turnLeft(forkliftIndex, window);
            this->moveForward(forkliftIndex, window);
            forklift->setOrientation(Orientation::LEFT);
            break;
        case Orientation::DOWN:
            this->turnRight(forkliftIndex, window);
            this->moveForward(forkliftIndex, window);
            forklift->setOrientation(Orientation::LEFT);
            break;
        case Orientation::LEFT:
            this->moveForward(forkliftIndex, window);
            break;
        case Orientation::RIGHT:
            this->turnBack(forkliftIndex, window);
            this->moveForward(forkliftIndex, window);
            forklift->setOrientation(Orientation::LEFT);
            break;
    }
    this->generateBox(forkliftIndex);
    forklift->setX(forklift->x()-1);
    this->pickup(forkliftIndex, window);
    this->dropoff(forkliftIndex);
}

void Game::right(unsigned int forkliftIndex, GLFWwindow* window)
{
    Forklift* forklift{&this->_forklifts[forkliftIndex]};
    switch (forklift->orientation())
    {
        case Orientation::UP:
            this->turnRight(forkliftIndex, window);
            this->moveForward(forkliftIndex, window);
            forklift->setOrientation(Orientation::RIGHT);
            break;
        case Orientation::DOWN:
            this->turnLeft(forkliftIndex, window);
            this->moveForward(forkliftIndex, window);
            forklift->setOrientation(Orientation::RIGHT);
            break;
        case Orientation::LEFT:
            this->turnBack(forkliftIndex, window);
            this->moveForward(forkliftIndex, window);
            forklift->setOrientation(Orientation::RIGHT);
            break;
        case Orientation::RIGHT:
            this->moveForward(forkliftIndex, window);
            break;
    }
    this->generateBox(forkliftIndex);
    forklift->setX(forklift->x()+1);
    this->pickup(forkliftIndex, window);
    this->dropoff(forkliftIndex);
}

void Game::run(const std::string& logFile, GLFWwindow* window)
{
    std::vector<std::pair<unsigned int, Orientation>> commands{prossessLogFile(logFile)};
    for (const auto& command: commands)
    {
        if (glfwWindowShouldClose(window))
            return;
        switch (command.second) 
        {        
            case Orientation::UP:
                this->up(command.first, window);
                break;
            case Orientation::DOWN:
                this->down(command.first, window);
                break;
            case Orientation::LEFT:
                this->left(command.first, window);
                break;
            case Orientation::RIGHT:
                this->right(command.first, window);
                break;
        }
    }
}

