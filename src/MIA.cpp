#pragma once

#include "parser.h"
#include "nall.h"
#define DEBUG_TEMP_
int debug_cnt = 0;

namespace MIA{
  void Filler::setHard(){
    is_hard_ = true;
  }

  void Filler::setSoft(){
    is_hard_ = false;
  }

  int Filler::getWidth(){
    if (is_valid_ == false) {
      return 0;
    }
    return width_;
  }

  Filler::Filler(Cell* cell, int l_or_r, int width){
    cell_ = cell;
    left_or_right_ = l_or_r;
    if (width == 0) {
      is_valid_ = false;
      width = 1;
    }
    else is_valid_ = true;
    width_ = width;
    height_ = cell->height_;
    type_ = cell->VT_type_;
    is_hard_ = false;
    y_ = cell->cur_y_;
    if (left_or_right_ == 1){ //left
      x_ = cell->cur_x_ - width_;
    }
    else if (left_or_right_ == 2){ //right
      x_ = cell->cur_x_ + cell->width_;
    }
    else {
      std::cout << "Error: Filler::wrong left_or_right_ value" << std::endl;
    }
  }

  void Filler::changeWidth(int w){
    if (w == 0) {
      is_valid_ = false;
      w = 1;
    }
    else {
      is_valid_ = true;
    }
    if (left_or_right_ == 1){ //left
      x_ = x_ + width_ - w;
      width_ = w;
    }
    else if (left_or_right_ == 2){ //right
      width_ = w;
    }
    else {
      std::cout << "Error: Filler::wrong left_or_right_ value" << std::endl;
    }
  }

  void Filler::setPos(int x, int y){
    x_ = x;
    y_ = y;
  }

  double Filler::calCost(Cell* income_cell){
    if (income_cell == nullptr){
      std::cout << "Filler::calCost: income_cell is nullptr" << std::endl;
      return 0;
    }
    double cost = 0;
    if (is_valid_ == false) {
      return cost;
    }
    int theta_ij = 1;
    if (income_cell->VT_type_ == type_){
      if (income_cell->is_intra_MIA_vio_ == true){
        theta_ij = 2;
      }
      cost = - MIA_WEIGHT 
             * theta_ij 
             *(std::min(income_cell->height_, height_) 
             / std::max(income_cell->height_, height_));
    }
    else {
      if (is_hard_ == true){
        cost = FIXED_HARD_FILLER_WEIGHT;
      }
      else {
        cost = 0;
      }
    }
    return cost;
  }
}

bool Naller::addVTOccupy(Cell* cell){
  return addVTOccupy(cell->cur_x_, cell->cur_y_, cell->width_, cell->height_, cell->VT_type_);
}

bool Naller::addVTOccupy(const int& s_x, const int& s_y, 
                          const int& width, const int& height, MIA::VT_type vt){
    for (int j = s_y; j < (s_y + height); j+=defaultH)
    {
        for (int i = s_x; i < (s_x + width); ++i)
        {
            NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + i];
            if (vt == MIA::LVT) node_info.LVT_occupied_++;
            else if (vt == MIA::HVT) node_info.HVT_occupied_++;
            else if (vt == MIA::SVT) node_info.SVT_occupied_++;
            else {
              std::cout << "addVTOccupy: Invalid VT type" << std::endl;
              return false;
            }
        }
    }
  return true;
}

bool Naller::reduceVTOccupy(Cell* cell){
  return reduceVTOccupy(cell->cur_x_, cell->cur_y_, cell->width_, cell->height_, cell->VT_type_);
}

bool Naller::reduceVTOccupy(const int& s_x, const int& s_y,
                          const int& width, const int& height, MIA::VT_type vt){
    for (int j = s_y; j < (s_y + height); j+=defaultH)
    {
        for (int i = s_x; i < (s_x + width); ++i)
        {
            NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + i];
            if (vt == MIA::LVT) node_info.LVT_occupied_--;
            else if (vt == MIA::HVT) node_info.HVT_occupied_--;
            else if (vt == MIA::SVT) node_info.SVT_occupied_--;
            else {
              std::cout << "reduceVTOccupy: Invalid VT type" << std::endl;
              return false;
            }
            if (node_info.LVT_occupied_ < 0 || node_info.HVT_occupied_ < 0 || node_info.SVT_occupied_ < 0)
            {
              std::cout << "ERROR: reduceVTOccupy: " 
                        << node_info.LVT_occupied_ << " " 
                        << node_info.HVT_occupied_ << " " 
                        << node_info.SVT_occupied_ << std::endl;
              return false;
            }
        }
    }
  return true;
}

bool Naller::initNodeVTOccupy() {
  for (unsigned i : ckt.defaultCellIds){
    Cell* sp = ckt.cells[ i ]; 
    addVTOccupy(sp);
  }
  return true;
}

int Naller::getNodeVTOccupy(const int& s_x, const int& s_y, MIA::VT_type vt){
  if (vt == MIA::LVT) return node.node_infos[g_max_x*s_y/defaultH + s_x].LVT_occupied_;
  else if (vt == MIA::HVT) return node.node_infos[g_max_x*s_y/defaultH + s_x].HVT_occupied_;
  else if (vt == MIA::SVT) return node.node_infos[g_max_x*s_y/defaultH + s_x].SVT_occupied_;
  else return -1;
}
int Naller::getNodeOccupy(const int& s_x, const int& s_y){
  return node.node_infos[g_max_x*s_y/defaultH + s_x].LVT_occupied_ +
         node.node_infos[g_max_x*s_y/defaultH + s_x].HVT_occupied_ +
         node.node_infos[g_max_x*s_y/defaultH + s_x].SVT_occupied_;
}

bool Naller::addNodeFillerOccupy(MIA::Filler* filler){
  if (filler == nullptr){
    std::cout << "addNodeFillerOccupy: filler is nullptr" << std::endl;
    return false;
  }
  int s_x = filler->x_;
  int s_y = filler->y_;
  int w = filler->width_;
  int h = filler->height_;

  if (outBox(s_x, s_y, w, h)) return false;
  for (int j = s_y; j < s_y + h; j+=defaultH){
    for (int i = s_x; i < s_x + w; ++i){
      node.node_infos[g_max_x*j/defaultH + i].fillerList.push_back(filler);
    }
  }
  if (filler->is_valid_) addVTOccupy(s_x, s_y, w, h, filler->type_);
  return true;
}

bool Naller::reduceNodeFillerOccupy(MIA::Filler* filler){
  if (filler == nullptr){
    std::cout << "reduceNodeFillerOccupy: filler is nullptr" << std::endl;
    return false;
  }
  int s_x = filler->x_;
  int s_y = filler->y_;
  int w = filler->width_;
  int h = filler->height_;

  
  if (outBox(s_x, s_y, w, h)) return false;
  for (int j = s_y; j < s_y + h; j+=defaultH){
    for (int i = s_x; i < s_x + w; ++i){
      auto it = std::find(node.node_infos[g_max_x*j/defaultH + i].fillerList.begin(),
                          node.node_infos[g_max_x*j/defaultH + i].fillerList.end(), filler);
      if (it != node.node_infos[g_max_x*j/defaultH + i].fillerList.end()){
        node.node_infos[g_max_x*j/defaultH + i].fillerList.erase(it);
      }
      else{
        std::cout << "reduceNodeFillerOccupy: Filler not found in node" << std::endl;
        return false;
      }
    }
  }
  if (filler->is_valid_) reduceVTOccupy(s_x, s_y, w, h, filler->type_);
  return true;
}

bool Naller::changeFillerWidth(MIA::Filler* filler, const int& w, bool update_node){
  if (filler == nullptr){
    std::cout << "changeFillerWidth: Filler is null" << std::endl;
    return false;
  }
  bool res;
  if (update_node){
    res = reduceNodeFillerOccupy(filler);
  }
  filler->changeWidth(w);
  if (update_node){
    addNodeFillerOccupy(filler);
  }
  return res;
}

bool Naller::genFillers(Cell* cell, const int& left_w, const int& right_w, 
                        bool is_hard){
  if (cell == nullptr) {
    std::cout << "genFillers: cell is nullptr" << std::endl;
    return false;
  }
  int left_x = cell->cur_x_ - left_w;
  int right_x = cell->cur_x_ + cell->width_;
  int s_y = cell->cur_y_;
  int height = cell->height_;
  if (outBox(left_x, s_y, left_w, height) || outBox(right_x, s_y, right_w, height)){
    std::cout << "genFillers: out of box" << std::endl;
    return false;
  } 
  MIA::Filler* filler_left = new MIA::Filler(cell, 1, left_w);
  MIA::Filler* filler_right = new MIA::Filler(cell, 2, right_w);
  if (is_hard == true){
    filler_left->setHard();
    filler_right->setHard();
  }
  addNodeFillerOccupy(filler_left);
  addNodeFillerOccupy(filler_right);
  cell->filler_l = filler_left;
  cell->filler_r = filler_right;
  return true;
}

bool Naller::genInterRowFillers(Cell* cell, const int& left_w, const int& right_w, 
                    bool is_hard, int overlap_w, int s_x, int top_or_bottom){
  if (cell == nullptr) {
    std::cout << "genInterRowFillers: cell is nullptr" << std::endl;
    return false;
  }
  if (overlap_w <= 0 or 
      overlap_w > ckt.MIA_min_width_ or 
      s_x <= 0 or 
      !(top_or_bottom == -1 or top_or_bottom == 1)) {
    std::cout << "genInterRowFillers: invalid parameters" << std::endl;
    return false;
  }
  int left_x = s_x - std::max(left_w, 1);
  int right_x = s_x + overlap_w;
  int s_y_l;
  int s_y_r;
  if (top_or_bottom == -1){
    s_y_l = cell->cur_y_ - defaultH;
    s_y_r = cell->cur_y_;
  } else {
    s_y_l = cell->cur_y_ + cell->height_ ;
    s_y_r = cell->cur_y_ + cell->height_ - defaultH;
  }
  int height = defaultH;
  if (outBox(left_x, s_y_l, left_w, height) || outBox(right_x, s_y_r, right_w, height)){
    std::cout << "genInterRowFillers: out of box" << std::endl;
    return false;
  }
  MIA::Filler* filler_left = new MIA::Filler(cell, 1, left_w);
  MIA::Filler* filler_right = new MIA::Filler(cell, 2, right_w);
  if (is_hard == true){
    filler_left->setHard();
    filler_right->setHard();
  }
  filler_left->setPos(left_x, s_y_l);
  filler_left->height_ = height;
  filler_right->setPos(right_x, s_y_r);
  filler_right->height_ = height;
  addNodeFillerOccupy(filler_left);
  addNodeFillerOccupy(filler_right);
  if (top_or_bottom == 1) {
    cell->inter_filler_tl = filler_left;
    cell->inter_filler_tr = filler_right;
  } else if (top_or_bottom == -1) {
    cell->inter_filler_bl = filler_left;
    cell->inter_filler_br = filler_right;
  }
  return true;
}

bool Naller::DVFA(Cell* cell){
  if (cell == nullptr) {
    std::cout << "DVFA: cell is nullptr" << std::endl;
    return false;
  }
#ifdef DEBUG_TEMP
  if (cell->name_ == "FE_OCPC1868_n_15768") {
    debug_cnt++;
    if (debug_cnt == 2270) {
      debug_cnt = 2270;
    }
  }
#endif 
  bool intra_row = false;
  bool inter_row_t = false;
  bool inter_row_b = false;
  if (cell->is_intra_MIA_vio_) {
    intra_row = true;
  }
  if (cell->inter_row_overlap_t_ > 0){
    inter_row_t = true;
  }
  if (cell->inter_row_overlap_b_ > 0){
    inter_row_b = true;
  }
  if (intra_row == false && inter_row_t == false && inter_row_b == false){
    //std::cout << "DVFA: no need to do DVFA" << std::endl;
    return false;
  }
  
  DVFA_cnt++; 
  std::vector<int> s_x_list;
  std::vector<int> s_y_l_list;
  std::vector<int> s_y_r_list;
  std::vector<int> height_list;
  std::vector<int> width_list;
  std::vector<int> MIA_type_list;
  int cnt = 0;
  if (intra_row){
    s_x_list.push_back(cell->cur_x_);
    s_y_l_list.push_back(cell->cur_y_);
    s_y_r_list.push_back(cell->cur_y_);
    height_list.push_back(cell->height_);
    width_list.push_back(cell->width_);
    MIA_type_list.push_back(0);
    cnt++;
  }
  if (inter_row_t){
    int overlap_w = cell->inter_row_overlap_t_;
    if (overlap_w <= 0 or overlap_w > ckt.MIA_min_width_) {
      std::cout << "DVFA: error config when inter row" << std::endl;
      return false;
    }
    int s_x = cell->cur_x_ + cell->width_ - overlap_w;
    int s_y_l = cell->cur_y_ + cell->height_;
    int s_y_r = cell->cur_y_ + cell->height_ - defaultH;
    int height = defaultH;
    int width = overlap_w;
    if (cell->is_intra_MIA_vio_){
      if (cell->filler_r == nullptr) {
        std::cout << "DVFA: inter row for intra row cell error: filler_r is nullptr" << std::endl;
        return false;
      }
      if (cell->filler_r->is_hard_ == false) {
        std::cout << "DVFA: inter row for intra row cell error: filler_r is not hard" << std::endl;
        return false;
      }
      s_x += cell->filler_r->getWidth();
    }
    s_x_list.push_back(s_x);
    s_y_l_list.push_back(s_y_l);
    s_y_r_list.push_back(s_y_r);
    height_list.push_back(height);
    width_list.push_back(width);
    MIA_type_list.push_back(1);
    cnt++;
  }
  if (inter_row_b){
    int overlap_w = cell->inter_row_overlap_b_;
    if (overlap_w <= 0 or overlap_w > ckt.MIA_min_width_) {
      std::cout << "DVFA: error config when inter row" << std::endl;
      return false;
    }
    int s_x = cell->cur_x_ + cell->width_ - overlap_w;
    int s_y_l = cell->cur_y_ - defaultH;
    int s_y_r = cell->cur_y_;
    int height = defaultH;
    int width = overlap_w;
    if (cell->is_intra_MIA_vio_){
      if (cell->filler_r == nullptr) {
        std::cout << "DVFA: inter row for intra row cell error: filler_r is nullptr" << std::endl;
        return false;
      }
      if (cell->filler_r->is_hard_ == false) {
        std::cout << "DVFA: inter row for intra row cell error: filler_r is not hard" << std::endl;
        return false;
      }
      s_x += cell->filler_r->getWidth();
    }
    s_x_list.push_back(s_x);
    s_y_l_list.push_back(s_y_l);
    s_y_r_list.push_back(s_y_r);
    height_list.push_back(height);
    width_list.push_back(width);
    MIA_type_list.push_back(2);
    cnt++;
  }

  int MIA_min_width = ckt.MIA_min_width_;
  MIA::VT_type vt = cell->VT_type_;
  int res = 1;
  for (int i = 0; i < cnt; i++){ 
    int s_x = s_x_list[i];
    int s_y_l = s_y_l_list[i];
    int s_y_r = s_y_r_list[i];
    int height = height_list[i];
    int width = width_list[i];
    int MIA_type = MIA_type_list[i];
    int filler_total_width = MIA_min_width - width;

    if (filler_total_width <= 0) return false;

    int left_w = 0;
    bool left_abutt_same_vt = false;
    bool left_abutt_diff_vt = false;
    int right_w = 0;
    bool right_abutt_same_vt = false;
    bool right_abutt_diff_vt = false;

    if (MIA_type != 0) {
      if (!(getNodeVTOccupy(s_x, s_y_l, vt) > 0 and 
           (getNodeOccupy(s_x, s_y_l) == getNodeVTOccupy(s_x, s_y_l, vt)
           ))) 
      {
        if (MIA_type == 1) clearInterRowFiller(cell, false, true, false);
        if (MIA_type == 2) clearInterRowFiller(cell, false, false, true);
        continue;
      }
    }
    if (MIA_type == 0) {
      if (cell->filler_l!= nullptr) reduceNodeFillerOccupy(cell->filler_l);
      if (cell->filler_r!= nullptr) reduceNodeFillerOccupy(cell->filler_r);
    }
    if (MIA_type == 1) {
      if (cell->inter_filler_tl!= nullptr) reduceNodeFillerOccupy(cell->inter_filler_tl);
      if (cell->inter_filler_tr!= nullptr) reduceNodeFillerOccupy(cell->inter_filler_tr);
    }
    if (MIA_type == 2) {
      if (cell->inter_filler_bl!= nullptr) reduceNodeFillerOccupy(cell->inter_filler_bl);
      if (cell->inter_filler_br!= nullptr) reduceNodeFillerOccupy(cell->inter_filler_br);
    }

    //left detect
    int left_i;
    int left_max_w = filler_total_width;
    while (outBox(s_x - left_max_w, s_y_l, 1, defaultH) and left_max_w > 0) {
      left_max_w--;
    }
    for (left_i = 1; left_i <= left_max_w; left_i++){
      int to_break = 0;
      for (int j = s_y_l; j < (s_y_l + height); j+=defaultH){
        int total_occupied = getNodeOccupy(s_x - left_i, j);
        int same_vt_occupied = getNodeVTOccupy(s_x - left_i, j, vt);
        if (total_occupied == 0) 
          continue;
        else {
          if (same_vt_occupied == total_occupied) {
            left_abutt_same_vt = true;
            to_break = 1;
            break;
          }
          else if (same_vt_occupied > 0) {
            left_abutt_same_vt = true;
            left_abutt_diff_vt = true;
            to_break = 1;
            break;
          }
          else if (same_vt_occupied == 0 && total_occupied > 0) {
            left_abutt_diff_vt = true;
            to_break = 1;
            break;
          }
          else {
            std::cout << "error: unexpected case for left detection in DVFA" << std::endl;
            return false;
            }
        }
      }
      if (to_break == 1) break;
    }
    left_w = left_i - 1;

    //right detect
    int right_i = 1;
    s_x += (width - 1);
    int right_max_w = filler_total_width;
    while (outBox(s_x + right_max_w, s_y_r, 1, defaultH) and right_max_w > 0) {
      right_max_w--;
    }
    for (right_i = 1; right_i <= right_max_w; right_i++){
      int to_break = 0;
      for (int j = s_y_r; j < (s_y_r + height); j+=defaultH){
        int total_occupied = getNodeOccupy(s_x + right_i, j);
        int same_vt_occupied = getNodeVTOccupy(s_x + right_i, j, vt);
        if (total_occupied == 0)
          continue;
        else {
          if (same_vt_occupied == total_occupied) {
            right_abutt_same_vt = true;
            to_break = 1;
            break;
          }
          else if (same_vt_occupied > 0) {
            right_abutt_same_vt = true;
            right_abutt_diff_vt = true;
            to_break = 1;
            break;
          }
          else if (same_vt_occupied == 0 && total_occupied > 0) {
            right_abutt_diff_vt = true;
            to_break = 1;
            break;
          }
          else {
            std::cout << "error: unexpected case for right detection in DVFA" << std::endl;
            return false;
          }
        }
      }
      if (to_break == 1) break;
    }
    right_w = right_i - 1;
    s_x -= (width - 1);

    //filler config
    bool is_hard = false;
    
    if (left_abutt_same_vt && right_abutt_same_vt && cell->height_ == defaultH){
      is_hard = true;
    }
    else if (left_abutt_same_vt and right_abutt_same_vt == false){
      left_w = std::min(filler_total_width, left_max_w);
      right_w = filler_total_width - left_w;
      is_hard = true;
    }
    else if (right_abutt_same_vt and left_abutt_same_vt == false){
      right_w = std::min(filler_total_width, right_max_w);
      left_w = filler_total_width - right_w;
      is_hard = true;
    }
    else if (left_w + right_w <= filler_total_width){
      if (left_w > right_w){
        left_w = std::min(left_max_w, std::max(left_w, filler_total_width / 2));
        right_w = std::min(right_max_w, (filler_total_width - left_w));
        left_w = filler_total_width - right_w;
        is_hard = true;
      }
      else if (right_w >= left_w){
        right_w = std::min(right_max_w, std::max(right_w, filler_total_width / 2));
        left_w = std::min(left_max_w, (filler_total_width - right_w));
        right_w = filler_total_width - left_w;
        is_hard = true;
      }
    }

    //gen or mod fillers
#ifdef DEBUG_TEMP
    // if (debug_cnt == 1134) {
    // debug_cnt = 1134;}
#endif 
    if (MIA_type == 0) { // intra row
      bool to_break = false;
      if (cell->filler_l && cell->filler_r){
        if (left_w != cell->filler_l->getWidth())
          res *= changeFillerWidth(cell->filler_l, left_w, false);
        if (right_w != cell->filler_r->getWidth()){
            res *= changeFillerWidth(cell->filler_r, right_w, false);
            to_break = true;
          }
        if (is_hard){
          cell->filler_l->setHard();
          cell->filler_r->setHard();
        }
        addNodeFillerOccupy(cell->filler_l);
        addNodeFillerOccupy(cell->filler_r);
      }
      else {
        res *= genFillers(cell, left_w, right_w, is_hard);
        to_break = true;
      }
      if (to_break) {
        clearInterRowFiller(cell, false);
        break;
      }
    }
    if (MIA_type == 1) { // inter row top
      if (cell->inter_filler_tl && cell->inter_filler_tr){
        if (left_w != cell->inter_filler_tl->getWidth())
          res *= changeFillerWidth(cell->inter_filler_tl, left_w, false);
        if (right_w != cell->inter_filler_tr->getWidth())
          res *= changeFillerWidth(cell->inter_filler_tr, right_w, false);
        if (is_hard){
          cell->inter_filler_tl->setHard();
          cell->inter_filler_tr->setHard();
        }
        addNodeFillerOccupy(cell->inter_filler_tl);
        addNodeFillerOccupy(cell->inter_filler_tr);
      }
      else {
        res *= genInterRowFillers(cell, left_w, right_w, is_hard, width, s_x, 1);
      }
    }
    if (MIA_type == 2) { // inter row bottom
      if (cell->inter_filler_bl && cell->inter_filler_br){
        if (left_w != cell->inter_filler_bl->getWidth())
          res *= changeFillerWidth(cell->inter_filler_bl, left_w, false);
        if (right_w != cell->inter_filler_br->getWidth())
          res *= changeFillerWidth(cell->inter_filler_br, right_w, false);
        if (is_hard){
          cell->inter_filler_bl->setHard();
          cell->inter_filler_br->setHard();
        }
        addNodeFillerOccupy(cell->inter_filler_bl);
        addNodeFillerOccupy(cell->inter_filler_br);
      }
      else {
        res *= genInterRowFillers(cell, left_w, right_w, is_hard, width, s_x, -1);
      }
    } 
  }
  return (res == 1);
}

bool Naller::initFiller(){
  int res = 1;
  std::vector<string> error_cells;
  for (unsigned i : ckt.defaultCellIds){
    Cell* sp = ckt.cells[ i ];
    if(sp->isFixed_) continue;
    if (sp->is_intra_MIA_vio_){
      res = DVFA(sp);
      if (res == 0){
        error_cells.push_back(sp->name_);
      }
    }   
  }
  if (!error_cells.empty()){
    std::cout << "MIA: Failed to init fillers in cells: ";
    for (string cell_name : error_cells){
      std::cout << cell_name << " ";
    }
    std::cout << std::endl;
    return false;
  }
  else{
    return true;
  }
}


bool Naller::isStripCongested(Cell* cell, const int& lBound, int regionId){
    if (cell == nullptr){
        std::cout << "Cell is nullptr when call isStripCongested" << std::endl;
        return false;
    }
    int s_x = cell->cur_x_;
    int s_y = cell->cur_y_;
    int width = cell->width_;
    int height = cell->height_;
    MIA::VT_type vt = cell->VT_type_;
    for (int j = s_y; j < (s_y + height); j+=defaultH)
    {
        for (int i = s_x; i < (s_x + width); ++i)
        {
            NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + i];
            if(node_info.usage_ > lBound) {
                return true;
            }
            if(node_info.regionId_ != regionId) {
                return true;
            }
            if(!node_info.fillerList.empty()){
                for (auto filler : node_info.fillerList){
                    if(filler->type_ != vt and filler->is_hard_ and filler->is_valid_){
                        return true;
                    }
                }
            }      
        }
    }
    return false;
}

void Naller::moveInCell(Cell* cell, double p_factor_pThread){
  if (cell == nullptr) {
    std::cout << "moveInCell: cell is nullptr" << std::endl;
    return;
  }
  int s_x = cell->cur_x_;
  int s_y = cell->cur_y_;
  int width = cell->width_;
  int height = cell->height_;
  MIA::VT_type vt = cell->VT_type_;

  if (ckt.check_inter_row_MIA){
    if (cell->inter_row_overlap_b_ != 0 or cell->inter_row_overlap_t_ != 0){
      std::cout << "moveInCell: cell has inter_row_overlap" << std::endl;
      return;
    }
    if (cell->inter_filler_bl != nullptr
        or cell->inter_filler_br != nullptr
        or cell->inter_filler_tl != nullptr
        or cell->inter_filler_tr != nullptr){
      std::cout << "moveInCell: cell has inter_filler" << std::endl;
      return;
    }
  }


  addVTOccupy(cell);

  std::set<Cell *> cells_to_do_DVFA;
  for (int j = s_y; j < (s_y + height); j+=defaultH)
  {
    for (int i = s_x; i < (s_x + width); ++i)
    {
      NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + i];
      ++node_info.usage_;
      node_info.updated_ = false;
      if (fabs(p_factor_pThread + 1) <= EPSILON)
        node_info.pCost_ = pow(node_info.usage_  + 1, mi) * p_factor;
      else
        node_info.pCost_ = pow(node_info.usage_  + 1, mi) * p_factor_pThread;
      node_info.pathCost_ = node_info.pCost_ * node_info.hisCost_;

      if(ckt.do_detailed_MIA){
        if (!node_info.fillerList.empty()){
          for (auto& filler: node_info.fillerList){
            cells_to_do_DVFA.insert(filler->cell_);
          }
        }
      } 
    }
    if(ckt.do_detailed_MIA){
      if (!outBox(s_x - 1, j, 1, defaultH)){
        NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + s_x - 1];
        if (!node_info.fillerList.empty()){
          for (auto& filler: node_info.fillerList){
            cells_to_do_DVFA.insert(filler->cell_);
          }
        } 
      }
      if (!outBox(s_x + width, j, 1, defaultH)){
        NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + s_x + width];
        if (!node_info.fillerList.empty()){
          for (auto& filler: node_info.fillerList){
            cells_to_do_DVFA.insert(filler->cell_);
          }
        }
      }
    }
  }
  if(ckt.do_detailed_MIA){  
    for (Cell* cell_to_do_DVFA: cells_to_do_DVFA){
      if (cell_to_do_DVFA == cell) continue;
      DVFA(cell_to_do_DVFA);
    }
  }

  if (cell->is_intra_MIA_vio_){
    if (cell->filler_l == nullptr or cell->filler_r == nullptr) {
      std::cout << "moveInCell: filler is nullptr " << std::endl;
      return;
    }
    if (cell->filler_l->is_valid_ == true or cell->filler_r->is_valid_ == true) {
      std::cout << "moveInCell: filler is valid " << std::endl;
      return;
    }
    reduceNodeFillerOccupy(cell->filler_l);
    reduceNodeFillerOccupy(cell->filler_r);
    cell->filler_l->setPos(s_x - 1, s_y);
    cell->filler_r->setPos(s_x + width, s_y);
    addNodeFillerOccupy(cell->filler_l);
    addNodeFillerOccupy(cell->filler_r);
    if(ckt.do_detailed_MIA) DVFA(cell);
  }
}


void Naller::moveOutCell(Cell* cell, double p_factor_pThread){
  if (cell == nullptr) {
    std::cout << "moveOutCell: cell is nullptr" << std::endl;
    return;
  }
  int s_x = cell->cur_x_;
  int s_y = cell->cur_y_;
  int width = cell->width_;
  int height = cell->height_;
  MIA::VT_type vt = cell->VT_type_;

  if (cell->is_intra_MIA_vio_){
    if (cell->filler_l == nullptr or cell->filler_r == nullptr) {
      std::cout << "moveOutCell: filler is nullptr" << std::endl;
      return;
    }
    changeFillerWidth(cell->filler_l, 0, 1);
    changeFillerWidth(cell->filler_r, 0, 1);
  }

  //inter row filler clear
  if (ckt.check_inter_row_MIA){
    clearInterRowFiller(cell); 
  }

  reduceVTOccupy(cell);

  std::set<Cell *> cells_to_do_DVFA;
  for (int j = s_y; j < (s_y + height); j+=defaultH)
  {
    for (int i = s_x; i < (s_x + width); ++i)
    {
      NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + i];
      --node_info.usage_;
      node_info.updated_ = false;
      if (fabs(p_factor_pThread + 1) <= EPSILON)
        node_info.pCost_ = pow(node_info.usage_  + 1, mi) * p_factor;
      else 
        node_info.pCost_ = pow(node_info.usage_  + 1, mi) * p_factor_pThread;
      node_info.pathCost_ = node_info.pCost_ * node_info.hisCost_;

      if(ckt.do_detailed_MIA){
        if (!node_info.fillerList.empty()){
          for (auto& filler: node_info.fillerList){
            cells_to_do_DVFA.insert(filler->cell_);
          }
        }
      } 
    }
    if(ckt.do_detailed_MIA){
      if (!outBox(s_x - 1, j, 1, defaultH)){
        NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + s_x - 1];
        if (!node_info.fillerList.empty()){
          for (auto& filler: node_info.fillerList){
            cells_to_do_DVFA.insert(filler->cell_);
          }
        } 
      }
      if (!outBox(s_x + width, j, 1, defaultH)){
        NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + s_x + width];
        if (!node_info.fillerList.empty()){
          for (auto& filler: node_info.fillerList){
            cells_to_do_DVFA.insert(filler->cell_);
          }
        }
      }
    }
  }
  if(ckt.do_detailed_MIA){  
    for (Cell* cell_to_do_DVFA: cells_to_do_DVFA){
      if (cell_to_do_DVFA == cell) continue;
      DVFA(cell_to_do_DVFA);
    }
  }
}

int Naller::countMIAOverflows(bool update_his){
  int of_cnt = 0;
    for (unsigned i = 0; i < cell_num; ++i) 
    {
      Cell* sp = ckt.cells[ i ];
      if(sp->isFixed_) {
        continue;
      }
      calCellMIAOf(sp, of_cnt, update_his); 
    }
    return of_cnt;
}

int Naller::countMIAOverflows(const std::vector<unsigned>& batch) {
  int of_cnt = 0;
  for (auto i : batch)  
  {
    Cell* sp = ckt.cells[ i ];
    if(sp->isFixed_) {
      continue;
    }
    calCellMIAOf(sp, of_cnt); 
  }
  return of_cnt;
}

int Naller::countMIAOverflows(int index, int threshold) {
  int of_cnt = 0;
  batches_of_netIds[index].clear();
  for (auto cellId : batches[index])  
  {
    Cell* sp = ckt.cells[ cellId ];
    assert(!sp->isFixed_);
    calCellMIAOf(sp, of_cnt); 
    if(threshold > 0) {
        if(sp->of_ > 0) {
            batches_of_netIds[index].push_back(cellId);
        }
    }
  }
  return of_cnt;
}

void Naller::calCellMIAOf(Cell* cell, int& of_cnt, bool update_his){
  int s_x = cell->cur_x_;
  int s_y = cell->cur_y_;
  int width = cell->width_;
  int height = cell->height_;
  cell->of_ = 0;
  for (int j = s_y; j < (s_y + height) ; j+=defaultH){
    for (int i = s_x; i < (s_x + width); ++i)
    {
      NodeInfo& node_info = node.node_infos[g_max_x*j/defaultH + i];
      cell->of_ += node_info.usage_ - 1;
      if((node_info.usage_) <= 0) {
        printlog(LOG_INFO, "  cur_x_ : %f, cur_y_ : %f, width : %f, height : %f, usage %d", 
          s_x * min_width, s_y * min_width, 
          width * min_width, height * min_width, node_info.usage_);
        printlog(LOG_INFO, "i : %f, j is %f", i * min_width, j * min_width);
      }
      int MIA_of = 0;
      if (!node_info.fillerList.empty()){
        for (auto& filler : node_info.fillerList){
          if (filler->type_ != cell->VT_type_ and filler->is_hard_ and filler->is_valid_){
            MIA_of++;
          }
        }
      }
      cell->of_ += MIA_of;
      if(!update_his) continue;
      if(!node_info.updated_ and (node_info.usage_ > 1 or MIA_of > 0)) {
        node_info.updated_ = true;
        if (node_info.usage_ > 1){
          node_info.hisCost_ += 1 * (node_info.usage_ - 1);
        }
        if (MIA_of > 0){
          node_info.hisCost_ += 1 * MIA_of;
        }
        node_info.pathCost_ = node_info.pCost_ * node_info.hisCost_;           
      }
    }
  }
  of_cnt += cell->of_;

  if (ckt.check_inter_row_MIA){
    if (cell->VT_type_ != MIA::SVT){
      if(checkInterRowMIA(cell)){
        DVFA(cell);
      }
    }
  }
}

void Naller::clearInterRowFiller(Cell* cell, bool check_filler_exist, bool check_top, bool check_bottom){
  if (check_top and cell->inter_row_overlap_t_ != 0) {
    if(check_filler_exist) {  
      if (cell->inter_filler_tl == nullptr or cell->inter_filler_tr == nullptr) {
        std::cout << "clearInterRowFiller: inter_filler is nullptr" << std::endl;
        return;
      }
    }
    if (cell->inter_filler_tl == nullptr or cell->inter_filler_tr == nullptr) {
      cell->inter_row_overlap_t_ = 0;
    }
    else {
      reduceNodeFillerOccupy(cell->inter_filler_tl);
      reduceNodeFillerOccupy(cell->inter_filler_tr);
      delete cell->inter_filler_tl;
      delete cell->inter_filler_tr;
      cell->inter_filler_tl = nullptr;
      cell->inter_filler_tr = nullptr;
      cell->inter_row_overlap_t_ = 0;
    }
  }
  if (check_bottom and cell->inter_row_overlap_b_ != 0) {
    if(check_filler_exist) {  
      if (cell->inter_filler_bl == nullptr or cell->inter_filler_br == nullptr) {
        std::cout << "clearInterRowFiller: inter_filler is nullptr" << std::endl;
        return;
      }
    }
    if (cell->inter_filler_bl == nullptr or cell->inter_filler_br == nullptr) {
      cell->inter_row_overlap_b_ = 0;
    }
    else {
      reduceNodeFillerOccupy(cell->inter_filler_bl);
      reduceNodeFillerOccupy(cell->inter_filler_br);
      delete cell->inter_filler_bl;
      delete cell->inter_filler_br;
      cell->inter_filler_bl = nullptr;
      cell->inter_filler_br = nullptr;
      cell->inter_row_overlap_b_ = 0;
    }
  }
}

bool Naller::checkInterRowMIA(Cell* cell){
  bool res = false;

  if (cell->of_ == 0) {  
    int s_x = cell->cur_x_ + cell->width_ - 1;
    int s_y = cell->cur_y_;
    int height = cell->height_;
    MIA::VT_type vt = cell->VT_type_;
    if (cell->is_intra_MIA_vio_) {
      if (cell->filler_r == nullptr) {
        std::cout << "checkInterRowMIA: filler_r is nullptr" << std::endl;
        return false;
        }
      if (cell->filler_r->is_hard_ == false) return false;
      int filler_s_x = cell->filler_r->x_;
      int filler_s_y = cell->filler_r->y_;
      int filler_width = cell->filler_r->getWidth();
      int filler_height = cell->filler_r->height_;
      for (int j = filler_s_y; j < filler_s_y + filler_height; j+= defaultH){
        for (int i = filler_s_x; i < filler_s_x + filler_width; i++){
          if (getNodeOccupy(i, j) > 1) return false;
        }
      }
      s_x += filler_width;
      if (cell->width_ + cell->filler_l->getWidth() + filler_width <= ckt.MIA_min_width_){
        return false;
      }
    }

    if (outBox(s_x + 1, s_y, 1, defaultH)) return false;
    if (getNodeVTOccupy(s_x + 1, s_y, vt) > 0) return false; 
    
    for (int i = -1; i < 2; i += 2) {
      int y;
      if (i == -1) y = s_y - defaultH;
      else y = s_y + height;
      if (outBox(s_x, y, 1, defaultH)) continue;
      bool check_diff_vt = false;
      for (int i_ = 0; i_ < ckt.MIA_min_width_; ++i_) {
        int same_vt_occupy = getNodeVTOccupy(s_x - i_, y, vt);
        if (check_diff_vt) {
          if (same_vt_occupy == 0) {
            if (i == -1) cell->inter_row_overlap_b_ = i_;
            else cell->inter_row_overlap_t_ = i_;
            res = true;
            break;
          }
        }
        int total_occupy = getNodeOccupy(s_x - i_, y);
        if (same_vt_occupy > 0 and same_vt_occupy == total_occupy) {
          check_diff_vt = true;
        }
        else break;
      }
    }
  }


  if (res == false) clearInterRowFiller(cell);

  return res;
}


void circuit::write_temp_result(std::string filename){
  std::ofstream outfile (filename);
  for( size_t i = 0; i < cell_num; ++i ){
    Cell* theCell = cells[i];
    Macro* theMacro = &macros[theCell->type_];
    int ix = round(theCell->init_x_*DEFdist2Microns*min_width);
    int iy = round(theCell->init_y_*DEFdist2Microns*min_width);
    int cx = round(theCell->cur_x_*DEFdist2Microns*min_width);
    int cy = round(theCell->cur_y_*DEFdist2Microns*min_width);
    int cw = round(theCell->width_*DEFdist2Microns*min_width);
    int ch = round(theCell->height_*DEFdist2Microns*min_width);
    string vt;
    if (theCell->VT_type_ == MIA::HVT) vt = "HVT";
    else if (theCell->VT_type_ == MIA::LVT) vt = "LVT";
    else if (theCell->VT_type_ == MIA::SVT) vt = "SVT";
    else vt = "N/A";
    outfile << "   - " << theCell->name_ << " " << theMacro->name << " VT_TYPE = " << vt << std::endl;
    if(theCell->isFixed_)
      outfile << "      + " << "FIXED ( " << ix << " " << iy << " ) " << theCell->cellorient_ << " ;" << std::endl;
    else  {
      outfile << "      + " << "PLACED ( " << cx << " " << cy << " ) " << theCell->cellorient_ << " ;" << std::endl; 
    } 
    outfile << "      + " << "SIZE ( " << cw << " " << ch << " ) ;" << std::endl;
    if (do_MIA) {
      std::vector<MIA::Filler*> filler_list;
      if (theCell->is_intra_MIA_vio_) {
        if (theCell->filler_l == nullptr or theCell->filler_r == nullptr) {
          std::cout << "write_temp_result:: intra cell filler is null" << std::endl;
        }
        filler_list.push_back(theCell->filler_l);
        filler_list.push_back(theCell->filler_r);
      }
      if (theCell->inter_row_overlap_b_ > 0) {
        if (theCell->inter_filler_bl == nullptr or theCell->inter_filler_br == nullptr) {
          std::cout << "write_temp_result:: inter cell bottom filler is null" << std::endl;
        }
        filler_list.push_back(theCell->inter_filler_bl);
        filler_list.push_back(theCell->inter_filler_br);
      }
      if (theCell->inter_row_overlap_t_ > 0) {
        if (theCell->inter_filler_tl == nullptr or theCell->inter_filler_tr == nullptr) {
          std::cout << "write_temp_result:: inter cell top filler is null" << std::endl;
        }
        filler_list.push_back(theCell->inter_filler_tl);
        filler_list.push_back(theCell->inter_filler_tr);
      }
      for (auto &theFiller : filler_list) {
        if (theFiller->is_valid_ == false) continue;
        int fx = round(theFiller->x_*DEFdist2Microns*min_width);
        int fy = round(theFiller->y_*DEFdist2Microns*min_width);
        int fw = round(theFiller->width_*DEFdist2Microns*min_width);
        int fh = round(theFiller->height_*DEFdist2Microns*min_width);
        string hard_or_soft;
        if (theFiller->is_hard_) hard_or_soft = "HARD";
        else hard_or_soft = "SOFT";
        string vt_;
        if (theFiller->type_ == MIA::HVT) vt_ = "HVT";
        else if (theFiller->type_ == MIA::LVT) vt_ = "LVT";
        else if (theFiller->type_ == MIA::SVT) vt_ = "SVT";
        else vt_ = "N/A";
        if (vt_ != vt) {
          std::cout << "write_temp_result:: filler vt not match" << std::endl;
        }
        outfile << "            + " << "Filler "<< hard_or_soft <<std::endl;
        outfile << "                  + " << "PLACED ( "<< fx << " " << fy << " ) ;" << std::endl;
        outfile << "                  + " << "SIZE ( " << fw << " " << fh << " ) ;" << std::endl;
      }
    }
  }
}