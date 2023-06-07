#pragma once
#include <vector>
using namespace std;

class Conductor {

	private:
		const int conductor_id;
		const int net_id;
		const int layer_id;
		const int boundary_left, boundary_bottom, boundary_right, boundary_top;
		const bool OC_flag;

	public:
		Conductor(const int conductor_id_, const int net_id_, const int layer_id_, const bool OC_flag_,
			      const int boundary_left_, const int boundary_bottom_, const int boundary_right_, const int boundary_top_) :
			conductor_id(conductor_id_), net_id(net_id_), layer_id(layer_id_), OC_flag(OC_flag_),
			boundary_left(boundary_left_), boundary_bottom(boundary_bottom_), boundary_right(boundary_right_), boundary_top(boundary_top_){}
		const int get_CID() {return conductor_id;}
		const int get_NID() {return net_id;}
		const int get_LID() {return layer_id;}
		const vector<int> get_boundary() {return {boundary_left, boundary_bottom, boundary_right, boundary_top};}
		const bool is_ctirical() {return OC_flag;}

};