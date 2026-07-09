#define _USE_MATH_DEFINES
#define REAL double

#include "pointGenerators.h"

// gets points data from msh file
int readMSH(std::string filename, std::vector<double> &points, std::vector<int>& surfaceIndices) {
	std::ifstream filein(filename);
	if (!filein.is_open()) {
		std::cout << "Failed to open " << filename << std::endl;
		return 0;
	}

	std::string checkString;
	int numberOfPoints = 0;
	int numberOfElements = 0;

	while (filein >> checkString) {
		if (checkString == "$Nodes") {
			filein >> numberOfPoints;
			int index;
			double x, y, z;
			for (int i = 0; i < numberOfPoints; i++) {
				filein >> index >> x >> y >> z;
				points.push_back(x);
				points.push_back(y);
				points.push_back(z);
			}
		}
		else if (checkString == "$Elements") {
			filein >> numberOfElements;
			int elmNum, elmType, numTags;

			for (int i = 0; i < numberOfElements; i++) {
				filein >> elmNum >> elmType >> numTags;

				// Skip the tags (we don't need physical/elementary group tags right now)
				for (int j = 0; j < numTags; j++) {
					int tag;
					filein >> tag;
				}

				// Element type 2 is a 3-node triangle (Surface Mesh)
				if (elmType == 2) {
					int n1, n2, n3;
					filein >> n1 >> n2 >> n3;

					// Subtract 1 to convert from Gmsh 1-based to C++ 0-based indexing
					surfaceIndices.push_back(n1 - 1);
					surfaceIndices.push_back(n2 - 1);
					surfaceIndices.push_back(n3 - 1);
				}
				// Handle other common Gmsh types to skip their node data properly
				else if (elmType == 1) { // 2-node line
					int n; filein >> n >> n;
				}
				else if (elmType == 4) { // 4-node tetrahedron
					int n; filein >> n >> n >> n >> n;
				}
				else if (elmType == 15) { // 1-node point
					int n; filein >> n;
				}
			}
		}
	}
	filein.close();
	return numberOfPoints;
}

// generates msh files (gmsh version 2 files for tetrahedral meshes)
void generateMSH(std::string filename, std::vector<double>& points, std::vector<int>& tetras) {
	
	int numberOfPoints = points.size() / 3;
	int numberOfTetrahedra = tetras.size() / 4;
	std::ofstream fileout;
	fileout.open(filename);

	fileout << "$MeshFormat\n" << "2.2 0 8\n" << "$EndMeshFormat" << std::endl;

	// Writing Node data
	fileout << "$Nodes" << std::endl << numberOfPoints << std::endl;
	for (int i = 0; i < numberOfPoints; i++) {

		fileout << i + 1 << " " << points.at(3 * i) << " " << points.at((3 * i) + 1) << " " << points.at((3 * i) + 2) << std::endl;
	}
	fileout << "$EndNodes" << std::endl;
	
	// Writing Element data
	fileout << "$Elements" << std::endl << numberOfTetrahedra << std::endl;
	for (int i = 0; i < numberOfTetrahedra; i++) {
		fileout << i+1 << " 4 2 0 1 " 
		<< tetras.at(4 * i) + 1 << " " 
		<< tetras.at((4 * i) + 1) + 1 << " " 
		<< tetras.at((4 * i) + 2) + 1 << " " 
		<< tetras.at((4 * i) + 3) + 1 << "\n";
	}
	fileout << "$EndElements" << std::endl;

	fileout.close();
}

// checks if any tetrahedron lies inside the inner cylinder
double isCentroidInside3d(int nthTetrahedron, int* indices, double* points3d, double inRadius, double outRadius) {

	int point1 = indices[4 * nthTetrahedron];
	int point2 = indices[(4 * nthTetrahedron) + 1];
	int point3 = indices[(4 * nthTetrahedron) + 2];
	int point4 = indices[(4 * nthTetrahedron) + 3];

	// y coordinates are not required as we use a planar rejection criteria
	double x1 = points3d[3 * point1];
	double y1 = points3d[(3 * point1) + 1];
	double z1 = points3d[(3 * point1) + 2];

	double x2 = points3d[3 * point2];
	double y2 = points3d[(3 * point2) + 1];
	double z2 = points3d[(3 * point2) + 2];

	double x3 = points3d[3 * point3];
	double y3 = points3d[(3 * point3) + 1];
	double z3 = points3d[(3 * point3) + 2];

	double x4 = points3d[3 * point4];
	double y4 = points3d[(3 * point4) + 1];
	double z4 = points3d[(3 * point4) + 2];

	double centroidx = (x1 + x2 + x3 + x4) / 4.0;
	double centroidy = (y1 + y2 + y3 + y4) / 4.0;
	double centroidz = (z1 + z2 + z3 + z4) / 4.0;

	double dist = sqrt(centroidx * centroidx + centroidz * centroidz);

	//return int(dist > inRadius) * int(dist < outRadius);
	return dist;
}

// generates points on a circle to render a smooth circle (opengl)
void drawCircle(float* circleVertices, float x, float y, float z, float radius, int noOfVertices)
{
	for (int i = 0; i < noOfVertices; i++) {
		circleVertices[i * 3] = x + (radius * cos(i * 2 * M_PI / (noOfVertices - 2)));
		circleVertices[(i * 3) + 1] = y + (radius * sin(i * 2 * M_PI / (noOfVertices - 2)));
		circleVertices[(i * 3) + 2] = z;
	}
}

// generates random points on a given circular plane (given y plane)
void randomPointsCircle(std::vector <float> &points, float x, float y, float z, float radius, int noOfPoints)
{
	for (int i = 0; i < noOfPoints; i++) {

		float t = 2 * M_PI * (float)rand() / (RAND_MAX + 1);
		points.push_back(radius * cos(t)); // x
		points.push_back(y);			   // y
		points.push_back(radius * sin(t)); // z
	}

	// size of vector = 3 * noOfPoints
}

// Generates array of linearly spaced numbers
void linspace(std::vector <float> &yPoints, float start, float end, int noOfPoints) {

	float delta = (end - start) / ((float)noOfPoints - 1);

	for (int j = 0; j < noOfPoints; j++) {
		yPoints.push_back(start + ((float)j * delta));
	}
}

// generates points on a cylinder - type 1
void generateCylPts(std::vector <float> &points, float radius, float height, int noOfPts) {
	
	std::vector <float> ypts;
	linspace(ypts, -height/2, +height/2, noOfPts);
	
	for (int i = 0; i < noOfPts; i++) {

		float y = ypts.at(i);

		for (int j = 0; j < noOfPts; j++) {

			//float theta = 2 * M_PI * (float)rand() / (RAND_MAX + 1); // for rough mesh
			float theta = 2 * M_PI * j / (noOfPts - 2); // for a smooth mesh
			
			points.push_back(radius * cos(theta));
			points.push_back(y);
			points.push_back(radius * sin(theta));
		}
	}
}

// generates points on a cylinder - type 2
void generateCylPts2(std::vector <float> &points, float radius, float height, int noOfPts) {
	
	for (int i = 0; i < noOfPts; i++) {

		float random = (float)rand() / (RAND_MAX + 1);
		float theta = 2 * M_PI * random;

		points.push_back(radius * cos(theta));
		points.push_back(height * random);
		points.push_back(radius * sin(theta));
	}
}
