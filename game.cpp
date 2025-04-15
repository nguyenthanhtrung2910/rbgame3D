#include <iostream>
#include <fstream>
#include <regex>
#include <memory>
#include <vector>
#include <string>
#include <unistd.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "game.hpp"

// std::vector<Cell> generateCells()
// {
//     std::vector<Cell> cells;
//     for (unsigned int y=0; y<9; ++y)
//     {
//         for (unsigned int x=0; x<9; ++x)
//         {
//             if (y == 8)
//                 cells.emplace_back(x, y, 'r', 0);
//             else if (y == 7 && (x == 2 || x == 4 ||x == 6)) 
//                 cells.emplace_back(x, y, 'g', 0);
//             else if (x == 0 && y == 6)
//                 cells.emplace_back(x, y, 'y', 1);
//             else if (x == 0 && y == 4)
//                 cells.emplace_back(x, y, 'y', 2);
//             else if (x == 0 && y == 2)
//                 cells.emplace_back(x, y, 'y', 3);
//             else if (x == 8 && y == 6)
//                 cells.emplace_back(x, y, 'y', 9);
//             else if (x == 8 && y == 4)
//                 cells.emplace_back(x, y, 'y', 8);
//             else if (x == 8 && y == 2)
//                 cells.emplace_back(x, y, 'y', 6);
//             else if (x == 2 && y == 0)
//                 cells.emplace_back(x, y, 'y', 4);
//             else if (x == 4 && y == 0)
//                 cells.emplace_back(x, y, 'y', 7);
//             else if (x == 6 && y == 0)
//                 cells.emplace_back(x, y, 'y', 5);
//             else if (x == 0 && y == 0)
//                 cells.emplace_back(x, y, 'b', 0);
//             else if (x == 8 && y == 0)
//                 cells.emplace_back(x, y, 'b', 0);
//             else
//                 cells.emplace_back(x, y, 'w', 0);
//         }
//     }
//     return cells;
// }

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


Game::Game(const Board& board, const std::string& logFile)
: _board{board}
, _notexture{setupShader("shader-code/forklift.vs", "shader-code/forklift.fs")}
, _withtexture{setupShader("shader-code/board.vs", "shader-code/board.fs")}
{
    this->_setupForklifts(logFile);
    this->_boxLeft = std::make_unique<Box>(
        "box/box.obj", 
        this->_board.projectionMatrix(),
        this->_board.viewMatrix(),
        glm::translate(this->_board.modelMatrix(), glm::vec3{2.0f, 0.0f, 3.0f})
    );
    this->_boxCenter = std::make_unique<Box>(
        "box/box.obj", 
        this->_board.projectionMatrix(),
        this->_board.viewMatrix(),
        glm::translate(this->_board.modelMatrix(), glm::vec3{0.0f, 0.0f, 3.0f})
    );
    this->_boxRight = std::make_unique<Box>(
        "box/box.obj",
        this->_board.projectionMatrix(),
        this->_board.viewMatrix(),
        glm::translate(this->_board.modelMatrix(), glm::vec3{-2.0f, 0.0f, 3.0f})
    );
}

void Game::_setupForklifts(const std::string& logFile)
{
    std::ifstream file(logFile);
    if (!file.is_open()) {
        std::cerr << "Failed to open file.\n";
        exit(1);
    }
    std::regex pattern(R"(\[(\d),(\d)\])");
    std::string line;
    while (std::getline(file, line)) {
        std::smatch match;
        if (std::regex_search(line, match, pattern)) {
            int x = std::stoi(match[1]);
            int y = std::stoi(match[2]);  
            this->_forklifts.emplace_back(
                "forklift/forklift.obj",
                this->_board.projectionMatrix(),
                this->_board.viewMatrix(),
                glm::translate(this->_board.modelMatrix(), glm::vec3(4-x, 0.0f, y-4)),
                x,
                y
            );
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
    glfwPollEvents();
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
            // std::cout << "At t=" << this->_time << " robot " << forkliftIndex << "pickup in position ("<< forklift->x() << ',' << forklift->y() << ") with orientation " << forklift->orientation() << "\n";
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
            // std::cout << "At t=" << this->_time << " robot " << forkliftIndex << "pickup in position ("<< forklift->x() << ',' << forklift->y() << ") with orientation " << forklift->orientation() << "\n";
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
            // std::cout << "At t=" << this->_time << " robot " << forkliftIndex << "pickup in position ("<< forklift->x() << ',' << forklift->y() << ") with orientation " << forklift->orientation() << "\n";
        }
    }
}
void Game::generateBox(unsigned int forkliftIndex)
{
    Forklift* forklift{&this->_forklifts[forkliftIndex]};
    if (forklift->y() == 7)
    {
        if (forklift->x() == 2)
            this->_boxLeft = std::make_unique<Box>(
                "box/box.obj", 
                this->_board.projectionMatrix(),
                this->_board.viewMatrix(),
                glm::translate(this->_board.modelMatrix(), glm::vec3{2.0f, 0.0f, 3.0f})
            );
        if (forklift->x() == 4)
            this->_boxCenter = std::make_unique<Box>(
                "box/box.obj", 
                this->_board.projectionMatrix(),
                this->_board.viewMatrix(),
                glm::translate(this->_board.modelMatrix(), glm::vec3{0.0f, 0.0f, 3.0f})
            );
        if (forklift->x() == 6)
            this->_boxRight = std::make_unique<Box>(
                "box/box.obj", 
                this->_board.projectionMatrix(),
                this->_board.viewMatrix(),
                glm::translate(this->_board.modelMatrix(), glm::vec3{-2.0f, 0.0f, 3.0f})
            );
    }
}

void Game::dropoff(unsigned int forkliftIndex)
{
    Forklift* forklift{&this->_forklifts[forkliftIndex]};
    if (yellowCells.find({forklift->x(), forklift->y()}) != yellowCells.end()) {
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
    std::cout << "At t=" << this->_time << " robot " << forkliftIndex << "go up to position ("<< forklift->x() << ',' << forklift->y() << ") with orientation " << forklift->orientation() << "\n";
    this->_time += 1;
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
    std::cout << "At t=" << this->_time << " robot" << forkliftIndex << "go down to position ("<< forklift->x() << ',' << forklift->y() << ") with orientation" << forklift->orientation() << "\n";
    this->_time += 1;
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
    std::cout << "At t=" << this->_time << " robot" << forkliftIndex << "go left to position ("<< forklift->x() << ',' << forklift->y() << ") with orientation" << forklift->orientation() << "\n";
    this->_time += 1;
}

void Game::right(unsigned int forkliftIndex, GLFWwindow* window)
{
    Forklift* forklift{&this->_forklifts[forkliftIndex]};
    switch (forklift->orientation()) {
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
    std::cout << "At t=" << this->_time << " robot" << forkliftIndex << "go right to position ("<< forklift->x() << ',' << forklift->y() << ") with orientation" << forklift->orientation() << "\n";
    this->_time += 1;
}

void Game::run(const std::string& logFile, GLFWwindow* window)
{
    std::vector<std::pair<unsigned int, Orientation>> commands{prossessLogFile(logFile)};
    int i{0};
    for (const auto& command: commands)
    {
        std::cout << "At t=" << i << " " << command.first << " : "<< command.second << ".\n";
        i += 1;
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
        // if (command.second == "U") 
        // {
        //     // std::cout << "At t=" << this->_time << " robot " << command.first << "go "<< command.second <<" to position () with orientation " << "\n";
        //     this->up(command.first, window);
        //     // this->_time += 1;
        // }
        // else if (command.second == "D") 
        // {
        //     // std::cout << "At t=" << this->_time << " robot " << command.first << "go "<< command.second <<" to position () with orientation " << "\n";
        //     this->down(command.first, window);
        //     // this->_time += 1;
        // }
        // else if (command.second == "L") 
        // {
        //     // std::cout << "At t=" << this->_time << " robot " << command.first << "go "<< command.second <<" to position () with orientation " << "\n";
        //     this->left(command.first, window);
        //     // this->_time += 1;
        // }
        // else if (command.second == "R") 
        // {
        //     // std::cout << "At t=" << this->_time << " robot " << command.first << "go "<< command.second <<" to position () with orientation " << "\n";
        //     this->right(command.first, window);
        //     // this->_time += 1;
        // }
    }

}

