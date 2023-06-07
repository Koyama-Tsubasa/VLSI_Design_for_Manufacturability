#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include "../include/Layer.hpp"
#include "../include/Conductor.hpp"
using namespace std;

class Grid {

	////////////////////////////////////////////////////////
	// -----------										  //
	// Grid Values										  //
	// -----------										  //
	// first:											  //
	// - 0: empty grid 	(no features/dummy fills)		  //
	// - 1: feature 	(all kinds of conductors)		  //
	// - 2: min_spacing	(min-spacing area of conductors)  //
	// - 3: KO area		(keep out areas of critical nets) //
	// - 4: dummy fill	(dummy fills)					  //
	// second: density									  //
	////////////////////////////////////////////////////////

	private:
		const vector<int> chip_boundary;
		const unordered_map<int, Conductor*> Conductor_infos;
		pair<int, int> create_layout_grid(vector<vector<pair<int, float>>>&, int);
		string conductor_insertion(vector<vector<pair<int, float>>>&, vector<int>&, int, int, int,
								   unordered_map<string, vector<pair<pair<int, int>, pair<int, int>>>>&);
		float set_density(unordered_map<string, vector<pair<pair<int, int>, pair<int, int>>>>&, 
						  int, int, int, int, int, int, int);

	public:
		Grid (const vector<int> &chip_boundary_, const unordered_map<int, Conductor*> &Conductor_infos_) : 
			chip_boundary(chip_boundary_), Conductor_infos(Conductor_infos_){}
		string create_layout(vector<vector<pair<int, float>>>&, vector<int>&, int, 
							 unordered_map<string, vector<pair<pair<int, int>, pair<int, int>>>>&);
		void print_layout(vector<vector<pair<int, float>>>&, int, int, int, int);

};