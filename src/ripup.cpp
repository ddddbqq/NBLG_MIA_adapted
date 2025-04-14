#include "nall.h"

// Whether or not it crosses the chip boundary
bool Naller::outBox(const int& s_x, const int& s_y, const int& width, const int& height) {
    if(s_y < 0 or (s_y + height) > g_max_y ) {
        return true;
    }
    if((s_x + width) > g_max_x  or s_x < 0) {      
        return true;
    }
    return false;
}
void Naller::addNodeCost(const int& s_x, const int& s_y, 
                          const int& width, const int& height) {
    for (int j = s_y; j < (s_y + height); j+=defaultH)
    {
        for (int i = s_x; i < (s_x + width); ++i)
        {
            NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + i];
            ++node_info.usage_;
            node_info.updated_ = false;
            node_info.pCost_ = pow(node_info.usage_  + 1, mi) * p_factor;
            node_info.pathCost_ = node_info.pCost_ * node_info.hisCost_;
        }
    }
}
void Naller::addNodeCost(const int& s_x, const int& s_y, 
                          const int& width, const int& height, double& p_factor_pThread) {
  for (int j = s_y; j < (s_y + height); j+=defaultH)
  {
    for (int i = s_x; i < (s_x + width); ++i)
    {

      NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + i];
      ++node_info.usage_;
      node_info.updated_ = false;
      node_info.pCost_ = pow(node_info.usage_  + 1, mi) * p_factor_pThread;
      node_info.pathCost_ = node_info.pCost_ * node_info.hisCost_;
    }
  }
}


void Naller::reduceNodeCost(const int& s_x, const int& s_y, 
                          const int& width, const int& height) {  //rip-up
    for (int j = s_y; j < (s_y + height); j+=defaultH)
    {
        for (int i = s_x; i < (s_x + width); ++i)
        {
            NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + i];
            --node_info.usage_;
            node_info.updated_ = false;
            node_info.pCost_ = pow(node_info.usage_  + 1, mi) * p_factor;
            node_info.pathCost_ = node_info.pCost_ * node_info.hisCost_;
        }
    }
}

void Naller::reduceNodeCost(const int& s_x, const int& s_y, 
                          const int& width, const int& height, double& p_factor_pThread) {  //rip-up
    for (int j = s_y; j < (s_y + height); j+=defaultH)
    {
        for (int i = s_x; i < (s_x + width); ++i)
        {
            NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + i];
            --node_info.usage_;
            node_info.updated_ = false;
            node_info.pCost_ = pow(node_info.usage_  + 1, mi) * p_factor_pThread;
            node_info.pathCost_ = node_info.pCost_ * node_info.hisCost_;
        }
    }
}

void Naller::calTempCost(double& cost_temp, const int& s_x, const int& s_y, const int& width,
                    const int& height,  const std::vector<Macro_pin>& signal_pins, int regionId) {  
    bool checkTech = false;
    for (int j = s_y; j < (s_y + height); j += defaultH)
    {
        for (int i = s_x; i < (s_x + width); ++i)
        {
            //TODO : re-calculate cost and use costTable. 
            NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + i];

            if(node_info.regionId_ != regionId) {
                cost_temp += 10000000;
            }    
            if(ckt.doTech) {
                if(node_info.layer_ >= 0) {
                    checkTech = true;
                }
            }    
            cost_temp += node_info.pathCost_;
        }
    }
    if(checkTech) {
        for(auto& pin : signal_pins) {  //  You can't use the width and height of a cell to determine if it's short-circuited.
            for (int j = (s_y + pin.yLL_offset_psite); j < (s_y + pin.yUR_offset_psite); j+=defaultH)
            {
                for (int i = (s_x + pin.xLL_offset_psite); i < (s_x + pin.xUR_offset_psite); ++i)
                {
                    NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + i];
                    if((node_info.layer_ - pin.layer) == 0 or (node_info.layer_ - pin.layer) == 1){
                        if(node_info.layer_ == 2) 
                            cost_temp += 10000000;
                        else
                            cost_temp += penalty_tech * defaultH;
                        // return;
                    }
                }
            }      
        }
    }
}

void Naller::calTempCost(Cell* cell, double& cost_temp, const int& s_x, const int& s_y, const int& width,
                    const int& height,  const std::vector<Macro_pin>& signal_pins, int regionId) {  
    bool checkTech = false;
    bool intra_vio = false;
    if (cell->is_intra_MIA_vio_) intra_vio = true;
    for (int j = s_y; j < (s_y + height); j += defaultH)
    {
        for (int i = s_x; i < (s_x + width); ++i)
        {
            NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + i];
            if(node_info.regionId_ != regionId) {
                cost_temp += 10000000;
            }    
            if(ckt.doTech) {
                if(node_info.layer_ >= 0) {
                    checkTech = true;
                }
            }    
            cost_temp += node_info.pathCost_;
            // add MIA::Filler cost
            if (!node_info.fillerList.empty()) {
                for (auto& filler : node_info.fillerList) {
                    cost_temp += filler->calCost(cell) * defaultH;
                }
            }
        }
        // if intra MIA vio cell abutting same VT type cell, reduce cost
        if(intra_vio) {
            if(!outBox(s_x - 1, j, 1, defaultH)) {
                if (getNodeVTOccupy(s_x - 1, j, cell->VT_type_) > 0) {
                    cost_temp -= INTRA_MIA_ABUTT_WEIGHT * defaultH;
                }
            }
            if(!outBox(s_x + width, j, 1, defaultH)) {
                if (getNodeVTOccupy(s_x + width, j, cell->VT_type_) > 0) {
                    cost_temp -= INTRA_MIA_ABUTT_WEIGHT * defaultH;
                }
            }
        }
    }
    if(checkTech) {
        for(auto& pin : signal_pins) {  //  You can't use the width and height of a cell to determine if it's short-circuited.
            for (int j = (s_y + pin.yLL_offset_psite); j < (s_y + pin.yUR_offset_psite); j+=defaultH)
            {
                for (int i = (s_x + pin.xLL_offset_psite); i < (s_x + pin.xUR_offset_psite); ++i)
                {
                    NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + i];
                    if((node_info.layer_ - pin.layer) == 0 or (node_info.layer_ - pin.layer) == 1){
                        if(node_info.layer_ == 2) 
                            cost_temp += 10000000;
                        else
                            cost_temp += penalty_tech * defaultH;
                        // return;
                    }
                }
            }      
        }
    }
}

void Naller::route(const int cellId, double& p_factor_pThread) {     
    Cell* sp = ckt.cells[cellId];
    double min_cost = 1.e20;
    double min_s_am = 100000.0;
    int direc = 0;
    int row_step = (sp->aligendRow_ == 2) ? 1 : 2;

    // int total_direc_num1or2 = (moveOut2flag) ? (total_direc_num2) : (total_direc_num);
    for (int i = 0; i < total_direc_num; ++i)
    {
        int s_x = sp->cur_x_ +  dir_array_x[i];
        int s_y = sp->cur_y_ + row_step * dir_array_y[i] * defaultH;
        if(outBox(s_x, s_y, sp->width_, sp->height_)) {
            continue;
        }
    
        double cost_temp = 0.0;
        //not sure here
        if (ckt.do_MIA)  calTempCost(sp, cost_temp, s_x, s_y, sp->width_, sp->height_, sp->signal_pins_, sp->regionId_);
        else   calTempCost(cost_temp, s_x, s_y, sp->width_, sp->height_, sp->signal_pins_, sp->regionId_);
        if(cost_temp > 40000000.0) {
            continue;
        }      
        // double s_am_temp = fabs(s_x -  sp->init_x_) + fabs(s_y -  sp->init_y_);
        double abs_disp = fabs(s_x -  sp->init_x_) + fabs(s_y -  sp->init_y_);
        double s_am_temp = (num_unfixed_inst_ + 0.0) / ckt.kMacros[sp->height_].first * (abs_disp);
        if(sp->height_ * sp->width_ > (4 * defaultH * 10 / min_width)) {
            s_am_temp += 100.0 * std::max(abs_disp - max_disp_th, 0.0);
        }
        else {
            s_am_temp += max_disp_p * std::max(abs_disp - max_disp_th, 0.0);
        }
        // s_am_temp += max_disp_p * max(abs_disp - max_disp_th, 0.0);
        cost_temp += s_am_temp; /*+ ( abs(s_x - strips[s_id].start_x_) + abs(s_y - strips[s_id].start_y_) + 0.0) / 
        ( abs(strips[s_id].cur_x_ - strips[s_id].start_x_) + abs(strips[s_id].cur_y_ - strips[s_id].start_y_) )*/
        // priority_queue_.push(i, cost, s_am_temp);
        if(cost_temp < min_cost) {
            direc = i;
            min_cost = cost_temp;
            min_s_am = s_am_temp;      
        }
        else if(fabs(min_cost - cost_temp) < EPSILON) {
            if (s_am_temp < min_s_am ) {
                direc = i;
                min_cost = cost_temp;
                min_s_am = s_am_temp;          
            }
        }
    }
    sp->cur_x_ += dir_array_x[direc];
    sp->cur_y_ +=  row_step * dir_array_y[direc] * defaultH;
    if (ckt.do_MIA) moveInCell(sp, p_factor_pThread);
    else addNodeCost(sp->cur_x_, sp->cur_y_, sp->width_, sp->height_, p_factor_pThread);
}

void Naller::route(const int cellId, const bool moveOut2flag) {     
    Cell* sp = ckt.cells[cellId];
    double min_cost = 1.e20;
    double min_s_am = 100000.0;
    int direc = 0;
    int row_step = (sp->aligendRow_ == 2) ? 1 : 2;
    int total_direc_num2or3 = (moveOut2flag) ? (total_direc_num2) : (total_direc_num3);
    for (int i = 0; i < total_direc_num2or3; ++i)
    {
        int x_offset = (moveOut2flag) ? (dir_array_x2[i]) : (dir_array_x3[i]);
        int s_x = sp->cur_x_ +  x_offset;
        int y_offset = (moveOut2flag) ? (dir_array_y2[i]) : (dir_array_y3[i]);
        int s_y = sp->cur_y_ + row_step * y_offset * defaultH;
        if(outBox(s_x, s_y, sp->width_, sp->height_)) {
            continue;
        }
        double cost_temp = 0.0;
        //not sure here
        if (ckt.do_MIA)  calTempCost(sp, cost_temp, s_x, s_y, sp->width_, sp->height_, sp->signal_pins_, sp->regionId_);
        else    calTempCost(cost_temp, s_x, s_y, sp->width_, sp->height_, sp->signal_pins_, sp->regionId_);
        if(cost_temp > 1.e25) {  //when in blockRegion 
            continue;
        }      
        double abs_disp = fabs(s_x -  sp->init_x_) + fabs(s_y -  sp->init_y_);
        double s_am_temp = (num_unfixed_inst_ + 0.0) / ckt.kMacros[sp->height_].first * (abs_disp);
    
        cost_temp += s_am_temp; /*+ ( abs(s_x - strips[s_id].start_x_) + abs(s_y - strips[s_id].start_y_) + 0.0) / 
        ( abs(strips[s_id].cur_x_ - strips[s_id].start_x_) + abs(strips[s_id].cur_y_ - strips[s_id].start_y_) )*/
        if(cost_temp < min_cost) {
            direc = i;
            min_cost = cost_temp;
            min_s_am = s_am_temp;      
        }
        else if(fabs(min_cost - cost_temp) < EPSILON) {
            if (s_am_temp < min_s_am) {
                direc = i;
                min_cost = cost_temp;
                min_s_am = s_am_temp;          
            }
        }
    }
    int x_offset = (moveOut2flag) ? (dir_array_x2[direc]) : (dir_array_x3[direc]);
    int y_offset = (moveOut2flag) ? (dir_array_y2[direc]) : (dir_array_y3[direc]);
    sp->cur_x_ += x_offset;
    sp->cur_y_ +=  row_step * y_offset * defaultH;
    if(sp->cur_x_ < 0 or sp->cur_y_ < 0) {
        std::cout << sp->name_ << " 0.1 " << sp->init_x_ << " " << sp->init_y_ << std::endl;
    }
    if (ckt.do_MIA) moveInCell(sp);
    else addNodeCost(sp->cur_x_, sp->cur_y_, sp->width_, sp->height_);

}

void Naller::route(const int cellId) { 
    Cell* sp = ckt.cells[cellId];
    bool is_intra_mia = false;
    if (sp->is_intra_MIA_vio_) is_intra_mia = true;

    double min_cost = 1.e20;
    double min_s_am = 100000.0;
    int direc = 0;
    int row_step = (sp->aligendRow_ == 2) ? 1 : 2;

    int total_direc_num_norm_or_mia = (is_intra_mia) ? total_direc_num_mia : total_direc_num;
    for (int i = 0; i < total_direc_num_norm_or_mia; ++i)
    {
        int x_offset = (is_intra_mia) ? (dir_array_x_mia[i]) : (dir_array_x[i]);
        int y_offset = (is_intra_mia) ? (dir_array_y_mia[i]) : (dir_array_y[i]);
        int s_x = sp->cur_x_ +  x_offset;
        int s_y = sp->cur_y_ + row_step * y_offset * defaultH;
        if(outBox(s_x, s_y, sp->width_, sp->height_)) {
            continue;
        }
        double cost_temp = 0.0;
        if (ckt.do_MIA)  calTempCost(sp, cost_temp, s_x, s_y, sp->width_, sp->height_, sp->signal_pins_, sp->regionId_);
        else calTempCost(cost_temp, s_x, s_y, sp->width_, sp->height_, sp->signal_pins_, sp->regionId_);
        if(cost_temp > 40000000.0) {
            continue;
        }      
        double abs_disp = fabs(s_x -  sp->init_x_) + fabs(s_y -  sp->init_y_);
        double s_am_temp = (num_unfixed_inst_ + 0.0) / ckt.kMacros[sp->height_].first * (abs_disp);
        if(sp->height_ * sp->width_ > (4 * defaultH * 10 / min_width)) {
            s_am_temp += 100.0 * std::max(abs_disp - max_disp_th, 0.0);
        }
        else {
            s_am_temp += max_disp_p * std::max(abs_disp - max_disp_th, 0.0);
        }
        cost_temp += ABS_DISPLACEMENT_WEIGHT * abs_disp;
        //cost_temp += s_am_temp; 
        /*+ ( abs(s_x - strips[s_id].start_x_) + abs(s_y - strips[s_id].start_y_) + 0.0) / 
        ( abs(strips[s_id].cur_x_ - strips[s_id].start_x_) + abs(strips[s_id].cur_y_ - strips[s_id].start_y_) )*/
        
        // priority_queue_.push(i, cost, s_am_temp);
        if(cost_temp < min_cost) {
            direc = i;
            min_cost = cost_temp;
            min_s_am = s_am_temp;      
        }
        else if(fabs(min_cost - cost_temp) < EPSILON) {
            if (s_am_temp < min_s_am ) {
                direc = i;
                min_cost = cost_temp;
                min_s_am = s_am_temp;          
            }
        }
    }
    int x_offset = (is_intra_mia) ? (dir_array_x_mia[direc]) : (dir_array_x[direc]);
    int y_offset = (is_intra_mia) ? (dir_array_y_mia[direc]) : (dir_array_y[direc]);
    sp->cur_x_ += x_offset;
    sp->cur_y_ +=  row_step * y_offset * defaultH;
    if (ckt.do_MIA) moveInCell(sp);
    else addNodeCost(sp->cur_x_, sp->cur_y_, sp->width_, sp->height_);//update cost
}


