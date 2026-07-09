#ifndef POINT_GENERATORS_H
#define POINT_GENERATORS_H
#define _USE_MATH_DEFINES

#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <glad/glad.h>

int readMSH(std::string filename, std::vector<double> &points, std::vector<int>& surfaceIndices);

void generateMSH(std::string filename, std::vector<double>& points, std::vector<int>& indices);

void drawCircle(float* circleVertices, float x, float y, float z, float radius, int noOfVertices);

void randomPointsCircle(std:: vector<float> &points, float x, float y, float z, float radius, int noOfPoints);

void generateCylPts2(std::vector <float>& points, float radius, float height, int noOfPts);

void linspace(std::vector <float>& yPoints, float start, float end, int noOfPoints);

void generateCylPts(std::vector<float> &points, float radius, float height, int noOfPts);

double isCentroidInside3d(int nthTetrahedron, int* indices, double* points3d, double inRadius, double outRadius);


#endif // !POINT_GENERATORS_H