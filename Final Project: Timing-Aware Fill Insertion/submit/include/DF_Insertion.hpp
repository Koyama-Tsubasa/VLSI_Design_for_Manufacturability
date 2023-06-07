#include <iostream>
#include <fstream>
#include <tuple>
#include <string>
#include <vector>
#include <map>
#include <omp.h>
#include <unordered_map>
#include "../include/Grid.hpp"
using namespace std;

class DF_Insertion {

	private:
		const vector<int> chip_boundary;
		const int Window_Size;
		const int number_of_CP, number_of_L, number_of_C;
		const map<int, Layer*> Layer_infos;
		const unordered_map<int, Conductor*> Conductor_infos;

	public:
		DF_Insertion(const int chip_left_, const int chip_bottom_, const int chip_right_, const int chip_top_, 
					 const int Window_Size_, const int number_of_CP_, const int number_of_L_, const int number_of_C_, 
			  		 const map<int, Layer*> &Layer_infos_, const unordered_map<int, Conductor*> &Conductor_infos_) :
			chip_boundary({chip_left_, chip_bottom_, chip_right_, chip_top_}), 
			Window_Size(Window_Size_), number_of_CP(number_of_CP_), number_of_L(number_of_L_), number_of_C(number_of_C_),
			Layer_infos(Layer_infos_), Conductor_infos(Conductor_infos_){}
		void DFI(map<int, vector<pair<pair<int, int>, pair<int, int>>>>&);
		bool dummy_fill_insertion(vector<vector<pair<int, float>>>&, vector<pair<pair<int, int>, pair<int, int>>>&, 
								  int, int, int, int, int, int, int, int, string);
		void dummy_fill_removal(vector<vector<pair<int, float>>>&, vector<pair<pair<int, int>, pair<int, int>>>&,
								int, int, int, int, int, int);			  
		bool density_refinement(vector<vector<pair<int, float>>>&, vector<pair<pair<int, int>, pair<int, int>>>&, 
								int, int, float, float, unordered_map<string, vector<pair<pair<int, int>, pair<int, int>>>>&, string);
		bool detailed_dummy_fill_insertion(vector<vector<pair<int, float>>>&, vector<pair<pair<int, int>, pair<int, int>>>&, 
										   int, int, string, int, int, int, int, unordered_map<string, vector<pair<pair<int, int>, pair<int, int>>>>&,
										   vector<pair<pair<int, int>, pair<int, int>>>&);

};
