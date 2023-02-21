#include "tree.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <stack>

std::string strip(std::string str , bool start = true , bool end = true){

    if(start){
        auto firstCharacter {std::find_if_not(str.begin() , str.end() , isspace)};
        str.erase(str.begin() , firstCharacter);
    } 
    if(end) {
        std::reverse(str.begin() , str.end());
        auto lastCharacter {std::find_if_not(str.begin() , str.end() , isspace)};
        str.erase(lastCharacter , str.begin());
        std::reverse(str.begin() , str.end());
    }

    return str;
}

bool isXmlTagValid(const std::string & tag){

    // If the tag is empty
    if(tag.length() == 0)
        return false;

    // Find first attribute thanks to '='
    auto ei {std::find(tag.begin() , tag.end() , '=')};

    // While there is a remaining attribute
    while(ei != tag.end()){
        // looking for start attribute value delimiter
        auto aDelimIt {std::find_if_not(ei+1 , tag.end() , isspace)};
        // If there is no value , then it's invalid
        if(aDelimIt == tag.end())
            return false;

        // looking for end attribute value delimiter
        auto bDelimIt {std::find(aDelimIt+1 , tag.end() , *aDelimIt)};

        // if there is no end delimiter , then it's also invalid
        if(bDelimIt == tag.end())
            return false;

        // Looking for next attribute
        ei = std::find(ei+1 , tag.end() , '=');

        if(!strip({bDelimIt+1 , ei}).length() && ei != tag.end())
            return false;
    }

    if(std::count(tag.begin() , tag.end() , '<'))
        return false;

    return true;
}

void loadNodeAttributes(XMLNode & node , std::string & tagData){

    auto di = std::find_if(tagData.begin() , tagData.end() , [](char a){return isspace(a) || a == '/';});
    auto ei = std::find(di , tagData.end() , '=');
    while(ei != tagData.end()){
        std::string attributeName {di , ei};
        attributeName = strip(attributeName);

        auto guillIt = std::find(ei+1 , tagData.end() , '"');
        auto aposIt = std::find(ei+1 , tagData.end() , '\'');
        if(guillIt == tagData.end() || std::distance(aposIt , guillIt) > 0){
            auto aposItEnd = std::find(aposIt+1 , tagData.end() , '\'');
            std::string attributeValue {aposIt+1 , aposItEnd};
            node.content.attributes[attributeName] = attributeValue;
            ei = std::find(aposItEnd , tagData.end() , '=');
            di = aposItEnd+1;
        } else {
            auto guillItEnd = std::find(guillIt+1 , tagData.end() , '"');
            std::string attributeValue {guillIt+1 , guillItEnd};
            node.content.attributes[attributeName] = attributeValue;
            ei = std::find(guillItEnd , tagData.end() , '=');
            di = guillItEnd+1;
        }
    }

}

XMLNode parseFile(std::string path){

    std::ifstream reader {path};
    std::string fcontent;
    std::string line;

    // extracting file content in one string
    while (std::getline(reader , line)){
        line = strip(line , true , false);
        fcontent += line;
    }

    fcontent = strip(fcontent);

    std::vector<std::string> tags{};
    std::vector<std::string> tContents{};

    // extracting tags (opening and closing tags) and content between them
    auto ai = std::find(fcontent.begin() , fcontent.end() , '<');

    while(ai != fcontent.end()){
        if(isspace(*(ai+1))){
            throw std::runtime_error("Incorrect xml encoding !");
        }

        // if(*(ai+1) == '?'){
        
        //     auto ending {fcontent.find("?>")};
        //     if(ending != fcontent.npos){
        //         continue;
        //     } else {
        //         throw std::runtime_error("Incorrect xml encoding : invalid tag");
        //     }
        // }

        auto bi {std::find(ai , fcontent.end() , '>')};
        bool incorrectEnding = true;



        while(incorrectEnding){
            incorrectEnding = false;

            std::string tag {ai+1 , bi};
            if(!isXmlTagValid(tag)){
                incorrectEnding = true;
            }

            if(incorrectEnding)
                bi = std::find(bi+1 , fcontent.end() , '>');

            if(bi == fcontent.end() && incorrectEnding){
                throw std::runtime_error("Incorrect xml encoding : invalid tag");
            }
        }

        tags.push_back({ai+1 , bi});
        ai = std::find(ai+1 , fcontent.end() , '<');
        if(ai != fcontent.end())
            tContents.push_back({bi+1 , ai});
    }

    // final tree
    XMLNode xmlTree;
    // Tag content iterator
    unsigned int t = 0;

    std::stack<std::reference_wrapper<XMLNode>> nodeStack;

    for (auto & tag : tags){

        // if it's a closing tag
        if(tag[0] == '/'){
            nodeStack.pop();
            if(!nodeStack.empty())
                nodeStack.top().get().content.value += tContents[t];
            t++;
            continue;
        }

        // //////////////////
        // Process of data colecting
        // //////////////////
        auto endTagName = std::find_if(tag.begin() , tag.end() , [](char a){return isspace(a) || a == '/';});
        std::string tagName {tag.begin() , endTagName};

        XMLNode node;
        node.name = tagName;
        
        loadNodeAttributes(node , tag);

        if(tag.back() == '/'){
            nodeStack.top().get().addChild(node);
            nodeStack.top().get().content.value += tContents[t];
        }else{
            if(nodeStack.empty()){
                xmlTree = node;
                nodeStack.emplace(xmlTree);
                nodeStack.top().get().content.value = tContents[t];
            }
            else{
                nodeStack.top().get().addChild(node);
                nodeStack.emplace(nodeStack.top().get()[nodeStack.top().get().getChildrenNumber()-1]);
                nodeStack.top().get().content.value = tContents[t];
            }
        }

        t++;
        
    }
    xmlTree.updatePointers();
    return xmlTree;
}

std::ostream & operator<<(std::ostream& os , const XMLNode & node){
    os << node.name << std::endl;
    for(auto & child : node.getChildren()){
        os << child;
    }
    return os;
}