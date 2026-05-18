#include <iostream>
#include "nall.h"



using std::cout;

int main(int argc, char *argv[]) {
    // Parsing lef/def files
    circuit ckt;
    init_log(LOG_NORMAL);
    ckt.doTech = false;
    ckt.read_files(argc, argv);
    ckt.setMaxXY();
    //srand(time(0));
    srand(1);
    
    // for ispd14 benchmark
    ckt.double_or_triple_cell_height();

    ckt.setMIACells(0, true, true,0.1,0.1,10,4); //1 for debug
    
    // ckt.setTPNcells(0.3);
    
    // NBLG algorithm
    Naller naller(ckt);
    naller.nall();
    // Write def file
    ckt.write_temp_result("./temp_result.def");
    ckt.write_def();
    ckt.cal_hpwl();
    double temp_avg_disp = naller.avg_disp;
    int of_cnt = naller.of_cnt_for_temp_output;
    double delta_rate = ckt.temp_delta_rate_for_output;
    double score = (1 + delta_rate/100.0) * temp_avg_disp * (1 + of_cnt/100.0);
    std::string filename = "/data/jhqiao/workspace/NBLG/Mixed-Cell-Height_legalizer/temp_result";
    std::ofstream outfile (filename);
    outfile<<"score: " << score << std::endl;

#ifdef ENABLE_TIME_MEASUREMENT
    print_function_stats();
#endif
    return 0;
}