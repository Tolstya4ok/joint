#include <pyjoint/InterfaceDescriptor.hpp>

#include <joint/devkit/Logger.hpp>

#include <utils/PythonUtils.hpp>
#include <binding/Object.hpp>


namespace joint {
namespace python {
namespace pyjoint
{

	using namespace devkit;

	JOINT_DEVKIT_LOGGER("Joint.Python.PyJoint.InterfaceDescriptor")

	static int InterfaceDescriptor_init(InterfaceDescriptor* self, PyObject* args, PyObject* kwds);
	static void InterfaceDescriptor_del(InterfaceDescriptor* self);
	static int InterfaceDescriptor_traverse(InterfaceDescriptor* self, visitproc visit, void* arg);

	PyTypeObject InterfaceDescriptor_type = {
		PyVarObject_HEAD_INIT(NULL, 0)
		"pyjoint.InterfaceDescriptor",               // tp_name
		sizeof(InterfaceDescriptor),                 // tp_basicsize
		0,                                           // tp_itemsize
		(destructor)InterfaceDescriptor_del,         // tp_dealloc
		0,                                           // tp_print
		0,                                           // tp_getattr
		0,                                           // tp_setattr
		0,                                           // tp_reserved
		0,                                           // tp_repr
		0,                                           // tp_as_number
		0,                                           // tp_as_sequence
		0,                                           // tp_as_mapping
		0,                                           // tp_hash
		0,                                           // tp_call
		0,                                           // tp_str
		0,                                           // tp_getattro
		0,                                           // tp_setattro
		0,                                           // tp_as_buffer
		Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,     // tp_flags
		"joint module object",                       // tp_doc
		(traverseproc)InterfaceDescriptor_traverse,  // tp_traverse
		0,                                           // tp_clear
		0,                                           // tp_richcompare
		0,                                           // tp_weaklistoffset
		0,                                           // tp_iter
		0,                                           // tp_iternext
		NULL,                                        // tp_methods
		NULL,                                        // tp_members
		0,                                           // tp_getset
		0,                                           // tp_base
		0,                                           // tp_dict
		0,                                           // tp_descr_get
		0,                                           // tp_descr_set
		0,                                           // tp_dictoffset
		(initproc)InterfaceDescriptor_init,          // tp_init
		PyType_GenericAlloc,                         // tp_alloc
		PyType_GenericNew                            // tp_new
	};


	static int InterfaceDescriptor_init(InterfaceDescriptor* self, PyObject* args, PyObject* kwds)
	{
		PYJOINT_CPP_WRAP_BEGIN

		PyObject* py_desc;
		PYTHON_CHECK(PyArg_ParseTuple(args, "O", &py_desc), "Could not parse arguments");

		self->objects = new std::vector<PyObject*>();
		self->descriptor = new InterfaceDescriptorValue(py_desc, PythonBindingInfo(*self->objects));

		PYJOINT_CPP_WRAP_END(0, -1, Py_DECREF(self);)
	}


	static void InterfaceDescriptor_del(InterfaceDescriptor* self)
	{
		PYJOINT_CPP_WRAP_BEGIN

		PyObject_GC_UnTrack(self);
		Py_TRASHCAN_SAFE_BEGIN(self)
		delete self->descriptor;
		delete self->objects;
		Py_TYPE(self)->tp_free((PyObject*)self);
		Py_TRASHCAN_SAFE_END(self)

		PYJOINT_CPP_WRAP_END_VOID()
	}


	static int InterfaceDescriptor_traverse(InterfaceDescriptor* self, visitproc visit, void* arg)
	{
		if (self->objects)
			for (auto o : *self->objects)
				Py_VISIT(o);

		return 0;
	}

}}}
