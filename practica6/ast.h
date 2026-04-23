#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>

class Node {
public:
    std::string type;
    std::string value;
    std::vector<Node*> children;

    Node(std::string type, std::string value = "");

    void add_child(Node* child);

    void print(int indent = 0);

    void gen_dot(const std::string& filename);

private:
    void gen_dot_rec(std::ofstream& out, int& id);
};

#endif