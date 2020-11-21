#pragma once
#include<vector>
class Face;
class Loop;
class HalfEdge;
class Edge;
class Vertex;
class Solid
{
public:
	Solid *prevs, *nexts;
	Face *sfaces;
	Edge *sedges;
	Solid() :prevs(nullptr), nexts(nullptr), sfaces(nullptr), sedges(nullptr) { }
	void AddFace(Face *f);
	bool RemoveFace(Face *f);
	void AddEdge(Edge *edge);
	bool RemoveEdge(Edge *edge);
};

class Face
{
public:
	static int num;
	static std::vector<Face *> fList;
	Face *prevf, *nextf;
	Solid *fsolid;
	Loop *floops;
	int id;
	Face() :prevf(nullptr), nextf(nullptr), fsolid(nullptr), floops(nullptr) { id = num++; fList.push_back(this); }
	void AddLoop(Loop *lp);
	bool RemoveLoop(Loop *lp);
};

class Loop
{
public:
	Loop *prevl, *nextl;
	Face *lface;
	HalfEdge *ledg;
	bool inner;
	Loop() :prevl(nullptr), nextl(nullptr), lface(nullptr), ledg(nullptr), inner(false) { };
};

class HalfEdge
{
public:
	HalfEdge *prev, *next;
	Loop *wloop;
	Edge *edg;
	Vertex *vtx;//start vertex of halfedge
	HalfEdge() :prev(nullptr), next(nullptr), wloop(nullptr), edg(nullptr), vtx(nullptr) { }
};

class Edge
{
public:
	Edge *preve, *nexte;
	HalfEdge *he1, *he2;
	Edge() :preve(nullptr), nexte(nullptr), he1(nullptr), he2(nullptr) { }
};

class Vertex
{
public:
	static int num;
	static std::vector<Vertex *> vList;
	float x, y, z;
	int id;
	Vertex(float point[3]) :x(point[0]), y(point[1]), z(point[2]) { id = num++; vList.push_back(this); }
};
