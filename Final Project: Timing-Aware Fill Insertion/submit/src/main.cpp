#include "Parser.hpp"
#include "DF_Insertion.hpp"
#include "GlobalTimer.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;

int main(int argc, char **argv) {

	///////////////////////////////////////////
	// ----------				 //
	// Parameters				 //
	// ----------			         //
	// GlobalTimer:	    Global Timer         //
	// Parser: 	    I/O Instructions 	 //
	// DF_Insertion:    Dummy Fill Insertion //
	///////////////////////////////////////////
	
	// prepare for the log file
	ofstream logFile("./output/Fill_Insertion.log");
	if (!logFile.is_open()) {

		cerr << "Failed to open the log file." << endl;
		return 1;

	}
	streambuf* originalCoutBuffer = cout.rdbuf();
	cout.rdbuf(logFile.rdbuf());

	// set global timer
	GlobalTimer::initialTimerAndSetTimeLimit(chrono::seconds(10 * 60));
	auto initial_time = GlobalTimer::getInstance()->getDuration<>().count() / 1e9;

	// input data
	Parser Parser;
	auto [chip_left, chip_bottom, chip_right, chip_top, W_Size, CP_num, L_num, C_num,
		  CP_ids, Layer_info, Conductor_info] = Parser.Input(argv[1]);
	auto until_input_time = GlobalTimer::getInstance()->getDuration<>().count() / 1e9;
	
	// dummy fill insertion
	map<int, vector<pair<pair<int, int>, pair<int, int>>>> DFs;
	DF_Insertion DF_Insertion(chip_left, chip_bottom, chip_right, chip_top, W_Size, CP_num, L_num, C_num,
		  		 			  Layer_info, Conductor_info);
	DF_Insertion.DFI(DFs);
	auto until_DF_time = GlobalTimer::getInstance()->getDuration<>().count() / 1e9;
	
	// output the dummy fills
	Parser.Output(argv[2], DFs);
	auto until_output_time = GlobalTimer::getInstance()->getDuration<>().count() / 1e9;

	// time data
	auto total_time = GlobalTimer::getInstance();
	cout << "===== Runtime (sec.) =====" << endl;
	cout << "Total Time: " << fixed << setprecision(5) << total_time->getDuration<>().count() / 1e9 << endl;
	cout << "- Input: " << until_input_time - initial_time << endl;
	cout << "- Dummy Fill Insertion: " << until_DF_time - until_input_time << endl;
	cout << "- Output: " << until_output_time - until_DF_time << endl;
	if (total_time->overTime()) cout << "##### OverTime #####" << endl;
	cout << "==========================" << endl;

	// restore the original standard output and close the logfile
	cout.rdbuf(originalCoutBuffer);
	logFile.close();

	return 0;

}
