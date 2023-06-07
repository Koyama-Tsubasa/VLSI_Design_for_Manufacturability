#include "../include/DF_Insertion.hpp"

void DF_Insertion::DFI(map<int, vector<pair<pair<int, int>, pair<int, int>>>> &DFs) {

	////////////////////////////////
	// ----------				  //
	// Parameters				  //
	// ----------				  //
	// Grid: 	Grid Instructions //
	////////////////////////////////

	cout << "===== Dummy Fill Insertion =====" << endl;
	Grid Grid(chip_boundary, Conductor_infos);

	// do dummy fill insertion for each layer
	// use the technique of parallel programming
	#pragma omp parallel for num_threads(Layer_infos.size())
	for (int Layer_id=1; Layer_id<=Layer_infos.size(); Layer_id++) {
		
		auto Layer_info = Layer_infos.at(Layer_id);
		vector<vector<pair<int, float>>> Layout_Grid;
		unordered_map<string, vector<pair<pair<int, int>, pair<int, int>>>> CDG_mapping;
		vector<pair<pair<int, int>, pair<int, int>>> dummy_fills;
		
		// construcut the layout grid and calculate the routing direction
		#pragma omp critical
		cout << "[Layer: " << Layer_id << "] Layout Grid Construction ..." << endl;
		int grid_size = max(Layer_info->get_min_spacing(), Layer_info->get_min_w());
		string routing_direction = Grid.create_layout(Layout_Grid, Layer_info->get_conductor_in_this_layer(), grid_size, CDG_mapping);

		// insert dummy fills to not-keep-out area
		#pragma omp critical
		cout << "[Layer: " << Layer_id << "] Initial Dummy Fill Insertion ("  << routing_direction << ") ..." << endl;
		bool check_DF = dummy_fill_insertion(Layout_Grid, dummy_fills, Layer_info->get_MAX_w(), grid_size, 
											 0, 0, Layout_Grid[0].size()-2, Layout_Grid.size()-2, 0, -1, routing_direction);
		
		// insert dummy fills to keep-out are considering window density
		#pragma omp critical
		cout << "[Layer: " << Layer_id << "] Approximate Density Refinement ("  << routing_direction << ") ..." << endl;
		bool check_DR = density_refinement(Layout_Grid, dummy_fills, grid_size, Layer_info->get_MAX_w(), 
										   Layer_info->get_min_d(), Layer_info->get_MAX_d(), CDG_mapping, routing_direction);
		
		if (check_DR) {

			#pragma omp critical
			cout << "[Layer: " << Layer_id << "] Finish Dummy Fill Insertion !!!" << endl;

		}
		else {

			#pragma omp critical
			cout << "[Layer: " << Layer_id << "] ### Potential Constraint Violation ###" << endl;

		}
		DFs[Layer_id] = dummy_fills;
		// Grid.print_layout(Layout_Grid, 0, 400, 0, Layout_Grid.size()-1);
		// if (Layer_id == 1) break;

	}
				  
	cout << "================================" << endl << endl;

}

bool DF_Insertion::dummy_fill_insertion(
	vector<vector<pair<int, float>>> &Layout_Grid, vector<pair<pair<int, int>, pair<int, int>>> &dummy_fills, int Layer_MAXW, 
	int grid_size, int region_L, int region_B, int region_R, int region_T, int target, int desired_density, string routing_direction) {
	
	int row_MAX = Layout_Grid.size() - 1;
	int col_MAX = Layout_Grid[0].size() - 1;
	int MAX_fill_W = Layer_MAXW/grid_size;
	bool MAX_filled = false;
	bool check_DF = (desired_density == -1) ? true : false;

	if (routing_direction == "row") {

		int start_y = region_B;
		while (start_y <= region_T) {
			
			int y = start_y;
			int start_x = region_L;
			int x = region_L;
			int fill_wx = 0;
			while (x <= region_R) {
				
				// find the available longest fill size
				if (Layout_Grid[y][x].first == target) {
					
					fill_wx++;
					if ((fill_wx == MAX_fill_W) || (x == region_R)) MAX_filled = true;

				}
				else if (fill_wx > 0) {

					MAX_filled = true;
					x--;

				}
				else {

					while (Layout_Grid[y][x].first != target) {

						x++;
						if (x > region_R) break;

					}
					start_x = x;
					x--;

				}

				// start filling
				if (MAX_filled) {
					
					int fill_wy = 0;
					while (true) {
						
						bool check_y;
						
						if ((y > region_T) || (fill_wy >= MAX_fill_W)) check_y = false;
						else {
							
							check_y = true;
							for (int check=start_x; check<=x; check++) 
								if (Layout_Grid[y][check].first != target) {
									
									check_y = false;
									break;

								}

						}
						
						if (check_y) {
							
							for (int fill=start_x; fill<=x; fill++) Layout_Grid[y][fill] = {4, 1.0};
							y++;
							fill_wy++;

						}
						else {
							
							y--;
							
							// save the bottom-left idx and the top-right idx into the pair vector
							pair<pair<int, int>, pair<int, int>> new_DF = {{start_x*grid_size + chip_boundary[0], start_y*grid_size + chip_boundary[1]}, 
												   						   {(x+1)*grid_size + chip_boundary[0], (y+1)*grid_size + chip_boundary[1]}};
							dummy_fills.emplace_back(new_DF);
							
							// set minimum spacing
							int left_ms = max(start_x-1, 0); 
							int right_ms = min(x+1, col_MAX);
							int bottom_ms = max(start_y-1, 0);
							int top_ms = min(y+1, row_MAX);
							if (start_x != 0) for (int row=bottom_ms; row<=top_ms; row++) 
								if ((Layout_Grid[row][left_ms].first != 1) || (Layout_Grid[row][left_ms].first != 4)) Layout_Grid[row][left_ms].first = 2;
							if (start_y != 0) for (int col=left_ms; col<=right_ms; col++) 
								if ((Layout_Grid[bottom_ms][col].first != 1) || (Layout_Grid[bottom_ms][col].first != 4)) Layout_Grid[bottom_ms][col].first = 2;
							for (int row=bottom_ms; row<=top_ms; row++) 
								if ((Layout_Grid[row][right_ms].first != 1) || (Layout_Grid[row][right_ms].first != 4)) Layout_Grid[row][right_ms].first = 2;
							for (int col=left_ms; col<=right_ms; col++) 
								if ((Layout_Grid[top_ms][col].first != 1) || (Layout_Grid[top_ms][col].first != 4)) Layout_Grid[top_ms][col].first = 2;

							// check for the density constraint
							if (desired_density != -1) {

								desired_density -= ((x-start_x+1)*(y-start_y+1));
								if (desired_density <= 0) return true;

							}

							y = start_y;
							break;
							
						}

					}

					x++;
					start_x = x + 1;
					fill_wx = 0;
					MAX_filled = false;
					
				}

				x++;
				
			}
			start_y++;

		}

	}
	else {

		int start_x = region_L;
		while (start_x <= region_R) {
			
			int x = start_x;
			int start_y = region_B;
			int y = region_B;
			int fill_wy = 0;
			while (y <= region_T) {
				
				// find the available longest fill size
				if (Layout_Grid[y][x].first == target) {
					
					fill_wy++;
					if ((fill_wy == MAX_fill_W) || (y == region_T)) MAX_filled = true;

				}
				else if (fill_wy > 0) {

					MAX_filled = true;
					y--;

				}
				else {

					while (Layout_Grid[y][x].first != target) {

						y++;
						if (y > region_T) break;

					}
					start_y = y;
					y--;

				}

				// start filling
				if (MAX_filled) {
					
					int fill_wx = 0;
					while (true) {
						
						bool check_x;
						
						if ((x > region_R) || (fill_wx >= MAX_fill_W)) check_x = false;
						else {
							
							check_x = true;
							for (int check=start_y; check<=y; check++) 
								if (Layout_Grid[check][x].first != target) {
									
									check_x = false;
									break;

								}

						}
						
						if (check_x) {
							
							for (int fill=start_y; fill<=y; fill++) Layout_Grid[fill][x] = {4, 1.0};
							x++;
							fill_wx++;

						}
						else {
							
							x--;
							
							// save the bottom-left idx and the top-right idx into the pair vector
							pair<pair<int, int>, pair<int, int>> new_DF = {{start_x*grid_size + chip_boundary[0], start_y*grid_size + chip_boundary[1]}, 
												   						   {(x+1)*grid_size + chip_boundary[0], (y+1)*grid_size + chip_boundary[1]}};
							dummy_fills.emplace_back(new_DF);
							
							// set minimum spacing
							int left_ms = max(start_x-1, 0); 
							int right_ms = min(x+1, col_MAX);
							int bottom_ms = max(start_y-1, 0);
							int top_ms = min(y+1, row_MAX);
							if (start_x != 0) for (int row=bottom_ms; row<=top_ms; row++) 
								if ((Layout_Grid[row][left_ms].first != 1) || (Layout_Grid[row][left_ms].first != 4)) Layout_Grid[row][left_ms].first = 2;
							if (start_y != 0) for (int col=left_ms; col<=right_ms; col++) 
								if ((Layout_Grid[bottom_ms][col].first != 1) || (Layout_Grid[bottom_ms][col].first != 4)) Layout_Grid[bottom_ms][col].first = 2;
							for (int row=bottom_ms; row<=top_ms; row++) 
								if ((Layout_Grid[row][right_ms].first != 1) || (Layout_Grid[row][right_ms].first != 4)) Layout_Grid[row][right_ms].first = 2;
							for (int col=left_ms; col<=right_ms; col++) 
								if ((Layout_Grid[top_ms][col].first != 1) || (Layout_Grid[top_ms][col].first != 4)) Layout_Grid[top_ms][col].first = 2;

							// check for the density constraint
							if (desired_density != -1) {

								desired_density -= ((x-start_x+1)*(y-start_y+1));
								if (desired_density <= 0) return true;

							}

							x = start_x;
							break;
							
						}

					}

					y++;
					start_y = y + 1;
					fill_wy = 0;
					MAX_filled = false;
					
				}

				y++;
				
			}
			start_x++;

		}

	}

	return check_DF;

}

void DF_Insertion::dummy_fill_removal(vector<vector<pair<int, float>>> &Layout_Grid, vector<pair<pair<int, int>, pair<int, int>>> &dummy_fills,
									  int grid_size, int region_L, int region_B, int region_R, int region_T, int desired_density) {
	
	int row_MAX = Layout_Grid.size() - 1;
	int col_MAX = Layout_Grid[0].size() - 1;
	int limit = dummy_fills.size();
	int i=0;

	while (i < limit) {

		auto [bottom_left, top_right] = dummy_fills[i];
		if ((region_L <= bottom_left.first) && (region_B <= bottom_left.second) && 
			(top_right.first <= region_R) && (top_right.second <= region_T)) {
			
			dummy_fills.erase(dummy_fills.begin() + i);
			
			// dummy fill removal
			int left_idx = (bottom_left.first - chip_boundary[0])/grid_size;
			int bottom_idx = (bottom_left.second - chip_boundary[1])/grid_size;
			int right_idx = (top_right.first - chip_boundary[0])/grid_size - 1;
			int top_idx = (top_right.second - chip_boundary[1])/grid_size - 1;
			for (int x=left_idx; x<=right_idx; x++)
				for (int y=bottom_idx; y<=top_idx; y++)
					Layout_Grid[y][x] = {0, 0.0};
			
			// minimum spacing removal
			int left_ms = max(left_idx-1, 0); 
			int right_ms = min(right_idx+1, col_MAX);
			int bottom_ms = max(bottom_idx-1, 0);
			int top_ms = min(top_idx+1, row_MAX);
			if (left_idx != 0) for (int row=bottom_ms; row<=top_ms; row++) Layout_Grid[row][left_ms].first = 0;
			if (bottom_idx != 0) for (int col=left_ms; col<=right_ms; col++) Layout_Grid[bottom_ms][col].first = 0;
			for (int row=bottom_ms; row<=top_ms; row++) Layout_Grid[row][right_ms].first = 0;
			for (int col=left_ms; col<=right_ms; col++) Layout_Grid[top_ms][col].first = 0;

			// check for the density
			desired_density -= ((right_idx-left_idx+1)*(top_idx-bottom_idx+1));
			if (desired_density < 0) return;
			limit--;
			
		}
		else i++;

	}

}

bool DF_Insertion::density_refinement(vector<vector<pair<int, float>>> &Layout_Grid, vector<pair<pair<int, int>, pair<int, int>>> &dummy_fills, 
							    	  int grid_size, int Layer_MAXW, float min_density, float MAX_density, 
									  unordered_map<string, vector<pair<pair<int, int>, pair<int, int>>>> &CDG_mapping, string routing_direction) {

	int acceptable_violation_num = 22;	
	int constraint_violations = 0;		
	// bool accept = true;						
	int row_MAX = Layout_Grid.size() - 1, col_MAX = Layout_Grid[0].size() - 1;
	int relative_width = chip_boundary[2] - chip_boundary[0];
	int relative_height = chip_boundary[3] - chip_boundary[1];
	int WMove_unit = Window_Size/4;
	int grid_area = grid_size*grid_size;
	vector<pair<pair<int, int>, pair<int, int>>> DCDG_mapping;

	if (routing_direction == "row") {

		int curr_bottom = 0, curr_top = Window_Size - 1;
		while (curr_top <= relative_height) {

			int bottom_idx = curr_bottom/grid_size + 1;
			int top_idx = min(curr_top/grid_size - 1, row_MAX);

			int curr_left = 0;
			int curr_right = Window_Size - 1;
			while (curr_right <= relative_width) {
				
				int left_idx = curr_left/grid_size + 1;
				int right_idx = min(curr_right/grid_size - 1, col_MAX);
				float window_density = 0.0;
				int window_grid_num = (right_idx-left_idx+1)*(top_idx-bottom_idx+1);
				
				for (int x=left_idx; x<=right_idx; x++)
					for (int y=bottom_idx; y<=top_idx; y++) 
						if ((Layout_Grid[y][x].first == 1) || (Layout_Grid[y][x].first == 4))
						window_density += Layout_Grid[y][x].second;
				
				window_density = window_density*grid_size*grid_size/(Window_Size*Window_Size);
				if (min_density > window_density) {

					int current_approx_grids = window_grid_num*window_density;
					int desired_min_grids = window_grid_num*min_density;
					if (top_idx == row_MAX) top_idx--;
					if (right_idx == col_MAX) right_idx--;
					bool check_DF = dummy_fill_insertion(Layout_Grid, dummy_fills, Layer_MAXW, grid_size, left_idx, bottom_idx, right_idx, top_idx,
										 				 3, desired_min_grids-current_approx_grids, routing_direction);
					bool check_DDF = true;
					if (!check_DF) check_DDF = detailed_dummy_fill_insertion(Layout_Grid, dummy_fills, Layer_MAXW, grid_size, routing_direction,
																			 left_idx, bottom_idx, right_idx, top_idx, CDG_mapping, DCDG_mapping);
					if (!check_DDF) constraint_violations++;
					if (constraint_violations > acceptable_violation_num) return false;
					// if (!check_DDF) accept = false;

				}
				else if (window_density > MAX_density) {

					int current_approx_grids = window_grid_num*window_density;
					int desired_MAX_grids = window_grid_num*MAX_density;
					dummy_fill_removal(Layout_Grid, dummy_fills, grid_size, 
									   curr_left+chip_boundary[0], curr_bottom+chip_boundary[1], curr_right+chip_boundary[0], curr_top+chip_boundary[1],
									   current_approx_grids-desired_MAX_grids);

				}
				
				curr_left += WMove_unit;
				curr_right += WMove_unit;

			}
			
			curr_bottom += WMove_unit;
			curr_top += WMove_unit;

		}

	}
	else {

		int curr_left = 0, curr_right = Window_Size - 1;
		while (curr_right <= relative_width) {

			int left_idx = curr_left/grid_size + 1;
			int right_idx = min(curr_right/grid_size - 1, col_MAX);

			int curr_bottom = 0;
			int curr_top = Window_Size - 1;
			while (curr_top <= relative_height) {
				
				int bottom_idx = curr_bottom/grid_size + 1;
				int top_idx = min(curr_top/grid_size - 1, row_MAX);
				float window_density = 0.0;
				int window_grid_num = (right_idx-left_idx+1)*(top_idx-bottom_idx+1);
				
				for (int x=left_idx; x<=right_idx; x++)
					for (int y=bottom_idx; y<=top_idx; y++) 
						window_density += Layout_Grid[y][x].second;
				
				window_density = window_density*grid_size*grid_size/(Window_Size*Window_Size);
				if (min_density > window_density) {

					int current_approx_grids = window_grid_num*window_density;
					int desired_min_grids = window_grid_num*min_density;
					if (top_idx == row_MAX) top_idx--;
					if (right_idx == col_MAX) right_idx--;
					bool check_DF = dummy_fill_insertion(Layout_Grid, dummy_fills, Layer_MAXW, grid_size, left_idx, bottom_idx, right_idx, top_idx,
										 				 3, desired_min_grids-current_approx_grids, routing_direction);
					bool check_DDF = true;
					if (!check_DF) check_DDF = detailed_dummy_fill_insertion(Layout_Grid, dummy_fills, Layer_MAXW, grid_size, routing_direction,
																			 left_idx, bottom_idx, right_idx, top_idx, CDG_mapping, DCDG_mapping);
					if (!check_DDF) constraint_violations++;
					if (constraint_violations > acceptable_violation_num) return false;
					// if (!check_DDF) accept = false;

				}
				else if (window_density > MAX_density) {

					int current_approx_grids = window_grid_num*window_density;
					int desired_MAX_grids = window_grid_num*MAX_density;
					dummy_fill_removal(Layout_Grid, dummy_fills, grid_size, 
									   curr_left+chip_boundary[0], curr_bottom+chip_boundary[1], curr_right+chip_boundary[0], curr_top+chip_boundary[1],
									   current_approx_grids-desired_MAX_grids);

				}
				
				curr_bottom += WMove_unit;
				curr_top += WMove_unit;

			}
			
			curr_left += WMove_unit;
			curr_right += WMove_unit;

		}

	}

	return true;
	// return accept;

}

bool DF_Insertion::detailed_dummy_fill_insertion(
	vector<vector<pair<int, float>>> &Layout_Grid, vector<pair<pair<int, int>, pair<int, int>>> &dummy_fills, int Layer_MAXW, 
	int grid_size, string routing_direction, int region_L, int region_B, int region_R, int region_T, 
	unordered_map<string, vector<pair<pair<int, int>, pair<int, int>>>> &CDG_mapping,
	vector<pair<pair<int, int>, pair<int, int>>> &DCDG_mapping) {

	int Crow_MAX = Layout_Grid.size() - 1;
	int Ccol_MAX = Layout_Grid[0].size() - 1;
	int region_B_W = region_B;
	int region_T_W = region_T;
	int region_L_W = region_L;
	int region_R_W = region_R;
	for (int i=0; i<2; i++) {

		region_B_W = max(region_B-1, 0);
		region_T_W = min(region_T+1, Crow_MAX);
		region_L_W = max(region_L-1, 0);
		region_R_W = min(region_R+1, Ccol_MAX);

	}
	vector<vector<int>> DLG((region_T_W-region_B_W+1)*grid_size, vector<int>((region_R_W-region_L_W+1)*grid_size, 0));
	int row_MAX = (region_T_W != region_T) ? DLG.size() - grid_size - 1 : DLG.size() - 1;
	int row_min = (region_B_W != region_B) ? grid_size : 0;
	int col_MAX = (region_R_W != region_R) ? DLG[0].size() - grid_size - 1 : DLG[0].size() - 1;
	int col_min = (region_L_W != region_L) ? grid_size : 0;
	int DLG_RM = DLG.size() - 1;
	int DLG_CM = DLG[0].size() - 1;

	// set grid value
	for (int row=region_B_W; row<=region_T_W; row++)
		for (int col=region_L_W; col<=region_R_W; col++) {

			int relative_left = (col - region_L_W)*grid_size;
			int relative_right = relative_left + grid_size - 1;
			int relative_bottom = (row - region_B_W)*grid_size;
			int relative_top = relative_bottom + grid_size - 1;

			if (Layout_Grid[row][col].second == 1.0) {

				for (int fill_R=relative_bottom; fill_R<=relative_top; fill_R++)
					for (int fill_C=relative_left; fill_C<=relative_right; fill_C++)
						DLG[fill_R][fill_C] = 1;
				
				int left_ms = relative_left;
				int right_ms = relative_right;
				int bottom_ms = relative_bottom;
				int top_ms = relative_top;
				for (int ring=0; ring<grid_size; ring++) {

					left_ms = max(left_ms-1, 0);
					right_ms = min(right_ms+1, DLG_CM);
					bottom_ms = max(bottom_ms-1, 0);
					top_ms = min(top_ms+1, DLG_RM);
					for (int fill=bottom_ms; fill<=top_ms; fill++) {

						if (DLG[fill][left_ms] == 0) DLG[fill][left_ms] = 2;
						if (DLG[fill][right_ms] == 0) DLG[fill][right_ms] = 2;

					}
					for (int fill=left_ms; fill<=right_ms; fill++) {

						if (DLG[bottom_ms][fill] == 0) DLG[bottom_ms][fill] = 2;
						if (DLG[top_ms][fill] == 0) DLG[top_ms][fill] = 2;

					}

				}

			}
			else if ((Layout_Grid[row][col].second > 0.0) && (Layout_Grid[row][col].first == 1)) {

				for (auto &already_densed: CDG_mapping[to_string(row)+"_"+to_string(col)]) {
		
					for (int R=relative_bottom+already_densed.first.second; R<=relative_bottom+already_densed.second.second; R++) 
						for (int C=relative_left+already_densed.first.first; C<=relative_left+already_densed.second.first; C++)
							DLG[R][C] = 1;
					
					int bottom_ms = relative_bottom+already_densed.first.second;
					int top_ms = relative_bottom+already_densed.second.second;
					int left_ms = relative_left+already_densed.first.first;
					int right_ms = relative_left+already_densed.second.first;
					for (int ring=0; ring<grid_size; ring++) {

						left_ms = max(left_ms-1, 0);
						right_ms = min(right_ms+1, DLG_CM);
						bottom_ms = max(bottom_ms-1, 0);
						top_ms = min(top_ms+1, DLG_RM);
						for (int fill=bottom_ms; fill<=top_ms; fill++) {

							if (DLG[fill][left_ms] == 0) DLG[fill][left_ms] = 2;
							if (DLG[fill][right_ms] == 0) DLG[fill][right_ms] = 2;

						}
						for (int fill=left_ms; fill<=right_ms; fill++) {

							if (DLG[bottom_ms][fill] == 0) DLG[bottom_ms][fill] = 2;
							if (DLG[top_ms][fill] == 0) DLG[top_ms][fill] = 2;

						}

					}

				}

			}

		}
	
	int Bregion_L = region_L_W*grid_size;
	int Bregion_R = (region_R_W + 1)*grid_size - 1;
	int Bregion_B = region_B_W*grid_size;
	int Bregion_T = (region_T_W + 1)*grid_size - 1;
	for (auto &detailed_filled: DCDG_mapping) {
		
		int Region_bottom = max(detailed_filled.first.second, Bregion_B);
		int Region_top = min(detailed_filled.second.second, Bregion_T);
		int Region_left = max(detailed_filled.first.first, Bregion_L);
		int Region_right = min(detailed_filled.second.first, Bregion_R);
		
		if ((Region_bottom <= Region_top) && (Region_left <= Region_right)) {
			
			for (int R=Region_bottom-Bregion_B; R<=Region_top-Bregion_B; R++)
				for (int C=Region_left-Bregion_L; C<=Region_right-Bregion_L; C++)
					DLG[R][C] = 1;
			
			int left_ms = Region_left - Bregion_L;
			int right_ms = Region_right - Bregion_L;
			int bottom_ms = Region_bottom - Bregion_B;
			int top_ms = Region_top - Bregion_B;
			for (int ring=0; ring<grid_size; ring++) {

				left_ms = max(left_ms-1, 0);
				right_ms = min(right_ms+1, DLG_CM);
				bottom_ms = max(bottom_ms-1, 0);
				top_ms = min(top_ms+1, DLG_RM);
				for (int fill=bottom_ms; fill<=top_ms; fill++) {

					if (DLG[fill][left_ms] == 0) DLG[fill][left_ms] = 2;
					if (DLG[fill][right_ms] == 0) DLG[fill][right_ms] = 2;

				}
				for (int fill=left_ms; fill<=right_ms; fill++) {

					if (DLG[bottom_ms][fill] == 0) DLG[bottom_ms][fill] = 2;
					if (DLG[top_ms][fill] == 0) DLG[top_ms][fill] = 2;

				}

			}
			
		}

	}
	
	// calculate density
	float desired_density = Window_Size*Window_Size*0.4;
	for (int row=row_min; row<=row_MAX; row++)
		for (int col=col_min; col<=col_MAX; col++)
			if (DLG[row][col] == 1) desired_density -= 1.0;

	// dummy fill insertion
	bool check_DDF = false;
	bool MAX_filled = false;
	if (routing_direction == "row") {

		int start_y = row_min;
		while (start_y <= row_MAX) {
			
			int y = start_y;
			int start_x = col_min;
			int x = col_min;
			int fill_wx = 0;
			while (x <= col_MAX) {
				
				// find the available longest fill size
				if (DLG[y][x] == 0) {
					
					fill_wx++;
					if ((fill_wx == Layer_MAXW) || (x == col_MAX)) MAX_filled = true;

				}
				else if (fill_wx >= grid_size) {
					
					MAX_filled = true;
					x--;

				}
				else {

					fill_wx = 0;
					while (DLG[y][x] != 0) {

						x++;
						if (x > col_MAX) break;

					}
					start_x = x;
					x--;

				}

				// start filling
				if (MAX_filled) {
					
					if (fill_wx < grid_size) break;
					int fill_wy = 0;
					while (true) {
						
						bool check_y;
						
						if ((y > row_MAX) || (fill_wy >= Layer_MAXW)) check_y = false;
						else {
							
							check_y = true;
							for (int check=start_x; check<=x; check++) 
								if (DLG[y][check] != 0) {
									
									check_y = false;
									break;

								}

						}
						
						if (check_y) {
							
							for (int fill=start_x; fill<=x; fill++) DLG[y][fill] = 1;
							y++;
							fill_wy++;

						}
						else {
							
							y--;
							if (fill_wy >= grid_size) {
								
								// save the bottom-left idx and the top-right idx into the pair vector
								pair<pair<int, int>, pair<int, int>> new_DF = {{region_L_W*grid_size + start_x + chip_boundary[0], 
																				region_B_W*grid_size + start_y + chip_boundary[1]}, 
																			   {region_L_W*grid_size + x + 1 + chip_boundary[0], 
																			    region_B_W*grid_size + y + 1 + chip_boundary[1]}};
								dummy_fills.emplace_back(new_DF);
								pair<pair<int, int>, pair<int, int>> relative_coordinate = {{region_L_W*grid_size + start_x, region_B_W*grid_size + start_y}, 
																							{region_L_W*grid_size + x, region_B_W*grid_size + y}};
								DCDG_mapping.emplace_back(relative_coordinate);
								
								// set minimum spacing
								int bottom_ms = start_y;
								int top_ms = y;
								int left_ms = start_x;
								int right_ms = x;
								for (int ring=0; ring<grid_size; ring++) {

									left_ms = max(left_ms-1, 0);
									right_ms = min(right_ms+1, col_MAX);
									bottom_ms = max(bottom_ms-1, 0);
									top_ms = min(top_ms+1, row_MAX);
									for (int fill=bottom_ms; fill<=top_ms; fill++) {

										if (DLG[fill][left_ms] == 0) DLG[fill][left_ms] = 2;
										if (DLG[fill][right_ms] == 0) DLG[fill][right_ms] = 2;

									}
									for (int fill=left_ms; fill<=right_ms; fill++) {

										if (DLG[bottom_ms][fill] == 0) DLG[bottom_ms][fill] = 2;
										if (DLG[top_ms][fill] == 0) DLG[top_ms][fill] = 2;

									}

								}

								// check for the density constraint
								desired_density -= float((x-start_x+1)*(y-start_y+1));
								if (desired_density <= 0.0) return true;

							}

							y = start_y;
							break;
							
						}

					}

					x++;
					start_x = x + 1;
					fill_wx = 0;
					MAX_filled = false;
					
				}

				x++;
				
			}
			start_y++;

		}

	}
	else {

		int start_x = col_min;
		while (start_x <= col_MAX) {
			
			int x = start_x;
			int start_y = row_min;
			int y = row_min;
			int fill_wy = 0;
			while (y <= row_MAX) {
				
				// find the available longest fill size
				if (DLG[y][x] == 0) {
					
					fill_wy++;
					if ((fill_wy == Layer_MAXW) || (y == row_MAX)) MAX_filled = true;

				}
				else if (fill_wy >= grid_size) {

					MAX_filled = true;
					y--;

				}
				else {

					fill_wy = 0;
					while (DLG[y][x] != 0) {

						y++;
						if (y > row_MAX) break;

					}
					start_y = y;
					y--;

				}

				// start filling
				if (MAX_filled) {
					
					if (fill_wy < grid_size) break;
					int fill_wx = 0;
					while (true) {
						
						bool check_x;
						
						if ((x > col_MAX) || (fill_wx >= Layer_MAXW)) check_x = false;
						else {
							
							check_x = true;
							for (int check=start_y; check<=y; check++) 
								if (DLG[check][x] != 0) {
									
									check_x = false;
									break;

								}

						}
						
						if (check_x) {
							
							for (int fill=start_y; fill<=y; fill++) DLG[fill][x] = 3;
							x++;
							fill_wx++;

						}
						else {
							
							x--;
							if (fill_wx >= grid_size) {

								// save the bottom-left idx and the top-right idx into the pair vector
								pair<pair<int, int>, pair<int, int>> new_DF = {{region_L_W*grid_size + start_x + chip_boundary[0],
																				region_B_W*grid_size + start_y + chip_boundary[1]}, 
																			   {region_L_W*grid_size + x + 1 + chip_boundary[0], 
																			   	region_B_W*grid_size + y + 1 + chip_boundary[1]}};
								dummy_fills.emplace_back(new_DF);
								pair<pair<int, int>, pair<int, int>> relative_coordinate = {{region_L_W*grid_size + start_x, region_B_W*grid_size + start_y}, 
																							{region_L_W*grid_size + x, region_B_W*grid_size + y}};
								DCDG_mapping.emplace_back(relative_coordinate);

								// set minimum spacing
								int bottom_ms = start_y;
								int top_ms = y;
								int left_ms = start_x;
								int right_ms = x;
								for (int ring=0; ring<grid_size; ring++) {

									left_ms = max(left_ms-1, 0);
									right_ms = min(right_ms+1, col_MAX);
									bottom_ms = max(bottom_ms-1, 0);
									top_ms = min(top_ms+1, row_MAX);
									for (int fill=bottom_ms; fill<=top_ms; fill++) {

										if (DLG[fill][left_ms] == 0) DLG[fill][left_ms] = 2;
										if (DLG[fill][right_ms] == 0) DLG[fill][right_ms] = 2;

									}
									for (int fill=left_ms; fill<=right_ms; fill++) {

										if (DLG[bottom_ms][fill] == 0) DLG[bottom_ms][fill] = 2;
										if (DLG[top_ms][fill] == 0) DLG[top_ms][fill] = 2;

									}

								}

								// check for the density constraint
								desired_density -= float((x-start_x+1)*(y-start_y+1));
								if (desired_density <= 0.0) return true;

							}

							x = start_x;
							break;
							
						}

					}

					y++;
					start_y = y + 1;
					fill_wy = 0;
					MAX_filled = false;
					
				}

				y++;
				
			}
			start_x++;

		}

	}

	return check_DDF;

}