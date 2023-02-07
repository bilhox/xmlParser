#include "tree.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>

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

    if(tag.length() == 0)
        return false;

    auto ei {std::find(tag.begin() , tag.end() , '=')};

    while(ei != tag.end()){
        auto aDelimIt {std::find_if_not(ei+1 , tag.end() , isspace)};

        if(aDelimIt == tag.end())
            return false;

        auto bDelimIt {std::find(aDelimIt+1 , tag.end() , *aDelimIt)};

        if(bDelimIt == tag.end())
            return false;

        ei = std::find(ei+1 , tag.end() , '=');

        if(strip({bDelimIt+1 , ei}).length())
            return false;
    }

    if(std::count(tag.begin() , tag.end() , '<'))
        return false;

    return true;
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

        auto bi {std::find(ai , fcontent.end() , '>')};
        bool incorrectEnding = true;



        while(incorrectEnding){
            incorrectEnding = false;

            if(!isXmlTagValid({ai+1 , bi})){
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
    // ptr to the parent
    XMLNode* xmlNodePtr = &xmlTree;
    // height
    unsigned int level = 0;
    unsigned int tContentIt = 0;

    // Child N to use , in which level
    std::unordered_map<unsigned int , unsigned int> childN;

    for (auto & tag : tags){

        // if it's a closing tag
        if(tag[0] == '/'){
            xmlNodePtr = (*xmlNodePtr).parent;
            childN[level] = 0;
            level--;
            continue;
        }

        if(childN.find(level) != childN.end()){
            childN[level] ++;
        } else {
            childN[level] = 0;
        }

        // //////////////////
        // Process of data colecting
        // Not related to the problem
        // //////////////////
        auto endTagName = std::find_if(tag.begin() , tag.end() , [](char a){return isspace(a) || a == '/';});
        std::string tagName {tag.begin() , endTagName};

        XMLNode node;
        node.name = tagName;
        
        auto di = endTagName;
        auto ei = std::find(di , tag.end() , '=');
        while(ei != tag.end()){
            std::string attributeName {di , ei};
            attributeName = strip(attributeName);

            auto guillIt = std::find(ei+1 , tag.end() , '"');
            auto aposIt = std::find(ei+1 , tag.end() , '\'');
            if(guillIt == tag.end() || std::distance(aposIt , guillIt) > 0){
                auto aposItEnd = std::find(aposIt+1 , tag.end() , '\'');
                std::string attributeValue {aposIt+1 , aposItEnd};
                node.content.attributes[attributeName] = attributeValue;
                ei = std::find(aposItEnd , tag.end() , '=');
                di = aposItEnd+1;
            } else {
                auto guillItEnd = std::find(guillIt+1 , tag.end() , '"');
                std::string attributeValue {guillIt+1 , guillIt};
                node.content.attributes[attributeName] = attributeValue;
                ei = std::find(guillItEnd , tag.end() , '=');
                di = guillItEnd+1;
            }
        }
        // /////////////////

        // From here

        if(*tag.crbegin() == '/'){
            (*xmlNodePtr).addChild(node);
            (*xmlNodePtr).content.value += tContents[tContentIt];
        } else {
            if(xmlTree.name == ""){
                node.content.value = tContents[tContentIt];
                xmlTree = node;
            } else {
                node.parent = xmlNodePtr;
                node.content.value = tContents[tContentIt];
                (*xmlNodePtr).addChild(node);
                xmlNodePtr = &(*xmlNodePtr)[childN[level]];
            }
            level++;
        }

        tContentIt++;
        
    }
    return xmlTree;
}

std::ostream & operator<<(std::ostream& os , const XMLNode & node){
    os << node.name << std::endl;
    for(auto & child : node.getChildren()){
        os << child;
    }
    return os;
}