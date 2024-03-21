#include "Python.h"
#include "voronoi.h"

static PyObject *parse_voronoi(bst_t* voronoi) {
    segment_t* segment;
    PyObject *voronoi_segments = PyList_New(0);
    PyObject *voronoi_rays = PyList_New(0);
    PyObject *delaunay_segments  = PyList_New(0);
    while (!bst_rootkey(voronoi, &segment)) {
        PyObject *v_item;
        PyObject *d_item;     
        if (segment->label == SEG_SEG) {
            v_item = Py_BuildValue("((dd)(dd))", segment->options.seg.p1.x, 
                            segment->options.seg.p1.y, 
                            segment->options.seg.p2.x,
                            segment->options.seg.p2.y);
            PyList_Append(voronoi_segments, v_item);                 
        } else {
            v_item = Py_BuildValue("(ddd)", segment->options.ray.p.x,
                                   segment->options.ray.p.y, 
                                   segment->options.ray.gradient);
            PyList_Append(voronoi_rays, v_item);   
        }
        d_item = Py_BuildValue("((dd)(dd))", segment->dual.p1.x, 
                        segment->dual.p1.y, 
                        segment->dual.p2.x,
                        segment->dual.p2.y);
        PyList_Append(delaunay_segments, d_item);  
        bst_delete(voronoi, segment, NULL, NULL);
    }
    return Py_BuildValue("((OO)O)", voronoi_segments, voronoi_rays, delaunay_segments);
}

static PyObject *voronoi(PyObject *self, PyObject *args) {
    PyObject *vertices_list;
    int vertices_count;
    bst_t* voronoi_list;
    pqueue_t* pq;
    event_t* event;
    double x, y;

    if (!PyArg_ParseTuple(args, "O", &vertices_list))
        return NULL;

    vertices_count = PyObject_Length(vertices_list);
    pq  = pqueue_new(*event_compare);


    for (int index = 0; index < vertices_count; index++) {
        PyObject *item;
        item = PyList_GetItem(vertices_list, index);
        if(!PyArg_ParseTuple(item, "dd", &x, &y)) {
            return NULL;
        }
        event = new_event(SITE_EVENT, x, y, NULL, NULL, NULL);
        pqueue_insert(pq, event);
    }
    voronoi_list = compute_voronoi(pq);
    return parse_voronoi(voronoi_list);
}

char voronoifunc_docs[] = "Hello world description.";

PyMethodDef voronoi_funcs[] = {
	{	"voronoi",
		(PyCFunction)voronoi,
		METH_VARARGS,
		voronoifunc_docs},
	{	NULL}
};

char voronoimod_docs[] = "This is hello world module.";

PyModuleDef voronoi_mod = {
	PyModuleDef_HEAD_INIT,
	"voronoi",
	voronoimod_docs,
	-1,
	voronoi_funcs,
	NULL,
	NULL,
	NULL,
	NULL
};

PyMODINIT_FUNC PyInit_voronoi(void) {
	return PyModule_Create(&voronoi_mod);
}

