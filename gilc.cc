
#include <cmath>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <thread>
#include <tuple>

namespace py = pybind11;

constexpr double square(double value) { return value * value; }

#define TIMING_END_DEFAULT 10'000'000
#define TIMING_BURN_IN_DEFAULT 10'000

class Timing {
public:
  Timing(int64_t end = TIMING_END_DEFAULT,
         int64_t burn_in = TIMING_BURN_IN_DEFAULT)
      : start_(-burn_in), end_(end), calls_(start_) {}

  bool call() {
    if (calls_ == 0) {
      start_time_ = std::chrono::system_clock::now();
    }
    if (++calls_ == end_) {
      end_time_ = std::chrono::system_clock::now();
    }
    return calls_ >= end_;
  }

  int64_t num_calls() { return calls_; }

  double delta() {
    std::chrono::duration<double> d = end_time_ - start_time_;
    return d.count();
  }

private:
  const int64_t start_;
  const int64_t end_;
  int64_t calls_;

  std::chrono::time_point<std::chrono::system_clock> start_time_;
  std::chrono::time_point<std::chrono::system_clock> end_time_;
};

class MeanVar {
public:
  void add(double value) {
    /* Online update.
       See
         http://www.incompleteideas.net/book/first/ebook/node19.html
       and
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
};

struct PyTiming {
  PyObject_HEAD /**/;
  Timing *timing;
  static PyObject *call(PyTiming *self) {
    if (self->timing->call()) {
      Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
  }

  static PyObject *call_nogil(PyTiming *self) {
    bool b;
    /* clang-format off */
    Py_BEGIN_ALLOW_THREADS
    b = self->timing->call();
    Py_END_ALLOW_THREADS
    if (b) {
      Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
    /* clang-format on */
  }

  static PyObject *num_calls(PyTiming *self) {
    return PyLong_FromLong(self->timing->num_calls());
  }

  static PyObject *delta(PyTiming *self) {
    return PyFloat_FromDouble(self->timing->delta());
  }

  static void tp_dealloc(PyObject *pself) {
    PyTiming *self = (PyTiming *)pself;
    delete self->timing;
    Py_TYPE(self)->tp_free(pself);
  }
  static PyObject *tp_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PyTiming *self = (PyTiming *)type->tp_alloc(type, 0);

    int64_t end = TIMING_END_DEFAULT;
    int64_t burn_in = TIMING_BURN_IN_DEFAULT;
    static const char *kwlist[] = {"end", "burn_in", NULL};

    if (!PyArg_ParseTupleAndKeywords(
            args, kwds, "|LL", const_cast<char **>(kwlist), &end, &burn_in))
      return nullptr;

    self->timing = new Timing(end, burn_in);
    return (PyObject *)self;
  }
  static int tp_init(PyObject *pself, PyObject *args, PyObject *kwds) {
    PyTiming *self = (PyTiming *)pself;
    /* Could set members of self. */
    return 0;
  }
};

static PyTypeObject PyTimingType = {PyVarObject_HEAD_INIT(nullptr, 0)};

static PyMethodDef PyTimingMethods[] = {
    {"call", (PyCFunction)&PyTiming::call, METH_NOARGS, "Doc for call"},
    {"call_nogil", (PyCFunction)&PyTiming::call_nogil, METH_NOARGS, ""},
    {"num_calls", (PyCFunction)&PyTiming::num_calls, METH_NOARGS, ""},
    {"delta", (PyCFunction)&PyTiming::delta, METH_NOARGS, ""},
    {nullptr, nullptr, 0, nullptr}};

static PyTypeObject *createPyTimingType() {
  PyTimingType.tp_name = "PyTiming";
  PyTimingType.tp_basicsize = sizeof(PyTiming);
  PyTimingType.tp_itemsize = 0;
  PyTimingType.tp_flags = Py_TPFLAGS_DEFAULT;
  PyTimingType.tp_methods = PyTimingMethods;
  PyTimingType.tp_dealloc = PyTiming::tp_dealloc;
  PyTimingType.tp_init = PyTiming::tp_init;
  PyTimingType.tp_new = PyTiming::tp_new;

  if (PyType_Ready(&PyTimingType) < 0) {
    throw std::runtime_error("PyType_Ready failed");
  }
  Py_INCREF(&PyTimingType);
  return &PyTimingType;
}

double cpp_loop() {
  Timing t;
  while (!t.call()) {
  }
  return t.num_calls() / t.delta();
}

std::vector<double> cpp_loop_threads(int num_threads = 2) {
  std::vector<double> result(num_threads);
  std::vector<std::thread> threads;

  for (double &r : result) {
    threads.emplace_back([&r] { r = cpp_loop(); });
  }
  for (std::thread &thread : threads) {
    thread.join();
  }
  return result;
}

PYBIND11_MODULE(gilc, m) {
  m.def("cpp_loop", cpp_loop);
  m.def("cpp_loop_threads", cpp_loop_threads);

  py::class_<Timing, std::shared_ptr<Timing>>(m, "Timing")
      .def(py::init<int64_t, int64_t>(), py::arg("end") = TIMING_END_DEFAULT,
           py::arg("burn_in") = TIMING_BURN_IN_DEFAULT)
      .def("call", &Timing::call)
      .def("call_nogil", &Timing::call,
           py::call_guard<py::gil_scoped_release>())
      .def("num_calls", &Timing::num_calls)
      .def("delta", &Timing::delta);

  py::class_<MeanVar, std::shared_ptr<MeanVar>>(m, "MeanVar")
      .def(py::init<>())
      .def("add", &MeanVar::add)
      .def("count", &MeanVar::count)
      .def("mean", &MeanVar::mean)
      .def("var", &MeanVar::var)
      .def("std", &MeanVar::std);

  PyModule_AddObject(m.ptr(), "CTiming", (PyObject *)createPyTimingType());
}
