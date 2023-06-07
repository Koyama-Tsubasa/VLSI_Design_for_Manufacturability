#include "../include/Grid.hpp"

string Grid::create_layout(vector<vector<pair<int, float>>> &Layout_Grid, vector<int> &Conductor_IDs, int grid_size,
						   unordered_map<string, vector<pair<pair<int, int>, pair<int, int>>>> &CDG_mapping) {
	
	// recreate the layout grid to the corresponding layer
	auto [row_MAX, col_MAX] = create_layout_grid(Layout_Grid, grid_size);
	
	// fill the features
	string routing_direction = conductor_insertion(Layout_Grid, Conductor_IDs, grid_size, row_MAX, col_MAX, CDG_mapping);

	return routing_direction;

}

pair<int, int> Grid::create_layout_grid(vector<vector<pair<int, float>>> &Layout_Grid, int grid_size) {

	int row_num = (chip_boundary[3]-chip_boundary[1])/grid_size;
	int col_num = (chip_boundary[2]-chip_boundary[0])/grid_size;
	Layout_Grid.resize(row_num+1, vector<pair<int, float>>(col_num+1, {0, 0.0}));

	return {row_num, col_num};

}

string Grid::conductor_insertion(vector<vector<pair<int, float>>> &Layout_Grid, vector<int> &Conductor_IDs, int grid_size, int row_MAX, int col_MAX,
								 unordered_map<string, vector<pair<pair<int, int>, pair<int, int>>>> &CDG_mapping) {

	long long width = 0, height = 0;
	vector<int> critical_ids;

	for (auto cid: Conductor_IDs) {

		// conductor boundary
		auto conductor = Conductor_infos.at(cid);
		auto conductor_boundary = conductor->get_boundary();
		width += (conductor_boundary[2]-conductor_boundary[0]);
		height += (conductor_boundary[3]-conductor_boundary[1]);
		int relative_left = conductor_boundary[0]-chip_boundary[0];
		int relative_right = conductor_boundary[2]-chip_boundary[0]-1;
		int relative_bottom = conductor_boundary[1]-chip_boundary[1];
		int relative_top = conductor_boundary[3]-chip_boundary[1]-1;
		if (conductor->is_ctirical()) critical_ids.emplace_back(cid);
		
		// insert inner ring conductor
		int left_idx = relative_left/grid_size;
		int right_idx = min(relative_right/grid_size, col_MAX);
		int bottom_idx = relative_bottom/grid_size;
		int top_idx = min(relative_top/grid_size, row_MAX);
		for (int row=bottom_idx+1; row<=top_idx-1; row++)
			for (int col=left_idx+1; col<=right_idx-1; col++)
				Layout_Grid[row][col] = {1, 1.0};
		
		// insert outer ring conductor
		int left_per = (relative_left%grid_size != 0) ? relative_left%grid_size : 0;
		int right_per = (relative_right%grid_size != 0) ? relative_right%grid_size : 0;
		int bottom_per = (relative_bottom%grid_size != 0) ? relative_bottom%grid_size : 0;
		int top_per = (relative_top%grid_size != 0) ? relative_top%grid_size : 0;
		for (int col=left_idx+1; col<=right_idx-1; col++) {
			
			if (Layout_Grid[bottom_idx][col].second != 1.0)
				Layout_Grid[bottom_idx][col] = {1, set_density(CDG_mapping, grid_size, bottom_idx, col,
															   0, grid_size-1, bottom_per, grid_size-1)};
			if (Layout_Grid[top_idx][col].second != 1.0)
				Layout_Grid[top_idx][col] = {1, set_density(CDG_mapping, grid_size, top_idx, col,
															0, grid_size-1, 0, top_per)};

		}
		for (int row=bottom_idx+1; row<=top_idx-1; row++) {

			if (Layout_Grid[row][left_idx].second != 1.0)
				Layout_Grid[row][left_idx] = {1, set_density(CDG_mapping, grid_size, row, left_idx, 
															 left_per, grid_size-1, 0, grid_size-1)};
			if (Layout_Grid[row][right_idx].second != 1.0)
				Layout_Grid[row][right_idx] = {1, set_density(CDG_mapping, grid_size, row, right_idx, 
															  0, right_per, 0, grid_size-1)};

		}
		if (Layout_Grid[bottom_idx][left_idx].second != 1.0)
			Layout_Grid[bottom_idx][left_idx] = {1, set_density(CDG_mapping, grid_size, bottom_idx, left_idx,
																left_per, grid_size-1, bottom_per, grid_size-1)};
		if (Layout_Grid[bottom_idx][right_idx].second != 1.0)
			Layout_Grid[bottom_idx][right_idx] = {1, set_density(CDG_mapping, grid_size, bottom_idx, right_idx,
																 0, right_per, bottom_per, grid_size-1)};
		if (Layout_Grid[top_idx][right_idx].second != 1.0)
			Layout_Grid[top_idx][right_idx] = {1, set_density(CDG_mapping, grid_size, top_idx, right_idx,
															  0, right_per, 0, top_per)};
		if (Layout_Grid[top_idx][left_idx].second != 1.0)
			Layout_Grid[top_idx][left_idx] = {1, set_density(CDG_mapping, grid_size, top_idx, left_idx,
															 left_per, grid_size-1, 0, top_per)};
		
		// set minimum spacing areas
		int left_ms = max(left_idx-1, 0); 
		int right_ms = min(right_idx+1, col_MAX);
		int bottom_ms = max(bottom_idx-1, 0);
		int top_ms = min(top_idx+1, row_MAX);
		if (left_ms != left_idx) for (int row=bottom_ms; row<=top_ms; row++) if (Layout_Grid[row][left_ms].first != 1) Layout_Grid[row][left_ms].first = 2;
		if (right_ms != right_idx) for (int row=bottom_ms; row<=top_ms; row++) if (Layout_Grid[row][right_ms].first != 1) Layout_Grid[row][right_ms].first = 2;
		if (bottom_ms != bottom_idx) for (int col=left_ms; col<=right_ms; col++) if (Layout_Grid[bottom_ms][col].first != 1) Layout_Grid[bottom_ms][col].first = 2;
		if (top_ms != top_idx) for (int col=left_ms; col<=right_ms; col++) if (Layout_Grid[top_ms][col].first != 1) Layout_Grid[top_ms][col].first = 2;
		
	}

	// set keep out regions if the conductor is critical
	for (auto ccid: critical_ids) {

		auto conductor = Conductor_infos.at(ccid);
		bool check;
		auto conductor_boundary = conductor->get_boundary();
		int left_idx = (conductor_boundary[0]-chip_boundary[0])/grid_size;
		int right_idx = min((conductor_boundary[2]-chip_boundary[0]-1)/grid_size, col_MAX);
		int bottom_idx = (conductor_boundary[1]-chip_boundary[1])/grid_size;
		int top_idx = min((conductor_boundary[3]-chip_boundary[1]-1)/grid_size, row_MAX);
		int left_idx_critical = (conductor_boundary[0]-1600-chip_boundary[0])/grid_size;
		int right_idx_critical = (conductor_boundary[2]+1600-chip_boundary[0])/grid_size;
		int bottom_idx_critical = (conductor_boundary[1]-1600-chip_boundary[1])/grid_size;
		int top_idx_critical = (conductor_boundary[3]+1600-chip_boundary[1])/grid_size;


		// left-right
		for (int row=bottom_idx; row<=top_idx; row++) {

			for (int col=left_idx-2; col>=max(left_idx_critical, 0); col--) {

				if (Layout_Grid[row][col].first == 0) Layout_Grid[row][col].first = 3;
				else break;

			}

			for (int col=right_idx+2; col<=min(right_idx_critical, col_MAX); col++) {

				if (Layout_Grid[row][col].first == 0) Layout_Grid[row][col].first = 3;
				else break;

			}

		}

		// bottom-top
		for (int col=left_idx; col<=right_idx; col++) {

			for (int row=bottom_idx-2; row>=max(bottom_idx_critical, 0); row--) {

				if (Layout_Grid[row][col].first == 0) Layout_Grid[row][col].first = 3;
				else break;

			}

			for (int row=top_idx+2; row<=min(top_idx_critical, row_MAX); row++) {

				if (Layout_Grid[row][col].first == 0) Layout_Grid[row][col].first = 3;
				else break;

			}

		}

	}

	return (width > height) ? "row" : "column";

}

float Grid::set_density(unordered_map<string, vector<pair<pair<int, int>, pair<int, int>>>> &CDG_mapping,
						int grid_size, int row, int col, int relative_left, int relative_right, int relative_bottom, int relative_top) {

	vector<vector<int>> density_check(grid_size, vector<int>(grid_size, 0));
	
	int grid_density = 0;
	string Grid_ID = to_string(row)+"_"+to_string(col);

	for (auto &already_densed: CDG_mapping[Grid_ID]) 
		for (int R=already_densed.first.second; R<=already_densed.second.second; R++) 
			for (int C=already_densed.first.first; C<=already_densed.second.first; C++)
				density_check[R][C] = 1;
	
	for (int R=relative_bottom; R<=relative_top; R++) 
		for (int C=relative_left; C<=relative_right; C++)
			density_check[R][C] = 1;
	
	for (int R=0; R<grid_size; R++)
		for (int C=0; C<grid_size; C++)
			grid_density += density_check[R][C];
	
	pair<pair<int, int>, pair<int, int>> relative_coordinate = {{relative_left, relative_bottom}, {relative_right, relative_top}};
	CDG_mapping[Grid_ID].emplace_back(relative_coordinate);
	
	return float(grid_density)/(grid_size*grid_size);

}

void Grid::print_layout(vector<vector<pair<int, float>>> &Layout_Grid, int x_min, int x_MAX, int y_min, int y_MAX) {

	for (int y=y_MAX; y>=y_min; y--){
		for (int x=x_min; x<=x_MAX; x++) {
			if (Layout_Grid[y][x].first == 1) cout << "-";
			else if (Layout_Grid[y][x].first == 2) cout << "X";
			else if (Layout_Grid[y][x].first == 3) cout << "!";
			else if (Layout_Grid[y][x].first == 4) cout << "=";
			else cout << " ";
		}
		cout << endl;
	}
	cout << endl;

}