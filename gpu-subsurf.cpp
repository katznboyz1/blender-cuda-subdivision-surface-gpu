// trying to get the catmull clark subdiv method working in c++ so I can translate it to cuda

#include <iostream>
#include <math.h>
#include <string>
#include <fstream>
#include <iterator>
#include <sstream>
#include <vector>

using namespace std;

struct vec3 {
    double x;
    double y;
    double z;
};

struct vec2 {
    double x;
    double y;
};

struct vertex {
    vec3 position;
    vec2 textureCoordinate;
    vec3 normal;
};

std::vector<std::string> stringSplit(std::string string, char delimiter) {

    std::vector<std::string> splitString;
    std::string currentString = "";

    for (int i = 0; i < string.length(); i++) {
        if (string[i] == delimiter) {

            splitString.push_back(currentString);
            currentString = "";
        } else {

            currentString += string[i];

            if (i + 1 == string.length()) {
                splitString.push_back(currentString);
            }
        }
    }

    return splitString;
}

// can be gpu accelerated at least a little bit since it requires many lines to be read
// for a simple cube there will be no speedup, but for a mesh with millions of verts it will be faster
// currently only reads verts, faces and edges are todo
// __global__
void readObj(std::string path, std::vector<vertex> vertices) {
    
    std::ifstream objFile(path);

    // tell the program to not count new lines
    objFile.unsetf(std::ios_base::skipws);

    std::string objFileLine;

    while (getline(objFile, objFileLine)) {

        std::stringstream ss{objFileLine};
        char objFileLineChar;
        ss >> objFileLineChar;

        std::vector<std::string> lineDataSplitBySpaces = stringSplit(objFileLine, ' ');
        std::string lineType = lineDataSplitBySpaces[0];

        vertex currentVert;

        cout << objFileLine << endl;

        if (lineType.compare("v") == 0) {
            currentVert.position.x = std::stod(lineDataSplitBySpaces[1]);
            currentVert.position.y = std::stod(lineDataSplitBySpaces[2]);
            currentVert.position.z = std::stod(lineDataSplitBySpaces[3]);

        } else if (lineType.compare("vt") == 0) {
            currentVert.textureCoordinate.x = std::stod(lineDataSplitBySpaces[1]);
            currentVert.textureCoordinate.x = std::stod(lineDataSplitBySpaces[2]);

        } else if (lineType.compare("vn") == 0) {
            currentVert.normal.x = std::stod(lineDataSplitBySpaces[1]);
            currentVert.normal.y = std::stod(lineDataSplitBySpaces[2]);
            currentVert.normal.z = std::stod(lineDataSplitBySpaces[3]);

        } else {
        }
    }
}

int main (void) {

    std::string objPath = "./testCube.obj";
    std::vector<vertex> objVertices;

    readObj(objPath, objVertices);

    return 0;
}