#define TETLIBRARY

#include "pointGenerators.h"
#include "tetgen.h"

#include "meshView.h"

#include <chrono>

int main() {
	auto start_time = std::chrono::system_clock::now();
	
	std::string input_format = "generated"; // or "gmsh"
	
	std::vector <double> totalPoints3D;

	tetgenio in, out;
	float inRadius = 10;
	float outRadius = 12;
	float height = 10;

	if (input_format == "generated")
	{
		// Generating point data
		const int noOfRandomPts = 15; // number of points in circle plane

		std::vector <float> inPoints3D;
		std::vector <float> outPoints3D;

		generateCylPts(inPoints3D, inRadius, height, noOfRandomPts);
		generateCylPts(outPoints3D, outRadius, height, noOfRandomPts);

		totalPoints3D.reserve(inPoints3D.size() + outPoints3D.size());
		totalPoints3D.insert(totalPoints3D.end(), inPoints3D.begin(), inPoints3D.end());
		totalPoints3D.insert(totalPoints3D.end(), outPoints3D.begin(), outPoints3D.end());
		

		in.pointlist = new REAL[in.numberofpoints * 3];
		in.numberofpoints = (totalPoints3D.size()) / 3;
		in.pointlist = new REAL[in.numberofpoints * 3];

		for (int i = 0; i < totalPoints3D.size(); i++) {
			in.pointlist[i] = totalPoints3D.at(i);
		}

		// for tetgen - no modifications
		char switches[] = { '\0' };
		tetrahedralize(switches, &in, &out);
	}
	else
	{
		// read from a gmsh file
		std::vector<int> surfaceIndices;

		int numberOfPoints;

		// adding all points to be indexed
		numberOfPoints = readMSH("./meshfiles/sphere.msh", totalPoints3D, surfaceIndices);
		inRadius = 0; // for this geometry we need to set inRadius = 0

		// pass node data to tetgen
		in.numberofpoints = numberOfPoints;
		in.pointlist = new double[in.numberofpoints * 3];
		std::copy(totalPoints3D.begin(), totalPoints3D.end(), in.pointlist);

		// pass surface triangles to tetgen
		in.numberoffacets = surfaceIndices.size() / 3;
		in.facetlist = new tetgenio::facet[in.numberoffacets];
		in.facetmarkerlist = new int[in.numberoffacets];

		for (int i = 0; i < in.numberoffacets; i++) {
			tetgenio::facet* f = &in.facetlist[i];
			f->polygonlist = new tetgenio::polygon[1];
			f->numberofpolygons = 1;
			f->holelist = NULL;
			f->numberofholes = 0;

			tetgenio::polygon* p = &f->polygonlist[0];
			p->numberofvertices = 3;
			p->vertexlist = new int[3];

			// Feed the 3 nodes of each triangle into the polygon vertex list
			p->vertexlist[0] = surfaceIndices[i * 3];
			p->vertexlist[1] = surfaceIndices[i * 3 + 1];
			p->vertexlist[2] = surfaceIndices[i * 3 + 2];

			in.facetmarkerlist[i] = 1; // Arbitrary marker for the boundary
		}
		// for tetgen - enable PLC
		char switches[] = "p";
		tetrahedralize(switches, &in, &out);

		totalPoints3D.clear(); // Empty the old nodes
	
		// Repopulate with the final node list (including new Steiner points)
		for (int i = 0; i < out.numberofpoints * 3; i++) {
			totalPoints3D.push_back(out.pointlist[i]);
		}
	}

	std::vector<int> newTetrahedronList;
	std::vector<GLuint> indices; 


	for (int x = 0; x < out.numberoftetrahedra; x++) {

		double centroid_dist = isCentroidInside3d(x, out.tetrahedronlist, out.pointlist, inRadius, outRadius);
		int decision = centroid_dist > inRadius;

		if (decision) {
			newTetrahedronList.push_back(out.tetrahedronlist[4 * x]);		// face 1 - 0-1-2
			newTetrahedronList.push_back(out.tetrahedronlist[4 * x + 1]);
			newTetrahedronList.push_back(out.tetrahedronlist[4 * x + 2]);
			newTetrahedronList.push_back(out.tetrahedronlist[4 * x + 3]);
			
		}

	}

	auto end_time = std::chrono::system_clock::now();

	auto time_taken = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
	std::cout << "time taken : " << time_taken << "ms" << std::endl;

	for (int x = 0; x < newTetrahedronList.size()/4; x++) {

		indices.push_back(newTetrahedronList[4 * x]);			// face 1 - 0-1-2
		indices.push_back(newTetrahedronList[4 * x + 1]);
		indices.push_back(newTetrahedronList[4 * x + 2]);
						  				
		indices.push_back(newTetrahedronList[4 * x + 1]);		// face 2 - 1-2-3
		indices.push_back(newTetrahedronList[4 * x + 2]);
		indices.push_back(newTetrahedronList[4 * x + 3]);
						  				
		indices.push_back(newTetrahedronList[4 * x + 2]);		// face 3 - 2-3-0
		indices.push_back(newTetrahedronList[4 * x + 3]);
		indices.push_back(newTetrahedronList[4 * x]);
						  				
		indices.push_back(newTetrahedronList[4 * x + 3]);		// face 4 - 3-0-1
		indices.push_back(newTetrahedronList[4 * x]);
		indices.push_back(newTetrahedronList[4 * x + 1]);
	}

	displayMesh(totalPoints3D, indices); // this broke for large data sets, works for smaller ones though

	//generateMSH("final2.msh", totalPoints3D, newTetrahedronList);

	return 0;
}
