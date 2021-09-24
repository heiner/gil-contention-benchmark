
#include <cmath>
#include <pybind11/pybind11.h>

namespace py = pybind11;

constexpr double square(double value) { return value * value; }

class Timing {
public:
  void time() {
    if (count_ == 0) {
      ++count_;
      last_time_ = std::chrono::system_clock::now();
      return;
    };

    auto now = std::chrono::system_clock::now();
    std::chrono::duration<double> delta = now - last_time_;
    last_time_ = std::move(now);

    call(delta.count());
  }

  void call(double value) {
    int64_t n = count_ - 1;

    /* Online update. See
       http://www.incompleteideas.net/book/first/ebook/node19.html) and
       https://math.stackexchange.com/a/103025/5051
    */
    double mean = mean_ + (value - mean_) / (count_ + 1);
    double var =
        (count_ * var_ + count_ * square(mean_ - mean) + square(value - mean)) /
        (count_ + 1);

    ++count_;
    mean_ = mean;
    var_ = var;
  }

  int64_t count() { return count_; }
  double mean() { return mean_; }
  double var() { return var_; }
  double std() { return std::sqrt(var_); }

private:
  int64_t count_ = 0;
  double mean_ = 0.0;
  double var_ = 0.0;
  std::chrono::time_point<std::chrono::system_clock> last_time_;
};

int myCModuleCounter = 0;
struct MyCModule {
  std::string name = "mycmodule_" + std::to_string(++myCModuleCounter);

  struct MyCObject {
    PyObject_HEAD /**/;
    Timing *timing;
    static PyObject *time(MyCObject *self) {
      self->timing->time();
      Py_RETURN_NONE;
    }

    static PyObject *time_nogil(MyCObject *self) {
      // clang-format off
      Py_BEGIN_ALLOW_THREADS
      self->timing->time();
      Py_END_ALLOW_THREADS
      Py_RETURN_NONE;
      // clang-format on
    }

    static void tp_dealloc(PyObject *pself) {
      MyCObject *self = (MyCObject *)pself;
      delete self->timing;
      Py_TYPE(self)->tp_free(pself);
    }
    static PyObject *tp_new(PyTypeObject *type, PyObject *args,
                            PyObject *kwds) {
      MyCObject *self = (MyCObject *)type->tp_alloc(type, 0);
      self->timing = new Timing;
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
      {"time", (PyCFunction)&MyCObject::time, METH_NOARGS, "Doc for time"},
      {"time_nogil", (PyCFunction)&MyCObject::time_nogil, METH_NOARGS, ""},
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
  py::class_<Timing>(m, "Timing")
      .def(py::init<>())
      .def("time", &Timing::time)
      .def("time_nogil", &Timing::time,
           py::call_guard<py::gil_scoped_release>())
      .def("call", &Timing::call)
      .def("mean", &Timing::mean)
      .def("var", &Timing::var)
      .def("std", &Timing::std)
      .def("count", &Timing::count);
  m.attr("mycmodule") = MyCModule::get().module;
}
