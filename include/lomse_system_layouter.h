//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SYSTEM_LAYOUTER_H__        //to avoid nested includes
#define __LOMSE_SYSTEM_LAYOUTER_H__

#include "lomse_basic.h"
#include "lomse_time.h"
#include "lomse_score_enums.h"
#include "lomse_logger.h"
#include <list>
#include <vector>
using namespace std;

namespace lomse
{

//forward declarations
class ImoInstrument;
class ImoScore;
class ImoStaffObj;
class ImoStaff;
class GmoShape;
class GmoBoxSystem;
class GmoBoxSliceInstr;
class GmoBoxSlice;
class ScoreMeter;
class ShapesStorage;

class ScoreLayouter;
class SystemLayouter;
class ColumnLayouter;
class ColumnStorage;
class LineTable;
class LineEntry;
class BreaksTable;
class BreakPoints;
class ColumnSplitter;
class ColumnResizer;
class LineResizer;
class LineSpacer;
class TimeGridLineExplorer;

//some helper definitions
#define LineEntryIterator		  std::vector<LineEntry*>::iterator
#define LinesIterator             std::vector<LineTable*>::iterator
#define LineSpacersIterator       std::vector<LineSpacer*>::iterator



//----------------------------------------------------------------------------------------
//BreaksTable definition: table to contain possible break points
//----------------------------------------------------------------------------------------

//an entry of the BreaksTable
typedef struct BreaksTimeEntry_Struct
{
    float   rTimepos;
    float   rPriority;
    LUnits  uxStart;
    LUnits  uxEnd;
    bool    fInBeam;
    LUnits  uxBeam;
}
BreaksTimeEntry;

//the breaks table
class BreaksTable
{
private:
    std::list<BreaksTimeEntry*>               m_BreaksTable;      //the table
    std::list<BreaksTimeEntry*>::iterator     m_it;               //for get_first(), get_next();

public:
    BreaksTable();
    ~BreaksTable();

    void add_entry(float rTime, LUnits uxStart, LUnits uWidth, bool fInBeam,
                  LUnits uxBeam, float rPriority = 0.8f);
    void add_entry(BreaksTimeEntry* pBTE);
    void change_priority(int iEntry, float rMultiplier);
//    std::string dump();
    inline bool is_empty() { return m_BreaksTable.empty(); }

    //traversing the table
    BreaksTimeEntry* get_first();
    BreaksTimeEntry* get_next();

};



//----------------------------------------------------------------------------------------
//LineEntry: an entry in LineTable
//----------------------------------------------------------------------------------------
class LineEntry
{
public:
    // constructor and destructor
    LineEntry(ImoStaffObj* pSO, GmoShape* pShape, bool fProlog, float rTime);
    ~LineEntry() {}

    void reposition_at(LUnits uxNewXLeft);
	void assign_fixed_and_variable_space(ColumnLayouter* pTT, float rFactor);
    void move_shape(UPoint sliceOrg);
    void add_shape_info();

    //access to entry data
    inline bool is_barline_entry() { return m_fIsBarlineEntry; }
    inline ImoStaffObj* get_staffobj() { return m_pSO; }
    inline GmoShape*  get_shape() { return m_pShape; }
	inline bool is_prolog_object() { return m_fProlog; }
    inline float get_timepos() { return m_rTimePos; }
    inline LUnits get_position() { return m_xLeft; }
    inline LUnits get_anchor() { return m_uxAnchor; }
    inline LUnits get_x_final() { return m_xFinal; }
    inline LUnits get_shape_size() { return m_uSize; }
    LUnits get_fixed_space() { return m_uFixedSpace; }
    inline LUnits get_variable_space() { return m_uVariableSpace; }

    //setters and getters
    inline LUnits get_total_size() { return m_uSize + m_uFixedSpace + m_uVariableSpace; }
    inline void set_variable_space(LUnits space) { m_uVariableSpace = space; }
    inline void set_fixed_space(LUnits space) { m_uFixedSpace = space; }
    inline void set_size(LUnits width) { m_uSize = width; }
    inline float get_duration() { return 0.0f; } //TODO m_pSO->GetTimePosIncrement(); }
    inline void set_position(LUnits uPos) { m_xLeft = uPos; }
    inline void mark_as_barline_entry() { m_fIsBarlineEntry = true; }
    LUnits get_shift_to_noterest_center();
    bool is_note_rest();
    bool has_barline();
    void update_x_final() { m_xFinal = m_xLeft + get_total_size(); };

    //debug
    void dump(int iEntry, ostream& outStream);
    static void dump_header(ostream& outStream);


protected:
    //member variables (one entry of the table)
    //----------------------------------------------------------------------------
    bool            m_fIsBarlineEntry;  //is last entry. Contains barline or nothing
    ImoStaffObj*    m_pSO;              //ptr to the StaffObj
    GmoShape*       m_pShape;           //ptr to the shape
	bool			m_fProlog;          //this shape is a prolog object (clef, KS, TS at start of system)
    float           m_rTimePos;         //timepos for this pSO or -1 if not anchored in time
    LUnits          m_xLeft;            //current position of the left border of the object
    LUnits          m_uxAnchor;         //offset to anchor line
    LUnits          m_xFinal;           //next position (right border position + trailing space)
    //to redistribute objects we need to know:
    LUnits          m_uSize;            //size of the shape (notehead, etc.)
    LUnits          m_uFixedSpace;      //fixed space added after shape
    LUnits          m_uVariableSpace;   //any variable added space we can adjust

    //debug
    bool m_fShapeInfoLoaded;

};


//----------------------------------------------------------------------------------------
// LineTable: an object to encapsulate positioning data for a line
//----------------------------------------------------------------------------------------
class LineTable
{
protected:
	std::vector<LineEntry*>	m_LineEntries;	    //the entries that form this table
	int     m_line;		        //line (0..n-1)
	int     m_nInstr;		    //instrument (1..n)
	int     m_nVoice;		    //voice (1..n) [ = m_line + 1 ]
    LUnits  m_uxLineStart;      //initial position
    LUnits  m_uInitialSpace;    //space at beginning

public:
    LineTable(int line, int nInstr, LUnits uxStart, LUnits uSpace);
    ~LineTable();

    //access to an item
    inline LineEntry* front() { return m_LineEntries.front(); }
    inline LineEntry* back() { return m_LineEntries.back(); }
    inline LineEntry* item(int i) { return m_LineEntries[i]; }
    inline LineEntry* get_last_entry() { return m_LineEntries.back(); }

    //iterator to an item
    inline LineEntryIterator begin() { return m_LineEntries.begin(); }
    inline LineEntryIterator end() { return m_LineEntries.end(); }

    //table manipulation
    inline void clear() { m_LineEntries.clear(); }
    inline void push_back(LineEntry* pEntry) { m_LineEntries.push_back(pEntry); }
	LineEntry* add_entry(ImoStaffObj* pSO, GmoShape* pShape, float rTime);
	LineEntry* add_final_entry(ImoStaffObj* pSO, GmoShape* pShape, float rTime);
    LineEntry* add_final_entry_without_barline();

    //properties
    inline LUnits get_line_start_position() { return m_uxLineStart; }
    inline LUnits get_space_at_beginning() { return m_uInitialSpace; }
    LUnits get_line_width();
    inline size_t size() { return m_LineEntries.size(); }
    inline bool is_for_line(int line) { return m_line == line; }
    inline bool is_line_for_instrument(int nInstr) { return m_nInstr == nInstr; }
    inline bool is_line_for_voice(int nVoice) { return m_nVoice == 0 || m_nVoice == nVoice; }
    inline bool is_voiced_defined() { return m_nVoice != 0; }
    inline void set_voice(int nVoice) { m_nVoice = nVoice; }
    inline int get_instrument() { return m_nInstr; }
    inline int get_voice() { return m_nVoice; }
    bool contains_barline();
    bool is_closed();

    //other
//    void ClearDirtyFlags();
    void add_shapes(GmoBoxSliceInstr* pSliceInstrBox);
    void delete_shapes();

    //Debug and Unit Tests
    inline int get_num_objects_in_line() { return (int)m_LineEntries.size(); }
    void dump_main_table(ostream& outStream);

};


//class DirtyFlagsCleaner
//{
//protected:
//    ColumnStorage*    m_pColStorage;
//
//public:
//    DirtyFlagsCleaner(ColumnStorage* pColStorage);
//
//    void ClearDirtyFlags();
//};


//----------------------------------------------------------------------------------------
//ColumnStorage: encapsulates the lines for a column and stores the auxiliary shapes
//----------------------------------------------------------------------------------------
class ColumnStorage
{
protected:
	std::vector<LineTable*> m_Lines;	    //lines that form this column

public:
    ColumnStorage();
    ~ColumnStorage();

    void initialize();

    //access to an item
    LineTable* front() { return m_Lines.front(); }

    //iterator
    inline LinesIterator begin() { return m_Lines.begin(); }
    inline LinesIterator end() { return m_Lines.end(); }
    inline LinesIterator get_last_line() {
	    LinesIterator it = m_Lines.end();
	    return --it;
    }
//    LinesIterator FindLineForInstrAndVoice(int nInstr, int nVoice);
    LinesIterator find_line(int line);

    //storage manipulation
    LineTable* open_new_line(int line, int instr, LUnits uxStart, LUnits uSpace);
    //inline void save_staff_pointer(int iStaff, ImoStaff* pStaff) { m_pStaff[iStaff] = pStaff; }
    void close_all_open_lines(LUnits xStart);

    //properties
    inline size_t size() { return m_Lines.size(); }
    inline bool is_end_of_table(LinesIterator it) { return it == m_Lines.end(); }

    //access to column measurements
    LUnits get_column_width();
    LUnits get_start_of_bar_position();

    ////units conversion
    //LUnits tenths_to_logical(Tenths value, int staff=0);

    //adding shapes to graphic model
    void add_shapes(GmoBoxSliceInstr* pSliceInstrBox, int iInstr);
    void delete_shapes();

    //debug
    void dump_column_storage(ostream& outStream);

    //Public methods coded only for Unit Tests
    inline int get_num_objects_in_line(int iLine) { return (int)m_Lines[iLine]->size(); }

//    //other methods
//    void ClearDirtyFlags();

protected:
    void delete_lines();

};


//----------------------------------------------------------------------------------------
//LinesBuilder: receives information about objects in a column, organizes this
//info into lines and stores them in the received column storage
//----------------------------------------------------------------------------------------
class LinesBuilder
{
protected:
    ColumnStorage* m_pColStorage;       //music lines for this column
    //std::vector<int> m_nStaffVoice;     //voice assigned to each staff
	LineEntry* m_pCurEntry;				//ptr to last added entry
	LinesIterator m_itCurLine;          //point to the pos table for current line

    LUnits m_uxStart;
    LUnits m_uInitialSpace;

public:
    LinesBuilder(ColumnStorage* pStorage);
    virtual ~LinesBuilder();

    //void initialize();
    inline void set_start_position(LUnits uxStart) { m_uxStart = uxStart; }
    inline void set_initial_space(LUnits uSpace) { m_uInitialSpace = uSpace; }

    //methods to build the lines
    //void start_measurements_for_instrument(int iInstr, LUnits uxStart,
    //                                       ImoInstrument* pInstr, LUnits uSpace);
    void close_line(int iInstr, ImoStaffObj* pSO, GmoShape* pShape, LUnits xStart,
                    float rTime);
    void include_object(int iLine, int iInstr, ImoInstrument* pInstr, ImoStaffObj* pSO,
                        float rTime, int nStaff, GmoShape* pShape);
    void end_of_data();        //inform that all data has been suplied
    void finish_bar_measurements(LUnits xStart);

protected:
    //void reset_default_voices();
    //int decide_voice_to_use(ImoStaffObj* pSO, int nStaff);
    void start_line(int line, int instr);
    //void start_line_inherit_initial_postion_and_space(int line, int instr, int voice);
    //void create_lines_for_each_staff(int nInstr, LUnits uxStart,
    //                                 ImoInstrument* pInstr, LUnits uSpace);

    inline bool is_there_current_line() { return m_itCurLine != m_pColStorage->end(); }


};


//----------------------------------------------------------------------------------------
//ColumnLayouter: column layout algorithm
//  - explores all lines, by time pos, aligning objects
//----------------------------------------------------------------------------------------
class ColumnLayouter
{
protected:
    ColumnStorage* m_pColStorage;               //music lines for this column
    ScoreMeter* m_pScoreMeter;
    bool m_fHasSystemBreak;
    std::vector<LineSpacer*> m_LineSpacers;     //one spacer for each line
    std::vector<GmoBoxSliceInstr*> m_sliceInstrBoxes;   //instr.boxes for this column
    GmoBoxSlice* m_pBoxSlice;       //box for this column
    LUnits  m_uMinColumnSize;       //minimum size for this column

public:
    ColumnLayouter(ColumnStorage* pStorage, ScoreMeter* pScoreMeter);
    ~ColumnLayouter();

    inline void initialize() {}
    inline void set_slice_box(GmoBoxSlice* pBoxSlice) { m_pBoxSlice = pBoxSlice; };
    inline GmoBoxSlice* get_slice_box() { return m_pBoxSlice; };

    //methods to compute results
    void do_spacing(bool fTrace = false);
    inline void increment_column_size(LUnits uIncr) { m_uMinColumnSize += uIncr; }

    //access to info
    bool is_there_barline();
    inline LUnits get_minimum_size() { return m_uMinColumnSize; }
    inline bool has_system_break() { return m_fHasSystemBreak; }
    inline void set_system_break(bool value) { m_fHasSystemBreak = value; }

    //methods for spacing
	LUnits tenths_to_logical(Tenths value, int iInstr, int staff);
    bool is_proportional_spacing();
    Tenths get_fixed_spacing_value() const;

    //boxes and shapes
    void add_shapes_to_boxes(ShapesStorage* pStorage);
    GmoBoxSliceInstr* create_slice_instr(ImoInstrument* pInstr, LUnits yTop);
    void delete_shapes(ShapesStorage* pStorage);
    inline GmoBoxSliceInstr* get_slice_instr(int iInstr) { return m_sliceInstrBoxes[iInstr]; }
    void set_slice_width(LUnits width);
    void set_slice_final_position(LUnits left, LUnits top);

    //public methods coded only for Unit Tests
    inline int get_num_lines() { return int(m_pColStorage->size()); }

private:
    LUnits compute_spacing();
    void delete_line_spacers();
    inline bool there_are_objects() { return m_fThereAreObjects; }
    inline bool there_are_lines() { return m_pColStorage->size() > 0; }

    //variables and methods for column traversal ---------------------------------
    bool    m_fThereAreObjects;
    float   m_rCurrentTime;         //current tiempos being aligned
    LUnits  m_rCurrentPos;          //xPos to start placing objects

    void create_line_spacers();
    void process_non_timed_at_prolog();
    void process_timed_at_current_timepos();
    void process_non_timed_at_current_timepos();
    //----------------------------------------------------------------------------

};


//---------------------------------------------------------------------------------------
//BreakPoints:
//  encloses the algorithm to determine optimum break points to split a column
//---------------------------------------------------------------------------------------
class BreakPoints
{
protected:
    ColumnStorage* m_pColStorage;      //the column to split
    BreaksTable* m_pPossibleBreaks;
    BreaksTimeEntry* m_pOptimumEntry;

public:
    BreakPoints(ColumnStorage* pColStorage);
    ~BreakPoints();

    bool find_optimum_break_point_for_space(LUnits uAvailable);
    float get_optimum_time_for_found_break_point();
    LUnits get_optimum_position_for_break_point();

protected:
    void compute_breaks_table();
    void delete_breaks_table();

};



//---------------------------------------------------------------------------------------
// SystemLayouter: orchestrates the layout of a system
//---------------------------------------------------------------------------------------
class SystemLayouter
{
protected:
    ScoreLayouter* m_pScoreLyt;
    ScoreMeter* m_pScoreMeter;
    LUnits m_uPrologWidth;
    GmoBoxSystem* m_pBoxSystem;     //box for this system

public:
    SystemLayouter(ScoreLayouter* pScoreLyt, ScoreMeter* pScoreMeter);
    ~SystemLayouter();

    GmoBoxSystem* create_system_box(LUnits left, LUnits top, LUnits width);

    //caller informs that all data for this system has been suplied
    void end_of_system_measurements();

    //    void AddTimeGridToBoxSlice(int iCol, GmoBoxSlice* pBSlice);

        //Access to information
    inline void set_prolog_width(LUnits width) { m_uPrologWidth = width; }
    inline LUnits get_prolog_width() { return m_uPrologWidth; }
    inline GmoBoxSystem* get_box_system() { return m_pBoxSystem; }

//    //other methods
//    void ClearDirtyFlags(int iCol);
};


//---------------------------------------------------------------------------------------
//ColumnSplitter:
//  Algorithm to determine optimum break points to split a column
//---------------------------------------------------------------------------------------
class ColumnSplitter
{
protected:
    LineTable* m_pLineTable;       //the line to process

public:
    ColumnSplitter(LineTable* pLineTable);
    ~ColumnSplitter();

    void compute_break_points(BreaksTable* pBT);

};


//---------------------------------------------------------------------------------------
//LineResizer: encapsulates the methods to recompute shapes positions so that the line
//will have the desired width, and to move the shapes to those positions
//---------------------------------------------------------------------------------------
class LineResizer
{
protected:
    LineTable*  m_pTable;           //table for the line to resize
    LUnits      m_uOldBarSize;
    LUnits      m_uNewBarSize;
    LUnits      m_uNewStart;
    UPoint      m_sliceOrg;
    LineEntryIterator m_itCurrent;

public:
    LineResizer(LineTable* pTable, LUnits uOldBarSize, LUnits uNewBarSize,
                LUnits uNewStart, UPoint sliceOrg);

    float move_prolog_shapes();
    void reasign_position_to_all_other_objects(LUnits uFizedSizeAtStart);
    LUnits get_time_line_position_for_time(float rFirstTime);

protected:
//    void InformAttachedObjs();

};


//---------------------------------------------------------------------------------------
//LineSpacer:
//  encapsulates the algorithm to assign spaces and positions to a single line
//---------------------------------------------------------------------------------------
class LineSpacer
{
private:
    LineTable*          m_pTable;           //the line to assign space
    float               m_rFactor;          //spacing factor
    ColumnLayouter*     m_pColLyt;          //for tenths/logical conversion
    LineEntryIterator   m_itCur;            //current entry
    float               m_rCurTime;         //current time
	LUnits              m_uxCurPos;         //current xPos at start of current time
    LUnits              m_uxRemovable;      //space that can be removed if required
    LineEntryIterator   m_itNonTimedAtCurPos;
    LUnits              m_uxNotTimedFinalPos;

public:
    LineSpacer(LineTable* pLineTable, ColumnLayouter* pColLyt, float rFactor);

    void process_non_timed_at_prolog(LUnits uSpaceAfterProlog);
    void process_non_timed_at_current_timepos(LUnits uxPos);
    void process_timed_at_current_timepos(LUnits uxPos);
	inline bool current_time_is(float rTime) { return m_rCurTime == rTime; }
    inline bool are_there_timed_objs() {
        return m_itCur != m_pTable->end()
               && is_equal_time((*m_itCur)->get_timepos(), m_rCurTime);
    }
    inline bool are_there_more_objects() { return (m_itCur != m_pTable->end()); }
    float get_next_available_time();
    LUnits get_next_position();

protected:
    void add_shapes_info_to_table();
    void prepare_for_traversing();
    LUnits compute_shift_to_avoid_overlap_with_previous();
    void drag_any_previous_clef_to_place_it_near_this_one();

    inline bool is_non_timed_object(LineEntryIterator it) {
        return (it != m_pTable->end() && (*it)->get_timepos() < 0.0f);
    }
    inline bool is_timed_object(LineEntryIterator it) {
        return (it != m_pTable->end() && (*it)->get_timepos() >= 0.0f);
    }
    inline bool is_current_object_non_timed() { return is_non_timed_object(m_itCur); }

    void assign_fixed_and_variable_space(LineEntry* pEntry);
    void set_note_rest_space(LineEntry* pEntry);
    LUnits compute_ideal_distance(LineEntry* pEntry);
    LUnits compute_ideal_distance_fixed(LineEntry* pEntry);
    LUnits compute_ideal_distance_proportional(LineEntry* pEntry);
    void assign_variable_space(LineEntry* pEntry, LUnits uIdeal);
    void assign_no_space(LineEntry* pEntry);
    void assign_minimum_fixed_space(LineEntry* pEntry);


    //variables and methods used only to position non-timed objects ------
    // create helper object?
    LUnits m_uxMaxOcuppiedSpace;
    LUnits m_uxMinOcuppiedSpace;

    void compute_max_and_min_occupied_space();
    void position_non_timed();
    void shift_non_timed(LUnits uxShift);

    void position_using_max_space_with_shift(LUnits uShift);
    void position_using_min_space_with_shift(LUnits uShift);

};


//---------------------------------------------------------------------------------------
//TimeGridTable:
//  A table with occupied times and durations, and connecting time with position
//---------------------------------------------------------------------------------------

//an item in the positions and times table
typedef struct
{
    float rTimepos;
    float rDuration;
    LUnits uxPos;
}
PosTimeItem;

//the table
class TimeGridTable
{
protected:
    ColumnStorage* m_pColStorage;
    std::vector<PosTimeItem> m_PosTimes;         //the table

public:
    TimeGridTable(ColumnStorage* pColStorage);
    ~TimeGridTable();

    inline int get_size() { return (int)m_PosTimes.size(); }

    //access to an entry values
    inline float get_timepos(int iItem) { return m_PosTimes[iItem].rTimepos; }
    inline float get_duration(int iItem) { return m_PosTimes[iItem].rDuration; }
    inline LUnits get_x_pos(int iItem) { return m_PosTimes[iItem].uxPos; }

    //access by position
    float get_time_for_position(LUnits uxPos);

//    //debug
//    void dump();

protected:
    //variables and methods for column traversal
    std::vector<TimeGridLineExplorer*> m_LineExplorers;
    float m_rCurrentTime;
    float m_rMinDuration;
    LUnits m_uCurPos;
    bool m_fTimedObjectsFound;

    inline bool timed_objects_found() { return m_fTimedObjectsFound; }
    bool there_are_objects();
    void create_line_explorers();
    void delete_line_explorers();
    void skip_non_timed_at_current_timepos();
    void find_shortest_noterest_at_current_timepos();
    void create_table_entry();
    void get_current_time();
    void interpolate_missing_times();

};

//---------------------------------------------------------------------------------------
// helper class to interpolate missing entries
//---------------------------------------------------------------------------------------
class TimeInserter
{
protected:
    std::vector<PosTimeItem>& m_PosTimes;

    std::vector<PosTimeItem>::iterator  m_itInsertionPoint;
    float m_rTimeBeforeInsertionPoint;
    LUnits m_uPositionBeforeInsertionPoint;

public:
    TimeInserter(std::vector<PosTimeItem>& oPosTimes);
    void interpolate_missing_times();

protected:
    bool is_time_in_table(float rTimepos);
    void find_insertion_point(float rTimepos);
    void insert_time_interpolating_position(float rTimepos);

};


//---------------------------------------------------------------------------------------
// helper class to encapsulate the line traversal algorithm
// for creating the time-pos table
//---------------------------------------------------------------------------------------
class TimeGridLineExplorer
{
private:
    LineTable* m_pTable;           //the line to assign space
    LineEntryIterator m_itCur;            //current entry
    float m_rCurTime;
	LUnits m_uCurPos;
    LUnits m_uShiftToNoteRestCenter;
    float m_rMinDuration;

public:
    TimeGridLineExplorer(LineTable* pLineTable);
    ~TimeGridLineExplorer();

    bool skip_non_timed_at_current_timepos();
    bool find_shortest_noterest_at_current_timepos();
    inline bool there_are_objects() { return (m_itCur != m_pTable->end()); }
    float get_current_time();
    float get_duration_for_found_entry();
    LUnits get_position_for_found_entry();

protected:
    inline bool is_current_object_non_timed() {
        return (m_itCur != m_pTable->end() && (*m_itCur)->get_timepos() < 0.0f);
    }

    inline bool current_object_is_timed() {
        return (m_itCur != m_pTable->end() && (*m_itCur)->get_timepos() >= 0.0f);
    }
};



//---------------------------------------------------------------------------------------
//ColumnResizer: encapsulates the methods to recompute shapes positions so that the
//column will have the desired width, and to move the shapes to those positions
//---------------------------------------------------------------------------------------
class ColumnResizer
{
protected:
    ColumnStorage* m_pColStorage;      //column to resize
    LUnits m_uNewBarSize;
    LUnits m_uOldBarSize;
    LUnits m_uNewStart;
    UPoint m_sliceOrg;
    float m_rFirstTime;
    LUnits m_uFixedPart;
    std::vector<LineResizer*> m_LineResizers;

public:
    ColumnResizer(ColumnStorage* pColStorage, LUnits uNewBarSize);
    LUnits reposition_shapes(LUnits uNewStart, LUnits uNewWidth, UPoint org);

protected:
    void create_line_resizers();
    void move_prolog_shapes_and_get_initial_time();
    void reposition_all_other_shapes();
    void determine_fixed_size_at_start_of_column();
    void delete_line_resizers();

};


}   //namespace lomse

#endif    // __LOMSE_SYSTEM_LAYOUTER_H__

