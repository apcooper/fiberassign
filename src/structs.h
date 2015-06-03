#ifndef STRUCTS_H
#define STRUCTS_H

#include	<cstdlib>
#include	<cmath>
#include	<fstream>
#include	<sstream>
#include	<iostream>
#include	<iomanip>
#include	<string>
#include	<vector>
#include	<algorithm>
#include	<exception>
#include	<sys/time.h>
#include        <map>
#include        "misc.h"
#include        "feat.h"
#include        "collision.h"

// PP ---------------------------------------------------
class PP { // PP for plate parameters
	public:
	Dlist fp; // All fiber positions (x,y) in mm
	List spectrom; // All spectrometer assignments of fibers
	Table fibers_of_sp; // Inv map of spectrom, fibers_of_sp[sp] are fibers of spectrom sp, redundant but optimizes
	Table N; // Identify neighboring positionners : neighb of fiber k are N[k]
	
	PP();
	void read_fiber_positions(const Feat& F);
	void get_neighbors(const Feat& F);
	void compute_fibsofsp(const Feat& F); // Computes fibers_of_sp
	List fibs_of_same_pet(int k) const;
	dpair coords(int k) const; // Coords of fiber k
};

// galaxy -------------------------------------------------
class galaxy {
	public:
	int id;
	double nhat[3];
	double ra, dec, z;
	Plist av_tfs; // available tile/fibers

	void print_av_tfs();
	str kind(const Feat& F) const;
};
class Gals : public std::vector<struct galaxy> {};

Gals read_galaxies(const Feat& F);

// Plate -------------------------------------------------
struct onplate { // The position of a galaxy in plate coordinates
	int id;
	double pos[2];
};
class Onplates : public std::vector<struct onplate> {};

class plate {
	public:
	int idp;
	double nhat[3]; // Unit vector pointing to plate
	int ipass; // Pass
	Table av_gals; // av_gals[k] : available galaxies of fiber k

	void print_plate(const Feat& F) const;
	List av_gals_plate(const Feat& F) const; // Av gals of the plate
};
class Plates : public std::vector<struct plate> {};

Plates read_plate_centers(const Feat& F);
List av_gals_of_kind(int kind, int j, int k, const Gals& G, const Plates& P, const Feat& F);

// Assignment ---------------------------------------------
// 2 mappings of assignments : (j,k) -> id(gal) ; id(gal)[5] -> (j,k)
class Assignment {
	public:
	//// ----- Members
	Table TF; // TF for tile fiber, #tiles X #fibers
	List order; // Order of tiles we want to assign, only 1-n in simple increasing order for the moment
	int next_plate; // Next plate in the order

	// Redundant information (optimizes computation time)
	Ptable GL; // GL for galaxy - list : #galaxies X (variable) #chosen TF
	Cube kinds; // Cube[j][sp][id] : number of fibers of spectrometer sp and plate j that have the kind id
	List nobsv; // List of nobs, redundant but optimizes
	List nobsv_tmp; // List of nobs, redundant but optimizes
	List once_obs;
	List probas; // Number of galaxies of this kind (not used but could be useful for some strategy)

	//// ----- Methods
	Assignment(const Gals& G, const Feat& F);
	~Assignment();
	void assign(int j, int k, int g, const Gals& G, const Plates& P, const PP& pp);
	void unassign(int j, int k, int g, const Gals& G, const Plates& P, const PP& pp);
	int find_collision(int j, int k, int g, const PP& pp, const Gals& G, const Plates& P, const Feat& F, int col=-1) const;
	bool find_collision(int j, int k, int kn, int g, int gn, const PP& pp, const Gals& G, const Plates& P, const Feat& F, int col=-1) const;
	int is_collision(int j, int k, const PP& pp, const Gals& G, const Plates& P, const Feat& F) const;
	void verif(const Plates& P, const Gals& G, const PP& pp, const Feat& F) const; // Verif mappings are right
	int is_assigned_jg(int j, int g) const;
	int is_assigned_jg(int j, int g, const Gals& G, const Feat& F) const;
	bool is_assigned_tf(int j, int k) const; 
	int na(const Feat& F, int begin=0, int size=-1) const; // Number of assignments within plates begin to begin+size
	int nobs(int g, const Gals& G, const Feat& F, bool tmp=true) const; // Counts how many time ob should be observed else more. If tmp=true, return MaxObs for this kind (temporary information)
	Plist chosen_tfs(int g, const Feat& F, int begin=0, int size=-1) const; // Pairs (j,k) chosen by g, amongst size plates from begin
	int nkind(int j, int k, int kind, const Gals& G, const Plates& P, const PP& pp, const Feat& F, bool pet=false) const; // Number of fibers assigned to the kind "kind" on the petal of (j,k). If pet=true, we don't take k but the petal directly instead
	List fibs_of_kind(int kind, int j, int pet, const Gals& G, const PP& pp, const Feat& F) const; // Sublist of fibers assigned to a galaxy of type kind for (j,p)
	List fibs_unassigned(int j, int pet, const Gals& G, const PP& pp, const Feat& F) const; // Subist of unassigned fibers for (j,p)

	// Update information
	void update_nobsv_tmp_for_one(int j, const Feat& F);
	void update_once_obs(int j, const Feat& F);

	// Used to compute results at the end
	Table infos_petal(int j, int pet, const Gals& G, const Plates& P, const PP& pp, const Feat& F) const;
	List unused_f(const Feat& F) const;
	Table unused_fbp(const PP& pp, const Feat& F) const; // Unused fibers by petal
	Table used_by_kind(str kind, const Gals& G, const PP& pp, const Feat& F) const; // Table (j X p) with numbers of assigned TF to a galaxy of kind
	float colrate(const PP& pp, const Gals& G, const Plates& P, const Feat& F, int j=-1) const; // Get collision rate, j = plate number, doesn't take into account 3-collisions
	int nobs_time(int g, int j, const Gals& G, const Feat& F) const; // Know the number of remaining observations of g when the program is at the tile j, for pyplotTile

	// Not used (but could be useful)
	int unused_f(int j, const Feat& F) const; // Number of unused fiber on the j'th plate
	int unused_fbp(int j, int k, const PP& pp, const Feat& F) const; // Number of unassigned fibers of the petal corresponding to (j,k)
	double get_proba(int i, const Gals& G, const Feat& F); // p(fake QSO | QSO) for example
	void update_nobsv_tmp(const Feat& F);
};

bool collision(dpair O1, dpair G1, dpair O2, dpair G2, const Feat& F); // Intersection of fh looking at galaxy G1 with fiber positionner centered in 01 and ...

int fprio(int g, const Gals& G, const Feat& F, const Assignment& A);

double plate_dist(const double theta);
struct onplate change_coords(const struct galaxy& O, const struct plate& P);
dpair projection(int g, int j, const Gals& G, const Plates& P); // Projection of g on j

// Pyplot -----------------------------------------------
class pyplot {
	public:
	polygon pol;
	Slist text;
	Dplist textpos;

	pyplot(polygon p);
	void addtext(dpair p, str s);
	void plot_tile(str directory, int j, const Feat& F) const;
};


#endif
