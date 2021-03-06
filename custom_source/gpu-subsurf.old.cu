#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>

using namespace std;

// from https://forums.developer.nvidia.com/t/throughput-test-add-mul-mod-giving-strange-result/32021
// remove when done
#define CUDA_CHECK_RETURN(value) {\
    cudaError_t _m_cudaStat = value;\
    if (_m_cudaStat != cudaSuccess) {\
        fprintf(stderr, "Error %s at line %d in file %s\n",\
                cudaGetErrorString(_m_cudaStat), __LINE__, __FILE__);\
        exit(1);\
    }\
}

struct vec3 {
    double x = 0;
    double y = 0;
    double z = 0;
    bool modified = false;
    int status = 0;
};

struct vertex {
    vec3 position;
    int id;
    int neighboringFaces = 4;
};

struct quadFace {
    int vertexIndex[4];
    vec3 midpoint;
    int midpointVertID;
    int edgeSimplificationMatches = 0;
};

__device__ vertex* objVertices;
__device__ quadFace* objFaces;
__device__ vec3* faceMidpoints;
__device__ quadFace* newFaces;
__device__ vertex* newVertices;

__host__
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

__host__
void readObj(std::string path, std::vector<vertex>& vertices, std::vector<quadFace>& faces) {
    
    std::ifstream objFile(path);

    // tell the program to not count new lines
    objFile.unsetf(std::ios_base::skipws);

    std::string objFileLine;

    int dataCount_v = 0;
    int id = 0;

    while (getline(objFile, objFileLine)) {

        std::stringstream ss{objFileLine};
        char objFileLineChar;
        ss >> objFileLineChar;

        std::vector<std::string> lineDataSplitBySpaces = stringSplit(objFileLine, ' ');
        std::string lineType = lineDataSplitBySpaces[0];

        vertex currentVert;

        bool wasVert = false;
        int vertType = 0; // 0 = none, 1 = vert, 2 = texture coordinate, 3 = normal vert

        if (lineType.compare("v") == 0) {
            currentVert.position.x = std::stod(lineDataSplitBySpaces[1]);
            currentVert.position.y = std::stod(lineDataSplitBySpaces[2]);
            currentVert.position.z = std::stod(lineDataSplitBySpaces[3]);
            currentVert.position.modified = true;
            currentVert.id = id;

            wasVert = true;
            vertType = 1;
            dataCount_v++;

        } else if (lineType.compare("f") == 0) {

            quadFace currentFace;

            for (int i = 1; i < lineDataSplitBySpaces.size(); i++) {
                
                std::vector<std::string> lineDataSplitBySlashes = stringSplit(lineDataSplitBySpaces[i], '/');

                // vertex_index, texture_index, normal_index
                currentFace.vertexIndex[i - 1] = std::stod(lineDataSplitBySlashes[0]) - 1;

            }

            faces.push_back(currentFace);
        }

        if (wasVert) {

            if (currentVert.id < dataCount_v) vertices.push_back(currentVert);

            // check for which part of the vert has already been written to since the verts are written before the normals verts
            // if the vert type is 1 (v) and the vert hasnt been modified on the verts array
            if (vertType == 1 && !vertices[(dataCount_v - 1)].position.modified) {

                vertices[(dataCount_v - 1)].position.x = currentVert.position.x;
                vertices[(dataCount_v - 1)].position.y = currentVert.position.y;
                vertices[(dataCount_v - 1)].position.z = currentVert.position.z;
                vertices[(dataCount_v - 1)].position.modified = true;
            }

            id++;
        }
    }

    objFile.close();
}

__global__ 
void catmullClarkFacePointsAndEdges(int facesSize_lcl, int maxVertsAtStart_lcl, int totalNewVertsToAllocate) {

    int i = (blockIdx.x * blockDim.x) + threadIdx.x;

    quadFace currentSubdividedFaces[4];
    
    for (int j = 0; j < 4; j++) currentSubdividedFaces[j].vertexIndex[3] = objFaces[i].midpointVertID; // face point [0] will be the center of the subdivided face

    // vertex ids for the edges

    int vertexIDs[4];
    
    for (int j = 0; j < 4; j++) {

        vec3 edgeAveragePoint;

        vertex edgePoint;

        edgeAveragePoint.x = (objVertices[objFaces[i].vertexIndex[(j + 1) % 4]].position.x + objVertices[objFaces[i].vertexIndex[(j + 0) % 4]].position.x) / 2;
        edgeAveragePoint.y = (objVertices[objFaces[i].vertexIndex[(j + 1) % 4]].position.y + objVertices[objFaces[i].vertexIndex[(j + 0) % 4]].position.y) / 2;
        edgeAveragePoint.z = (objVertices[objFaces[i].vertexIndex[(j + 1) % 4]].position.z + objVertices[objFaces[i].vertexIndex[(j + 0) % 4]].position.z) / 2;

        currentSubdividedFaces[j].vertexIndex[1] = objFaces[i].vertexIndex[(j + 0) % 4];

        // find the averages for the face points

        edgePoint.id = maxVertsAtStart_lcl + (i * 5) + (j + 1);

        vertexIDs[j] = edgePoint.id;

        currentSubdividedFaces[j].vertexIndex[0] = edgePoint.id;
        currentSubdividedFaces[(j + 1) % 4].vertexIndex[2] = edgePoint.id;

        objVertices[vertexIDs[j]].position = edgeAveragePoint;
    }

    for (int j = 0; j < 4; j++) {

        newFaces[(i * 4) + j] = currentSubdividedFaces[j];
    }

    objVertices[objFaces[i].midpointVertID].position = faceMidpoints[i];
}

__global__
void replaceNewVerticesWithOldVertices() {

    newVertices = objVertices;
}

__global__
void averageCornerVertices(int facesSize) {

    int i = (blockIdx.x * blockDim.x) + threadIdx.x;

    for (int j = 0; j < 4; j++) {

        int matchedPoints = 0;

        //vec3 neighboringFaceMidpointsAverage;
        vec3 edgeMidpointsAverage;
        //vec3 finalMidpointAverage;

        for (int k = 0; k < facesSize; k++) {

            for (int l = 0; l < 4; l++) {

                if (
                    newVertices[objFaces[i].vertexIndex[j]].position.x == newVertices[objFaces[k].vertexIndex[l]].position.x &&
                    newVertices[objFaces[i].vertexIndex[j]].position.y == newVertices[objFaces[k].vertexIndex[l]].position.y &&
                    newVertices[objFaces[i].vertexIndex[j]].position.z == newVertices[objFaces[k].vertexIndex[l]].position.z
                ) {

                    edgeMidpointsAverage.x += (objVertices[objFaces[i].vertexIndex[j]].position.x + objVertices[objFaces[k].vertexIndex[(l + 1) % 4]].position.x) / 2;
                    edgeMidpointsAverage.y += (objVertices[objFaces[i].vertexIndex[j]].position.y + objVertices[objFaces[k].vertexIndex[(l + 1) % 4]].position.y) / 2;
                    edgeMidpointsAverage.z += (objVertices[objFaces[i].vertexIndex[j]].position.z + objVertices[objFaces[k].vertexIndex[(l + 1) % 4]].position.z) / 2;

                    matchedPoints++;

                    if (matchedPoints > 3) {

                        k = facesSize;
                        l = 4;
                    }
                }
            }
        }

        // will be re-implemented later
        /*
        for (int k = 0; k < matchedPoints; k++) {

            neighboringFaceMidpointsAverage.x += faceMidpoints[neighboringFaceIDs[k]].x;
            neighboringFaceMidpointsAverage.y += faceMidpoints[neighboringFaceIDs[k]].y;
            neighboringFaceMidpointsAverage.z += faceMidpoints[neighboringFaceIDs[k]].z;
        }

        neighboringFaceMidpointsAverage.x /= matchedPoints;
        neighboringFaceMidpointsAverage.y /= matchedPoints;
        neighboringFaceMidpointsAverage.z /= matchedPoints;
        */

        edgeMidpointsAverage.x /= matchedPoints;
        edgeMidpointsAverage.y /= matchedPoints;
        edgeMidpointsAverage.z /= matchedPoints;

        // will be re-implemented later
        /*
        finalMidpointAverage.x = (neighboringFaceMidpointsAverage.x + edgeMidpointsAverage.x) / 2;
        finalMidpointAverage.y = (neighboringFaceMidpointsAverage.y + edgeMidpointsAverage.y) / 2;
        finalMidpointAverage.z = (neighboringFaceMidpointsAverage.z + edgeMidpointsAverage.z) / 2;
        */

        newVertices[objFaces[i].vertexIndex[j]].position = edgeMidpointsAverage;
    }
}


__global__
void mergeByDistance(int facesSize, int verticesSize) {

    int i = (blockIdx.x * blockDim.x) + threadIdx.x;

    for (int j = 0; j < verticesSize; j++) {

        for (int k = 0; k < 4; k++) {

            if (
                newVertices[newFaces[i].vertexIndex[k]].position.x == newVertices[j].position.x &&
                newVertices[newFaces[i].vertexIndex[k]].position.y == newVertices[j].position.y &&
                newVertices[newFaces[i].vertexIndex[k]].position.z == newVertices[j].position.z
            ) {

                newFaces[i].vertexIndex[k] = j;
                newFaces[i].edgeSimplificationMatches++;

                if (newFaces[i].edgeSimplificationMatches >= 3) return;

                k = 4;
            }
        }
    }
}

__host__
void subdivideMeshFromFile(std::string inputFilePath, std::string outputFilePath, bool mergeMeshByDistance) {

    auto startTime = std::chrono::steady_clock::now();

    std::vector<vertex> vertices;
    std::vector<quadFace> faces;

    const int BLOCK_SIZE = 256;

    std::cout << "[CPU] [readObj] READING MESH FROM " << inputFilePath << endl;
    readObj(inputFilePath, vertices, faces); 
    std::cout << "[CPU] [readObj] FINISHED READING MESH" << endl;

    auto endTime = std::chrono::steady_clock::now();
    std::cout << "[CPU] [main] ELAPSED TIME " << std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count()) << "MS" << endl;

    int facesSize = faces.size();
    int facesSizeAfterSubdivision = facesSize * 4;
    int verticesSize = vertices.size();
    int totalNewVertsToAllocate = facesSize * 5;

    std::cout << "[CPU] [main] " << std::to_string(facesSize) << " FACES AND " << std::to_string(verticesSize) << " VERTICES READ FROM DISK" << endl;
    std::cout << "[CPU] [main] " << std::to_string(facesSizeAfterSubdivision) << " FACES AND " << std::to_string(verticesSize + totalNewVertsToAllocate) << " VERTICES WILL BE ALLOCATED" << endl;

    vertex* objVertices_tmp = new vertex[verticesSize + totalNewVertsToAllocate]; 
    quadFace* objFaces_tmp = new quadFace[facesSize]; 
    vec3* faceMidpoints_tmp = new vec3[facesSize]; 
    quadFace* newFaces_tmp = new quadFace[facesSize * 4]; 
    vertex* newVertices_tmp = new vertex[verticesSize + totalNewVertsToAllocate]; 
    
    CUDA_CHECK_RETURN(cudaMallocManaged((void **)&objVertices_tmp, sizeof(vertex) * (verticesSize + totalNewVertsToAllocate)));
    CUDA_CHECK_RETURN(cudaMallocManaged((void **)&objFaces_tmp, sizeof(quadFace) * (facesSize)));
    CUDA_CHECK_RETURN(cudaMallocManaged((void **)&faceMidpoints_tmp, sizeof(vec3) * (facesSize)));
    CUDA_CHECK_RETURN(cudaMallocManaged((void **)&newFaces_tmp, sizeof(quadFace) * (facesSize * 4)));
    CUDA_CHECK_RETURN(cudaMallocManaged((void **)&newVertices_tmp, sizeof(vertex) * (verticesSize + (facesSize * 5))));

    for (int j = 0; j < verticesSize; j++) {

        objVertices_tmp[j] = vertices[j];
    } 

    for (int j = verticesSize; j < verticesSize + totalNewVertsToAllocate; j++) {

        vertex tmp;
        objVertices_tmp[j] = tmp;
    }

    for (int j = 0; j < facesSize; j++) {

        objFaces_tmp[j] = faces[j];
    }

    for (int j = 0; j < verticesSize + totalNewVertsToAllocate; j++) {

        vertex tmp;
        newVertices_tmp[j] = tmp;
    }

    for (int j = 0; j < facesSize; j++) {

        vec3 faceAverageMiddlePoint;

        faceAverageMiddlePoint.x = (
            (objVertices_tmp[objFaces_tmp[j].vertexIndex[0]].position.x) + 
            (objVertices_tmp[objFaces_tmp[j].vertexIndex[1]].position.x) + 
            (objVertices_tmp[objFaces_tmp[j].vertexIndex[2]].position.x) + 
            (objVertices_tmp[objFaces_tmp[j].vertexIndex[3]].position.x)
        ) / 4;

        faceAverageMiddlePoint.y = (
            (objVertices_tmp[objFaces_tmp[j].vertexIndex[0]].position.y) + 
            (objVertices_tmp[objFaces_tmp[j].vertexIndex[1]].position.y) + 
            (objVertices_tmp[objFaces_tmp[j].vertexIndex[2]].position.y) + 
            (objVertices_tmp[objFaces_tmp[j].vertexIndex[3]].position.y)
        ) / 4;

        faceAverageMiddlePoint.z = (
            (objVertices_tmp[objFaces_tmp[j].vertexIndex[0]].position.z) + 
            (objVertices_tmp[objFaces_tmp[j].vertexIndex[1]].position.z) + 
            (objVertices_tmp[objFaces_tmp[j].vertexIndex[2]].position.z) + 
            (objVertices_tmp[objFaces_tmp[j].vertexIndex[3]].position.z)
        ) / 4;

        faceMidpoints_tmp[j] = faceAverageMiddlePoint;
        objFaces_tmp[j].midpointVertID = verticesSize + (j * 5);
    }

    CUDA_CHECK_RETURN(cudaMemcpyToSymbol(objVertices, &objVertices_tmp, sizeof(objVertices_tmp)));
    CUDA_CHECK_RETURN(cudaMemcpyToSymbol(objFaces, &objFaces_tmp, sizeof(objFaces_tmp)));
    CUDA_CHECK_RETURN(cudaMemcpyToSymbol(faceMidpoints, &faceMidpoints_tmp, sizeof(faceMidpoints_tmp)));
    CUDA_CHECK_RETURN(cudaMemcpyToSymbol(newFaces, &newFaces_tmp, sizeof(newFaces_tmp)));
    CUDA_CHECK_RETURN(cudaMemcpyToSymbol(newVertices, &newVertices_tmp, sizeof(newVertices_tmp)));

    catmullClarkFacePointsAndEdges<<<(facesSize + BLOCK_SIZE - 1) / BLOCK_SIZE, BLOCK_SIZE>>>(facesSize, verticesSize, totalNewVertsToAllocate);
    std::cout << "[GPU] [catmullClarkFacePointsAndEdges] FINISHED CALLING KERNELS" << endl;
    CUDA_CHECK_RETURN(cudaDeviceSynchronize());
    std::cout << "[GPU] [catmullClarkFacePointsAndEdges] DONE" << endl;

    replaceNewVerticesWithOldVertices<<<1, 1>>>();
    std::cout << "[GPU] [replaceNewVerticesWithOldVertices] FINISHED CALLING KERNEL" << endl;
    CUDA_CHECK_RETURN(cudaDeviceSynchronize());
    std::cout << "[GPU] [replaceNewVerticesWithOldVertices] DONE" << endl;

    averageCornerVertices<<<(facesSize + BLOCK_SIZE - 1) / BLOCK_SIZE, BLOCK_SIZE>>>(facesSize);
    std::cout << "[GPU] [averageCornerVertices] FINISHED CALLING KERNELS" << endl;
    CUDA_CHECK_RETURN(cudaDeviceSynchronize());
    std::cout << "[GPU] [averageCornerVertices] DONE" << endl;

    if (mergeByDistance) {
        
        mergeByDistance<<<(facesSizeAfterSubdivision + BLOCK_SIZE - 1) / BLOCK_SIZE, BLOCK_SIZE>>>(facesSizeAfterSubdivision, verticesSize + totalNewVertsToAllocate);
        std::cout << "[GPU] [mergeByDistance] FINISHED CALLING KERNELS" << endl;
        CUDA_CHECK_RETURN(cudaDeviceSynchronize());
        std::cout << "[GPU] [mergeByDistance] DONE" << endl;
    }

    quadFace* newFaces_tmp_returnVal = new quadFace[facesSize * 4]; 
    vertex* newVertices_tmp_returnVal = new vertex[verticesSize + totalNewVertsToAllocate]; 

    std::cout << "[GPU] [cudaMemcpyFromSymbol] COPYING MESH DATA TO HOST" << endl;
    CUDA_CHECK_RETURN(cudaMemcpyFromSymbol(&newFaces_tmp_returnVal, newFaces, sizeof(newFaces)));
    CUDA_CHECK_RETURN(cudaMemcpyFromSymbol(&newVertices_tmp_returnVal, newVertices, sizeof(newVertices)));
    CUDA_CHECK_RETURN(cudaDeviceSynchronize());
    std::cout << "[GPU] [cudaMemcpyFromSymbol] DONE COPYING MESH DATA TO HOST" << endl;

    endTime = std::chrono::steady_clock::now();
    std::cout << "[CPU] [main] ELAPSED TIME " << std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count()) << "MS" << endl;

    std::cout << "[CPU] [main] WRITING MESH TO " << outputFilePath << endl;

    std::ofstream objFile;
    objFile.open(outputFilePath, ios::out | ios::trunc);

    objFile << "o EXPERIMENTAL_MESH" << endl;

    for (int i = 0; i < verticesSize + totalNewVertsToAllocate; i++) {
        
        objFile << "v " << std::to_string(newVertices_tmp_returnVal[i].position.x) << " " << std::to_string(newVertices_tmp_returnVal[i].position.y) << " " << std::to_string(newVertices_tmp_returnVal[i].position.z) << endl;
    }

    for (int i = 0; i < facesSizeAfterSubdivision ; i++) {

        objFile << "f ";

        for (int j = 0; j < 4; j++) {

            objFile << std::to_string(newFaces_tmp_returnVal[i].vertexIndex[j] + 1) << " ";
        }

        objFile << endl;
    }

    objFile.close();

    std::cout << "[CPU] [main] DONE WRITING MESH TO DISK" << endl;

    endTime = std::chrono::steady_clock::now();
    std::cout << "[END] PROGRAM TOOK " << std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count()) << "MS" << endl;
}

int main (void) {

    subdivideMeshFromFile("testMesh.obj", "testMeshOutput.obj", false);

    return 0;
}