#pragma once
using namespace std;

class Layer {

	private:
		const int layer_id;
		const int min_spacing;
		const int min_width, MAX_width;
		const float min_density, MAX_density;
		const float weight;
		vector<int> Conductor_IDs;

	public:
		Layer(const int layer_id_, const int min_spacing_, const int min_width_, const int MAX_width_, 
			  const float min_density_, const float MAX_density_, const float weight_) :
			layer_id(layer_id_), min_spacing(min_spacing_), min_width(min_width_), MAX_width(MAX_width_),
			min_density(min_density_), MAX_density(MAX_density_), weight(weight_), Conductor_IDs({}){}
		const int get_LID() {return layer_id;}
		const int get_min_spacing() {return min_spacing;}
		const int get_min_w() {return min_width;}
		const int get_MAX_w() {return MAX_width;}
		const float get_min_d() {return min_density;}
		const float get_MAX_d() {return MAX_density;}
		const float get_LWeight() {return weight;}
		vector<int> &get_conductor_in_this_layer() {return Conductor_IDs;}

};