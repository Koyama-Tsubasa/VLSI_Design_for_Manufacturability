#include "../include/Parser.hpp"

tuple<int, int, int, int, int, int, int, int, 
	  set<int>, map<int, Layer*>, unordered_map<int, Conductor*>> Parser::Input(string benchmark) {

	ifstream read_file(benchmark);
	cout << "===== Input Data =====" << endl;
	cout << "Benchmark: " << benchmark << endl;

	// read the 1st part
	int chip_left, chip_bottom, chip_right, chip_top;
	int Window_Size;
	int number_of_CP, number_of_L, number_of_C;
	read_file >> chip_left >> chip_bottom >> chip_right >> chip_top >> Window_Size >> number_of_CP >> number_of_L >> number_of_C;
	cout << "Chip Boundary: " << chip_left << " (Left), " << chip_bottom << " (Bottom), " << chip_right << " (Right), "<< chip_top << " (Top)" << endl;
	cout << "Window Size: " << Window_Size << endl;
	cout << "Number of Critical Paths: " << number_of_CP << endl;
	cout << "Number of Layers: " << number_of_L << endl;
	cout << "Number of Conductors: " << number_of_C << endl;

	// read the 2nd part
	// cout << "\n----- Critical Path IDs -----" << endl;
	set<int> CP_ids;
	for (int i=0; i<number_of_CP; i++) {

		int CP_id;

		read_file >> CP_id;
		CP_ids.insert(CP_id);

		// cout << "ID: " << CP_id << endl;

	}

	// read the 3rd part
	map<int, Layer*> Layer_info;
	for (int i=0; i<number_of_L; i++) {

		int id, min_width, min_spacing, MAX_width;
		float min_density, MAX_density, weight;

		read_file >> id >> min_width >> min_spacing >> MAX_width >> min_density >> MAX_density >> weight;
		Layer *new_layer = new Layer(id, min_spacing, min_width, MAX_width, min_density, MAX_density, weight);
		Layer_info[id] = new_layer;

		// cout << "\n----- Layer " << id << " -----" << endl;
		// cout << "Minimum Spacing: " << min_spacing << endl;
		// cout << "Minimum Fill Width: " << min_width << endl;
		// cout << "Maximum Fill Width: " << MAX_width << endl;
		// cout << "Minimum Metal Density: " << min_density << endl;
		// cout << "Maximum Metal Density: " << MAX_density << endl;
		// cout << "Layer Weight: " << weight << endl;

	}

	// read the 4th part
	unordered_map<int, Conductor*> Conductor_info;
	for (int i=0; i<number_of_C; i++) {

		int cid, nid, lid;
		int boundary_left, boundary_bottom, boundary_right, boundary_top;
		bool OC_flag = false;

		read_file >> cid >> boundary_left >> boundary_bottom >> boundary_right >> boundary_top >> nid >> lid;
		if (CP_ids.count(nid)) OC_flag = true;
		Conductor *new_conductor = new Conductor(cid, nid, lid, OC_flag, boundary_left, boundary_bottom, boundary_right, boundary_top);
		Conductor_info[cid] = new_conductor;
		Layer_info[lid]->get_conductor_in_this_layer().emplace_back(cid);
		
		// cout << "\n----- Conductor " << cid << " -----" << endl;
		// cout << "Net ID: " << nid << endl;
		// cout << "Layer ID: " << lid << endl;
		// cout << "Conductor Boundary: ";
		// cout << boundary_left << " (Left), " << boundary_bottom << " (Bottom), " << boundary_right << " (Right), "<< boundary_top << " (Top)" << endl;

	}

	cout << "======================" << endl << endl;
	return {chip_left, chip_bottom, chip_right, chip_top, Window_Size, number_of_CP, number_of_L, number_of_C,
			CP_ids, Layer_info, Conductor_info};

}

void Parser::Output(string output_file_path, map<int, vector<pair<pair<int, int>, pair<int, int>>>>& DFs) {

	cout << "===== Output Data =====" << endl;
	cout << "Number of Dummy Fills:" << endl;

	ofstream output_file(output_file_path);
	for (auto &[layer_id, dummy_fills]: DFs) {

		cout << "Layer " << layer_id << ": " << dummy_fills.size() << endl;
		for (auto &[left_bottom, right_top]: dummy_fills) 
			output_file << left_bottom.first << " " << left_bottom.second << " " << right_top.first << " " << right_top.second << " " << layer_id << endl;

	}

	cout << "=======================" << endl << endl;

}