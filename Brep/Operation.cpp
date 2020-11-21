#include "Operation.h"
#include <vector>

void EulerOperation::mvfs(Solid **ob, Vertex **v, Face **f, float point[3])
{
	*ob = new Solid;
	*f = new Face;
	*v = new Vertex(point);
	Loop *lp = new Loop;
	(*ob)->AddFace(*f);
	(*f)->AddLoop(lp);
}

void EulerOperation::mev(Vertex *v1, Vertex **ve2, Edge **ed, Loop *lp, float point[3])
{
	HalfEdge *he1 = new HalfEdge;
	HalfEdge *he2 = new HalfEdge;
	Edge *eg = new Edge;
	Vertex *v2 = new Vertex(point);
	eg->he1 = he1;
	eg->he2 = he2;
	he1->edg = eg;
	he2->edg = eg;
	he1->vtx = v1;
	he2->vtx = v2;
	he1->wloop = he2->wloop = lp;
	he1->next = he2;
	he2->prev = he1;
	if (lp->ledg == nullptr) {
		he2->next = he1;
		he1->prev = he2;
		lp->ledg = he1;
	}
	else {
		HalfEdge *he;
		for (he = lp->ledg; he->next->vtx != v1; he = he->next);
		he2->next = he->next;
		he->next->prev = he2;
		he->next = he1;
		he1->prev = he;
	}

	lp->lface->fsolid->AddEdge(eg);

	*ve2 = v2;
	*ed = eg;
}

void EulerOperation::mef(Vertex *v1, Vertex *v2, Loop *lp1, Edge **ed, Face **fa2)
{
	HalfEdge *he1 = new HalfEdge;
	HalfEdge *he2 = new HalfEdge;
	Loop *lp2 = new Loop;
	Face *f2 = new Face;
	Edge *e = new Edge;

	//add f2 to solid
	lp1->lface->fsolid->AddFace(f2);

	//add Loop lp2 to Face f2
	f2->AddLoop(lp2);

	//devide lp1 to 2 Loops
	HalfEdge *tmp1, *tmp2;
	he1->vtx = v1;
	he2->vtx = v2;
	for (tmp1 = lp1->ledg; tmp1->next->vtx != v1; tmp1 = tmp1->next);
	for (tmp2 = lp1->ledg; tmp2->next->vtx != v2; tmp2 = tmp2->next);
	he1->next = tmp2->next;
	he1->prev = tmp1;
	he2->next = tmp1->next;
	he2->prev = tmp2;
	he1->next->prev = he1;
	he2->next->prev = he2;
	tmp1->next = he1;
	tmp2->next = he2;

	//add links between Loops and HalfEdges
	he1->wloop = lp1;
	he2->wloop = lp2;
	for (tmp2 = he2->next; tmp2->vtx != v2; tmp2 = tmp2->next) tmp2->wloop = lp2;
	lp1->ledg = he1;
	lp2->ledg = he2;

	//add links between HalfEdges and Edges
	e->he1 = he1;
	e->he2 = he2;
	he1->edg = he2->edg = e;

	//add edge to solid
	lp1->lface->fsolid->AddEdge(e);

	*fa2 = f2;
	*ed = e;
}

void EulerOperation::kemr(Vertex *v1, Vertex *v2, Loop *lp, Loop **newlp)
{
	Loop *lp2 = new Loop;

	//devide lp to 2 Loops
	HalfEdge *tmphe1, *tmphe2;
	HalfEdge *he1 = lp->ledg;
	HalfEdge *he2 = lp->ledg;
	while (he1->vtx != v1 || he1->next->vtx != v2) he1 = he1->next;
	while (he2->vtx != v2 || he2->next->vtx != v1) he2 = he2->next;
	tmphe1 = he1->prev;
	tmphe2 = he2->prev;
	
	tmphe1->next = he2->next;
	he2->next->prev = tmphe1;
	tmphe2->next = he1->next;
	he1->next->prev = tmphe2;

	//add links between Loops and HalfEdges
	lp->ledg = tmphe1;
	lp2->ledg = tmphe2;
	tmphe2->wloop = lp2;
	for (HalfEdge *hf = tmphe2->next; hf != tmphe2; hf = hf->next)
		hf->wloop = lp2;

	//set lp2 to inner loop
	lp2->inner = true;

	//add lp2 to face
	lp->lface->AddLoop(lp2);

	//remove Edge and HalfEdge
	lp->lface->fsolid->RemoveEdge(he1->edg);
	delete he1;
	delete he2;

	*newlp = lp2;
}

void EulerOperation::kfmrh(Face *f1, Face *f2)
{
	//lp=outer loop of f2
	Loop *lp = f2->floops;
	while (lp != nullptr&&lp->inner == true) lp = lp->nextl;

	f1->AddLoop(lp);
	f1->fsolid->RemoveFace(f2);
}

void Sweeping::translate_sweep(Face *f, float v[3], float d)
{//vector of vertex in f
	std::vector<Vertex *> vecv;

	Loop *lp = f->floops;
	//find the outer loop of f
	while (lp->inner == true) lp = lp->nextl;

	//add verteces to vector
	HalfEdge *firsthe = lp->ledg;
	vecv.push_back(firsthe->vtx);
	for (HalfEdge *he = firsthe->next; he != firsthe; he = he->next) {
		vecv.push_back(he->vtx);
	}
	
	//new faces and edges need to add to solid
	Face *newface;
	Edge *newedge;

	float point[3];
	point[0] = vecv[0]->x + d * v[0];
	point[1] = vecv[0]->y + d * v[1];
	point[2] = vecv[0]->z + d * v[2];
	Vertex *firstup;
	EulerOperation::mev(vecv[0], &firstup, &newedge, lp, point);
	Vertex *prevup = firstup;
	Vertex *up;

	for (int i = 1; i < (int)vecv.size(); i++) {
		point[0] = vecv[i]->x + d * v[0];
		point[1] = vecv[i]->y + d * v[1];
		point[2] = vecv[i]->z + d * v[2];
		EulerOperation::mev(vecv[i], &up, &newedge, lp, point);
		EulerOperation::mef(prevup, up, lp, &newedge, &newface);
		prevup = up;
	}

	EulerOperation::mef(prevup, firstup, lp, &newedge, &newface);
}