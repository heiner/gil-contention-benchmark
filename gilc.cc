
#include <pybind11/pybind11.h>

namespace py = pybind11;

class Gilc {
public:
  Gilc() : calls_(0) {}
  double gilc() {
    ++calls_;
    if (calls_ % 1000 == 1) {
      auto now = std::chrono::system_clock::now();
      std::chrono::duration<double> delta = now - last_time_;
      last_time_ = std::move(now);
      if (calls_ > 1) // Not first time.
        return 1000.0 / delta.count();
    }
    return 0.0;
  }

private:
  int64_t calls_;
  std::chrono::time_point<std::chrono::system_clock> last_time_;
};

int myCModuleCounter = 0;
struct MyCModule {
  std::string name = "mycmodule_" + std::to_string(++myCModuleCounter);

  struct MyCObject {
    PyObject_HEAD /**/;
    Gilc *gilc;
    static PyObject *call(MyCObject *self) {
      return PyFloat_FromDouble(self->gilc->gilc());
    }
    static PyObject *call_nogil(MyCObject *self) {
      double result;
      Py_BEGIN_ALLOW_THREADS result = self->gilc->gilc();
      Py_END_ALLOW_THREADS return PyFloat_FromDouble(result);
    }

    static void tp_dealloc(PyObject *pself) {
      MyCObject *self = (MyCObject *)pself;
      delete self->gilc;
      Py_TYPE(self)->tp_free(pself);
    }
    static PyObject *tp_new(PyTypeObject *type, PyObject *args,
                            PyObject *kwds) {
      MyCObject *self = (MyCObject *)type->tp_alloc(type, 0);
      self->gilc = new Gilc;
      return (PyObject *)self;
    }
    static int tp_init(PyObject *pself, PyObject *args, PyObject *kwds) {
      MyCObject *self = (MyCObject *)pself;
      /* Could set members of self. */
      return 0;
    }
  };
  PyModuleDef moduleDef = {
      PyModuleDef_HEAD_INIT,
  };
  PyMethodDef myCObjectMethods[5] = {
      {"call", (PyCFunction)&MyCObject::call, METH_NOARGS, "Doc for call"},
      {"call_nogil", (PyCFunction)&MyCObject::call_nogil, METH_NOARGS,
       "Doc for call_nogil"},
      {nullptr, nullptr, 0, nullptr}};
  PyTypeObject myCObjectType = {PyVarObject_HEAD_INIT(nullptr, 0)};

  py::object module;
  MyCObject myCObject;
  ~MyCModule() {
    // Python has already shut down
    module.release();
  }
  MyCModule() {
    moduleDef.m_name = name.c_str();
    moduleDef.m_doc = "Example module";
    moduleDef.m_size = -1;

    myCObjectType.tp_name = "MyCObject";
    myCObjectType.tp_basicsize = sizeof(MyCObject);
    myCObjectType.tp_itemsize = 0;
    myCObjectType.tp_flags = Py_TPFLAGS_DEFAULT;
    myCObjectType.tp_methods = myCObjectMethods;
    myCObjectType.tp_dealloc = MyCObject::tp_dealloc;
    myCObjectType.tp_init = MyCObject::tp_init;
    myCObjectType.tp_new = MyCObject::tp_new;

    if (PyType_Ready(&myCObjectType) < 0) {
      throw std::runtime_error("PyType_Ready failed");
    }
    module = py::reinterpret_steal<py::object>(PyModule_Create(&moduleDef));
    if (!module) {
      throw std::runtime_error("PyModule_Create failed");
    }
    Py_INCREF(&myCObjectType);
    PyModule_AddObject(module.ptr(), "MyCObject", (PyObject *)&myCObjectType);

    /* Add static object */
    memset(&myCObject, 0, sizeof(myCObject));
    PyObject *o = PyObject_Init((PyObject *)&myCObject, &myCObjectType);
    if (o != (PyObject *)&myCObject) {
      throw std::runtime_error("PyObject_Init failed");
    }
    Py_INCREF(o);

    PyModule_AddObject(module.ptr(), "mycobject", o);
  }

  static MyCModule &get() {
    static MyCModule o;
    return o;
  }
};

PYBIND11_MODULE(gilc, m) {
  py::class_<Gilc>(m, "Gilc")
      .def(py::init<>())
      .def("gilc", &Gilc::gilc)
      .def("gilc_nogil", &Gilc::gilc, py::call_guard<py::gil_scoped_release>());
  m.attr("mycmodule") = MyCModule::get().module;
}
