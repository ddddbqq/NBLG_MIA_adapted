#pragma once
#include <global.h>


static FILE* fout;
static void* userData;
#define PI_PIN 1
#define PO_PIN 2
#define NONPIO_PIN 3

#ifdef ENABLE_TIME_MEASUREMENT
class PrecisionTimer{
public:
    using Clock = std::chrono::high_resolution_clock;
    using Duration = std::chrono::nanoseconds;
    
    void start() {
        start_time = Clock::now();
    }
    
    Duration stop() {
        return std::chrono::duration_cast<Duration>(Clock::now() - start_time);
    }
    
private:
    std::chrono::time_point<Clock> start_time;
};

struct FunctionStats {
    std::atomic<uint64_t> call_count{0};
    std::atomic<uint64_t> total_ns{0};
    std::atomic<uint64_t> min_ns{std::numeric_limits<uint64_t>::max()};
    std::atomic<uint64_t> max_ns{0};
    
    void record(uint64_t duration_ns) {
        call_count.fetch_add(1, std::memory_order_relaxed);
        total_ns.fetch_add(duration_ns, std::memory_order_relaxed);
        
        uint64_t current_min = min_ns.load(std::memory_order_relaxed);
        while (duration_ns < current_min && 
               !min_ns.compare_exchange_weak(current_min, duration_ns, 
                                             std::memory_order_relaxed)) {}
        
        uint64_t current_max = max_ns.load(std::memory_order_relaxed);
        while (duration_ns > current_max && 
               !max_ns.compare_exchange_weak(current_max, duration_ns, 
                                            std::memory_order_relaxed)) {}
    }
};



void print_function_stats();

#endif


class Cell;
namespace MIA{

//const float MIA_WEIGHT = 3.0;
//const float FIXED_HARD_FILLER_WEIGHT = 3.0;

enum VT_type{
  SVT,
  LVT,
  HVT,
  TPN
};

class Filler{
public:
  Filler(){
    x_ = -1;
    y_ = -1;
    width_ = 0;
    height_ = 0;
    cell_ = NULL;
    type_ = SVT;
    is_hard_ = false;
    left_or_right_ = 0;
    is_valid_ = true;
  };
  Filler(Cell* cell, int l_or_r, int width);  
  void setSoft();
  void setHard();
  void changeWidth(int w);
  void setPos(int x, int y);
  double calCost(Cell* income_cell);
  int getWidth();

public:
  int x_;
  int y_; //bottom left corner
  int width_;
  int height_;
  Cell* cell_;
  VT_type type_;
  bool is_hard_;
  bool is_valid_;
  unsigned left_or_right_; //0 for none, 1 for left, 2 for right, located at left or right of the cell

};

}

class site {
public:
  std::string name;
  double width;                /* in microns */
  double height;               /* in microns */
  std::string type;                 /* equivalent to class, I/O pad or CORE */
  std::vector< std::string > symmetries; /* {X | Y | R90} */

  site() : name(""), width(0.0), height(0.0), type("") {}
  site(const site& s)
      : name(s.name),
        width(s.width),
        height(s.height),
        type(s.type),
        symmetries(s.symmetries) {}
};

class layer {
public:
  std::string name;
  std::string type;
  std::string direction;
  double xPitch;  /* in microns */
  double yPitch;  /* in microns */
  double xOffset; /* in microns */
  double yOffset; /* in microns */
  double width;   /* in microns */

  // -------------
  layer()
      : name(""),
        type(""),
        direction(""),
        xPitch(0.0),
        yPitch(0.0),
        xOffset(0.0),
        yOffset(0.0),
        width(0.0) {}
};

class Macro_pin
{
public:
    std::string direction;     
    std::string name;                 
    unsigned layer;
    double xLL, yLL;         /* in microns */
    double xUR, yUR;         /* in microns */
    int xLL_offset_psite, xUR_offset_psite;
    int yLL_offset_psite, yUR_offset_psite;

    Macro_pin() : direction(""), layer(0),
                        xLL(std::numeric_limits<double>::max()), yLL(std::numeric_limits<double>::max()), 
                                xUR(std::numeric_limits<double>::min()), yUR(std::numeric_limits<double>::min()) {}
    Macro_pin(unsigned layer0, int xLL_offset_psite0, int xUR_offset_psite0,
        int yLL_offset_psite0, int yUR_offset_psite0) : layer(layer0),
                        xLL_offset_psite(xLL_offset_psite0), xUR_offset_psite(xUR_offset_psite0), 
                                yLL_offset_psite(yLL_offset_psite0), yUR_offset_psite(yUR_offset_psite0) {}                            
};


template<typename _T>
class Rect
{
public:
    _T xLL_, yLL_;       
    _T xUR_, yUR_;
    
    Rect() {}

    Rect( const _T xll,  const _T yll,
    const _T xur,  const _T yur)
      :  xLL_(xll),  yLL_(yll), xUR_(xur), yUR_(yur) { }

    inline void set(const _T& xLL, const _T& yLL, const _T& xUR, const _T& yUR) {
        xLL_ = xLL;
        yLL_ = yLL;
        xUR_ = xUR;
        yUR_ = yUR;
    }
    inline void update(const _T& x, const _T& y) {
        if (x > xUR_) xUR_ = x;
        if (x < xLL_) xLL_ = x;
        if (y > yUR_) yUR_ = y;
        if (y < yLL_) yLL_ = y;
    }    
    inline void update_x(const _T& x) {  //yLL_, yUR_ not change
        if (x > xUR_) xUR_ = x;
        if (x < xLL_) xLL_ = x;
    }  
    inline void update_y(const _T& y) {  //xLL_, xUR_ not change
        if (y > yUR_) yUR_ = y;
        if (y < yLL_) yLL_ = y;
    }  
};

class Macro
{
public:
    std::string name;
    std::string type;                               /* equivalent to class, I/O pad or CORE */
    bool isFlop;                               /* clocked element or not */
    double xOrig;                              /* in microns */
    double yOrig;                              /* in microns */
    double width;                              /* in microns */
    double height;                             /* in microns */
    int numVDD;                               /* number of VDD ports */
    int numVSS;                               /* number of VSS ports */
    int aligendRow;                /*  PG alignment 0:VSS 1:VDD 2:ANY */
    int lEdgeT_;
    int rEdgeT_;
    std::vector<unsigned> sites;  
    std::unordered_map<std::string, Macro_pin> pins;  
    std::vector<Rect<double>> obses;   
                         /* keyword OBS for non-rectangular shapes in micros */

  Macro() : name(""), type(""), isFlop(false), xOrig(0.0), yOrig(0.0), 
        width(0.0), height(0.0), numVDD(0), numVSS(0), lEdgeT_(0), rEdgeT_(0) {}

};

class Row
{
public:
    /* from DEF file */
    std::string name;
    unsigned site;
    int origX;               /* (in DBU) */
    int origY;               /* (in DBU) */
    int stepX;               /* (in DBU) */
    int stepY;               /* (in DBU) */
    int numSites;
    std::string siteorient;
    // for contest ITDP2015
    // int siteIndexOffset;
    Row() : name(""), site(std::numeric_limits<unsigned>::max()), 
            origX(0), origY(0), stepX(0), stepY(0), numSites(0), siteorient("") {}
};

class Cell                     //personal use
{
public:
    std::string name_;
    std::string cellorient_;
    unsigned id_;
    unsigned type_;                              /* index to some predefined macro */
    int regionId_;  //-1:whitespace
    
    int cur_x_; 
    int cur_y_;                     // must be aligned 
    int cur_x0_;
    int cur_y0_;

    int lEdgeT_;  //  -1 : white or fixed cells 
    int rEdgeT_;
    // vector<double> usageVec_;  //if non-fulled grid used
    int ripup_cnt_; 
    int of_;
    int width_;                      
    int height_;                      //
                             /* fixed cell or not */

    int aligendRow_;                /*  PG alignment 0:VSS 1:VDD 2:ANY*/
    
    bool isBottomVss_;
    bool isFixed_; 
    double init_x_, init_y_;    
    double cur_x_temp_;
    double cur_y_temp_;
    Rect<int> extendedWin_;
    std::vector<Macro_pin> signal_pins_;  //dont use ptr for multi-thread safe

    MIA::Filler* filler_l;
    MIA::Filler* filler_r;
    bool is_intra_MIA_vio_;
    inline void setIntraMIAvio(){
        is_intra_MIA_vio_ = true;
    }
    MIA::VT_type VT_type_;                     
    inline void setSVT(){
        VT_type_ = MIA::SVT;
    }
    inline void setLVT(){
        VT_type_ = MIA::LVT;
    }
    inline void setHVT(){
        VT_type_ = MIA::HVT;
    }
    inline void setTPN(){
        VT_type_ = MIA::TPN;
        is_TPN_cell_ = true;
        is_intra_MIA_vio_ = false;
    }

    int inter_row_overlap_t_;
    int inter_row_overlap_b_;
    MIA::Filler* inter_filler_tl;
    MIA::Filler* inter_filler_tr;
    MIA::Filler* inter_filler_bl;
    MIA::Filler* inter_filler_br;
    
    bool is_TPN_cell_;

    
    Cell ()  { name_ = ""; cellorient_ = ""; id_ = 0; type_ = 0; regionId_ = -1; cur_x_ = 0; cur_y_ = 0; 
        cur_x0_ = 0; cur_y0_ = 0; lEdgeT_ = 0; rEdgeT_ = 0; of_ = 0; ripup_cnt_ = 0; width_ = 0; height_ = 0;
        aligendRow_ = 2; isBottomVss_ = true; isFixed_ = false; init_x_ = 0.0; init_y_ = 0.0;  cur_x_temp_ = 0.0; 
        cur_y_temp_ = 0.0;  setWinSize(0, 0, 0, 0); VT_type_ = MIA::SVT; is_intra_MIA_vio_ = false; 
        filler_l = nullptr; filler_r = nullptr; inter_row_overlap_t_ = 0; 
        inter_row_overlap_b_ = 0; inter_filler_tl = nullptr; inter_filler_tr = nullptr; 
        inter_filler_bl = nullptr; inter_filler_br = nullptr; is_TPN_cell_ = false;
        }
    inline void setWinSize(int xLL, int yLL, int xUR, int yUR) {
        extendedWin_.set(xLL, yLL, xUR, yUR);
    }

};

class FenceRegion
{
public:
    FenceRegion() : id_(0), name_("") {}
public:
    unsigned id_;
    std::string name_;
    std::vector<Rect<int>> rects_;
};

class Pin
{
public:
    // from verilog
    std::string inst_name;                         /* Name of pins : instance name + "_" + port_name */
    std::string port_name;
    unsigned id;
    unsigned owner;                      /* The owners of PIs or POs are UINT_MAX */
    unsigned net;
    unsigned type;                       /* 1=PI_PIN, 2=PO_PIN, 3=others */
    unsigned layer;                      // from 0 to k-1
    bool isFlopInput;                    /* is this pin an input  of a clocked element? */
    bool isFlopCkPort;

    // from .sdc
    double cap;                          /* PO load if exists (in Farad) */
    double delay;                        /* PI/PO input/output delay if exists (in sec) */
    double rTran, fTran;                 /* input rise/fall transition if exits (in sec) */
    int driverType;                      /* input driver for PIs */

    // from .def
    double x_coord, y_coord;             /* (in DBU) */
    double x_offset, y_offset;           /* COG of VIA relative to the origin of a cell, (in DBU) */
    bool isFixed;                        /* is this node fixed? */

    // from timer
    double earlySlk, lateSlk;
    
    //for PO's weight HSIEH
    double earlyWeight, lateWeight;
    //for time
    double earlyAt, lateAt, earlyRat, lateRat;
  
    /* used for placement */
    bool marked;
    unsigned portType; // 0 : input port, 1 : output port, 2 : primary Input, 3 : primary Output
    Pin() : inst_name(""),
            port_name(""),
          id(std::numeric_limits<unsigned>::max()), 
                    owner(std::numeric_limits<unsigned>::max()),
          net(std::numeric_limits<unsigned>::max()), 
          type(std::numeric_limits<unsigned>::max()), 
          layer(0),
                    isFlopInput(false), isFlopCkPort(false),
                    cap(0.0), delay(0.0), rTran(0.0), fTran(0.0), driverType(std::numeric_limits<unsigned>::max()),
                    x_coord(0.0), y_coord(0.0), x_offset(0.0), y_offset(0.0), 
                    isFixed(false), earlySlk(0.0), lateSlk(0.0) {}

};

class SpNetRegion
{
public:
    SpNetRegion() {
        layer_ = -1;
        rect_.set(0, 0, 0, 0);
    }
public:
    int layer_;
    Rect<int> rect_;

    inline void setRect(int xLL, int yLL, int xUR, int yUR) {
        rect_.set(xLL, yLL, xUR, yUR);
    }

};
class Net
{
public:
    std::string name;
    unsigned source;             /* input pin index to the net */
    double lumped_wire_cap;      /* total lump sum wire cap for the net*/
    std::vector<unsigned> sinks;      /* sink pins indices of the net */
    std::vector< std::pair< std::pair <std::string, std::string>, double > > wire_segs;   /* connecting pin (source, sink) names & length */
    
    //HSIEH for net weight
    std::vector<double> earlyWeights; /* weights to every nets */
    std::vector<double> lateWeights;
  
    Net() : name(""), source(std::numeric_limits<unsigned>::max()), lumped_wire_cap(0.0) {}

};

class circuit
{
public:
    double temp_delta_rate_for_output;
    std::map<std::string, unsigned > site2id;  /* between site     name and ID */
    std::map<std::string, unsigned > layer2id; /* between layer    name and ID */
    std::map<std::string, unsigned > macro2id; /* between macro    name and ID */
    std::map<std::string, unsigned > row2id;    /* between row      name and ID */
    std::map<std::string, unsigned > cell2id;   /* between cell2id  name and ID */
    std::map<std::string, unsigned > pin2id;    /* between pin2id   name and ID */
    std::map<std::string, unsigned> region2id;
    std::map<std::string, unsigned> net2id;           /* between net      name and ID */
    


    std::vector< site > sites;   /* site list */
    std::vector< layer > layers; /* layer list */
    std::vector< Macro > macros; /* macro list */
    std::vector< Row > rows;     /* row list */
    std::vector<Pin> pins;            /* pin list   {def: pins, cell:port in nets}  */
    std::vector<Rect<int>> blockRegions;  /* blockage regions  */
    std::vector<FenceRegion*> fenceRegions;
    std::vector<SpNetRegion*> spNetRegions;
    std::vector<Cell*> cells;   /* cell list */
    std::vector<unsigned> cellIds;
    std::vector<unsigned> defaultCellIds;
    std::vector<Net> nets;              /* net list */
    std::vector<std::vector<unsigned>> fenceCellIds;
    std::map<std::string, std::vector<unsigned>> group2cellIds;
    std::unordered_map<std::string, std::vector<unsigned>> region2cellIds;
    
    unsigned countComponents = 0;
    unsigned countPins = 0;

    std::string design_name;
    unsigned DEFdist2Microns;
    double rowHeight;
    double lx, rx, by, ty;          /* placement image's left/right/bottom/top end coordintes */

    std::string defLoc = "";
    char  *constraints = NULL, *out_def = NULL, *size = NULL;
    // global var
    int g_max_x;
    int g_max_y;
    int defaultH;
    double min_width;
    const char* FFClkPortName = "ck";
    std::vector<int> edge_table;

    bool doTech;// pin short/access
    unsigned num_unfixed_inst_;
    std::map<int, std::pair<int,double>> kMacros;   //height: {nums, displacement}
    unsigned cell_num;
    bool doParallel;
    double max_disp;
    double s_am;
    int Np;
    int Ne;
public:
    site* locateOrCreateSite(const std::string& siteName);
    layer* locateOrCreateLayer(const std::string& layerName);
    Macro* locateOrCreateMacro(const std::string& macroName);
    Row* locateOrCreateRow(const std::string& rowName);
    Cell* locateOrCreateCell(const std::string& cellName);
    FenceRegion* locateOrCreateRegion(const std::string &regionName);
    Pin* locateOrCreatePin(const std::string &instName, const std::string &portName);
    Net* locateOrCreateNet(const std::string& netName);

    void read_lef_macro_define_top_power(Macro* myMacro);

    int ReadLef(const std::vector<std::string>& lefStor);
    int ReadDef(const std::string& defName);
    void read_files(int argc, char *argv[]);
    void writeComponents(std::ofstream &os);
    void write_def();
    void cal_hpwl();
    // set max_x, max_y
    void setMaxXY();

    //std::vector<MIA::Filler*> fillers;
    bool do_MIA;
    bool check_inter_row_MIA;
    bool do_detailed_MIA;
    void setMIACells(int debug_flag = 0, bool check_inter_row = false, bool do_detailed = false, 
                     float LVT_ratio = 0.1, float HVT_ratio = 0.1, int MIA_min_width = 4, 
                     int MIA_inter_row_min_width = 2);
    void setTPNcells(float TPN_ratio = 0.3);
    void double_or_triple_cell_height(float double_rate = 0.1, float triple_rate = 0.05);
    int MIA_min_width_;
    int MIA_inter_row_min_width_;
    void write_temp_result(std::string filename);

public:
    circuit() : edge_table(5, 0) {
        doParallel = false;
        do_MIA = false;
        check_inter_row_MIA = false;
    } 
};

class CircuitParser
{
public:
    circuit* ckt_;
    static Macro* topMacro_;
    static std::string temp_groupName; // for group member
    static int iter_i; // for group member
    CircuitParser(circuit* ckt_);
    circuit* Circuit() { return ckt_; };
public:
    
    // LEF CallBacks.
    static int LefSpacingCbk( lefrCallbackType_e c, lefiSpacing* l, lefiUserData ud );
    static int LefPropCbk( lefrCallbackType_e c, lefiProp* p, lefiUserData ud );
    static int LefSiteCbk( lefrCallbackType_e c, lefiSite* si, lefiUserData ud );
    static int LefLayerCbk( lefrCallbackType_e c, lefiLayer* la, lefiUserData ud );
    static int LefStartCbk( lefrCallbackType_e c, const char* name, lefiUserData ud );
    static int LefMacroCbk( lefrCallbackType_e c, lefiMacro* ma, lefiUserData ud ); 
    static int LefMacroPinCbk( lefrCallbackType_e c, lefiPin * ma, lefiUserData ud );
    static int LefMacroObsCbk( lefrCallbackType_e c, lefiObstruction* obs, lefiUserData ud );
    static int LefEndCbk( lefrCallbackType_e c, const char* name, lefiUserData ud );

    // DEF CallBacks.
    static int DefDesignCbk(defrCallbackType_e c, const char* string, defiUserData ud);
    static int DefUnitsCbk(defrCallbackType_e c, double d, defiUserData ud);
    static int DefDieAreaCbk( defrCallbackType_e c, defiBox* box, defiUserData ud );

    static int DefRowCbk(defrCallbackType_e c, defiRow* ro, defiUserData ud);
    static int DefStartCbk(defrCallbackType_e c, int num, defiUserData ud);
    static int DefComponentCbk(defrCallbackType_e c, defiComponent* co, defiUserData ud);
    static int DefRegionCbk(defrCallbackType_e c, defiRegion* re, defiUserData ud);
    static int DefPinCbk(defrCallbackType_e c, defiPin* pi, defiUserData ud);
    static int DefNetCbk(defrCallbackType_e c, defiNet* net, defiUserData ud);
    static int DefSNetCbk(defrCallbackType_e c, defiNet* wire, defiUserData ud);
    static int DefSNetWireCbk(defrCallbackType_e c, defiNet* wire, defiUserData ud);

    static int DefGroupCbk(defrCallbackType_e c, defiGroup* go, defiUserData ud);
    static int DefGroupNameCbk(defrCallbackType_e c, const char* name, defiUserData ud);
    static int DefGroupMemberCbk(defrCallbackType_e c, const char* name, defiUserData ud);
    
    static int DefBlockageCbk(defrCallbackType_e c, defiBlockage* block, defiUserData ud);

    static int DefEndCbk(defrCallbackType_e c, void*, defiUserData ud);
};


void printWarning(const char *str);
void lineNumberCB(int lineNo);
void errorCB(const char* msg);
void warningCB(const char* msg);
void printMacro(defiUserData ud, const std::string& name);

void lineNumberCB(long long lineNo);
int unUsedCB(defrCallbackType_e, void*, defiUserData);
std::string getKeyByValue(std::map<std::string, unsigned>& map, unsigned value);

int ReadLef(const std::vector<std::string>& lefStor);
void read_files(int argc, char *argv[]);
void myLogFunction(const char* errMsg);
void myWarningLogFunction(const char* errMsg);

