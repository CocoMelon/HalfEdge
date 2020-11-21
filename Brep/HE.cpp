#include "HE.h"
#include <iostream>

void Solid::AddFace(Face *f)
{
	Face *pf = sfaces;
	if (pf == nullptr) {
		sfaces = f;
		f->prevf = f->nextf = nullptr;
	}
	else {
		while (pf->nextf != nullptr) pf = pf->nextf;
		pf->nextf = f;
		f->prevf = pf;
		f->nextf = nullptr;
	}
	f->fsolid = this;
}
bool Solid::RemoveFace(Face *f)
{
	Face *pf = sfaces;
	while (pf != nullptr&&pf != f) pf = pf->nextf;
	if (pf == nullptr) return false;
	if (pf->prevf) {
		pf->prevf->nextf = pf->nextf;
	}
	else {
		sfaces = pf->nextf;
	}
	if (pf->nextf) pf->nextf->prevf = pf->prevf;
	delete f;
	return true;
}
void Solid::AddEdge(Edge *edge)
{
	Edge *tmpe = sedges;
	if (tmpe == nullptr) {
		sedges = edge;
		edge->preve = edge->nexte = nullptr;
	}
	else {
		while (tmpe->nexte != nullptr) tmpe = tmpe->nexte;
		tmpe->nexte = edge;
		edge->preve = tmpe;
		edge->nexte = nullptr;
	}
}
bool Solid::RemoveEdge(Edge *edge)
{
	Edge *tmpe = sedges;
	while (tmpe != nullptr&&tmpe != edge) tmpe = tmpe->nexte;
	if (tmpe == nullptr) return false;
	if (tmpe->preve) {
		tmpe->preve->nexte = tmpe->nexte;
	}
	else {
		sedges = tmpe->nexte;
	}
	if (tmpe->nexte) tmpe->nexte->preve = tmpe->preve;
	delete edge;
	return true;
}

void Face::AddLoop(Loop *lp)
{
	Loop *plp = floops;
	if (plp == nullptr) {
		floops = lp;
		lp->prevl = lp->nextl = nullptr;
	}
	else {
		while (plp->nextl != nullptr) plp = plp->nextl;
		plp->nextl = lp;
		lp->prevl = plp;
		lp->nextl = nullptr;
	}
	lp->lface = this;
}
bool Face::RemoveLoop(Loop *lp)
{
	Loop *plp = floops;
	while (plp != nullptr&&plp != lp) plp = plp->nextl;
	if (plp == nullptr) return false;
	if (plp->prevl) {
		plp->prevl->nextl = plp->nextl;
	}
	else {
		floops = plp->nextl;
	}
	if (plp->nextl) plp->nextl->prevl = plp->prevl;
	delete lp;
	return true;
}

int Vertex::num = 0;
int Face::num = 0;
std::vector<Face *> Face::fList = std::vector<Face *>();
std::vector<Vertex *> Vertex::vList = std::vector<Vertex *>();