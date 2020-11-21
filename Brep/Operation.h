#pragma once
#include "HE.h"

namespace EulerOperation
{
	//input a vertex v and start to construct an object
	void mvfs(Solid **ob, Vertex **v, Face **f, float point[3]);
	/*
	intput the coordinate of a new vertex v2
	construct v2
	construct an edge between vertex v1 and a new vertex v2
	the new edge is in the Loop lp
	point[3] is the coordinate of v2
	*/
	void mev(Vertex *v1, Vertex **ve2, Edge **ed, Loop *lp, float point[3]);
	/*
	construct a new edge between vertex v1 and v2
	construct a new face f2
	v1 and v2 are in Loop lp1
	*/
	void mef(Vertex *v1, Vertex *v2, Loop *lp1, Edge **ed, Face **fa2);
	/*
	delete the edge between vertex v1 and v2
	construct a new inner loop
	v1 and v2 are in Loop lp before operation
	v1 in outer loop, v2 in inner loop after operation
	*/
	void kemr(Vertex *v1, Vertex *v2, Loop *lp, Loop **newlp);
	/*
	remove inner face of Face f1 and construct a new inner loop of f1
	this operation is used to create a through hole in the object
	f2 is the inner face of f1
	*/
	void kfmrh(Face *f1, Face *f2);
}

namespace Sweeping
{
	/*
	do the translate sweeping
	Face f sweep along vector v on a distance of d
	*/
	void translate_sweep(Face *f, float v[3], float d);
}