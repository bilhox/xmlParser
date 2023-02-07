#pragma once

#include <vector>
#include <string>
#include <unordered_map>

template<typename contentType>
class Node{

    public:

        Node<contentType>* parent = nullptr;

        std::string name;
        contentType content;

        Node(){
            m_children = {};
            name = "";
        }
        void addChild(const Node<contentType> & node){
            m_children.push_back(node);
            updatePointers();
        }

        Node<contentType>& operator[](size_t index){
            return m_children[index];
        }

        const std::vector<Node<contentType>> & getChildren() const {
            return m_children;
        }
    
    private:

        std::vector<Node<contentType>> m_children;

        void updatePointers(){
            for(Node<contentType> & child : m_children){
                child.parent = this;
                child.updatePointers();
            }
        }
};

struct XMLContent{
    std::string value;
    std::unordered_map<std::string , std::string> attributes;
};

typedef Node<XMLContent> XMLNode;

std::ostream & operator<<(std::ostream& os , const XMLNode & node);

XMLNode parseFile(std::string path);