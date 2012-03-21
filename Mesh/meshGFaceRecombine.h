// Gmsh - Copyright (C) 1997-2012 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.
//
// Contributor(s):
//   Amaury Johnen (a.johnen@ulg.ac.be)
//

#ifndef _MESH_GFACE_RECOMBINE_H_
#define _MESH_GFACE_RECOMBINE_H_

#define REC2D_EDGE_BASE 2
#define REC2D_EDGE_QUAD 1
#define REC2D_ALIGNMENT .5
#define REC2D_NUM_SON 6

#include "GFace.h"
#include "BackgroundMesh.h"
//#include "GModel.h"
//#include "MEdge.h"
//#include "MQuadrangle.h"

class Rec2DNode;
class Rec2DVertex;
class Rec2DEdge;
class Rec2DElement;
class Rec2DAction;
class Rec2DData;
class Rec2DDataChange;
class Rec2DCollapse;
struct lessRec2DAction {
  bool operator()(Rec2DAction*, Rec2DAction*) const;
};
struct gterRec2DAction {
  bool operator()(Rec2DAction*, Rec2DAction*) const;
};

struct lessRec2DNode {
  bool operator()(Rec2DNode*, Rec2DNode*) const;
};
struct gterRec2DNode {
  bool operator()(Rec2DNode*, Rec2DNode*) const;
};
struct moreRec2DNode {
  bool operator()(Rec2DNode*, Rec2DNode*) const;
};

//
class Recombine2D {
  private :
    GFace *_gf;
    backgroundMesh *_bgm;
    Rec2DData *_data;
    static Recombine2D *_current;
    
    int _strategy, _numChange;
    
  public :
    Recombine2D(GFace*);
    ~Recombine2D();
    
    bool recombine();
    double recombine(int depth);
    bool developTree();
    int getNumTri() const;
    static void nextTreeActions(std::vector<Rec2DAction*>&,
                                const std::vector<Rec2DElement*> &neighbours);
    
    inline void setStrategy(int s) {_strategy = s;}
    void drawState(double shiftx, double shifty) const;
    void printState() const;
    
    static inline GFace* getGFace() {return _current->_gf;}
    static inline int getNumChange() {return _current->_numChange;}
    static inline void incNumChange() {++_current->_numChange;}
    static inline backgroundMesh* bgm() {return _current->_bgm;}
    static void add(MQuadrangle *q);
    static void add(MTriangle *t);
    
    static void clearChanges();
    
  private :
    double _geomAngle(const MVertex*,
                      const std::vector<GEdge*>&,
                      const std::vector<MElement*>&) const;
    bool _remainAllQuad(const Rec2DAction *action) const;
    bool _remainAllQuad(std::set<Rec2DElement*>&) const;
};

class Rec2DData {
  private :
    int _numEdge, _numVert;
    long double _valEdge, _valVert;
    static Rec2DData *_current;
    int _remainingTri;
    
    std::set<Rec2DEdge*> _edges;
    std::set<Rec2DVertex*> _vertices;
    std::set<Rec2DElement*> _elements;
    
    std::list<Rec2DAction*> _actions;
    std::vector<Rec2DNode*> _endNodes;
    std::vector<Rec2DDataChange*> _changes;
    
    std::map<int, std::vector<Rec2DVertex*> > _parities;
    std::map<int, std::vector<Rec2DVertex*> > _assumedParities;
    std::map<Rec2DVertex*, int> _oldParity;
    
  public :
    Rec2DData();
    ~Rec2DData();
    
    void printState() const;
    void printActions() const;
    void sortActions() {_actions.sort(lessRec2DAction());}
    void drawTriangles(double shiftx, double shifty) const;
    void drawChanges(double shiftx, double shifty) const;
#ifdef REC2D_DRAW
    std::vector<MTriangle*> _tri;
    std::vector<MQuadrangle*> _quad;
#endif
    static inline int getNumTri() {return _current->_remainingTri;}
    static inline void setNumTri(int n) {_current->_remainingTri = n;}
    
    static inline int getNumEndNode() {return _current->_endNodes.size();}
    static inline int getNumElement() {return _current->_elements.size();}
    static Rec2DDataChange* getNewDataChange();
    static bool revertDataChange(Rec2DDataChange*);
    static void clearChanges();
    static int getNumChanges() {return _current->_changes.size();}
    
    static double getGlobalQuality();
    static double getGlobalQuality(int numEdge, double valEdge,
                                 int numVert, double valVert   );
    static inline void addVert(int num, double val) {
      _current->_numVert += num;
      _current->_valVert += (long double)val;
    }
    static inline void addValVert(double val) {
      _current->_valVert += (long double)val;
    }
    static inline void addEdge(int num, double val) {
      _current->_numEdge += num;
      _current->_valEdge += (long double)val;
    }
    
    static inline int getNumEdge() {return _current->_numEdge;}
    static inline double getValEdge() {return (double)_current->_valEdge;}
    static inline int getNumVert() {return _current->_numVert;}
    static inline double getValVert() {return (double)_current->_valVert;}
    static Rec2DAction* getBestAction();
    static Rec2DAction* getRandomAction();
    static inline bool hasAction() {return !_current->_actions.empty();}
    
    typedef std::set<Rec2DEdge*>::iterator iter_re;
    typedef std::set<Rec2DVertex*>::iterator iter_rv;
    typedef std::set<Rec2DElement*>::iterator iter_rel;
    static inline iter_re firstEdge() {return _current->_edges.begin();}
    static inline iter_rv firstVertex() {return _current->_vertices.begin();}
    static inline iter_rel firstElement() {return _current->_elements.begin();}
    static inline iter_re lastEdge() {return _current->_edges.end();}
    static inline iter_rv lastVertex() {return _current->_vertices.end();}
    static inline iter_rel lastElement() {return _current->_elements.end();}
    
    static inline void add(const Rec2DEdge *re) {
      _current->_edges.insert((Rec2DEdge*)re);
    }
    static inline void add(const Rec2DVertex *rv) {
      _current->_vertices.insert((Rec2DVertex*)rv);
    }
#ifndef REC2D_DRAW
    static inline void add(const Rec2DElement *rel) {
      _current->_elements.insert((Rec2DElement*)rel);
      }
#else
    static void add(Rec2DElement *rel);
#endif
    static inline void add(const Rec2DAction *ra) {
      _current->_actions.push_back((Rec2DAction*)ra);
    }
    static void remove(const Rec2DEdge*);
    static void remove(const Rec2DVertex*);
    static void remove(/*const*/ Rec2DElement*);
    static void remove(const Rec2DAction*);
    
    static inline void addEndNode(const Rec2DNode *rn) {
      _current->_endNodes.push_back((Rec2DNode*)rn);
    }
    static void sortEndNode();
    static inline void drawEndNode(int num);
    
    static int getNewParity();
    static void removeParity(const Rec2DVertex*, int);
    static inline void addParity(const Rec2DVertex *rv, int p) {
      _current->_parities[p].push_back((Rec2DVertex*)rv);
    }
    static void associateParity(int pOld, int pNew, Rec2DDataChange *rdc = NULL);
    static void removeAssumedParity(const Rec2DVertex*, int);
    static inline void addAssumedParity(const Rec2DVertex *rv, int p) {
      _current->_assumedParities[p].push_back((Rec2DVertex*)rv);
    }
    static void saveAssumedParity(const Rec2DVertex*, int);
    static void associateAssumedParity(int pOld, int pNew,
                                       std::vector<Rec2DVertex*>&);
    static inline void clearAssumedParities() {_current->_oldParity.clear();}
    static void revertAssumedParities();
};

enum Rec2DChangeType {
  HideEdge, HideVertex, HideElement,
  CreatedEdge, CreatedVertex, CreatedElement,
  HideAction, HideActions,
  CreatedAction, CreatedActions,
  SwapVertInAction, SwapVertInEdge,
  SwapEdgeInAction, SwapEdgeInElem,
  RemoveElem, AddElem, Relocate, ChangePar, SavePar,
  Error, Reverted
};

class Rec2DChange {
  private : 
    Rec2DChangeType _type;
    void *_entity;
    void *_info;
  
  public :
    Rec2DChange() {Msg::Error("[Rec2DChange] I should not be created in this manner");}
    Rec2DChange(Rec2DEdge*, bool toHide = false);
    Rec2DChange(Rec2DVertex*, bool toHide = false);
    Rec2DChange(Rec2DElement*, bool toHide = false);
    Rec2DChange(Rec2DAction*, bool toHide = false);
    Rec2DChange(const std::vector<Rec2DAction*>&, bool toHide = false);
    Rec2DChange(Rec2DVertex*, int newParity, Rec2DChangeType);
    Rec2DChange(Rec2DVertex*, SPoint2 newCoord);
    Rec2DChange(const std::vector<Rec2DVertex*>&, Rec2DChangeType); // save Parity
    Rec2DChange(Rec2DVertex*, const std::vector<Rec2DElement*>&,
                Rec2DChangeType                                 ); // add or remove element in vertex
    Rec2DChange(Rec2DVertex*, Rec2DVertex*,
                const std::vector<Rec2DEdge*>&,
                Rec2DChangeType                ); // swap vertex1 to vertex2 (edge)
    Rec2DChange(Rec2DVertex*, Rec2DVertex*,
                const std::vector<Rec2DAction*>&,
                Rec2DChangeType                  ); // swap vertex1 to vertex2 (action)
    Rec2DChange(Rec2DEdge*, Rec2DEdge*, Rec2DChangeType); // swap edge1 to edge2 (element)
    Rec2DChange(Rec2DEdge*, Rec2DEdge*,
                const std::vector<Rec2DAction*>&,
                Rec2DChangeType                  ); // swap edge1 to edge2 (action)
    
    void revert();
};

class Rec2DDataChange {
  private :
    std::vector<Rec2DChange*> _changes;
    Rec2DAction *_ra;
    
    //std::vector<Rec2DEdge*> _hiddenEdge/*, _newEdge*/;
    //std::vector<Rec2DVertex*> _hiddenVertex/*, _newVertex*/;
    //std::vector<Rec2DElement*> _hiddenElement, _newElement;
    //std::vector<Rec2DAction*> _hiddenAction, _newAction;
    //std::vector<std::pair<Rec2DVertex*, SPoint2> > _oldCoordinate;
    //std::vector<std::pair<Rec2DVertex*, int> > _oldParity;
    //std::vector<std::pair<Rec2DEdge*, std::pair<Rec2DVertex*, Rec2DVertex*> > > _vertSwapE;
    //std::vector<std::pair<Rec2DElement*, std::pair<Rec2DEdge*, Rec2DEdge*> > > _edgeSwap;
    //std::vector<std::pair<Rec2DElement*, std::pair<Rec2DVertex*, Rec2DVertex*> > > _vertSwapEL;
    
    
  public :
    ~Rec2DDataChange();
    
    inline void hide(Rec2DEdge *re) {_changes.push_back(new Rec2DChange(re, 1));}
    inline void hide(Rec2DVertex *rv) {_changes.push_back(new Rec2DChange(rv, 1));} 
    inline void hide(Rec2DElement *rel) {_changes.push_back(new Rec2DChange(rel, 1));}
    inline void hide(Rec2DAction *ra) {_changes.push_back(new Rec2DChange(ra, 1));}
    inline void hide(std::vector<Rec2DAction*> &vect) {_changes.push_back(new Rec2DChange(vect, 1));}
    
    inline void append(Rec2DElement *rel) {_changes.push_back(new Rec2DChange(rel));}
    inline void append(Rec2DAction *ra) {_changes.push_back(new Rec2DChange(ra));}
    
    void swapFor(Rec2DEdge*, Rec2DEdge*);
    void swapFor(Rec2DVertex*, Rec2DVertex*);
    
    inline void relocate(Rec2DVertex *rv, const SPoint2 &p) {
      _changes.push_back(new Rec2DChange(rv, p));
    }
    inline void changeParity(Rec2DVertex *rv, int p) {
      _changes.push_back(new Rec2DChange(rv, p, ChangePar));
    }
    inline void saveParity(const std::vector<Rec2DVertex*> &verts) {
      _changes.push_back(new Rec2DChange(verts, SavePar));
    }
    //void checkObsoleteActions() {_changes.push_back(new Rec2DChange());}
    
    //void hide(Rec2DEdge*); {_elementaryChanges.push_back(new Rec2DChange(re));}
    //void hide(Rec2DVertex*);
    //void hide(Rec2DElement*);
    //void hide(Rec2DAction*);
    //void hide(std::vector<Rec2DAction*>);
    //
    //void append(const Rec2DElement*);
    //void append(const Rec2DAction*);
    //
    //void swapFor(Rec2DEdge*, Rec2DEdge*);
    //void swapFor(Rec2DVertex*, Rec2DVertex*);
    //
    //void relocate(Rec2DVertex*, double, double);
    //
    //void changeParity(Rec2DVertex*, int);
    //void saveParity(std::vector<Rec2DVertex*>&);
    void checkObsoleteActions(Rec2DVertex*const*, int size);
    
    void revert();
    
    void setAction(const Rec2DAction *action) {_ra = (Rec2DAction*)action;}
    Rec2DAction* getAction() const {return _ra;}
};

class Rec2DAction {
  protected :
    double _globQualIfExecuted;
    int _lastUpdate;
    
  public :
    Rec2DAction();
    virtual ~Rec2DAction() {}
    virtual void hide() = 0;
    virtual void reveal() = 0;
    
    bool operator<(Rec2DAction&);
    double getReward();
    virtual void color(int, int, int) const = 0;
    virtual void apply(std::vector<Rec2DVertex*> &newPar) = 0;
    virtual void apply(Rec2DDataChange*) const = 0;
    virtual bool isObsolete() const = 0;
    virtual bool isAssumedObsolete() const = 0;
    virtual void getAssumedParities(int*) const = 0;
    virtual bool whatWouldYouDo(std::map<Rec2DVertex*, std::vector<int> >&) = 0;
    virtual Rec2DVertex* getVertex(int) const = 0;
    virtual int getNumElement() = 0;
    virtual void getElements(std::vector<Rec2DElement*>&) const = 0;
    virtual void getNeighbourElements(std::vector<Rec2DElement*>&) const = 0;
    virtual int getNum(double shiftx, double shifty) = 0;
    virtual Rec2DElement* getRandomElement() const = 0;
    //virtual void print() = 0;
    virtual bool haveElem() = 0;
    inline virtual Rec2DAction* getBase() const = 0;
    static void removeDuplicate(std::vector<Rec2DAction*>&);
    virtual void swap(Rec2DVertex*, Rec2DVertex*) = 0;
    virtual void swap(Rec2DEdge*, Rec2DEdge*) = 0;
    
  private :
    virtual void _computeGlobQual() = 0;
};

class Rec2DTwoTri2Quad : public Rec2DAction {
  private :
    Rec2DElement *_triangles[2];
    Rec2DEdge *_edges[5]; // 4 boundary, 1 embedded
    Rec2DVertex *_vertices[4]; // 4 boundary (2 on embedded edge + 2)
    friend class Rec2DCollapse;
    
  public :
    Rec2DTwoTri2Quad(Rec2DElement*, Rec2DElement*);
    ~Rec2DTwoTri2Quad() {hide();}
    virtual void hide();
    virtual void reveal();
    
    virtual void color(int, int, int) const;
    virtual void apply(std::vector<Rec2DVertex*> &newPar);
    virtual void apply(Rec2DDataChange*) const;
    
    virtual bool isObsolete() const;
    virtual bool isAssumedObsolete() const;
    static bool isObsolete(const int*);
    virtual void getAssumedParities(int*) const;
    virtual bool whatWouldYouDo(std::map<Rec2DVertex*, std::vector<int> >&);
    
    virtual inline Rec2DVertex* getVertex(int i) const {return _vertices[i];} //-
    virtual inline int getNumElement() {return 2;}
    virtual void getElements(std::vector<Rec2DElement*>&) const;
    virtual void getNeighbourElements(std::vector<Rec2DElement*>&) const;
    virtual int getNum(double shiftx, double shifty);
    virtual inline Rec2DElement* getRandomElement() const;
    //virtual void print();
    virtual bool haveElem() {return true;}
    inline virtual Rec2DAction* getBase() const {return NULL;}
    virtual void swap(Rec2DVertex*, Rec2DVertex*);
    virtual void swap(Rec2DEdge*, Rec2DEdge*);
    
  private :
    virtual void _computeGlobQual();
    void _doWhatYouHaveToDoWithParity(Rec2DDataChange*) const;
};

class Rec2DCollapse : public Rec2DAction {
  private :
    Rec2DTwoTri2Quad *_rec;
    //Rec2DElement **const&_triangles;
    //Rec2DEdge **const&_edges;
    //Rec2DVertex **const&_vertices;
    
  public :
    Rec2DCollapse(Rec2DTwoTri2Quad*);
    ~Rec2DCollapse() {hide();}
    virtual void hide();
    virtual void reveal();
    
    virtual void color(int c1, int c2, int c3) const {
      _rec->color(c1, c2, c3);
    }
    virtual void apply(std::vector<Rec2DVertex*> &newPar);
    virtual void apply(Rec2DDataChange*) const;
    
    virtual bool isObsolete() const;
    virtual bool isAssumedObsolete() const;
    static bool isObsolete(const int*);
    virtual void getAssumedParities(int*) const;
    virtual bool whatWouldYouDo(std::map<Rec2DVertex*, std::vector<int> >&);
    
    virtual inline Rec2DVertex* getVertex(int i) const {
      return _rec->getVertex(i);
    }
    virtual inline int getNumElement() {return 2;}
    virtual void getElements(std::vector<Rec2DElement*> &vec) const {
      _rec->getElements(vec);
    }
    virtual void getNeighbourElements(std::vector<Rec2DElement*> &vec) const {
      _rec->getNeighbourElements(vec);
    }
    virtual int getNum(double shiftx, double shifty) {
      return -1;
    }
    virtual inline Rec2DElement* getRandomElement() const {
      return _rec->getRandomElement();
    }
    //virtual void print();
    virtual bool haveElem() {return false;}
    inline virtual Rec2DAction* getBase() const {return _rec;}
    inline virtual void swap(Rec2DVertex *rv0, Rec2DVertex *rv1) {_rec->swap(rv0, rv1);}
    inline virtual void swap(Rec2DEdge *re0, Rec2DEdge *re1) {_rec->swap(re0, re1);}
    
  private :
    virtual void _computeGlobQual();
};

class Rec2DEdge {
  private :
    Rec2DVertex *_rv0, *_rv1;
    double _qual;
    int _lastUpdate, _weight;
    int _boundary; // pourrait faire sans !
    
  public :
    Rec2DEdge(Rec2DVertex*, Rec2DVertex*);
    ~Rec2DEdge() {hide();}
    void hide();
    void reveal();
    
    double getQual() const;
    //double getQualL() const;
    //double getQualO() const;
    double getWeightedQual() const;
    void print() const;
    
    inline void addHasTri() {_addWeight(-REC2D_EDGE_QUAD); ++_boundary;}
    inline void remHasTri() {_addWeight(REC2D_EDGE_QUAD); --_boundary;}
    inline void addHasQuad() {++_boundary;}
    inline void remHasQuad() {--_boundary;}
    inline bool isOnBoundary() const {return !_boundary;}
    
    inline Rec2DVertex* getVertex(int i) const {if (i) return _rv1; return _rv0;}
    Rec2DVertex* getOtherVertex(const Rec2DVertex*) const;
    static Rec2DElement* getUniqueElement(const Rec2DEdge*);
    static void getElements(const Rec2DEdge*, Rec2DElement**);
    void getActions(std::vector<Rec2DAction*>&) const;
    
    void swap(Rec2DVertex *oldRV, Rec2DVertex *newRV, bool upVert = true);
    
  private :
    void _computeQual();
    double _straightAdimLength() const;
    double _straightAlignment() const;
    void _addWeight(int);
};

struct AngleData {
  std::vector<GEdge*> _gEdges;
  std::vector<MElement*> _mElements;
  Rec2DVertex *_rv;
  
  AngleData() : _rv(NULL) {} 
};

class Rec2DVertex {
  private :
    MVertex *_v;
    const double _angle;
    int _onWhat; // _onWhat={-1:corner,0:edge,1:face}
    int _parity, _assumedParity, _lastMove;
    double _sumQualAngle;
    std::vector<Rec2DEdge*> _edges;
    std::vector<Rec2DElement*> _elements;
    SPoint2 _param;
    
    static double **_qualVSnum;
    static double **_gains;
    
  public :
    Rec2DVertex(MVertex*);
    Rec2DVertex(Rec2DVertex*, double angle);
    ~Rec2DVertex() {hide();}
    void hide();
    void reveal();
    
    inline int getNum() const {return _v->getNum();}
    inline double getAngle() const {return _angle;}
    inline double getQual() const {return getQualDegree() + getQualAngle();}
    inline double getQualAngle() const {return _sumQualAngle/(double)_elements.size();}
    double getQualDegree(int numEl = -1) const;
    double getGainDegree(int) const;
    double getGainMerge(const Rec2DElement*, const Rec2DElement*) const;
    
    inline void setOnBoundary();
    inline bool getOnBoundary() const {return _onWhat < 1;}
    bool setBoundaryParity(int p0, int p1);
    
    inline int getParity() const {return _parity;}
    void setParity(int, bool tree = false);
    void setParityWD(int pOld, int pNew);
    int getAssumedParity() const;
    bool setAssumedParity(int);
    void setAssumedParityWD(int pOld, int pNew);
    void revertAssumedParity(int);
    
    inline int getNumElements() const {return _elements.size();}
    inline void getEdges(std::vector<Rec2DEdge*> &v) const {v = _edges;}
    void getTriangles(std::set<Rec2DElement*>&) const;
    void getElements(std::vector<Rec2DElement*>&) const;
    inline MVertex* getMVertex() const {return _v;}
    
    inline int getLastMove() const {return _lastMove;}
    inline void getxyz(double *xyz) const {
      xyz[0] = _v->x();
      xyz[1] = _v->y();
      xyz[2] = _v->z();
    }
    inline double u() const {return _param[0];}
    inline double v() const {return _param[1];}
    void relocate(SPoint2 p);
    inline void getParam(SPoint2 *p) {*p = _param;}
    
    void add(Rec2DEdge*);
    bool has(Rec2DEdge*) const;
    void rmv(Rec2DEdge*);
    
    void add(Rec2DElement*);
    bool has(Rec2DElement*) const;
    void rmv(Rec2DElement*);
    
    void getUniqueActions(std::vector<Rec2DAction*>&) const;
    
    static void initStaticTable();
    static Rec2DEdge* getCommonEdge(const Rec2DVertex*, const Rec2DVertex*);
    static void getCommonElements(const Rec2DVertex*, const Rec2DVertex*,
                                  std::vector<Rec2DElement*>&            );
    
  private :
    bool _recursiveBoundParity(const Rec2DVertex *prev, int p0, int p1);
    inline double _angle2Qual(double ang) const {
      return 1. - fabs(ang*2./M_PI - 1.);
    }
};

class Rec2DElement {
  private :
    MElement *_mEl;       // can be NULL
    int _numEdge;
    Rec2DEdge *_edges[4];
    Rec2DElement *_elements[4];  // NULL if no neighbour
    std::vector<Rec2DAction*> _actions;
    
  public :
    Rec2DElement(MTriangle*, const Rec2DEdge**, Rec2DVertex **rv = NULL);
    Rec2DElement(MQuadrangle*, const Rec2DEdge**, Rec2DVertex **rv = NULL);
    ~Rec2DElement() {hide();}
    void hide();
    void reveal();
    
    bool inline isTri() const {return _numEdge == 3;}
    bool inline isQuad() const {return _numEdge == 4;}
    
    void add(Rec2DEdge*);
    bool has(const Rec2DEdge*) const;
    void add(const Rec2DAction*);
    void remove(const Rec2DAction*);
    void addNeighbour(const Rec2DEdge*, const Rec2DElement*);
    void rmvNeighbour(const Rec2DEdge*, const Rec2DElement*);
    
    void swap(Rec2DEdge*, Rec2DEdge*);
    
    inline MElement* getMElement() const {return _mEl;}
#ifdef REC2D_DRAW
    MTriangle* getMTriangle() {
      if (_numEdge == 3) {
        if (_mEl)
          return (MTriangle*) _mEl;
        else
          Msg::Error("[Rec2DElement] Do you thing I'll create a triangle for you ?");
      }
      return NULL;
    }
    MQuadrangle* getMQuadrangle() {
      if (_numEdge == 4) {
        if (!_mEl)
          _mEl = (MElement*) _createQuad();
        return (MQuadrangle*) _mEl;
      }
      return NULL;
    }
#endif
    void createElement(double shiftx, double shifty) const;
    
    double getAngle(const Rec2DVertex*) const;
    
    inline int getNumActions() const {return _actions.size();}
    inline Rec2DAction* getAction(int i) const {return _actions[i];}
    inline void getActions(std::vector<Rec2DAction*> &v) const {v = _actions;};
    void getUniqueActions(std::vector<Rec2DAction*>&) const;
    void getAssumedParities(int*) const;
    void getMoreEdges(std::vector<Rec2DEdge*>&) const;
    void getVertices(std::vector<Rec2DVertex*>&) const;
    void getMoreNeighbours(std::vector<Rec2DElement*>&) const;
    Rec2DVertex* getOtherVertex(const Rec2DVertex*, const Rec2DVertex*) const;
    static Rec2DEdge* getCommonEdge(const Rec2DElement*, const Rec2DElement*);
    static void getElements(const Rec2DEdge*, Rec2DElement**);
    
    inline int getNum() const {return _mEl->getNum();}
    
  private :
    MQuadrangle* _createQuad() const;
};

class Rec2DNode {
  private :
    Rec2DNode *_father;
    Rec2DNode *_son[REC2D_NUM_SON];
    Rec2DAction *_ra;
    double _globalQuality, _bestEndGlobQual;
    int _remainingTri;
    
    Rec2DDataChange *_dataChange;
    
  public :
    Rec2DNode(Rec2DNode *father, Rec2DAction*,
              double &bestEndGlobQual, int depth = -1);
    ~Rec2DNode();
    
    Rec2DNode* selectBestNode();
    void recoverSequence();
    void rmvSon(Rec2DNode*);
    void develop(int depth, double &bestEndGlobQual);
    inline bool hasSon() const {return _son[0];}
    bool makeChanges();
    
    bool operator<(Rec2DNode&);
    inline Rec2DNode* getFather() const {return _father;}
    inline Rec2DAction* getAction() const {return _ra;}
    inline double getGlobQual() const {return _globalQuality;}
    inline int getNumTri() const {return _remainingTri;}
};


#endif