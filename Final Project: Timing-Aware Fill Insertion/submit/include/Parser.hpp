#pragma once
#include <iostream>
#include <fstream>
#include <tuple>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include "../include/Layer.hpp"
#include "../include/Conductor.hpp"
using namespace std;

class Parser {

	public:
		tuple<int, int, int, int, int, int, int, int, 
			  set<int>, map<int, Layer*>, unordered_map<int, Conductor*>> Input(string);
		void Output(string, map<int, vector<pair<pair<int, int>, pair<int, int>>>>&);

};
